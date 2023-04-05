#ifndef INISETTING_H
#define INISETTING_H

#include <QString>
#include <QMap>

class IniFile;

// !!! this class is not thread safe !!!
class IniSetting
{
public:
    IniSetting(const QString &path);
    void set(const QString &key, const QString &value);
    bool save();

    static void test();

private:
    bool load();

private:
    QString m_path;
    IniFile* m_iniFile;
    QMap<QString, QString> m_map;
};

#endif // INISETTING_H
