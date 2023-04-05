#include "config.h"
#include "utils.h"

#include <QDir>
#include <QSettings>
#include <QVector>
#include <QString>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QApplication>
#include <QList>
#include <QNetworkInterface>
#include <QCryptographicHash>
#include <QFileInfoList>
#include <QMessageBox>
#include <QRegExp>
#include <QStandardPaths>

#include <mutex>
#include <memory>
#include <stdlib.h>
#include <stdio.h>

Config* Config::m_instance = NULL;
std::mutex Config::m_Mutex;

QString Config::GetHttpsCertificatePath()
{
    QString filePath = QString(":/certification_server.cer");
    return filePath;
}

QVector<TOOL_ADDON_INFO> Config::GetSystemAddonVector()
{
    return m_system_addon_info_vector;
}

QVector<TOOL_ADDON_INFO> Config::GetUsermAddonVector()
{
    return m_user_addon_info_vector;
}

QVector<TOOL_ADDON_INFO> Config::GetAllAddonVector()
{
    QVector<TOOL_ADDON_INFO> all_vector;
    auto has_target_info = [&](TOOL_ADDON_INFO input_tool_info)->bool{
        for(TOOL_ADDON_INFO index_info : all_vector)
        {
            if(index_info.addon_name == input_tool_info.addon_name)
            {
                return true;
            }
        }
        return false;
    };

    auto find_item_index = [&](TOOL_ADDON_INFO input_tool_info)->int{
        int item_count = all_vector.size();
        for(int index=0; index<item_count; ++index)
        {
            TOOL_ADDON_INFO index_item = all_vector.at(index);
            if((index_item.addon_name == input_tool_info.addon_name))
            {
                return index;
            }
        }
        return -1;
    };

    for(TOOL_ADDON_INFO index_info : m_system_addon_info_vector)
    {
        if(!has_target_info(index_info))
        {
            all_vector.push_back(index_info);
        }
    }
    for(TOOL_ADDON_INFO index_info : m_user_addon_info_vector)
    {
        if(has_target_info(index_info))
        {
            int index = find_item_index(index_info);
            if(index != -1)
            {
                all_vector.remove(index);
                all_vector.insert(index,index_info);
            }
        }
        else
        {
            all_vector.push_back(index_info);
        }
    }
    return all_vector;
}

QVector<TOOL_ADDON_INFO> Config::GetAvailableAddonVector()
{
    QVector<TOOL_ADDON_INFO>  tool_addon_vector = GetAllAddonVector();
    QVector<TOOL_ADDON_INFO> result_vector;
    int addon_count = tool_addon_vector.size();
    for(int index=0; index<addon_count; ++index)
    {
        TOOL_ADDON_INFO index_addon_info = tool_addon_vector.at(index);
        if(index_addon_info.addon_status != QString("deprecated"))
        {
            result_vector.push_back(index_addon_info);
        }
    }
    return result_vector;
}

QMap<QString, QString> Config::GetHotkeyMap()
{
    return m_addon_shortcut_map;
}

QString Config::GetLinuxServerDomain()
{
#ifndef _WIN32
    QString server_addr= QString("pinyin.thunisoft.com");
    QString port = "443";
    QString schema = QString("https");
    QString server_domain = QString("%1://%2:%3").arg(schema, server_addr, port);

    QString sys_config_dir(getenv("XDG_CONFIG_HOME"));
    if(sys_config_dir.isEmpty())
        sys_config_dir = QDir::homePath() + "/.config/";
    if(!sys_config_dir.endsWith('/'))
        sys_config_dir.append('/');
    sys_config_dir.append("fcitx-huayupy/");

    QString default_config_path = sys_config_dir.append("server.ini");
    if(QFile::exists(default_config_path))
    {
        QSettings default_conf(default_config_path, QSettings::IniFormat);
        default_conf.beginGroup("server");
        schema = default_conf.value("scheme", "").toString();
        port = default_conf.value("port", "").toString();
        server_addr = default_conf.value("addr", "").toString();
        default_conf.endGroup();
        if(!schema.isEmpty() && !port.isEmpty() && !server_addr.isEmpty())
        {
            server_domain = QString("%1://%2:%3").arg(schema, server_addr, port);
        }
    }
    return server_domain;
#endif
    return QString("");
}

void Config::LoadConfig()
{
    //依次加载系统配置/状态配置/用户配置
    LoadSystemConfig();
    LoadStateConfig();
    LoadUserConfig();
    MergeConfigInfo();
    LoadHotkeyInfo();
    LoadSystemAddon();
    LoadUserAddon();
}

void Config::SaveConfig()
{
    SaveStateConfig();
    SaveUserConfig();
}


void Config::SaveUserConfig()
{
    QString userConfigFilePath = configJsonFilePath();
    SaveConfigInfoToJson(userConfigFilePath,m_userChangedMap);
}

void Config::SaveStateConfig()
{
    QString state_config_path = get_state_config_dir_path() + QString("config.json");
    SaveConfigInfoToJson(state_config_path,m_stateConfigMap);
}



void Config::ReloadConfig()
{
    LoadConfig();
}

void Config::SaveHotkeyInfo()
{
    QString usershortcut_filepath = get_state_config_dir_path() + "keybindings.json";

    QList<QString> addon_list = m_addon_shortcut_map.keys();
    QJsonArray jsonArray;
    for(QString index_addon : addon_list)
    {
        QString key_value = m_addon_shortcut_map.value(index_addon);
        QJsonObject index_object;
        index_object.insert("key",key_value);
        index_object.insert("addon-name",index_addon);
        jsonArray.append(index_object);
    }

    QJsonDocument jsonDocument;
    jsonDocument.setArray(jsonArray);
    QByteArray jsonContent = jsonDocument.toJson();
    QFile output_file(usershortcut_filepath);
    output_file.open(QIODevice::WriteOnly|QIODevice::Text);
    QTextStream output(&output_file);
    output.setCodec("UTF-8");
    output << jsonContent;
    output_file.close();

}

void Config::ChangeHotkey(QString addon_name, QString shortcut)
{
    if(addon_name.isEmpty() || shortcut.isEmpty())
    {
        return;
    }

    if(m_addon_shortcut_map.contains(addon_name))
    {
        m_addon_shortcut_map[addon_name] = shortcut;
    }
    else
    {
        m_addon_shortcut_map.insert(addon_name,shortcut);
    }
}

void Config::UpdateToolStatus(QString addon_name, QString status)
{
    //处理用户插件
    bool is_user_addon_changed = false;
    int user_addon_count = m_user_addon_info_vector.size();
    for(int index=0; index<user_addon_count; ++index)
    {
        TOOL_ADDON_INFO index_info = m_user_addon_info_vector.at(index);
        if(index_info.addon_name == addon_name)
        {
            index_info.addon_status = status;
            m_user_addon_info_vector[index] = index_info;
            is_user_addon_changed = true;
        }
    }
    if(is_user_addon_changed)
    {
        SaveUserAddonInfo();
        return;
    }

    //处理系统插件
    int system_addon_count = m_system_addon_info_vector.size();
    for(int index=0; index<system_addon_count; ++index)
    {
        TOOL_ADDON_INFO index_info = m_system_addon_info_vector.at(index);
        if(index_info.addon_name == addon_name)
        {
            index_info.addon_status = status;
            m_system_addon_info_vector[index] = index_info;
            m_user_addon_info_vector.push_back(index_info);
            SaveUserAddonInfo();
        }
    }
}

void Config::AddNewAddonInfo(TOOL_ADDON_INDEX_INFO addon_index)
{
    QString dir_path = get_user_addon_dir_path();
    QDir user_dir(dir_path);
    if(!user_dir.exists())
    {
        user_dir.mkpath(dir_path);
    }
    QVector<TOOL_ADDON_INFO> addon_vector = GetAllAddonVector();
    int maxIndex = 0;
    for(TOOL_ADDON_INFO indexInfo : addon_vector)
    {
        if(indexInfo.addon_index > maxIndex)
        {
            maxIndex = indexInfo.addon_index;
        }
    }
    addon_index.addon_index = maxIndex + 10;
    //固定OCR和语音插件的位置
    if(addon_index.addon_name == "OCR")
    {
        addon_index.addon_index = 3;
    }
    if(addon_index.addon_name == "AsrInput")
    {
        addon_index.addon_index = 4;
    }
    //插件存在的话直接修改版本号就行
    bool is_addon_exist = IsAddonExistByName(addon_index.addon_name);
    if(is_addon_exist)
    {
        int user_addon_count = m_user_addon_info_vector.count();
        for(int index = 0; index<user_addon_count; ++index)
        {
            if(m_user_addon_info_vector.at(index).addon_name == addon_index.addon_name)
            {
                m_user_addon_info_vector[index].addon_version = addon_index.addon_version;
                m_user_addon_info_vector[index].addon_status = addon_index.addon_status;
                break;
            }
        }

    }
    else
    {
        TOOL_ADDON_INFO new_addon_info;
        new_addon_info.addon_index = addon_index.addon_index;
        new_addon_info.addon_name = addon_index.addon_name;
        new_addon_info.addon_version = addon_index.addon_version;
        new_addon_info.addon_id = addon_index.addo_id;
        new_addon_info.addon_status = addon_index.addon_status;
        m_user_addon_info_vector.push_back(new_addon_info);
    }
    SaveUserAddonInfo();
}

void Config::ReloadUserAddon()
{
    LoadUserAddon();
}

void Config::RemoveUserAddon(QString addonName)
{
    bool exist = IsAddonExistByName(addonName);
    if(exist)
    {
        int addonIndex = -1;
        for(int index = 0; index<m_user_addon_info_vector.size(); ++index)
        {
            if(m_user_addon_info_vector.at(index).addon_name == addonName)
            {
                addonIndex = index;
                break;
            }
        }
        if(addonIndex != -1)
        {
            m_user_addon_info_vector.removeAt(addonIndex);
            SaveUserAddonInfo();
        }
    }
}

Config* Config::Instance()
{
    if(m_instance == NULL)
    {
        std::unique_lock<std::mutex> lock(m_Mutex);
        if(m_instance == NULL)
        {
            m_instance = new Config();
        }
    }
    return m_instance;
}

QString Config::get_user_config_dir_path()
{
#ifdef _WIN32
    QStringList sls = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if (sls.isEmpty())
    {
        return "";
    }

    QString result = sls.first();
    result.append("/AppData/Local/Thunisoft/HuayuPY/");
    return result;
#else
    QStringList sls = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if (sls.isEmpty())
    {
        return "";
    }
    QString result = sls.first();
    result.append("/.config/fcitx-huayupy/");
    return result;
#endif
}

QString Config::get_state_config_dir_path()
{
    QStringList sls = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if (sls.isEmpty())
    {
        return "";
    }
    QString result = sls.first();
    result.append("/AppData/Local/Thunisoft/HuayuPY/");
    return result;
}

QString Config::get_user_addon_dir_path()
{
#ifdef _WIN32
    QStringList sls = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if (sls.isEmpty())
    {
        return "";
    }
    QString result = sls.first();
    result.append("/AppData/Local/Thunisoft/HuayuPY/addon/");
    return result;
#else
       QStringList sls = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
       if (sls.isEmpty())
       {
           return "";
       }
       QString result = sls.first();
       result.append("/.config/fcitx-huayupy/addon/");
       return result;
#endif
}

QString Config::get_install_bin_path()
{
#ifdef _WIN32
    QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Thunisoft\\Huayupy\\7.0",QSettings::NativeFormat);
    QString installPath = QDir::toNativeSeparators(settings.value("InstallDir","").toString().append("/config-tools/"));
    return installPath;
#else
    #ifdef USE_FCITX
        return "/opt/apps/huayupy/files/bin/";
    #else /*uos*/
        return "/opt/apps/com.thunisoft.input/files/bin/";
    #endif
#endif
}


Config::Config()
{
    LoadConfig();
}

Config::~Config()
{

}


QString Config::configJsonFilePath()
{
    if(m_stateConfigMap.contains("login")&&
            m_stateConfigMap.contains("loginid"))
    {
        if(m_stateConfigMap["login"].itemCurrentIntValue == 0)
        {
            return get_state_config_dir_path() + QString("config.json");
        }
        else
        {
            QString configPathDir = get_state_config_dir_path() +
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
        return get_state_config_dir_path() + QString("config.json");
    }
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
            Utils::write_log_tofile(QString("配置项%1的设置类型冲突").arg(key));
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
            Utils::write_log_tofile(configInfo);
        }
    }
    return 0;
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
        if(currentItem.itemGroupName != "state")
            continue;
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

bool Config::IsAddonExist(QString guid)
{
    int count = m_user_addon_info_vector.size();
    for(int index=0; index<count; ++index)
    {
        TOOL_ADDON_INFO current_addon_info = m_user_addon_info_vector.at(index);
        if(current_addon_info.addon_id == guid)
        {
            return true;
        }
    }
    return false;
}

bool Config::IsAddonExistByName(QString addon_name)
{
    int count = m_user_addon_info_vector.size();
    for(int index=0; index<count; ++index)
    {
        TOOL_ADDON_INFO current_addon_info = m_user_addon_info_vector.at(index);
        if(current_addon_info.addon_name == addon_name)
        {
            return true;
        }
    }
    return false;
}


TOOL_ADDON_INFO Config::GetToolAddonInfo(QString file_path)
{
    //查看系统的插件列表
    TOOL_ADDON_INFO current_addon_info;
    QString jsonFileFath  = file_path;
    QFile file(jsonFileFath);
    if(!file.exists())
    {
        return current_addon_info;
    }
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QFileInfo fileInfo(file_path);
    QString dir_path = fileInfo.dir().path() + "/";

    QTextStream inputStream(&file);
    inputStream.setCodec("UTF-8");
    QString value = inputStream.readAll();
    file.close();

    QJsonParseError parseJsonErr;
    QJsonDocument document = QJsonDocument::fromJson(value.toUtf8(),&parseJsonErr);
    if(!(parseJsonErr.error == QJsonParseError::NoError))
    {
        return current_addon_info;
    }

    QJsonObject childObject = document.object();
    QString  current_guid = childObject.value("id").toString();
    current_addon_info.addon_id = current_guid;
    current_addon_info.addon_displayname = childObject.value("displayName").toString();
    current_addon_info.addon_describe = childObject.value("description").toString();
    current_addon_info.addon_name = childObject.value("addonname").toString();
    current_addon_info.addon_version = childObject.value("version").toString();
    current_addon_info.author = childObject.value("author").toString();
    current_addon_info.call_paramer = childObject.value("paramer").toString();
    current_addon_info.icon_path = QDir::toNativeSeparators(dir_path + childObject.value("icon").toString());
    current_addon_info.exe_path = QDir::toNativeSeparators(dir_path + childObject.value("exepath").toString());
    return current_addon_info;
}

bool Config::LoadHotkeyinfoFromFile(QString file_path)
{
    //查看系统的插件列表
    QString jsonFileFath  = file_path;
    QFile file(jsonFileFath);
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

    QJsonArray document_array = document.array();
    int config_count = document_array.size();

    for(int index=0; index<config_count; ++index)
    {
        QJsonObject indexObject = document_array.at(index).toObject();
        QString key = indexObject.value("key").toString();
        QString addon_name = indexObject.value("addon-name").toString();
        m_addon_shortcut_map.insert(addon_name,key);
    }
    return true;
}

bool Config::LoadAddonInfoFromFile(QString file_path, QVector<TOOL_ADDON_INFO> &output_info_vector)
{
    QString jsonFileFath  = file_path;
    QFile file(jsonFileFath);
    if(!file.exists())
    {
        return false;
    }
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream inputStream(&file);
    inputStream.setCodec("UTF-8");
    QString value = inputStream.readAll();
    file.close();

    //查找插件的索引信息
    output_info_vector.clear();
    QJsonParseError parseJsonErr;
    QJsonDocument document = QJsonDocument::fromJson(value.toUtf8(),&parseJsonErr);
    if(parseJsonErr.error != QJsonParseError::NoError)
    {
        return false;
    }

    QJsonArray document_array = document.array();
    int addon_count = document_array.size();
    QVector<TOOL_ADDON_INDEX_INFO> index_info_vector;
    for(int index=0; index<addon_count; ++index)
    {
        QJsonObject indexObject = document_array.at(index).toObject();
        TOOL_ADDON_INDEX_INFO current_index_info;
        current_index_info.addon_index = indexObject.value("addon-index").toInt();
        current_index_info.addon_name = indexObject.value("addon-name").toString();
        current_index_info.addon_version = indexObject.value("addon-version").toString();
        current_index_info.addo_id = indexObject.value("addon-id").toString();
        current_index_info.addon_status = indexObject.value("addon-status").toString();
        index_info_vector.push_back(current_index_info);
    }
    //按照显示索引进行排序
    auto sortbyindex = [&](TOOL_ADDON_INDEX_INFO start,TOOL_ADDON_INDEX_INFO end)
    {
        return start.addon_index < end.addon_index;
    };
    std::sort(index_info_vector.begin(),index_info_vector.end(),sortbyindex);

    QFileInfo file_info(file_path);
    QString sud_addon_dir = file_info.dir().absolutePath();
    //加载对应的插件信息
    for(TOOL_ADDON_INDEX_INFO index_info : index_info_vector)
    {
        QString addon_name = index_info.addon_name;
        QString addon_version = index_info.addon_version;
        QString addon_status = index_info.addon_status;

        QDir user_path_dir(sud_addon_dir);
        user_path_dir.setFilter(QDir::Filter::Dirs);
        QFileInfoList file_info_list = user_path_dir.entryInfoList();
        QString sub_addon_dir_path = QString("%1-%2").arg(addon_name).arg(addon_version);

        //插件的目录名称需要以addonname-version开头
        for(QFileInfo index_file_info : file_info_list)
        {
            if(!index_file_info.fileName().isEmpty()
                    && !sub_addon_dir_path.isEmpty()
                    && index_file_info.fileName().startsWith(sub_addon_dir_path))
            {
                sub_addon_dir_path = index_file_info.fileName();
                break;
            }
        }
        QString addon_desc_filepath = QDir::toNativeSeparators(sud_addon_dir + QString("/%2/addon-desc.json").arg(sub_addon_dir_path));
        if(QFile::exists(addon_desc_filepath))
        {
            TOOL_ADDON_INFO current_addon_info = GetToolAddonInfo(addon_desc_filepath);
            if(current_addon_info.addon_name.isEmpty())
            {
                continue;
            }

            //防止快捷键冲突
            if(m_addon_shortcut_map.contains(current_addon_info.addon_name))            
            {
                current_addon_info.short_cut = m_addon_shortcut_map.value(current_addon_info.addon_name);
            }
            if(IsAddonExistByName(current_addon_info.addon_name))
            {
                continue;
            }
            current_addon_info.addon_status = addon_status;
            current_addon_info.addon_index = index_info.addon_index;
            output_info_vector.push_back(current_addon_info);
        }
    }
    return true;
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

void Config::SaveUserAddonInfo()
{
    QString dir_path = get_user_addon_dir_path();
    QDir user_dir(dir_path);
    if(!user_dir.exists())
    {
        user_dir.mkpath(dir_path);
    }
    QString user_addon_list_path = get_user_addon_dir_path() + "addon-list.json";

    QJsonArray array_info;
    int user_addon_count = m_user_addon_info_vector.size();
    for(int index=0; index<user_addon_count; ++index)
    {
        TOOL_ADDON_INFO index_addon_info = m_user_addon_info_vector.at(index);
        QJsonObject indexObjet;
        indexObjet.insert("addon-index",index_addon_info.addon_index);
        indexObjet.insert("addon-name",index_addon_info.addon_name);
        indexObjet.insert("addon-version",index_addon_info.addon_version);
        indexObjet.insert("addon-id",index_addon_info.addon_id);
        indexObjet.insert("addon-status",index_addon_info.addon_status);
        array_info.append(indexObjet);
    }

    QJsonDocument jsonDocument;
    jsonDocument.setArray(array_info);
    QByteArray jsonContent = jsonDocument.toJson();
    QFile output_file(user_addon_list_path);
    output_file.open(QIODevice::WriteOnly|QIODevice::Text);
    QTextStream output(&output_file);
    output.setCodec("UTF-8");
    output << jsonContent;
    output_file.close();
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

bool Confi(QString system_addon_dir)
{

    return true;
}

void Config::LoadSystemAddon()
{
    //查看系统的插件列表
    QString jsonFileFath  = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/addon/addon-list.json");
    LoadAddonInfoFromFile(jsonFileFath,m_system_addon_info_vector);
}

void Config::LoadUserAddon()
{
    //查看系统的插件列表
    QString  jsonFileFath = get_user_addon_dir_path() + "addon-list.json";
    LoadAddonInfoFromFile(jsonFileFath,m_user_addon_info_vector);
}

void Config::LoadHotkeyInfo()
{
    //查看系统的插件列表
    QString systemshort_filepath  = QApplication::applicationDirPath() + "\\keybindings.json";
    QString usershortcut_filepath = get_state_config_dir_path() + "keybindings.json";
    LoadHotkeyinfoFromFile(systemshort_filepath);
    LoadHotkeyinfoFromFile(usershortcut_filepath);
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


void Config::LoadStateConfig()
{
    m_stateConfigMap.clear();
    QString state_config_file_path = get_user_config_dir_path() + QString("config.json");
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

}

bool Config::GetToolAddonInfoByName(QString addon_name, TOOL_ADDON_INFO &index_addon_info)
{
    QVector<TOOL_ADDON_INFO> addon_vector = GetAvailableAddonVector();
     int count = addon_vector.size();
     for(int index=0; index<count; ++index)
     {
         TOOL_ADDON_INFO current_addon_info = addon_vector.at(index);
         if(current_addon_info.addon_name == addon_name)
         {
             index_addon_info = current_addon_info;
             return true;
         }
     }
     return false;
}
