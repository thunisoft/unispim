#include "toolupdatetask.h"
#include "networkhandler.h"
#include "utils.h"
#include "config.h"
#include <QJsonArray>
#include <QDir>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFileInfo>

void ToolUpdateTask::run()
{
    QString clientid = Config::GetHostMacAddress();
    QString loginid;
    ConfigItemStruct loginitem;
    if(Config::Instance()->GetConfigItemByJson("loginid",loginitem))
    {
        loginid = loginitem.itemCurrentStrValue;
    }
    QString os_type = Utils::get_os_type();
    QString cpu_type = Utils::get_cpu_type();

    QJsonArray jsonArray;
    QVector<TOOL_ADDON_INFO> all_tool_vector;
    if(m_task_type == ManuUpdate)
    {
        all_tool_vector.push_back(m_single_addon_info);
    }
    else
    {
        all_tool_vector = Config::Instance()->GetAllAddonVector();
    }
    for(TOOL_ADDON_INFO index : all_tool_vector)
    {
        QJsonObject addon_object;
        addon_object.insert("addonname",index.addon_name);
        addon_object.insert("version",index.addon_version);
        addon_object.insert("id",index.addon_id);
        jsonArray.append(addon_object);
        FilterCachedOldVersionAddon(index);
    }

    QJsonDocument jsonDocument;
    jsonDocument.setArray(jsonArray);
    QString jsonContent = jsonDocument.toJson();
    jsonContent.replace("\n","");
    QString addon_list_value = jsonContent.trimmed();
    QVector<SEVER_ADDON_INFO> server_addon_vector;

    NetworkHandler handler;
    int ret = handler.task_check_tools_status(clientid,loginid,os_type,cpu_type,addon_list_value,server_addon_vector);
    if((ret == 0) && (server_addon_vector.size() > 0))
    {
        bool addon_has_update = false;
        for(SEVER_ADDON_INFO index_info : server_addon_vector)
        {
            if(m_task_type == ManuUpdate)
            {
                if(index_info.addon_name == m_single_addon_info.addon_name)
                {
                    addon_has_update = true;
                }
                HandleManuTask(index_info);
            }
            else
            {
                HandleAutoTask(index_info);
            }
        }
        if(!addon_has_update)
        {
            emit has_no_update();
        }
    }
    else if((ret == 0) && (server_addon_vector.size() == 0))
    {
        emit has_no_update();
    }
    else
    {
        emit check_failed();
    }
}

void ToolUpdateTask::handle_inform_update_tool(const SEVER_ADDON_INFO &addon_info)
{
    Config::Instance()->UpdateToolStatus(addon_info.addon_name,addon_info.addon_status);
    emit update_tool_info();
}

void ToolUpdateTask::handle_force_update_tool(const SEVER_ADDON_INFO &addon_info)
{
    QString dir_path = Config::Instance()->get_user_addon_dir_path();
    QDir dir(dir_path);
    if(!dir.exists())
    {
        dir.mkpath(dir_path);
    }
    QString file_savepath = dir_path + addon_info.file_name;
    if(QFile::exists(file_savepath))
    {
        QFile::remove(file_savepath);
    }
    NetworkHandler handler;
    connect(&handler,SIGNAL(download_progress(qint64,qint64)),this,SLOT(slot_handle_download_progress(qint64,qint64)));
    int ret = handler.download_tool_from_url(addon_info.addon_download_url,file_savepath);
    if(ret == 0)
    {
        QFileInfo file_info(file_savepath);
        QString target_dir = QDir::toNativeSeparators(file_info.dir().absolutePath());
        if(QFile::exists(file_savepath))
        {
            Utils::depressed_zlib_file(file_savepath,target_dir);
            QFile::remove(file_savepath);
            TOOL_ADDON_INDEX_INFO index_info;
            index_info.addon_name = addon_info.addon_name;
            index_info.addon_name = addon_info.addon_name;
            index_info.addon_version = addon_info.addon_version;
            index_info.addon_status = "";
            Config::Instance()->AddNewAddonInfo(index_info);
            Config::Instance()->ReloadUserAddon();
#ifndef _WIN32
            TOOL_ADDON_INFO addon_info;
            Config::Instance()->GetToolAddonInfoByName(index_info.addon_name,addon_info);
            Utils::excute_command_line(QString("chmod 777 %1").arg(addon_info.exe_path));
            if(addon_info.addon_name == "OCR")
            {
                QFileInfo file_info(addon_info.exe_path);
                QString file_dir = file_info.absoluteDir().absolutePath();
                QString x86_file = file_dir + QString("/3rd/HYCapture.x86");
                QString arm_file = file_dir + QString("/3rd/HYCapture.aarch");
                QString mips_file = file_dir + QString("/3rd/HYCapture.mips64el");
                Utils::excute_command_line(QString("chmod 777 %1").arg(x86_file));
                Utils::excute_command_line(QString("chmod 777 %1").arg(arm_file));
                Utils::excute_command_line(QString("chmod 777 %1").arg(mips_file));
            }
#endif
            emit update_tool_info();
        }
    }

}

void ToolUpdateTask::handle_deprecated_tool(const SEVER_ADDON_INFO &addon_info)
{
    Config::Instance()->RemoveUserAddon(addon_info.addon_name);
    RemoveAddonFile(addon_info);
    emit update_tool_info();
}

void ToolUpdateTask::handle_publish_tool(const SEVER_ADDON_INFO &addon_info)
{
    QString dir_path = Config::Instance()->get_user_addon_dir_path();
    QDir dir(dir_path);
    if(!dir.exists())
    {
        dir.mkpath(dir_path);
    }
    QString file_savepath = dir_path + addon_info.file_name;
    if(QFile::exists(file_savepath))
    {
        QFile::remove(file_savepath);
    }
    NetworkHandler handler;
    int ret = handler.download_tool_from_url(addon_info.addon_download_url,file_savepath);
    if(ret == 0)
    {
        QFileInfo file_info(file_savepath);
        QString target_dir = QDir::toNativeSeparators(file_info.dir().absolutePath());
        if(QFile::exists(file_savepath))
        {
            Utils::depressed_zlib_file(file_savepath,target_dir);
            QFile::remove(file_savepath);
            TOOL_ADDON_INDEX_INFO index_info;
            index_info.addon_name = addon_info.addon_name;
            index_info.addon_name = addon_info.addon_name;
            index_info.addon_version = addon_info.addon_version;
            Config::Instance()->AddNewAddonInfo(index_info);
            Config::Instance()->ReloadUserAddon();
#ifndef _WIN32
            TOOL_ADDON_INFO addon_info;
            Config::Instance()->GetToolAddonInfoByName(index_info.addon_name,addon_info);
            Utils::excute_command_line(QString("chmod 777 %1").arg(addon_info.exe_path));
            if(addon_info.addon_name == "OCR")
            {
                QFileInfo file_info(addon_info.exe_path);
                QString file_dir = file_info.absoluteDir().absolutePath();
                QString x86_file = file_dir + QString("/3rd/HYCapture.x86");
                QString arm_file = file_dir + QString("/3rd/HYCapture.aarch");
                QString mips_file = file_dir + QString("/3rd/HYCapture.mips64el");
                Utils::excute_command_line(QString("chmod 777 %1").arg(x86_file));
                Utils::excute_command_line(QString("chmod 777 %1").arg(arm_file));
                Utils::excute_command_line(QString("chmod 777 %1").arg(mips_file));
            }
#endif
            emit update_tool_info();
        }
    }
}

void ToolUpdateTask::set_update_type(UPDATE_TYPE input_update_type)
{
    m_task_type = input_update_type;
}

void ToolUpdateTask::set_tool_addon_info(TOOL_ADDON_INFO input_addon_info)
{
     m_single_addon_info = input_addon_info;
}

void ToolUpdateTask::HandleManuTask(SEVER_ADDON_INFO server_addon_info)
{
    if(server_addon_info.addon_status == "hasupdate")
    {
        handle_force_update_tool(server_addon_info);
    }
    else if(server_addon_info.addon_status == "forceupdate")
    {
        handle_force_update_tool(server_addon_info);
    }
}

void ToolUpdateTask::HandleAutoTask(SEVER_ADDON_INFO server_addon_info)
{
    if(server_addon_info.addon_status == "hasupdate")
    {
        handle_inform_update_tool(server_addon_info);
    }
    else if(server_addon_info.addon_status == "forceupdate")
    {
        handle_force_update_tool(server_addon_info);
    }
    else if(server_addon_info.addon_status == "deprecated")
    {

        handle_deprecated_tool(server_addon_info);

    }
    else if(server_addon_info.addon_status == "autopublish")
    {
        handle_publish_tool(server_addon_info);
    }
}

void ToolUpdateTask::FilterCachedOldVersionAddon(TOOL_ADDON_INFO addon_index)
{
    QString dir_path = Config::Instance()->get_user_addon_dir_path();
    QDir dir(dir_path);
    if(!dir.exists())
    {
        return;
    }
    dir.setFilter(QDir::Dirs);
    QString addon_dir_name = addon_index.addon_name + QString("-") + addon_index.addon_version;
    QFileInfoList file_info_list = dir.entryInfoList();
    for(QFileInfo index_info : file_info_list)
    {
        QString file_name = index_info.fileName();
        if((file_name == ".") || (file_name == ".."))
        {
            continue;
        }
        if(!addon_index.addon_name.isEmpty() && file_name.startsWith(addon_index.addon_name) && (!file_name.startsWith(addon_dir_name)))
        {
            QDir sub_addon_dir;
            QString target_path = index_info.filePath();
            sub_addon_dir.setPath(target_path);
            sub_addon_dir.removeRecursively();
        }
    }
}

void ToolUpdateTask::RemoveAddonFile(SEVER_ADDON_INFO addon_index)
{
    QString dir_path = Config::Instance()->get_user_addon_dir_path();
    QDir dir(dir_path);
    if(!dir.exists())
    {
        return;
    }
    dir.setFilter(QDir::Dirs);
    QString addon_dir_name = addon_index.addon_name + QString("-") + addon_index.addon_version;
    QFileInfoList file_info_list = dir.entryInfoList();
    for(QFileInfo index_info : file_info_list)
    {
        QString file_name = index_info.fileName();
        if((file_name == ".") || (file_name == ".."))
        {
            continue;
        }
        if(!addon_index.addon_name.isEmpty() && file_name.startsWith(addon_index.addon_name) && (file_name.startsWith(addon_dir_name)))
        {
            QDir sub_addon_dir;
            QString target_path = index_info.filePath();
            sub_addon_dir.setPath(target_path);
            sub_addon_dir.removeRecursively();
        }
    }
}

void ToolUpdateTask::slot_handle_download_progress(qint64 receiveBytes, qint64 totalBytes)
{
    emit download_progress(receiveBytes,totalBytes);
}
