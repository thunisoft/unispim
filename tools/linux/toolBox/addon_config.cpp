#include "addon_config.h"

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

AddonConfig* AddonConfig::m_instance = NULL;
std::mutex AddonConfig::m_Mutex;


QVector<TOOL_ADDON_INFO> AddonConfig::GetSystemAddonVector()
{
    return m_system_addon_info_vector;
}

QVector<TOOL_ADDON_INFO> AddonConfig::GetUsermAddonVector()
{
    return m_user_addon_info_vector;
}

QVector<TOOL_ADDON_INFO> AddonConfig::GetAllAddonVector()
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

QVector<TOOL_ADDON_INFO> AddonConfig::GetAvailableAddonVector()
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


void AddonConfig::UpdateToolStatus(QString addon_name, QString status)
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

void AddonConfig::AddNewAddonInfo(TOOL_ADDON_INDEX_INFO addon_index)
{
    QString dir_path = get_user_addon_dir_path();
    QDir user_dir(dir_path);
    if(!user_dir.exists())
    {
        user_dir.mkpath(dir_path);
    }
    addon_index.addon_index = GetAllAddonVector().size() + 1;
    QString user_addon_list_path = get_user_addon_dir_path() + "addon-list.json";
    QFile user_addon_file(user_addon_list_path);

    QJsonObject index_object;
    index_object.insert("addon-index",addon_index.addon_index);
    index_object.insert("addon-name",addon_index.addon_name);
    index_object.insert("addon-version",addon_index.addon_version);
    index_object.insert("addon-id",addon_index.addo_id);
    index_object.insert("addon-status",addon_index.addon_status);
    QJsonArray array_info;
    if(!user_addon_file.exists())
    {
        array_info.append(index_object);
    }
    else
    {
        user_addon_file.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream inputStream(&user_addon_file);
        inputStream.setCodec("UTF-8");
        QString value = inputStream.readAll();
        user_addon_file.close();

        //查找插件的索引信息
        QJsonParseError parseJsonErr;
        QJsonDocument document = QJsonDocument::fromJson(value.toUtf8(),&parseJsonErr);
        if(!(parseJsonErr.error == QJsonParseError::NoError))
        {
            return;
        }
        array_info = document.array();
        array_info.append(index_object);
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


AddonConfig* AddonConfig::Instance()
{
    if(m_instance == NULL)
    {
        std::unique_lock<std::mutex> lock(m_Mutex);
        if(m_instance == NULL)
        {
            m_instance = new AddonConfig();
        }
    }
    return m_instance;
}


QString AddonConfig::get_user_addon_dir_path()
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


AddonConfig::AddonConfig()
{
    LoadSystemAddon();
    LoadUserAddon();
}

AddonConfig::~AddonConfig()
{

}

bool AddonConfig::IsAddonExist(QString addon_name)
{
   LoadUserAddon();
   QVector<TOOL_ADDON_INFO> addon_vector = GetAvailableAddonVector();
    int count = addon_vector.size();
    for(int index=0; index<count; ++index)
    {
        TOOL_ADDON_INFO current_addon_info = addon_vector.at(index);
        if(current_addon_info.addon_name == addon_name)
        {
            return true;
        }
    }
    return false;
}

bool AddonConfig::GetToolAddonInfoByName(QString addon_name, TOOL_ADDON_INFO &index_addon_info)
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


TOOL_ADDON_INFO AddonConfig::GetToolAddonInfo(QString file_path)
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


bool AddonConfig::LoadAddonInfoFromFile(QString file_path, QVector<TOOL_ADDON_INFO> &output_info_vector)
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
            current_addon_info.addon_status = addon_status;
            current_addon_info.addon_index = index_info.addon_index;
            output_info_vector.push_back(current_addon_info);
        }
    }
    return true;
}


void AddonConfig::SaveUserAddonInfo()
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


void AddonConfig::LoadSystemAddon()
{
    //查看系统的插件列表
    QString jsonFileFath  = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/addon/addon-list.json");
    LoadAddonInfoFromFile(jsonFileFath,m_system_addon_info_vector);
}

void AddonConfig::LoadUserAddon()
{
    //查看系统的插件列表
    QString  jsonFileFath = get_user_addon_dir_path() + "addon-list.json";
    LoadAddonInfoFromFile(jsonFileFath,m_user_addon_info_vector);
}

