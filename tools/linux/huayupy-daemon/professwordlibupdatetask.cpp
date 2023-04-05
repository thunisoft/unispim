#include "professwordlibupdatetask.h"
#include <QSettings>
#include <QDir>

void ProfessWordlibUpdateTask::run()
{
    try
    {
        while(m_task_flag)
        {
            m_server_wordlibinfo_vector.clear();
            m_hasupdate_wordlib_vector.clear();

            Config::Instance()->LoadConfig();
            bool wordlib_auto_update = Config::Instance()->GetWordlibAutoUpdateFlag();
            if(wordlib_auto_update == 0)
            {
                WriteLog("disabled profess wordlib auto update");
                task_sleep();
                continue;
            }

            int handle_code = NetworkHandler::Instance()->get_profess_wordlib_list(m_server_wordlibinfo_vector);
            Q_UNUSED(handle_code);
            auto check_md5 = [&](WordlibListInfo index_info){
                QString file_path = Config::Instance()->wordLibDirPath(WLDPL_DATA) + index_info.fileName;

                QString selected_wordlibs = Config::Instance()->SelectedWordlibs();
                QStringList wordlibs = selected_wordlibs.split(",");
                if(!wordlibs.contains(index_info.fileName) && (index_info.auto_publish == false))
                {
                    return;
                }
                bool is_file_exist = QFile::exists(file_path);

                if(is_file_exist && !index_info.auto_publish)
                {

                    QString file_md5 = Utils::GetFileMD5(file_path);
                    if(file_md5 != index_info.md5)
                    {
                        WriteLog(QString("%1 hasupdate").arg(index_info.fileName));
                        m_hasupdate_wordlib_vector.push_back(index_info);
                    }
                }
                else if(is_file_exist && index_info.auto_publish)
                {
                    WriteLog(QString("%1 autopublish wordlib conflict with system wordlib").arg(index_info.fileName));
                    return;
                }
                else if(!is_file_exist && index_info.auto_publish)
                {

                    QString cloud_dir = Utils::GetConfigDirPath().append("wordlib/cloud/");
                    QString file_path = cloud_dir + index_info.fileName;
                    if(QFile::exists(file_path) && (Utils::GetFileMD5(file_path) == index_info.md5))
                    {
                        WriteLog(QString("%1 local alreay has cloud wordlib").arg(index_info.fileName));
                        return;
                    }
                    WriteLog(QString("auto publish wordlib:%1").arg(index_info.fileName));
                    m_hasupdate_wordlib_vector.push_back(index_info);
                }

            };

            Config::Instance()->LoadConfig();
            FilterRepeatWordlibNameItem();
            for_each(m_server_wordlibinfo_vector.cbegin(),m_server_wordlibinfo_vector.cend(),check_md5);
            if(m_hasupdate_wordlib_vector.size()>0)
            {
                download_wordlib_from_server();
            }
            task_sleep();

        }


    }
    catch (std::exception& e)
    {
        Utils::WriteLogToFile(QString("profess wordlib update task trow exception:%1").arg(e.what()));
    }
}

bool ProfessWordlibUpdateTask::download_wordlib_from_server()
{
    int size = m_hasupdate_wordlib_vector.size();
    for(int index=0; index<size; ++index)
    {
        WordlibListInfo index_info = m_hasupdate_wordlib_vector.at(index);
        QString cloud_dir = Utils::GetConfigDirPath().append("wordlib/cloud/");
        QDir dir(cloud_dir);
        if(!dir.exists())
        {
            dir.mkpath(cloud_dir);
        }

        QString install_wordlib_dir = Utils::GetInstalledWordlibPath();
        QString file_path;
        QString target_dir;
        if(index_info.auto_publish)
        {
            file_path = cloud_dir + index_info.fileName;
            target_dir = cloud_dir;
        }
        else
        {
             file_path = install_wordlib_dir + index_info.fileName;
             target_dir = install_wordlib_dir;
        }

        int returnCode = NetworkHandler::Instance()->DownloadFileFromWeb(index_info.download_url,target_dir,
                                                                         file_path,index_info.fileName);
        QString new_md5 = Utils::GetFileMD5(file_path);
        if((returnCode != 0) || (new_md5 != index_info.md5))
        {
            continue;
        }
        if(index_info.auto_publish == true)
        {

            QString selected_wordlibs = Config::Instance()->SelectedWordlibs();
            QStringList wordlibs = selected_wordlibs.split(",");

            //check if has repeat item
            QSet<QString> wordlib_set = wordlibs.toSet();
            int wordlib_set_size = wordlib_set.size();
            int string_size = wordlibs.size();
            if(wordlib_set_size != string_size)
            {
                QStringList newList = QStringList::fromSet(wordlib_set);
                wordlibs = newList;
            }

            if(!wordlibs.contains(index_info.fileName))
            {
                wordlibs.push_back(index_info.fileName);
                Config::Instance()->SetSelectedWordlibs(wordlibs.join(","));
                Config::Instance()->SaveWordlibConfig();
                ConfigBus::instance()->valueChanged("loadWordlib", ""); //通知引擎加载词库
                WriteLog(QString("make autopublish wordlib:%1 work").arg(index_info.fileName));
            }
        }
    }
    return true;
}

void ProfessWordlibUpdateTask::task_sleep()
{

    QSettings config(Config::StateConfigPath(), QSettings::IniFormat);
    int time_intervale = config.value("timer/time_interval", 4*60).toInt();
    QThread::msleep(time_intervale*60*1000);
}

void ProfessWordlibUpdateTask::WriteLog(const QString inputStr)
{
    QString logdir = Config::configDirPath().append("logs/");
    QDir dir(logdir);
    if(!dir.exists())
    {
        dir.mkpath(logdir);
    }
    QString logFileAddr = logdir + "huayupy-professupdatetask-log";
    QString timeStamp = QDateTime::currentDateTime().toString("yyyy-M-d hh:mm:ss");
    QFile file(logFileAddr);
    bool isSucess = file.open(QIODevice::WriteOnly | QIODevice::Append);
    if(!isSucess)
    {
        return;
    }
    QTextStream output(&file);
    QString content = timeStamp + ":" + inputStr + "\n" ;
    output << content;
    file.close();
}

void ProfessWordlibUpdateTask::FilterRepeatWordlibNameItem()
{
    QString selected_wordlibs = Config::Instance()->SelectedWordlibs();
    QStringList wordlibs = selected_wordlibs.split(",");

    //check if has repeat item
    QSet<QString> wordlib_set = wordlibs.toSet();
    int wordlib_set_size = wordlib_set.size();
    int string_size = wordlibs.size();
    if(wordlib_set_size != string_size)
    {
        QStringList newList = QStringList::fromSet(wordlib_set);
        Config::Instance()->SetSelectedWordlibs(newList.join(","));
        Config::Instance()->SaveWordlibConfig();
        ConfigBus::instance()->valueChanged("loadWordlib", ""); //通知引擎加载词库
    }
}
