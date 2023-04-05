#include "syswordlibincretask.h"
#include "../public/config.h"
#include "../public/utils.h"
#include "dbusmsgmanager.h"
#include "configbus.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>
#include <QDir>
#include <QProcess>
#include <QSettings>

#include "networkhandler.h"




void SysWordlibIncreTask::run()
{

    QSettings config(Config::StateConfigPath(), QSettings::IniFormat);
    int min_time_interval = config.value("timer/time_interval", 60*4).toInt();
    WriteLogToFile(QString("sys wordlib increase task interval:%1min").arg(min_time_interval));

    do
    {
        WriteLogToFile("-----------------------------------");
        QVector<WORDLIB_BLOCK_INFO> wordlib_block_vector;

        //检查用户今日输入量，不活跃用户不更新
        if(Config::GetTodayInputCount() <= 0)
        {
            WriteLogToFile(QString("today input count is 0 user is InActive User"));
            task_sleep();
            continue;
        }
        int ret = check_sys_wordlib_update(wordlib_block_vector);

        if(ret == 0)
        {
            int wordlib_block_count = wordlib_block_vector.size();
            WriteLogToFile(QString("get block succeed the wordlib block count is %1").arg(wordlib_block_count));
            if(wordlib_block_count > 0)
            {
               auto index_sort = [=](const WORDLIB_BLOCK_INFO& block1, const WORDLIB_BLOCK_INFO& block2)->bool
                {
                    return block1.block_id < block2.block_id;
                };
                qSort(wordlib_block_vector.begin(),wordlib_block_vector.end(),index_sort);

                NetworkHandler handler;
                QString clinetid = Utils::GethostMac();
                bool islogin = Config::Instance()->IsLogin();
                QString loginid;
                if(islogin)
                {
                    loginid = Config::Instance()->GetLoginId();
                }
                QString addr_type = wordlib_block_vector.at(0).addr_type;
                if(addr_type != "cdn")
                {
                    WriteLogToFile("Start Download Wordlib Block From Server");
                    int ret_code = handler.DownloadSysWordlibBlockFromServer(clinetid,loginid,wordlib_block_vector);
                    if(ret_code == 0)
                    {
                        WriteLogToFile(QString("download sys wordlib block succeed"));
                        int ret = MergeSysWordlib(wordlib_block_vector);
                        WriteLogToFile(QString("mergeSyswordlib return %1").arg(ret));
                        ret = 2;
                        if(ret != 0)
                        {
                            WriteLogToFile(QString("merge sys wordlib block failed:%1,download full").arg(ret));
                            int download_full_code = download_full_syswordlib();
                            WriteLogToFile(QString("download full wordlib return:%1").arg(download_full_code));
                        }
                        else
                        {
                            WriteLogToFile(QString("merge sys wordlib block succeed"));
                            ConfigBus::instance()->valueChanged("loadWordlib", "");
                        }
                    }
                    else
                    {
                        WriteLogToFile(QString("download sys wordlib block errror return%1:%2").arg(ret_code).arg(Errorcode_Map[ret_code]));
                    }
                }
                else
                {
                    WriteLogToFile("Start Download Wordlib Block From CDN");
                    int ret = DownloadBlockFromCDN(wordlib_block_vector);
                    if(ret == 0)
                    {
                        WriteLogToFile("download wordlib block from cdn succeed");
                        int ret = MergeSysWordlibCdnBlock(wordlib_block_vector);
                        WriteLogToFile(QString("MergeSysWordlibCdnBlock return %1").arg(ret));
                        if(ret != 0)
                        {
                            WriteLogToFile(QString("MergeSysWordlibCdnBlock failed:%1,download full").arg(ret));
                            int download_full_code = download_full_syswordlib();
                            WriteLogToFile(QString("download full wordlib return:%1").arg(download_full_code));
                        }
                        else
                        {
                            WriteLogToFile(QString("MergeSysWordlibCdnBlock succeed"));
                            ConfigBus::instance()->valueChanged("loadWordlib", "");
                        }

                    }
                    else
                    {
                        WriteLogToFile("download wordlib block from cdn failed");
                    }

                }
            }
        }
        else if(ret == 1101)
        {
            int ret = download_full_syswordlib();
            WriteLogToFile(QString("Download full sys wordlib return code:%1").arg(ret));
        }
        else
        {
            WriteLogToFile(QString("check sys wordlib block return:%1 error:%2").arg(ret).arg(Errorcode_Map[ret]));
        }
        task_sleep();
    }
    while(1);

}

void SysWordlibIncreTask::WriteLogToFile(const QString &content)
{
    QString logdir = Config::configDirPath().append("logs/");
    QDir dir(logdir);
    if(!dir.exists())
    {
        dir.mkpath(logdir);
    }
    QString logFileAddr = logdir + "huayupy-syswordlibupdate-log";
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

QString SysWordlibIncreTask::get_wordlib_update_url()
{
    QString md5 = GetSysWordlibMD5();
    QString target_url = Config::Instance()->GetServerUrl().append("/testapi/requesturl?")
                                     .append("md5=")
                                     .append(md5)
                                     .append("&fromos=")
                                     .append(QString::number(Utils::GetOSTypeIndex()))
                                     .append("&reqver=")
                                     .append("2");
    Utils::AddSystemExtraParamer(target_url,OPType::AUTO);
    return target_url;
}


int SysWordlibIncreTask::check_sys_wordlib_update(QVector<WORDLIB_BLOCK_INFO>& wordlib_info_vector)
{
    QNetworkRequest request;
    QString check_url = get_wordlib_update_url();
    request.setUrl(check_url);
    qDebug() << check_url;
    request.setSslConfiguration(Utils::GetQsslConfig());
    QNetworkAccessManager* network_manager = new QNetworkAccessManager(this);
    QNetworkReply* reply = network_manager->get(request);

    QReplyTimeout *pTimeout = new QReplyTimeout(reply,20000);
    QEventLoop eventLoop;
    connect(reply, SIGNAL(finished()),&eventLoop, SLOT(quit()));
    connect(pTimeout, SIGNAL(timeout()),&eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    if(pTimeout->GetTimeoutFlag())
    {
        return 1;
    }

    QVariant http_request_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if(http_request_code.toInt() != 200)
    {
        return 2;
    }

    QJsonParseError json_error;
    QJsonDocument json_doc(QJsonDocument::fromJson(reply->readAll(), &json_error));
    if(json_error.error != QJsonParseError::NoError)
    {
        return 3;
    }

    QJsonObject json_object_root = json_doc.object();
    QString code = json_object_root.value("code").toString();
    if(code == "1101")
    {
        return 1101;
    }
    else if(code == "200")
    {
        QJsonArray result_array = json_object_root.value("result").toArray();
        if( result_array.size() <= 0)
        {
            return 0;
        }

        for(int i = 0; i < result_array.size(); i++)
        {
            QJsonObject object_uwl_info = result_array.at(i).toObject();
            if(!object_uwl_info.contains("fileName") || !object_uwl_info.contains("block")
                    ||!object_uwl_info.contains("fulldictmd5"))
            {
                WriteLogToFile("wordlib block json info is not valid");
                return 4;
            }
            else
            {
                WORDLIB_BLOCK_INFO uwl_info;
                uwl_info.block_id = object_uwl_info.value("block").toInt();
                uwl_info.file_name = object_uwl_info.value("fileName").toString();
                uwl_info.md5 = object_uwl_info.value("fulldictmd5").toString();

                if(object_uwl_info.contains("addrtype") && object_uwl_info.contains("addr") && object_uwl_info.contains("md5"))
                {
                    uwl_info.addr_type = object_uwl_info.value("addrtype").toString();
                    uwl_info.block_url = object_uwl_info.value("addr").toString();
                    uwl_info.block_md5 = object_uwl_info.value("md5").toString();
                }
                wordlib_info_vector.push_back(uwl_info);

            }
        }
        return 0;
    }
    else
    {
        QString insall_wordlib_path = Utils::GetHuayuPYInstallDirPath().append("wordlib/sys.uwl");
        if(!QFile::exists(insall_wordlib_path))
        {
           WriteLogToFile(QString("ERROR-----------------安装目录系统词库丢失！"));
           return 1101;
        }
        WriteLogToFile(QString("sys wordlib get increase info return:%1").arg(code));
        return 5;
    }
}

int SysWordlibIncreTask::download_full_syswordlib()
{
    QString md5 = GetSysWordlibMD5();
    NetworkHandler network_handler;
    QString download_url, package_md5;
    int ret = network_handler.check_system_wordlib_update(md5,download_url,package_md5);
    if((ret == 0) && !package_md5.isEmpty() && !download_url.isEmpty())
    {
        WriteLogToFile(QString("get full syswordlib has update"));
        int ret = network_handler.download_full_sys_wordlib(download_url,package_md5);
        WriteLogToFile(QString("download sys wordlib return:%1").arg(ret));
        if(ret == 0)
        {
            WriteLogToFile(QString("download full sys wordlib succeed"));
            ConfigBus::instance()->valueChanged("loadWordlib", "");
            return 0;
        }
        else
        {
            WriteLogToFile(QString("download full wordlib error:%1:%2").arg(ret).arg(Errorcode_Map[ret]));
        }
    }
    else if(ret == 0)
    {
        WriteLogToFile(QString("full sys wordlib dosen't exist update"));
        return 0;
    }
    else
    {
        WriteLogToFile(QString("full wordlib check error code:%1:%2").arg(ret).arg(Errorcode_Map[ret]));
        return 2;
    }
}

QString SysWordlibIncreTask::GetSysWordlibMD5()
{
    QString local_sys_wordlib_path = Config::configDirPath().append("wordlib/sys.uwl");
    QString insall_wordlib_path = Utils::GetHuayuPYInstallDirPath().append("wordlib/sys.uwl");
    if(!QFile::exists(local_sys_wordlib_path))
    {
        QFile default_sys(insall_wordlib_path);
        if(default_sys.exists())
            default_sys.copy(local_sys_wordlib_path);
    }
    return Utils::GetFileMD5(local_sys_wordlib_path);
}

int SysWordlibIncreTask::MergeSysWordlib(QVector<WORDLIB_BLOCK_INFO> block_vector)
{
    FILE* ptr2 = NULL;
    QString uwl_path = Utils::GetTmpDir().append("*.uwl");
    string command = QString(" rm -r %1").arg(uwl_path).toStdString();
    if((ptr2 = popen(command.c_str(), "r")) != NULL)
    {
        pclose(ptr2);
    }
    QString zip_package_path = Utils::GetTmpDir().append("tmp.zip");
    QString zip_dest_path = Utils::GetTmpDir();
    if(!Utils::DepressedZlibFile(zip_package_path,zip_dest_path))
        return 1;

    QString sys_tmp_path = Utils::GetTmpDir().append("sys.uwl");
    QFile sys_tmp(sys_tmp_path);
    if(sys_tmp.exists())
        sys_tmp.remove();

    QString sys_wordlib_path = Config::configDirPath().append("wordlib/sys.uwl");

    auto CopySysWordlibToTmp = [&]() ->bool {
        QFile sys_uwl(sys_wordlib_path);
        if(!sys_uwl.exists())
            return false;
        if(!sys_uwl.copy(sys_tmp_path))
            return false;
        return true;
    };

    if(!CopySysWordlibToTmp())
        return 2;

    QProcess* _process = new QProcess();

    for(auto iter = block_vector.begin(); iter!=block_vector.end(); iter++)
    {
        QStringList params;
        params << QString(sys_tmp_path);
        params << QString(Utils::GetTmpDir().append(iter->file_name));
        params << QString(sys_tmp_path);

        //one minute
        _process->execute(Utils::GetBinFilePath().append("huayupy-merge-wordlib"), params);
        QString local_md5 = Utils::GetFileMD5(sys_tmp_path);
        if(local_md5.compare(iter->md5) != 0)
        {
            return 3;
        }
    }
    Utils::CopyFileForce(sys_tmp_path, sys_wordlib_path);
    return 0;
}

int SysWordlibIncreTask::MergeSysWordlibCdnBlock(QVector<WORDLIB_BLOCK_INFO> block_vector)
{

    QString sys_tmp_path = Utils::GetTmpDir().append("sys.uwl");
    QFile sys_tmp(sys_tmp_path);
    if(sys_tmp.exists())
        sys_tmp.remove();

    QString sys_wordlib_path = Config::configDirPath().append("wordlib/sys.uwl");

    auto CopySysWordlibToTmp = [&]() ->bool {
        QFile sys_uwl(sys_wordlib_path);
        if(!sys_uwl.exists())
            return false;
        if(!sys_uwl.copy(sys_tmp_path))
            return false;
        return true;
    };

    if(!CopySysWordlibToTmp())
        return 2;

    QProcess* _process = new QProcess();

    for(auto iter = block_vector.begin(); iter!=block_vector.end(); iter++)
    {
        QStringList params;
        params << QString(sys_tmp_path);
        params << QString(Utils::GetTmpDir().append(iter->file_name));
        params << QString(sys_tmp_path);

        //one minute
        _process->execute(Utils::GetBinFilePath().append("huayupy-merge-wordlib"), params);
        QString local_md5 = Utils::GetFileMD5(sys_tmp_path);
        if(local_md5.compare(iter->md5) != 0)
        {
            return 3;
        }
    }
    Utils::CopyFileForce(sys_tmp_path, sys_wordlib_path);
    return 0;
}

void SysWordlibIncreTask::task_sleep()
{
    QSettings config(Config::StateConfigPath(), QSettings::IniFormat);
    int min_time_interval = config.value("timer/time_interval", 60*4).toInt();
    QThread::msleep(min_time_interval*60*1000);
}


int SysWordlibIncreTask::DownloadBlockFromCDN(QVector<WORDLIB_BLOCK_INFO>& wordlib_info_vector)
{
    try
    {
        if(wordlib_info_vector.isEmpty())
        {
            return 9;
        }

        int array_size = wordlib_info_vector.size();
        for(int index=0; index<array_size; ++index)
        {
            QString block_addr_type = wordlib_info_vector.at(index).addr_type;
            if(!block_addr_type.isEmpty() && (block_addr_type == "cdn"))
            {
                QString download_url = wordlib_info_vector.at(index).block_url;
                QString block_md5 = wordlib_info_vector.at(index).block_md5;
                QString file_name = wordlib_info_vector.at(index).file_name;
                int ret = DownloadFileFromCdnUrl(download_url,block_md5);
                if(ret != 0)
                {
                    WriteLogToFile(QString("download file:%1 from CND error").arg(file_name));
                    return 1;
                }
            }
        }

        return 0;
    }
    catch(std::exception& e)
    {
        return 5;
    }
}

int SysWordlibIncreTask::DownloadFileFromCdnUrl(QString &download_url, QString file_md5)
{
    //通过返回的请求头中获得文件的名称
    QNetworkRequest request;
    QUrl url(download_url);
    request.setSslConfiguration(Utils::GetQsslConfig());

    QNetworkAccessManager *accessManager=new QNetworkAccessManager(this);
    request.setUrl(url);
    QNetworkReply *reply  = accessManager->get(request);

    QReplyTimeout *pTimeout = new QReplyTimeout(reply,20000);
    QEventLoop eventLoop;
    connect(reply, SIGNAL(finished()),&eventLoop, SLOT(quit()));
    connect(pTimeout, SIGNAL(timeout()),&eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (statusCode.toInt() != 200)
    {
        return 2;
    }
    QFileInfo url_file_info(download_url);
    QString file_name = url_file_info.fileName();
    QString file_save_path = Utils::GetTmpDir() + file_name;
    QFile file(file_save_path);
    if(file.exists())
    {
        file.remove();
    }
    if(!file.open(QIODevice::WriteOnly))
    {
        Utils::WriteLogToFile(QString("%1errorInfo:%2").arg(file_name).arg(file.errorString()));
        return 4;
    }
    file.write(reply->readAll());
    file.close();
    QString download_file_md5 = Utils::GetFileMD5(file_save_path);
    if(download_file_md5 == file_md5)
    {
        return 0;
    }
    return 10;
}

