#include "spschememodel.h"
#include <QFile>
#include <QSettings>
#include <QTextCodec>
#include <QDebug>
#include <QSettings>
#include <QDir>
#include <QStringList>


SpSchemeModel* SpSchemeModel::CreateSpSchemeMoel(bool hasbasic, QString basic_scheme_path)
{
    SpSchemeModel* instance = NULL;
    if(hasbasic)
    {
        instance = new SpSchemeModel(basic_scheme_path);
    }
    else
    {
        instance = new SpSchemeModel();
    }
    return instance;
}

SpSchemeModel::SpSchemeModel()
{
    InitDataModel();
}

SpSchemeModel::~SpSchemeModel()
{

}

SpSchemeModel::SpSchemeModel(QString basic_scheme_path)
{
    ParserSPScheme(basic_scheme_path);
    InitDataModel();
}

SpSchemeModel::SpSchemeModel(SpSchemeModel &model)
{
    m_configurable_initial_map = model.m_configurable_initial_map;
    m_configurable_final_map = model.m_configurable_final_map;
    m_zero_final_map = model.m_zero_final_map;
    m_key_use_map = model.m_key_use_map;
    m_ismodel_valid = model.m_ismodel_valid;
}

SpSchemeModel& SpSchemeModel::operator=(const SpSchemeModel &model)
{
    m_configurable_initial_map = model.m_configurable_initial_map;
    m_configurable_final_map = model.m_configurable_final_map;
    m_zero_final_map = model.m_zero_final_map;
    m_key_use_map = model.m_key_use_map;
    m_ismodel_valid = model.m_ismodel_valid;
    return  *this;
}


void SpSchemeModel::SetConfigInitial(QMap<QString, QString> &config_initial_map)
{
    m_configurable_initial_map = config_initial_map;
    InitKeyMapModel();
}

void SpSchemeModel::SetConfigFinal(QMap<QString, QString> &config_final_map)
{
    m_configurable_final_map = config_final_map;
    InitKeyMapModel();
}

QVector<QString> SpSchemeModel::GetSyllableAvailableKeys(QString &syllable)
{
    //todo 这里面的一部分音节需要进行策略
    QVector<QString> result;
    result << "-";
    if(!m_all_syllable_vector.contains(syllable))
    {
        return result;
    }
    if(syllable == "zh" || syllable == "ch" || syllable == "sh")
    {
        for(QString key : m_all_initial_key_vector)
        {
            if(m_key_use_map[key].initial_syllable.isEmpty()
                    || m_key_use_map[key].initial_syllable == syllable)
            {
                result.push_back(key);
            }
        }
    }
    else
    {
        for(QString key : m_all_key_vector)
        {
            if(m_key_use_map[key].final_syllable_Set.size() <= 3
                    || m_key_use_map[key].final_syllable_Set.contains(syllable))
            {
                result.push_back(key);
            }
        }
    }
    return result;
}

void SpSchemeModel::InitDataModel()
{
    m_ismodel_valid = true;
    m_all_syllable_vector << "zh" << "ch" << "sh" << "ai" << "an" << "ang"
                          << "ao" << "ei" << "en" << "eng" << "er" << "ia"
                          << "ian" << "iang" << "iao" << "ie" << "in" << "ing"
                          << "iong" << "iu" << "ong" << "ou" << "ua" << "uai"
                          << "uan" << "uang" << "ue" << "ui" << "un" << "uo"
                          << "v" << "ve";

    m_all_key_vector  << "Q" << "W" << "E" << "R" << "T" << "Y" << "U" << "I"
                      << "O" << "P" << "A" << "S" << "D" << "F" << "G" << "H"
                      << "J" << "K" << "L" << ";" << "Z" << "X" << "C" << "V"
                      << "B"<< "N" << "M";

    m_all_initial_key_vector << "A" << "E" << "I" << "U" << "V";

    m_special_syllbale << "er" << "ia" << "iong" << "ua" << "uai" << "uang" << "ui";

    //固定的韵母
    for(QString final : m_all_initial_key_vector)
    {
        m_fixed_final_map[final.toLower()] = final;
    }

    //固定的声母
    for(int index=0; index<m_all_key_vector.size(); ++index)
    {
        QString current_key = m_all_key_vector.at(index);
        if((current_key == "V") || (current_key == ";"))
        {
            continue;
        }
        if(m_all_initial_key_vector.contains(current_key))
        {
            continue;
        }
        m_fixed_initial_map[current_key.toLower()] = current_key;
    }
    //初始化按键映射表
    InitKeyMapModel();
}

void SpSchemeModel::InitKeyMapModel()
{
    //初始化声母的按键列表
    m_key_use_map.clear();
    QList<QString> fix_initial_list = m_fixed_initial_map.keys();
    for(QString fix_initial : fix_initial_list)
    {
        QString inital_key = m_fixed_initial_map[fix_initial];

        m_key_use_map[inital_key].initial_syllable = fix_initial;

    }
    QList<QString> config_initial_list = m_configurable_initial_map.keys();
    for(QString config_initial : config_initial_list)
    {
        QString config_initial_key = m_configurable_initial_map[config_initial];
        if(config_initial_key.isEmpty())
        {
            continue;
        }
        m_key_use_map[config_initial_key].initial_syllable = config_initial;
    }
    //初始化韵母的按键映射列表
    QList<QString> fix_finial_list = m_fixed_final_map.keys();
    for(QString fix_final : fix_finial_list)
    {
        QString fix_final_key = m_fixed_final_map[fix_final];
        m_key_use_map[fix_final_key].final_syllable_Set.insert(fix_final);
    }

    QList<QString> config_final_list = m_configurable_final_map.keys();
    for(QString config_final : config_final_list)
    {
        QString config_final_key = m_configurable_final_map[config_final];
        if(config_final_key.isEmpty())
        {
            continue;
        }
        m_key_use_map[config_final_key].final_syllable_Set.insert(config_final);
    }

}

void SpSchemeModel::ParserSPScheme(QString scheme_path)
{
    if(!QFile::exists(scheme_path))
    {
        return;
    }
    try {
        //解析声母
        ParserINIFile(scheme_path,"Initial",m_configurable_initial_map);
        ParserINIFile(scheme_path, "Final",m_configurable_final_map);
        ParserINIFile(scheme_path, "ZeroFinal",m_zero_final_map);
    } catch (...) {
        qDebug() << "Parser the ini file throw exception";
    }
}

void SpSchemeModel::ParserINIFile(QString fileName, QString groupname, QMap<QString, QString> &result)
{
    QSettings settings(fileName,QSettings::IniFormat);
    QStringList groupsList = settings.childGroups();
    if(!groupsList.contains(groupname))
    {
        return;
    }

    settings.beginGroup(groupname);
    QStringList keyLists = settings.childKeys();
    for(QString keyIndex:keyLists)
    {
        QString value = settings.value(keyIndex).toString();
        if(value.isEmpty())
        {
             result[keyIndex] = ";";
        }
        else
        {
            result[keyIndex] = value;
        }

    }
    settings.endGroup();
}

void SpSchemeModel::WriteMapToDataStream(QMap<QString, QString> &data_map, QTextStream &outputStream)
{
    QList<QString> key_list = data_map.keys();
    for(int index=0; index<key_list.size(); ++index)
    {
        if(key_list.at(index).isEmpty() || data_map[key_list[index]].isEmpty())
            continue;
        QString targetLine = key_list.at(index) + "=" + data_map[key_list[index]] + "\n";
        outputStream << targetLine;
    }
}

void SpSchemeModel::SetSyllableAndKeypair(QString syllable, QString key)
{
    key = (key=="-")?QString(""):key;
    if(syllable == "zh" || syllable == "ch" || syllable == "sh")
    {
        m_configurable_initial_map[syllable] = key;
    }
    else
    {
        m_configurable_final_map[syllable] = key;
    }
    InitKeyMapModel();
}

int SpSchemeModel::SaveSPConfigToFile(QString filePath)
{
    if(QFile::exists(filePath))
    {
        return 1;
    }
    if(CheckSPPlan() != 0)
    {
        return 2;
    }
    QFile file(filePath);

    QFileInfo fileInfo(filePath);
    QString fileDir = fileInfo.path();
    QDir* dir = new QDir();
    if(!dir->exists(fileDir)){
        dir->mkdir(fileDir);
    }
    delete dir;

    if(file.open(QIODevice::WriteOnly))
    {
        QTextStream outputStream(&file);
        //outputStream.setGenerateByteOrderMark(true);
        outputStream.setCodec("UTF-8");
        outputStream << QString::fromLocal8Bit(";华宇双拼自定义方案\n");
        outputStream << "[Initial]" << "\n";
        WriteMapToDataStream(m_configurable_initial_map,outputStream);
        outputStream << "[Final]" << "\n";
        WriteMapToDataStream(m_configurable_final_map,outputStream);
        outputStream << "[ZeroFinal]" << "\n";
        WriteMapToDataStream(m_zero_final_map,outputStream);
        file.close();
        return 0;
    }
    return 1;
}

int SpSchemeModel::CheckSPPlan()
{
    for(QString syllable : m_all_syllable_vector)
    {
        if(syllable == "v")
        {
            continue;
        }
        if(syllable == "zh" || syllable == "ch" || syllable == "sh")
        {
            if(m_configurable_initial_map.contains(syllable)
                    && !m_configurable_initial_map[syllable].isEmpty())
            {
                continue;
            }
            return 1;
        }
        else
        {
            if(m_zero_final_map.contains(syllable) && !m_zero_final_map[syllable].isEmpty())
            {
                continue;
            }
            if(m_configurable_final_map.contains(syllable) && !m_configurable_final_map[syllable].isEmpty())
            {
                continue;
            }
            return 1;
        }
    }

    return 0;
}

QString SpSchemeModel::GetSPText()
{
    QString sp_content;
    sp_content.append(";华宇双拼自定义方案!请不要编辑本行\n");
    sp_content.append("[Initial]\n");
    sp_content.append(ConvertSPToString(m_configurable_initial_map));
    sp_content.append(GetUnUseSyllables(m_configurable_initial_map, INITIAL));
    sp_content.append("[Final]\n");
    sp_content.append(ConvertSPToString(m_configurable_final_map));
    sp_content.append(GetUnUseSyllables(m_configurable_final_map, FINAL));
    sp_content.append("[ZeroFinal]\n");
    sp_content.append(";韵母单独成音时，键位对应关系\n");
    sp_content.append(ConvertSPToString(m_zero_final_map));
    sp_content.append(GetUnUseSyllables(m_zero_final_map, ZEROFINAL));
    return sp_content;
}

QString SpSchemeModel::ConvertSPToString(QMap<QString, QString>& sp)
{
    QString text;
    auto iter = sp.begin();
    for(; iter != sp.end(); iter++)
    {
        if(iter.key().isEmpty())
            continue;
        text.append(iter.key()).append("=").append(iter.value()).append("\n");
    }
    return text;
}

QString SpSchemeModel::GetUnUseSyllables(QMap<QString, QString>& sp, SYLLABLE_TYPE type)
{
    QString result;
    QStringList _list ;
    switch(type)
    {
        case INITIAL:
        {
            _list << "zh" << "ch" << "sh";
            break;
        }
        case FINAL:
        {
            _list << "ai" << "an" << "ang"
                  << "ao" << "ei" << "en" << "eng" << "er" << "ia"
                  << "ian" << "iang" << "iao" << "ie" << "in" << "ing"
                  << "iong" << "iu" << "ong" << "ou" << "ua" << "uai"
                  << "uan" << "uang" << "ue" << "ui" << "un" << "uo"
                  << "v" << "ve";
            break;
        }
        case ZEROFINAL:
        {
            _list << "a" << "ai" << "an" << "ang" << "ao"
                  << "e" << "ei" << "en" << "eng" << "o"
                  << "ou";
            break;
        }
        default:
            break;

    }
    for(QString& syllable : _list)
    {
        if(!sp.contains(syllable))
            result.append(syllable).append("=\n");
    }
    return result;
}
