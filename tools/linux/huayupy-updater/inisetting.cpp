#include "inisetting.h"
#include <QSettings>
#include <QDebug>
#include <vector>
#include <QFile>

class IniLine
{
public:
    IniLine(const std::string &type) : m_type(type) {}
    virtual ~IniLine() {}
    const std::string &type() const { return m_type; }

    virtual void parse(const std::string &line) = 0;
    virtual void write(QIODevice &device) = 0;
    virtual bool isSettingOf(const std::string &key) = 0;

private:
    std::string m_type;
};

class IniSettingLine : public IniLine
{
public:
    IniSettingLine() : IniLine("IniSettingLine") {}
    virtual ~IniSettingLine() {}

    virtual void parse(const std::string &line)
    {
        std::string::size_type separator = line.find_first_of('=');
        if (separator != std::string::npos)
        {
            m_key = line.substr(0, separator);
            m_value = line.substr(separator + 1);
        }
    }

    virtual void write(QIODevice &device)
    {
        std::string data = m_key + "=" + m_value;
        device.write(data.c_str(), data.length());
    }

    void set(const std::string &key, const std::string &value)
    {
        m_key = key;
        m_value = value;
    }

    virtual bool isSettingOf(const std::string &key)
    {
        return m_key == key;
    }

private:
    std::string m_key;
    std::string m_value;
};

class IniCommentLine : public IniLine
{
public:
    IniCommentLine() : IniLine("IniCommentLine") {}
    virtual ~IniCommentLine() {}
    virtual void parse(const std::string &line)
    {
        m_comment = line;
    }

    virtual void write(QIODevice &device)
    {
        device.write(m_comment.c_str(), m_comment.length());
    }

    virtual bool isSettingOf(const std::string &key)
    {
        std::string comment = m_comment;
        comment = comment.substr(comment.find_first_not_of('#'));
        comment = comment.substr(comment.find_first_not_of(' '));
        std::string::size_type pos = comment.find(key);
        if (pos == 0)
        {
            return true;
        }
        return false;
    }

private:
    std::string m_comment;
};

class IniGroup
{
public:
    IniGroup(const std::string &name): m_name(name) {}
    ~IniGroup()
    {
        qDebug() << "Destory";
    }

    const std::string &name() const { return m_name; }

    IniLine* parse(const std::string &line)
    {
        IniLine *iniLine = NULL;
        if (line[0] == '#')
        {
            iniLine = new IniCommentLine();
        }
        else if (line.find_first_of('=') != std::string::npos)
        {
           iniLine = new IniSettingLine();
        }

        if (iniLine)
        {
            iniLine->parse(line);
        }

        return iniLine;
    }

    void append(IniLine *line)
    {
        m_lines.push_back(line);
    }

    void write(QIODevice &device)
    {
        // head
        std::string group_line = "[" + m_name + "]";
        device.write(group_line.c_str(), group_line.length());
        device.write("\n");

        // lines
        for (size_t i = 0; i < m_lines.size(); ++i)
        {
            m_lines[i]->write(device);
            device.write("\n");
        }
    }

    IniLine* findLine(const std::string &keyName)
    {
        for (int i = 0; i < m_lines.size(); ++i)
        {
            if (m_lines[i]->isSettingOf(keyName))
            {
                return m_lines[i];
            }
        }

        return NULL;
    }

    bool replaceLine(IniLine *oldLine, IniLine *newLine)
    {
        for (int i = 0; i < m_lines.size(); ++i)
        {
            if (m_lines[i] == oldLine)
            {
                delete m_lines[i];
                m_lines[i] = newLine;
                return true;
            }
        }
        return false;
    }

    void destory()
    {
        for (size_t i = 0; i < m_lines.size(); ++i)
        {
            delete m_lines[i];
        }
        m_lines.clear();
    }

private:
    std::vector<IniLine*> m_lines;
    std::string m_name;
};

class IniFile
{
public:

    ~IniFile()
    {
        qDebug() << "destory";
    }

    void read(QIODevice &device)
    {
        char buf[1024] = { 0 };
        qint64 lineLength = device.readLine(buf, sizeof(buf));
        IniGroup *currentGroup = NULL;
        while (lineLength != -1)
        {
            std::string line(buf, lineLength - 1);
            if (line[0] == '[' && line[line.length() - 1] == ']')
            {
                std::string groupName(line.substr(1, line.length() - 2));
                currentGroup = new IniGroup(groupName);
                m_groups.push_back(currentGroup);
            }
            else
            {
                IniLine * newLine = currentGroup->parse(line);
                if (newLine)
                {
                    currentGroup->append(newLine);
                }
            }

            lineLength = device.readLine(buf, sizeof(buf));
        }
    }

    void write(QIODevice &device)
    {
        for (std::size_t i = 0; i < m_groups.size(); ++i)
        {
            m_groups[i]->write(device);
            m_groups[i]->destory();
            delete m_groups[i];
        }
        m_groups.clear();
    }

    void merge(const QMap<QString, QString> &map)
    {
        for (QMap<QString, QString>::ConstIterator it = map.begin(); it != map.end(); ++it)
        {
            std::string key = it.key().toStdString();
            std::string::size_type separator = key.find_first_of('/');
            if (separator != std::string::npos)
            {
                std::string groupName = key.substr(0, separator);
                std::string keyName = key.substr(separator + 1);
                std::string value = it.value().toStdString();

                IniGroup *group = findGroup(groupName);
                if (group)
                {
                    IniLine *line = group->findLine(keyName);
                    if (!line)
                    {
                        IniSettingLine *settingLine = new IniSettingLine();
                        settingLine->set(keyName, value);
                        group->append(line);
                    }
                    else
                    {
                        if (line->type() == "IniSettingLine")
                        {
                            IniSettingLine *settingLine = static_cast<IniSettingLine*>(line);
                            settingLine->set(keyName, value);
                        }
                        else if (line->type() == "IniCommentLine")
                        {
                            IniSettingLine *settingLine = new IniSettingLine();
                            settingLine->set(keyName, value);
                            group->replaceLine(line, settingLine);
                        }
                    }
                }
                else
                {
                    group = new IniGroup(groupName);
                    IniSettingLine *line = new IniSettingLine();
                    line->set(keyName, it.value().toStdString());
                    group->append(line);
                    m_groups.push_back(group);
                }
            }
        }
    }

    IniGroup *findGroup(const std::string &name)
    {
        for (std::vector<IniGroup*>::const_iterator iter = m_groups.begin(); iter != m_groups.end(); ++iter)
        {
            if ((*iter)->name() == name)
            {
                return *iter;
            }
        }

        return NULL;
    }

private:
    std::vector<IniGroup*> m_groups;
};

IniSetting::IniSetting(const QString &path)
    : m_path(path)
{
    load();
}

void IniSetting::test()
{
    IniSetting setting("/home/cynhard/.config/fcitx/config");
    setting.set("Output/HalfPuncAfterNumber", "False");
    setting.save();
}

bool IniSetting::load()
{
    QFile file(m_path);
    if (!file.exists())
    {
        return false;
    }
    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    m_iniFile = new IniFile();
    m_iniFile->read(file);

    file.close();
    return true;
}

void IniSetting::set(const QString &key, const QString &value)
{
    m_map[key] = value;
}

bool IniSetting::save()
{
    QFile file(m_path);
    if (!file.exists())
    {
        return false;
    }
    if (!file.open(QIODevice::WriteOnly))
    {
        return false;
    }

    m_iniFile->merge(m_map);
    m_iniFile->write(file);

    file.close();

    delete m_iniFile; m_iniFile = NULL;
    return true;
}
