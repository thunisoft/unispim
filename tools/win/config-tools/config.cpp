#include "config.h"
#include "utils.h"

#include <QDir>
#include <QSettings>
#include <QVector>
#include <QString>
#include <stdlib.h>
#include <stdio.h>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QDebug>
#include <QDir>
#include <QApplication>
#include <QList>
#include <QNetworkInterface>
#include <QCryptographicHash>
#include <QFileInfoList>
#include <QMessageBox>
#include <QRegExp>
#include "mainwindow.h"
#include "wordlibstackedwidget.h"

Config* Config::m_instance = NULL;

QString Config::SystemWordlibPath()
{
    return QString("C:\\ProgramData\\Thunisoft\\HuayuPY\\wordlib\\sys.uwl");
}

QString Config::GetInstallDir()
{
    QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Thunisoft\\Huayupy\\7.0",QSettings::NativeFormat);
    QString installPath = settings.value("InstallDir","").toString();
    return installPath;
}

QString Config::GetProgramdataDir()
{
    return QString("C:\\ProgramData\\Thunisoft\\HuayuPY\\");
}

QString Config::GetIMEVersion()
{
    QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Thunisoft\\Huayupy\\7.0",QSettings::NativeFormat);
    QString installPath = settings.value("InstallDir","").toString();
    if(installPath.isEmpty())
    {
      return QString("7.0.0.0");
    }
    else
    {
        QString fullFilePath = installPath + "\\version";
        QFile file(fullFilePath);
        if(file.open(QIODevice::ReadOnly))
        {
            QString versionNum = file.readAll();
            file.close();
            return  versionNum.trimmed();
        }
        else
        {
            return QString("7.0.0.0");
        }
    }
}

bool Config::RemoveTaskBarRightClickName()
{
    QString application_filepath = QApplication::applicationFilePath();
    QString regkey = application_filepath + ".FriendlyAppName";
    QSettings settings("HKEY_CURRENT_USER\\Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\Shell\\MuiCache",QSettings::NativeFormat);
    QList<QString> key_list = settings.childKeys();
    if(key_list.contains(regkey))
    {
        QString searchKey = QDir::toNativeSeparators(regkey);
        QString keyValue = settings.value(regkey,"").toString();
        if(keyValue == "config-tools.exe")
        {
            settings.setValue(regkey,"输入法 配置工具");
        }
    }
    return false;
}

QString Config::GetHttpsCertificatePath()
{
    QString filePath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\server_certification.cer");
    return filePath;
}

QString Config::GetLoginID()
{
    if(m_stateConfigMap.contains("login")&&
            m_stateConfigMap.contains("loginid"))
    {
        if(m_stateConfigMap["login"].itemCurrentIntValue == 0)
        {
            return "";
        }
        else if(m_stateConfigMap["login"].itemCurrentIntValue == 1)
        {
            return m_stateConfigMap["loginid"].itemCurrentStrValue;
        }
    }
    else
    {
        return"";
    }
    return "";
}

bool Config::ChangeLoginStateWhenExit()
{
    if(m_stateConfigMap.contains("auto_enroll")&&
            m_stateConfigMap.contains("login"))
    {
        int is_login = m_stateConfigMap["login"].itemCurrentIntValue;
        int is_auto_enroll = m_stateConfigMap["auto_enroll"].itemCurrentIntValue;

        if((is_login == 1) && (is_auto_enroll == 0))
        {
            m_stateConfigMap["login"].itemCurrentIntValue = 0;
            m_currentConfigMap["login"].itemCurrentIntValue = 0;
        }
        SaveStateConfig();
        return true;
    }
    else
    {
        return false;
    }

}

QString Config::GetVersionCheckIniPath()
{
    return Utils::GetConfigDirPath() + "version_check.ini";
}

bool Config::HasNewVersion()
{
    bool new_dir_exist = false;
    QString version_check_file_path = GetVersionCheckIniPath();

    QString installPath = Config::Instance()->GetInstallDir();
    QString tempDir = installPath + "\\HuaYuPY.new";
    QDir checkDir;
    if(checkDir.exists(tempDir))
    {
        new_dir_exist = true;
    }
    else
    {
        new_dir_exist = false;
    }

    if(!QFile::exists(version_check_file_path))
    {
        return false;
    }
    else
    {
        QSettings settings(version_check_file_path,QSettings::IniFormat);
        settings.beginGroup("version");
        QString current_version = settings.value("current_version","").toString();
        QString server_version = settings.value("server_version","").toString();
        settings.endGroup();
        if(current_version != server_version)
        {
            return true;
        }
        else if((current_version == server_version) && new_dir_exist)
        {
            return true;
        }
        else
        {
             return false;
        }

    }
}

QString Config::DownloadPackageName()
{
    if(HasNewVersion())
    {
        QString version_check_file_path = GetVersionCheckIniPath();
        if(!QFile::exists(version_check_file_path))
        {
            return "";
        }
        else
        {
            QSettings settings(version_check_file_path,QSettings::IniFormat);
            settings.beginGroup("version");
            QString package_name = settings.value("package_name","").toString();
            settings.endGroup();
            return package_name;
        }
    }
    else
    {
        return "";
    }
}


void Config::LoadConfig()
{
    //依次加载系统配置/状态配置/用户配置
    LoadSystemConfig();
    LoadStateConfig();
    LoadUserConfig();
    MergeConfigInfo();

    //布局管理
    m_layoutMap["labellayout"] = QVector<int>() << 21<<16<<1<<77<<454;
    m_layoutMap["basicmainlayout"] = QVector<int>() << 30 << 60 << 10 << 50 << 30 << 120 << 20 << 30 << 30 << 10;
    m_layoutMap["part3_layout"] = QVector<int>() << 10 << 40 << 10;
    m_layoutMap["advancemainlayout"] = QVector<int>() << 1 << 1 << 2 << 1 << 1 << 3 << 2 << 1 << 1 << 2;
    m_geoScaledRate = 1.0;
    m_fontScaledRate = 1.0;

    has_domainChanged = false;
}

//------------------------------------

void Config::SaveConfig()
{
    SaveStateConfig();
    SaveUserConfig();
}

void Config::SaveSystemConfig()
{
    SaveStateConfig();
}

void Config::SaveUserConfig()
{
    QString userConfigFilePath = configJsonFilePath();
    SaveConfigInfoToJson(userConfigFilePath,m_userChangedMap);
}

void Config::SaveStateConfig()
{
    QString state_config_path = Utils::GetConfigDirPath() + QString("config.json");
    SaveConfigInfoToJson(state_config_path,m_stateConfigMap);
}

void Config::ShowNewFeature()
{
    ConfigItemStruct configInfo;
    if(GetConfigItemByJson("tool_bar_config",configInfo))
    {
        if(configInfo.itemDefaultIntValue != 31)
        {
            configInfo.itemCurrentIntValue |= USE_SPECIAL_SYMBOL;
            SetConfigItemByJson("tool_bar_config",configInfo);
            SaveConfig();
        }
    }
}

void Config::ReloadConfig()
{
    LoadConfig();
}

bool Config::LoadPublishWordlib()
{
    QString publish_wordlib_path = Utils::GetConfigDirPath() + QString("publish_wordlib_list.json");

    QFile file(publish_wordlib_path);
    if(!file.exists())
    {
        return false;
    }
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString value = file.readAll();
    file.close();

    QJsonParseError parseJsonErr;
    QJsonDocument document = QJsonDocument::fromJson(value.toUtf8(),&parseJsonErr);
    if(!(parseJsonErr.error == QJsonParseError::NoError))
    {
        return false;
    }
    QJsonArray jsonValueArray = document.array();

    QVector<QString> publish_wordlib_vector;

    int array_size = jsonValueArray.size();
    for(int index=0; index<array_size; ++index)
    {
        QJsonValue current_value = jsonValueArray.at(index);
        QJsonObject childObject = current_value.toObject();
        QString wordlib_name = childObject.value("wordlibname").toString();
        publish_wordlib_vector.push_back(wordlib_name);
    }
    QVector<QString> already_selected_vector = GetSelectedWordlibContainer();

    auto insert_system_default = [&](QString file_name){
        if(!already_selected_vector.contains(file_name))
        {
            already_selected_vector.push_back(file_name);
        }
    };
    for_each(WordlibStackedWidget::SYS_WORLIDB_LIST.cbegin(),WordlibStackedWidget::SYS_WORLIDB_LIST.cend(),insert_system_default);


    foreach(QString index_str, publish_wordlib_vector)
    {
        if(!already_selected_vector.contains(index_str))
        {
            already_selected_vector.push_back(index_str);
        }
    }

    SetSelectedWordlibContainer(already_selected_vector);
    return true;
}

//--------------------------------------

Config* Config::Instance()
{
    if(!m_instance)
    {
        m_instance = new Config();
    }
    return m_instance;
}


Config::Config()
{
    LoadConfig();
    m_configCountSem = CreateSemaphore(NULL,0,1,TEXT("config-tools-counter"));
}

Config::~Config()
{
    CloseHandle(m_configCountSem);
    m_configCountSem = NULL;
}


QString Config::configJsonFilePath()
{
    if(m_stateConfigMap.contains("login")&&
            m_stateConfigMap.contains("loginid"))
    {
        if(m_stateConfigMap["login"].itemCurrentIntValue == 0)
        {
            return Utils::GetConfigDirPath() + QString("config.json");
        }
        else
        {
            QString configPathDir = Utils::GetConfigDirPath() +
                    m_stateConfigMap["loginid"].itemCurrentStrValue + QString("\\");
            QDir dir;
            if(!dir.exists(configPathDir))
            {
                dir.mkpath(configPathDir);
            }
            QString fullFilePath = configPathDir + QString("config.json");

            return QDir::toNativeSeparators(fullFilePath);
        }

    }
    else
    {
        return Utils::GetConfigDirPath() + QString("config.json");
    }
}

QString Config::CustomPhraseFilePath()
{
    if(m_stateConfigMap.contains("login")&&
            m_stateConfigMap.contains("loginid"))
    {
        if(m_stateConfigMap["login"].itemCurrentIntValue == 0)
        {
            return Utils::GetConfigDirPath() + QString("phrase\\user_phrase.txt");
        }
        else
        {
            QString configPathDir = Utils::GetConfigDirPath() + QString("phrase\\") +
                    m_stateConfigMap["loginid"].itemCurrentStrValue + QString("\\");
            QDir dir;
            if(!dir.exists(configPathDir))
            {
                dir.mkpath(configPathDir);
            }
            QString fullFilePath = configPathDir + QString("user_phrase.txt");

            return QDir::toNativeSeparators(fullFilePath);
        }

    }
    else
    {
        return Utils::GetConfigDirPath() + QString("phrase\\user_phrase.txt");
    }
}

QString Config::UserWordlibFilePath()
{
    if(m_stateConfigMap.contains("login")&&
            m_stateConfigMap.contains("loginid"))
    {
        if(m_stateConfigMap["login"].itemCurrentIntValue == 0)
        {
            return Utils::GetConfigDirPath() + QString("wordlib\\user.uwl");
        }
        else
        {
            QString configPathDir = Utils::GetConfigDirPath() + QString("wordlib\\") +
                    m_stateConfigMap["loginid"].itemCurrentStrValue + QString("\\");
            QDir dir;
            if(!dir.exists(configPathDir))
            {
                dir.mkpath(configPathDir);
            }
            QString fullFilePath = configPathDir + QString("user.uwl");

            return QDir::toNativeSeparators(fullFilePath);
        }

    }
    else
    {
        return Utils::GetConfigDirPath() + QString("wordlib\\user.uwl");
    }
}

QString Config::databasePath()
{
    return Utils::GetConfigDirPath() + QString("MyDataBase.db");
}

QString Config::wordLibDirPath()
{
    return QString("C:\\ProgramData\\Thunisoft\\Huayupy\\wordlib\\");
}

bool Config::SaveConfigInfoToJson(QString filePath,QMap<QString,ConfigItemStruct> infoMap)
{
    QFile file(filePath);
    file.open(QIODevice::ReadOnly);

    QTextStream inputStream(&file);
    inputStream.setCodec("UTF-8");
    QString value = inputStream.readAll();

    file.close();

    QJsonObject rootObject;
    if(!value.isEmpty())
    {
        QJsonParseError parseJsonErr;
        QJsonDocument document = QJsonDocument::fromJson(value.toUtf8(),&parseJsonErr);
        if(parseJsonErr.error == QJsonParseError::NoError)
        {
           rootObject = document.object();
        }
    }

    //将结构体转成json
    QStringList keyList = infoMap.keys();
    for(int index=0; index<keyList.size(); ++index)
    {
        QString currentKey = keyList.at(index);
        QJsonObject currentObject;
        ConfigItemStruct currentValue = infoMap[currentKey];
        currentObject.insert("group",currentValue.itemGroupName);
        currentObject.insert("type",currentValue.itemType);
        if(currentValue.itemType == "int")
        {
            currentObject.insert("defaultvalue",currentValue.itemDefaultIntValue);
            currentObject.insert("currentvalue",currentValue.itemCurrentIntValue);
        }
        else if(currentValue.itemType == "string")
        {

            currentObject.insert("defaultvalue",currentValue.itemDefaultStrValue);
            currentObject.insert("currentvalue",currentValue.itemCurrentStrValue);
        }
        if(rootObject.contains(currentKey))
        {
            QJsonObject  key_object = rootObject.value(currentKey).toObject();
            if(key_object == currentObject)
            {
                continue;
            }
            else
            {
                rootObject.insert(currentKey,currentObject);

            }
        }
        else
        {
            rootObject.insert(currentKey,currentObject);
        }

    }

    QJsonDocument jsonDocument;
    jsonDocument.setObject(rootObject);
    QByteArray jsonContent = jsonDocument.toJson();
    QFile output_file(filePath);
    output_file.open(QIODevice::WriteOnly|QIODevice::Text);
    QTextStream output(&output_file);
    output.setCodec("UTF-8");
    output << jsonContent;
    output_file.close();
    return true;
}

bool Config::GetConfigItemByJson(QString key,ConfigItemStruct& keyValue)
{
    if(!m_currentConfigMap.contains(key))
    {
        return false;
    }

    keyValue = m_currentConfigMap[key];
    return true;
}

int  Config::SetConfigItemByJson(QString key,ConfigItemStruct keyValue)
{
    //针对配置项修改添加了对应的日志
    if(m_currentConfigMap.contains(key))
    {
        ConfigItemStruct old_value = m_currentConfigMap.value(key);
        keyValue.itemDefaultIntValue = old_value.itemDefaultIntValue;
        keyValue.itemDefaultStrValue = old_value.itemDefaultStrValue;

        if(keyValue.itemType != old_value.itemType)
        {
            Utils::WriteLogToFile(QString("配置项%1的设置类型冲突").arg(key));
            return 0;
        }
        QString oldValueConfigItem, newValueConfigItem;

        if(old_value.itemType == "string" )
        {
            newValueConfigItem = keyValue.itemCurrentStrValue;
            oldValueConfigItem = old_value.itemCurrentStrValue;
        }
        else if(old_value.itemType == "int")
        {
            newValueConfigItem = QString::number(keyValue.itemCurrentIntValue);
            oldValueConfigItem = QString::number(old_value.itemCurrentIntValue);
        }
        QStringList filterNameList;
        filterNameList << "wordlib_name" << "wordlib_count";
        if(!filterNameList.contains(key))
        {
            QString configInfo = QString("配置项%1原值为%2修改为%3").arg(key).arg(oldValueConfigItem).arg(newValueConfigItem);
            Utils::WriteLogToFile(configInfo);
        }
    }

    m_currentConfigMap[key] = keyValue;
    if(keyValue.itemGroupName == "state")
    {
        m_stateConfigMap[key] = keyValue;
    }
    else if(keyValue.itemGroupName == "user")
    {
        m_userChangedMap[key] = keyValue;
    }

    if(check_shift_ctrl_conflict())
    {
        return 1;
    }

    if(check_bracket_item_conflict())
    {
        return 2;
    }

    //每次配置之后及时生效
    make_config_work();

    return 0;
}

int Config::SetStateConfigItemByJson(QString key, ConfigItemStruct keyValue)
{
    if(keyValue.itemGroupName != "state")
    {
        return 1;
    }

    //针对配置项修改添加了对应的日志
    if(m_currentConfigMap.contains(key))
    {
        ConfigItemStruct old_value = m_currentConfigMap.value(key);
        keyValue.itemDefaultIntValue = old_value.itemDefaultIntValue;
        keyValue.itemDefaultStrValue = old_value.itemDefaultStrValue;

        if(keyValue.itemType != old_value.itemType)
        {
            Utils::WriteLogToFile(QString("配置项%1的设置类型冲突").arg(key));
            return 0;
        }
        QString oldValueConfigItem, newValueConfigItem;

        if(old_value.itemType == "string" )
        {
            newValueConfigItem = keyValue.itemCurrentStrValue;
            oldValueConfigItem = old_value.itemCurrentStrValue;
        }
        else if(old_value.itemType == "int")
        {
            newValueConfigItem = QString::number(keyValue.itemCurrentIntValue);
            oldValueConfigItem = QString::number(old_value.itemCurrentIntValue);
        }
        QStringList filterNameList;
        filterNameList << "wordlib_name" << "wordlib_count";
        if(!filterNameList.contains(key))
        {
            QString configInfo = QString("配置项%1原值为%2修改为%3").arg(key).arg(oldValueConfigItem).arg(newValueConfigItem);
            Utils::WriteLogToFile(configInfo);
        }
    }

    m_currentConfigMap[key] = keyValue;
    if(keyValue.itemGroupName == "state")
    {
        m_stateConfigMap[key] = keyValue;
    }

    make_state_config_work();
    return 0;
}

int Config::SetConfigItemByJsonWithoutCheck(QString key, ConfigItemStruct keyValue)
{
    //针对配置项修改添加了对应的日志
    if(m_currentConfigMap.contains(key))
    {
        ConfigItemStruct old_value = m_currentConfigMap.value(key);
        keyValue.itemDefaultIntValue = old_value.itemDefaultIntValue;
        keyValue.itemDefaultStrValue = old_value.itemDefaultStrValue;

        if(keyValue.itemType != old_value.itemType)
        {
            Utils::WriteLogToFile(QString("配置项%1的设置类型冲突").arg(key));
            return 0;
        }
        QString oldValueConfigItem, newValueConfigItem;

        if(old_value.itemType == "string" )
        {
            newValueConfigItem = keyValue.itemCurrentStrValue;
            oldValueConfigItem = old_value.itemCurrentStrValue;
        }
        else if(old_value.itemType == "int")
        {
            newValueConfigItem = QString::number(keyValue.itemCurrentIntValue);
            oldValueConfigItem = QString::number(old_value.itemCurrentIntValue);
        }
        QStringList filterNameList;
        filterNameList << "wordlib_name" << "wordlib_count";
        if(!filterNameList.contains(key))
        {
            QString configInfo = QString("配置项%1原值为%2修改为%3").arg(key).arg(oldValueConfigItem).arg(newValueConfigItem);
            Utils::WriteLogToFile(configInfo);
        }
    }

    m_currentConfigMap[key] = keyValue;
    if(keyValue.itemGroupName == "state")
    {
        m_stateConfigMap[key] = keyValue;
    }
    else if(keyValue.itemGroupName == "user")
    {
        m_userChangedMap[key] = keyValue;
    }

    //每次配置之后及时生效
    make_config_work();

    return 0;
}

void Config::SetTmpConfigItemByJson(QString key,ConfigItemStruct keyValue)
{
    m_currentConfigMap[key] = keyValue;
}

QList<QString> Config::GetSPSchemeList()
{
    QList<QString> result;

    result << "华宇双拼" << "搜狗双拼" <<"DOS双拼" <<"蓝天双拼"
    << "拼音加加" << "微软双拼" << "智能ABC" <<"中文之星" << "自然码" << "小鹤双拼";

    QString user_ini_file_path = Config::Instance()->GetUserSPIniFileDir();
    QDir fileDir(user_ini_file_path);
    QFileInfoList fileInfoList = fileDir.entryInfoList();

    int fileCount = fileInfoList.count();
    for(int inf = 0; inf < fileCount; inf ++)

    {
        QFileInfo currentInfo = fileInfoList.at(inf);
        QString suffix = currentInfo.suffix();

        if(currentInfo.isFile() && (suffix == "ini"))
        {
            QString fileName = currentInfo.baseName();
            result << fileName;
        }
    }
    return result;
}

QList<QString> Config::GetDefaultSpSchemeList()
{
    QList<QString> result;
    result << "华宇双拼" << "搜狗双拼" <<"DOS双拼" <<"蓝天双拼"
    << "拼音加加" << "微软双拼" << "智能ABC" <<"中文之星" << "自然码" << "小鹤双拼";
    return result;
}

void Config::AddOrDeleteTheSpScheme(QString fileName, bool flag)
{
    QString sp_ini_file_path = QDir::toNativeSeparators(Utils::GetConfigDirPath() + "SpFileSchemeList.ini");
    QSettings settings(sp_ini_file_path,QSettings::IniFormat);
    settings.beginGroup("usersp");
    QList<QString> file_name_list = settings.childKeys();
    if(flag)
    {
        settings.setValue(fileName, 1);
    }
    else
    {
        if(file_name_list.contains(fileName))
        {
            settings.remove(fileName);
        }
    }
    settings.endGroup();
}

void Config::setTempDomain(const QString tempDomain)
{
    m_tempDomain = tempDomain;
    has_domainChanged = true;
}

void Config::make_config_work()
{
    QString message;
    if (!CheckValidation(message))
    {
        QMessageBox::information(NULL, QString::fromLocal8Bit("系统提示"), message);
        return;
    }

    SaveConfig();

    if(IsToolBarChanged())
    {
        MainWindow::SendMsgToEngine();
    }

    auto inform_ime_update_config = [this](){
        //释放信号量
        if(ReleaseSemaphore(m_configCountSem,1,NULL))
        {
            Utils::WriteLogToFile("inform the ime to update config succeed");
        }
    };

    inform_ime_update_config();
    Config::Instance()->ReloadConfig();
}

void Config::make_state_config_work()
{
    QString message;
    if (!CheckValidation(message))
    {
        QMessageBox::information(NULL, QString::fromLocal8Bit("系统提示"), message);
        return;
    }

    SaveStateConfig();

    auto inform_ime_update_config = [this](){
        //释放信号量
        if(ReleaseSemaphore(m_configCountSem,1,NULL))
        {
            Utils::WriteLogToFile("inform the ime to update config succeed");
        }
    };

    inform_ime_update_config();
    Config::Instance()->ReloadConfig();
}


void Config::SetSelectedWordlibContainer(const QVector<QString> newWordlibVector)
{
    if(!newWordlibVector.contains("sys.uwl"))
    {
        Utils::WriteLogToFile("wordlib config don't has sys.uwl throw error");
        return;
    }

    int wordlibSize = newWordlibVector.size();

    QString resultStr;
    if(wordlibSize)
    {
        for(int index=0;index<wordlibSize;++index)
        {
            if(index != (wordlibSize-1))
            {
                resultStr.append(newWordlibVector.at(index));
                resultStr.append(";");
            }
            else
            {
                resultStr.append(newWordlibVector.at(index));
            }

        }
    }
    ConfigItemStruct configInifo;
    configInifo.itemName = "wordlib_name";
    configInifo.itemType = "string";
    configInifo.itemCurrentStrValue = resultStr;
    SetConfigItemByJson("wordlib_name",configInifo);

    ConfigItemStruct wordlibcount_info;
    wordlibcount_info.itemName = "wordlib_count";
    wordlibcount_info.itemType = "int";
    wordlibcount_info.itemCurrentIntValue = wordlibSize;
    SetConfigItemByJson("wordlib_count",wordlibcount_info);
}

QVector<QString> Config::GetSelectedWordlibContainer()
{
    QVector<QString> resultVector;
    ConfigItemStruct configInfo;
    if(GetConfigItemByJson("wordlib_name",configInfo))
    {
         QStringList wordlibList = configInfo.itemCurrentStrValue.split(";");
         for(int index=0; index<wordlibList.size(); ++index)
         {
             resultVector.push_back(wordlibList.at(index));
         }
    }
    return resultVector;
}

//合并用户配置和系统配置
void Config::MergeConfigInfo()
{
    m_currentConfigMap.clear();
    m_currentConfigMap = m_systemConfigMap;


    //合并状态配置
    QList<QString> state_config_keys = m_stateConfigMap.keys();
    for(int index=0; index<state_config_keys.size(); ++index)
    {        
        ConfigItemStruct currentItem = m_stateConfigMap[state_config_keys.at(index)];
        //由于将法院版和普通版本进行了合并,所以部分服务器的配置也有可能记录到状态配置文件中
        if((currentItem.itemGroupName == "state"))
        {
            if(m_currentConfigMap.contains(state_config_keys.at(index)))
            {
                if(currentItem.itemType.compare("string") == 0 && currentItem.itemCurrentStrValue.isEmpty())
                    continue;
                m_currentConfigMap[state_config_keys.at(index)] = currentItem;
            }
            else
            {
                m_currentConfigMap.insert(state_config_keys.at(index), currentItem);
            }
        }
    }

    //合并用户配置
    QList<QString> userConfigKeys = m_userConfigMap.keys();
    for(int index=0; index<userConfigKeys.size(); ++index)
    {
        ConfigItemStruct currentItem = m_userConfigMap[userConfigKeys.at(index)];
        if((currentItem.itemGroupName != "user"))
            continue;
        if(m_currentConfigMap.contains(userConfigKeys.at(index)))
        {
            if(currentItem.itemType.compare("string") == 0 && currentItem.itemCurrentStrValue.isEmpty())
                continue;
            m_currentConfigMap[userConfigKeys.at(index)] = currentItem;
        }
        else
        {
            m_currentConfigMap.insert(userConfigKeys.at(index), currentItem);
        }
    }
}

QString Config::GetUserSPIniFileDir()
{

    QString defaultIniPath = QDir::toNativeSeparators(Utils::GetConfigDirPath().append("ini\\"));
    QDir sp_dir;
    if(!sp_dir.exists(defaultIniPath))
    {
        sp_dir.mkpath(defaultIniPath);
    }

    if(m_stateConfigMap.contains("login")&&
            m_stateConfigMap.contains("loginid"))
    {
        if(m_stateConfigMap["login"].itemCurrentIntValue == 0)
        {
            return  defaultIniPath;
        }
        else
        {
            QString configPathDir = Utils::GetConfigDirPath().append("ini/") +
                    m_stateConfigMap["loginid"].itemCurrentStrValue + QString("\\");
            QDir dir;
            if(!dir.exists(configPathDir))
            {
                dir.mkpath(configPathDir);
            }

            return QDir::toNativeSeparators(configPathDir);
        }

    }
    else
    {
         return  defaultIniPath;
    }
}

//加载对应的系统配置
void Config::LoadSystemConfig()
{
    m_systemConfigMap.clear();
    QString jsonFileFath  = QApplication::applicationDirPath() + "\\config.json";
    LoadMapInfoFromJson(jsonFileFath,m_systemConfigMap);
}

//加载对应的用户配置
void Config::LoadUserConfig()
{
    m_userConfigMap.clear();
    m_userChangedMap.clear();
    QString jsonFileFath = configJsonFilePath();
    LoadMapInfoFromJson(jsonFileFath,m_userConfigMap);
}

//解析JSon数据到对应的Map结构
bool Config::LoadMapInfoFromJson(QString filePath, QMap<QString,ConfigItemStruct>& infoMap)
{

    QFile file(filePath);
    if(!file.exists())
    {
        return false;
    }
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream inputStream(&file);
    inputStream.setCodec("UTF-8");
    QString value = inputStream.readAll();
    file.close();
    QJsonParseError parseJsonErr;
    QJsonDocument document = QJsonDocument::fromJson(value.toUtf8(),&parseJsonErr);
    if(!(parseJsonErr.error == QJsonParseError::NoError))
    {
        return false;
    }
    QJsonObject jsonObject = document.object();
    infoMap.clear();

    QStringList keyList = jsonObject.keys();
    for(int index=0; index<keyList.size(); ++index)
    {
        QJsonObject childObject = jsonObject.value(keyList.at(index)).toObject();
        QString  currentKey = keyList.at(index);
        ConfigItemStruct currentConfigItem;
        currentConfigItem.itemName = currentKey;
        currentConfigItem.itemGroupName = childObject.value("group").toString();
        currentConfigItem.itemType = childObject.value("type").toString();
        if(currentConfigItem.itemType == "int")
        {
            currentConfigItem.itemDefaultIntValue = childObject.value("defaultvalue").toInt();
            currentConfigItem.itemCurrentIntValue = childObject.value("currentvalue").toInt();
        }
        else if(currentConfigItem.itemType == "string")
        {
            currentConfigItem.itemDefaultStrValue = childObject.value("defaultvalue").toString();
            currentConfigItem.itemCurrentStrValue = childObject.value("currentvalue").toString();
        }

        infoMap[currentKey] = currentConfigItem;
    }
    return true;
}

bool Config::check_bracket_item_conflict()
{
    if((m_currentConfigMap["key_pair_candidate_page"].itemCurrentIntValue & KEY_PAIR_2) &&
            (m_currentConfigMap["key_pair_zi_from_word"].itemCurrentIntValue & KEY_PAIR_2))
    {
        Utils::NoticeMsgBoxWithoutAbbre(QString::fromLocal8Bit("“候选翻页”和“以词定字”\n定义冲突\n请重新设置！"),false);
        return true;
    }
    return false;
}

bool Config::check_shift_ctrl_conflict()
{
    if((m_currentConfigMap["key_change_mode"].itemCurrentIntValue == KEY_SWITCH_SHIFT
                && m_currentConfigMap["key_candidate_2nd_3rd"].itemCurrentIntValue == KEY_2ND_3RD_SHIFT)
               ||(m_currentConfigMap["key_change_mode"].itemCurrentIntValue == KEY_SWITCH_CONTROL
                  &&  m_currentConfigMap["key_candidate_2nd_3rd"].itemCurrentIntValue == KEY_2ND_3RD_CONTROL))
       {
           Utils::NoticeMsgBoxWithoutAbbre(QString::fromLocal8Bit("“中英文切换键”和“二三候选键”\n定义冲突\n请重新设置！"),false);
           return true;
       }
    return false;
}

void Config::ClearUserConfig()
{
    m_userConfigMap.clear();
}

QString Config::GetHostMacAddress()
{
    QList<QNetworkInterface> nets = QNetworkInterface::allInterfaces();// 获取所有网络接口列表
    int nCnt = nets.count();
    QString strMacAddr = "";
    for(int i = 0; i < nCnt; i ++)
    {
        // 如果此网络接口被激活并且正在运行并且不是回环地址，则就是我们需要找的Mac地址
        if(nets[i].flags().testFlag(QNetworkInterface::IsUp) && nets[i].flags().testFlag(QNetworkInterface::IsRunning)
                && !nets[i].flags().testFlag(QNetworkInterface::IsLoopBack) &&  nets[i].hardwareAddress() != "00:50:56:C0:00:01" && nets[i].hardwareAddress() != "00:50:56:C0:00:08" )
        {
            strMacAddr = nets[i].hardwareAddress();
            break;
        }
    }

    strMacAddr = strMacAddr.toLower();

    //计算mc地址的md5值
    QString mcMd5;
    QByteArray ba,bb;
    QCryptographicHash md5(QCryptographicHash::Md5);
    ba.append(strMacAddr);
    md5.addData(ba);
    bb = md5.result();
    mcMd5.append(bb.toHex());

    return mcMd5;
}


bool Config::GetDefualtConfigItem(QString key, ConfigItemStruct& keyValue)
{
    if(!m_systemConfigMap.contains(key))
    {
        return false;
    }
    keyValue = m_systemConfigMap[key];
    return true;
}

bool Config::IsToolBarChanged()
{
    if(m_userChangedMap.contains("tool_bar_config") ||
            m_userChangedMap.contains("using_english_punctuation_in_chinese_mode") ||
            m_userChangedMap.contains("toolbar_visible") ||
            m_userChangedMap.contains("using_history_shortcut"))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void Config::LoadStateConfig()
{
    m_stateConfigMap.clear();
    QString state_config_file_path = Utils::GetConfigDirPath() + QString("config.json");
    LoadMapInfoFromJson(state_config_file_path,m_stateConfigMap);
    QList<QString> key_list = m_stateConfigMap.keys();
    for(int index=0; index<key_list.size(); ++index)
    {
        QString current_key = key_list.at(index);
        ConfigItemStruct item_struct = m_stateConfigMap.value(current_key);
        if(item_struct.itemGroupName != "state")
        {
            m_stateConfigMap.remove(current_key);
        }
    }

    has_domainChanged = false;
}

bool Config::CheckValidation(QString &message)
{
    return true;
}

QString Config::GetOSVersion()

{
    QSettings regSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",QSettings::NativeFormat);
    QStringList keyList = regSettings.childKeys();
    QString osVersion = regSettings.value("ProductName","unknown osversion").toString();
    return osVersion;

}

QString Config::GetCPUType()
{
    QSettings* cpu = new QSettings("HKEY_LOCAL_MACHINE\\HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", QSettings::NativeFormat);
    QString cpuDescribe = cpu->value("ProcessorNameString").toString();
    delete cpu;
    return cpuDescribe;
}
