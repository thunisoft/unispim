#include "userwordlibupdatetask.h"
#include "../public/config.h"
#include "../public/utils.h"
#include <QFile>
#include <QDateTime>
#include <QSettings>
#include <QDir>
#include <QProcess>
#include <QTextStream>
#include <iostream>
#include <fstream>
#include "networkhandler.h"


void UserWordlibUpdateTask::run()
{

    QSettings config(Config::StateConfigPath(), QSettings::IniFormat);
    int min_time_interval = config.value("timer/time_interval", 60*4).toInt();
    WriteLogToFile(QString("<<<<<<user wordlib update task initialized interval:%1min>>>>>>").arg(min_time_interval));

    do
    {
        WriteLogToFile("-----------------------------------");

        Config::Instance()->LoadConfig();
        if(Config::Instance()->IsLogin())
        {
            QSettings user_config_settings(Config::configFilePath(),QSettings::IniFormat);
            user_config_settings.beginGroup("account");
            int wordlib_update_flag = user_config_settings.value("wordlib_auto_update",1).toInt();
            user_config_settings.endGroup();
            if(wordlib_update_flag == 0)
            {
                WriteLogToFile(QString("User disabled wordlib_auto_update"));
                task_sleep();
                continue;
            }
        }

        int ret = RestoreUserWordlib();
        WriteLogToFile(QString("Restore user wordlib return:%1").arg(ret));
        int join_the_plan = Config::Instance()->join_the_plan();
        if(join_the_plan)
        {
            update_user_wordlib();
        }
        else
        {
            WriteLogToFile(QString("User donesn't accept the plan disable user.uwl update"));
        }
        task_sleep();
    }
    while(1);
}

int UserWordlibUpdateTask::update_user_wordlib()
{
    QString user_wordlib_path = GetUserWordlibPath();
    QString serverMD5Value;
    NetworkHandler request_handler;

    //用户词库存在的时候
    if(QFile::exists(user_wordlib_path))
    {
        WriteLogToFile(QString("%1 Local User wordlib exists").arg(QString(__FUNCTION__)));
        QString server_user_wordlib_md5, server_user_wordlib_url,perv_clientid;
        QString loginid = Config::Instance()->GetLoginId();
        QString clientid = Utils::GethostMac();
        int getmd5_ret = request_handler.GetCurrentUserUserwordlibMD5(loginid,clientid,server_user_wordlib_md5,server_user_wordlib_url,perv_clientid);
        if(getmd5_ret != 0)
        {
            WriteLogToFile(QString("get server user wordlib md5 error:%1:%2").arg(getmd5_ret).arg(Errorcode_Map[getmd5_ret]));
            return 1;
        }
        if(server_user_wordlib_md5.isEmpty())
        {
            WriteLogToFile("server user wordlib md5 is empty");
            return 2;
        }

        QString local_user_md5 = Utils::GetFileMD5(user_wordlib_path);
        if(server_user_wordlib_md5 == local_user_md5)
        {
            WriteLogToFile(QString("user.wordlib server md5:%1 is same with local file md5:%2").arg(server_user_wordlib_md5).arg(local_user_md5));
            return 0;
        }
        if (loginid.isEmpty())
        {
            WriteLogToFile("unlogined state update user wordlib");
            int returnCode = request_handler.UploadCustomWordlib(loginid,clientid,user_wordlib_path,serverMD5Value);
            if(returnCode == 0)
            {
                WriteLogToFile("unlogined state upload user wordlib succeed");
                return 0;
            }
            else if(returnCode == 710)
            {
                int ret = DownloadAndCombineUserWordlib();
                WriteLogToFile(QString("unlogined state upload wordlib return 710 download and combine:%1").arg(ret));
                return 1;
            }
            else
            {
                WriteLogToFile(QString("unlogined state upload user wordlib error:%1").arg(returnCode));
                return 2;
            }
        }
        else
        {
            int ret = LoginedUpdateUserWordlib();
            WriteLogToFile(QString("logined state process return:%1").arg(ret));
            return 0;
        }
    }
    else
    {
        int ret = DownloadAndCombineUserWordlib();
        if(ret != 0)
        {
            WriteLogToFile(QString("when local user.uwl doesn't exist downloadandcombine return:%1").arg(ret));
            return ret;
        }
        return 0;
    }
    return 1;
}


void UserWordlibUpdateTask::WriteLogToFile(const QString &content)
{
    QString logdir = Config::configDirPath().append("logs/");
    QDir dir(logdir);
    if(!dir.exists())
    {
        dir.mkpath(logdir);
    }
    QString logFileAddr = logdir + "huayupy-userwordlibupdate-log";
    QString timeStamp = QDateTime::currentDateTime().toString("yyyy-M-d hh:mm:ss");
    QFile file(logFileAddr);
    bool isSucess = file.open(QIODevice::WriteOnly | QIODevice::Append);
    if(!isSucess)
    {
        return;
    }
    QTextStream output(&file);
    QString output_content = timeStamp + ":" + content + "\n" ;
    output << output_content;
    file.close();
}


void UserWordlibUpdateTask::task_sleep()
{
    QSettings config(Config::StateConfigPath(), QSettings::IniFormat);
    int min_time_interval = config.value("timer/time_interval", 60*4).toInt();
    QThread::msleep(min_time_interval*60*1000);
}

int UserWordlibUpdateTask::RestoreUserWordlib()
{
    try
    {
        QString wordlib_path = GetUserWordlibPath();
        QString bak_wordlib_path = wordlib_path + ".bak";
        if(!QFile::exists(wordlib_path))
        {
            WriteLogToFile(QString("%1 local user.uwl doesn't exist").arg(QString(__FUNCTION__)));
            return 1;
        }

        QFile uwl_file(wordlib_path);
        QFile bak_wordlib_file(bak_wordlib_path);

        auto restore_bak_file = [&](){
            if(bak_wordlib_file.exists())
            {
                if(IsWordlibValid(bak_wordlib_path))
                {
                    WriteLogToFile("user.uwl is error and user.uwl.bak is ok ");
                    uwl_file.remove();
                    bak_wordlib_file.rename(wordlib_path);
                }
                else
                {
                    WriteLogToFile("user.uwl is error and user.uwl.bak is error ");
                    uwl_file.remove();
                    bak_wordlib_file.remove();
                }

            }
            else
            {
                WriteLogToFile("bak file doesn't exist");
                uwl_file.remove();
            }

        };


        int file_size = uwl_file.size();
        if((file_size % 1024) != 0)
        {
            WriteLogToFile("when restore user.uwl size is not valid");
            restore_bak_file();
            return 0;
        }
        if(!IsWordlibValid(wordlib_path))
        {
            restore_bak_file();
        }
        return 0;

    }
    catch(std::exception& e)
    {
        return 1;
    }
}


bool UserWordlibUpdateTask::IsWordlibValid(const QString &wordlib_path)
{
    try
    {
        if(!QFile::exists(wordlib_path))
        {
            return false;
        }


        WORDLIBHEADER wordlib_info;
        std::ifstream instream(wordlib_path.toStdString());
        instream.read((char*)&wordlib_info, sizeof(WORDLIBHEADER));
        QFile uwl_file(wordlib_path);
        int file_size = uwl_file.size();
        WriteLogToFile(QString("%1 filesize is: %2").arg(wordlib_path).arg(file_size));
        if((file_size % 1024) != 0)
        {
            WriteLogToFile("filesize is unvalid");
            return false;
        }

        QProcess* wordlib_process = new QProcess(this);
        QFileInfo wordlib_file_info(wordlib_path);

        QString file_dir_path = wordlib_file_info.absolutePath();
        QString txt_file_name = wordlib_file_info.baseName() + ".txt";
        QString target_txt_path = QDir::toNativeSeparators( file_dir_path + "/" + txt_file_name);

        QStringList params;
        params << QString("-export");
        params << QString(wordlib_path);
        params << QString(target_txt_path);

        QString wordlib_tool_path = Utils::GetBinFilePath().append("huayupy-wl-tool-fcitx");
        //one minute
        wordlib_process->execute(wordlib_tool_path, params);

        QFile file(target_txt_path);
        if (!file.exists())
        {
            WriteLogToFile("txt file is not exist");
            return false;
        }

        int current_file_count = 0;
        QFile txt_file(target_txt_path);
        if (txt_file.open(QIODevice::ReadOnly))
        {
            while (!txt_file.atEnd())
            {
                QByteArray tmp = txt_file.readLine();
                {
                    current_file_count++;
                }
            }
            txt_file.close();
        }
        WriteLogToFile(QString("txt wordlib file count is: %2").arg(current_file_count));

        if(current_file_count >= 5)
        {
            QFile::remove(target_txt_path);
            return true;
        }
        else
        {
            QFile::remove(target_txt_path);
            return false;
        }

    }
    catch(std::exception& e)
    {
        WriteLogToFile(QString("when valid throw exception:%1").arg(e.what()));
        return false;
    }

}

QString UserWordlibUpdateTask::GetUserWordlibPath()
{
    bool is_login = Config::Instance()->IsLogin();
    if(is_login)
    {
        QString loginid = Config::Instance()->GetLoginId();
        return Config::configDirPath().append("wordlib/").append(loginid).append("/user.uwl");
    }
    else
    {
        return Config::configDirPath().append("wordlib/").append("user.uwl");
    }

}


int UserWordlibUpdateTask::LoginedUpdateUserWordlib()
{
    QString clientid = Utils::GethostMac();
    QString loginid = Config::Instance()->GetLoginId();

    QString  server_md5, prev_clientid,server_url_addr;
    NetworkHandler request_handler;
    int getmd5_ret = request_handler.GetCurrentUserUserwordlibMD5(loginid, clientid, server_md5, server_url_addr, prev_clientid);
    if (getmd5_ret != 0)
    {
        WriteLogToFile(QString("logined state get last upload clientid error:%1").arg(getmd5_ret));
        return 1;
    }
    if (prev_clientid == clientid)
    {
        WriteLogToFile("last time upload at the same pc");
    }
    else
    {
        WriteLogToFile("last time upload in a different pc");
        int ret_code = DownloadAndCombineUserWordlib();
        if (ret_code != 0)
        {
            WriteLogToFile(QString("logined state download and combine wordlib error:%1").arg(ret_code));
            return 2;
        }
    }

    QString userUwlFilePath = GetUserWordlibPath();
    QString upload_result_md5;
    int upload_ret = request_handler.UploadCustomWordlib(loginid, clientid, userUwlFilePath, upload_result_md5);
    if (upload_ret == 0)
    {

        WriteLogToFile("logined state upload wordlib succeed");
        return 0;
    }
    else if (upload_ret == 710)
    {
        WriteLogToFile("logined state upload wordlib return 710");
        int ret = -1;
        if (prev_clientid == clientid)
        {
            ret = DownloadAndCombineUserWordlib();
        }
        else
        {
            //ret = DownloadAndReplaceLocalUserWordlib();
            WriteLogToFile(QString("download and replace wordlib return:%1").arg(ret));
        }
        if(ret == 0)
        {
            return 0;
        }
    }
    else
    {
        WriteLogToFile(QString("unlogined state upload user worlid error-ret:%1").arg(upload_ret));
    }
    return upload_ret;
}


int UserWordlibUpdateTask::DownloadAndCombineUserWordlib()
{
    try
    {
        NetworkHandler network_handler;
        QString loginid = Config::Instance()->GetLoginId();
        QString clientid = Utils::GethostMac();
        QString userUwlFilePath = GetUserWordlibPath();
        QString server_user_wordlib_md5, server_user_wordlib_url,prev_clientid;
        QString userUwlFilePathTmp = userUwlFilePath + QString(".tmp");
        QString userUwlFIlePathBak = userUwlFilePath + QString(".bak");
        QString local_for_combine = userUwlFilePath + QString(".tmp.combine");
        QString local_for_combine_bak = userUwlFilePath + QString(".tmp.combine.bak");

        int getmd5_ret = network_handler.GetCurrentUserUserwordlibMD5(loginid, clientid, server_user_wordlib_md5, server_user_wordlib_url, prev_clientid);
        if ((getmd5_ret == 0) && !server_user_wordlib_md5.isEmpty() && !server_user_wordlib_url.isEmpty())
        {
            WriteLogToFile("DownloadAndCombineUserWordlib when download get server info succeed");
            //如果服务器端和本地端的词库MD5值不同下载词库
            QFile userUwlFileTmp(userUwlFilePathTmp);
            if (userUwlFileTmp.exists())
                userUwlFileTmp.remove();

            QFile userUwlFileBak(userUwlFIlePathBak);
            if (userUwlFileBak.exists())
                userUwlFileBak.remove();

            QFile userwordlib(userUwlFilePath);

            int downloadReturnCode = network_handler.DownloadFileFromWeb(server_user_wordlib_url,userUwlFilePathTmp);
            if (downloadReturnCode != 0)
            {
                WriteLogToFile(QString("download useruwl failed return:%1").arg(downloadReturnCode));
                return 1;
            }
            QString download_md5 = Utils::GetFileMD5(userUwlFilePathTmp);

            if ((IsWordlibValid(userUwlFilePathTmp)) && (download_md5 == server_user_wordlib_md5))
            {
                WriteLogToFile("download user.uwl succeed");
                if (!userwordlib.exists())
                {
                    userUwlFileTmp.rename(userUwlFilePath);
                    return 0;
                }
                else
                {
                    userwordlib.copy(local_for_combine);
                    combine_wordlib(local_for_combine, userUwlFilePathTmp);

                    QFile local_for_combine_bak_file(local_for_combine_bak);
                    QFile local_combine_file(local_for_combine);
                    if ((IsWordlibValid(local_for_combine)))
                    {
                        WriteLogToFile("combine server user.uwl and local user.uwl succeed");
                        userwordlib.rename(userUwlFIlePathBak);
                        local_combine_file.rename(userUwlFilePath);
                        userUwlFileTmp.remove();
                        if (local_for_combine_bak_file.exists())
                        {
                            local_for_combine_bak_file.remove();
                        }
                        return 0;
                    }
                    local_combine_file.remove();
                    userUwlFileTmp.remove();
                    if (local_for_combine_bak_file.exists())
                    {
                        local_for_combine_bak_file.remove();
                    }
                    return 2;

                }
            }
            else
            {
                WriteLogToFile("download useruwl file is not valid");
                userUwlFileTmp.remove();
                return 3;
            }
        }
        else
        {
            WriteLogToFile(QString("get server info error code:%1,md5:2,url:%3").arg(getmd5_ret).arg(server_user_wordlib_md5).arg(server_user_wordlib_url));
            return 4;
        }
    }
    catch (std::exception& e)
    {
        WriteLogToFile(QString("download and combine wordlib throw exception:%1").arg(e.what()));
        return 5;
    }
}

bool UserWordlibUpdateTask::combine_wordlib(QString destwordlib_path, QString sourcewordlib_path)
{
    QProcess process;
    QStringList params;
    params << QString(destwordlib_path);
    params << QString(sourcewordlib_path);
    params << QString(destwordlib_path);

    int ret = process.execute(Utils::GetBinFilePath().append("huayupy-merge-wordlib"), params);
    if(ret == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}
