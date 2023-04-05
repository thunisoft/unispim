// 检查输入法本身是否需要更新

#include "updatechecker.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QEventLoop>
#include <QFile>
#include <QSettings>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonDocument>
#include "../public/config.h"
#include "../public/defines.h"
#include "../public/utils.h"
#include "../public/dbusmessager.h"
#include "networkhandler.h"

UpdateChecker::UpdateChecker():
    m_is_working(false)
   ,m_timer(NULL)
   ,m_local_version({"", "", ""})
{
    Init();
}

void UpdateChecker::Init()
{
}


QString UpdateChecker::GetUrl()
{
    m_url = Config::Instance()->GetServerUrl().append("/testapi/requesturl");
    AppendParam();
    Utils::AddSystemExtraParamer(m_url,OPType::AUTO);
    return m_url;
}

void UpdateChecker::GetLocalVersion()
{
    QFile version(Config::GetUpdaterIniPath());
    if(!version.exists())
        return;
    QSettings config(Config::GetUpdaterIniPath(), QSettings::IniFormat);
    config.beginGroup("package_info");
    m_local_version.version = config.value("version", "").toString();
    m_local_version.cpu_type = config.value("cpu_type", "").toString();
    m_local_version.os_type = config.value("os_type", "").toString();
    config.endGroup();
}

void UpdateChecker::AppendParam()
{
    GetLocalVersion();

    m_url.append("?version=").append(m_local_version.version)
         .append("&os=").append(m_local_version.os_type)
         .append("&cpu=").append(m_local_version.cpu_type);
#ifdef USE_IBUS
    m_url.append("&frame=").append("ibus");
#else
    m_url.append("&frame=").append("fcitx");
#endif
    m_url.append("&optype=").append(QString::number(OPType::AUTO));
    m_url.append("&reqver=").append("2");
    m_url.append("&clientid=").append(Utils::GethostMac());
}

int UpdateChecker::Check()
{
    bool check_flag = IsTimeToCheckUpdate();
    if(!check_flag)
    {
        WriteLog("has canceled the update window isnot time to check");
        return 1;
    }
    int autoupdate_flag = Config::Instance()->GetAutoUpdateFlag();
    if(autoupdate_flag == 0)
    {
        WriteLog("enable auto update flag is zero don't check update");
        return 1;
    }


    QNetworkRequest request;
    request.setUrl(QUrl(GetUrl()));
    request.setSslConfiguration(Utils::GetQsslConfig());
    QNetworkAccessManager network_manager;
    QNetworkReply* reply = network_manager.get(request);

    QReplyTimeout *pTimeout = new QReplyTimeout(reply,20000);
    QEventLoop eventLoop;
    connect(reply, SIGNAL(finished()),&eventLoop, SLOT(quit()));
    connect(pTimeout, SIGNAL(timeout()),&eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    if(pTimeout->GetTimeoutFlag())
    {
        WriteLog("check package new version timeout");
        return 2;
    }

    if (QNetworkReply::NoError == reply->error())
    {
        WriteLog(QString("check package version succeed"));
        int ret = ParseResult(reply);
        if(ret == 0)
        {
            WriteLog("notify the qimpanel");
            Notify();
        }
        else
        {
            WriteLog(QString("parser check request failed return:%1").arg(ret));
        }
        return 0;
    }
    else
    {
        WriteLog(QString("check server version return error:%1").arg(reply->error()));
        return 3;
    }
}

void UpdateChecker::CheckError(QNetworkReply::NetworkError error_code)
{
    qDebug() << "check error, code is: "<< error_code;
}

void UpdateChecker::DownloadInstallPackage(QString package_addr_url,QString package_md5,QString addrtype)
{
    try
    {
        m_package_md5.clear();
        m_package_md5 = package_md5;
        QNetworkRequest request;
        QUrl url(package_addr_url);
        if(addrtype == "cdn")
            m_package_file_name = *package_addr_url.split("/").rbegin();
        else
            m_package_file_name = *package_addr_url.split("=").rbegin();

        QString download_dir = Utils::GetConfigDirPath() + "packagedownload/";
        QDir dir(download_dir);
        if(!dir.exists())
        {
            WriteLog(QString("local doesn't exist packagedownload dir create it"));
            dir.mkpath(download_dir);
        }

        QString package_path = download_dir + m_package_file_name;
        if(QFile::exists(package_path))
        {

            QString file_md5 =Utils::GetFileMD5(package_path).toLower();
            WriteLog(QString("local exist the package md5:%1 servermd5:%2").arg(file_md5).arg(package_md5));
            if(file_md5 == package_md5)
            {
                WriteLog("local exist package dont't need to download");
                StartUpdateProcess();
                return;
            }
        }
        //clear other package
        RemovePackagePathContent();
        request.setUrl(url);
        request.setSslConfiguration(Utils::GetQsslConfig());
        QNetworkAccessManager network_manager;
        QNetworkReply* reply = network_manager.get(request);
        QEventLoop eventLoop;
        connect(&network_manager, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
        eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
        OnDownLoadFinished(reply);
        return;
    }
    catch(std::exception& e)
    {
        WriteLog(QString("download install packaget throw exception:%1").arg(e.what()));
        return;
    }
}

void UpdateChecker::OnDownLoadFinished(QNetworkReply *reply)
{
    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (!statusCode.isValid())
    {
       WriteLog(QString("ondownload finished status not valid code:%1").arg(statusCode.toInt()));
       return;
    }

    QString temp_save_location = Utils::GetConfigDirPath() + "packagedownload/";
    QDir dir(temp_save_location);
    if(!dir.exists())
    {
        WriteLog(QString("packagedownload dir doesn't exist create it"));
        dir.mkpath(temp_save_location);
    }
    QString package_download_path = QString("%1%2").arg(temp_save_location).arg(m_package_file_name);


    QFile file(package_download_path);
    if(!file.open(QIODevice::WriteOnly))
    {
        WriteLog("create package file failed");
        return;
    }
    file.write(reply->readAll());
    file.close();
    QString download_package_md5 = Utils::GetFileMD5(package_download_path).toLower();
    if(m_package_md5.isEmpty())
    {
        WriteLog("[OnDownLoadFinished] md5 is empty");
        return;
    }
    if(download_package_md5 != m_package_md5)
    {
        WriteLog(QString("dwonload file md5 are different from server,localmd5:%1,servermd5:%2").arg(download_package_md5).arg(m_package_md5));
        WriteLog("remove local package file");
        QFile::remove(package_download_path);
    }
    else
    {
        WriteLog(QString("download package:%1 succeed").arg(m_package_file_name));
        //下载完毕之后启动更新程序
        StartUpdateProcess();
    }
}

void UpdateChecker::SetWorking()
{

}

void UpdateChecker::SetUnwork()
{

}

bool UpdateChecker::IsWorking()
{
    return m_is_working;
}

int UpdateChecker::ParseResult(QNetworkReply* reply)
{
    QByteArray data = reply->readAll();
    QJsonParseError json_error;
    QJsonDocument json_doc(QJsonDocument::fromJson(data, &json_error));
    if(json_error.error != QJsonParseError::NoError)
        return 1;

    //解析出来的message的值是("200"), 转成int会失败, 故转换成string
    QJsonObject json_object_root = json_doc.object();
    if(!json_object_root.contains("code") ||
       json_object_root.value("code").toString().compare("200") != 0 ||
       !json_object_root.contains("result"))
    {
          return 2;
    }

    QJsonObject json_object_result = json_object_root.value("result").toObject();
    if(!json_object_result.contains("update") ||
       json_object_result.value("update").toString().compare("false") == 0)
    {
         WriteLog(QString("server don't have new veriosn"));
         return 3;
    }
    else
    {
        if(!json_object_result.contains("version") || json_object_result.value("version").toString().isEmpty()
                ||!json_object_result.contains("addr") || json_object_result.value("addr").toString().isEmpty()
                ||!json_object_result.contains("addrtype") || json_object_result.value("addrtype").toString().isEmpty()
                ||!json_object_result.contains("md5") || json_object_result.value("md5").toString().isEmpty())
        {
            return 4;
        }

        QString addrtype = json_object_result.value("addrtype").toString();
        QString download_path = json_object_result.value("addr").toString();
        QString version = json_object_result.value("version").toString();
         QString file_md5 = json_object_result.value("md5").toString();
        //判断是否启动CDN下载
        if(addrtype == "cdn")
        {
            WriteLog(QString("cdn has new version:%1").arg(version));
        }
        //从服务器直接下载,为了兼容旧的版本
        else if(addrtype == "server")
        {
            WriteLog(QString("server has new version:%1").arg(version));
        }
        else
        {
            WriteLog(QString("[ParseResult] addrtype return false"));
        }
        SaveInfo(download_path, version,addrtype);
        DownloadInstallPackage(download_path,file_md5,addrtype);
        return 0;
    }
}

void UpdateChecker::SaveInfo(const QString& path, const QString& version,const QString& addrType)
{
    QFile info(Utils::GetSaveNewVersionInfoPath());
    if(!info.exists())
    {
        if(!info.open(QIODevice::WriteOnly))
            return ;
        info.close();
    }

    QSettings config(Utils::GetSaveNewVersionInfoPath(), QSettings::IniFormat);
    config.beginGroup("new_version_info");
    config.setValue("version", version);
    config.setValue("path", path.toLocal8Bit().toPercentEncoding().data());
    config.setValue("os_type", m_local_version.os_type);
    config.setValue("addr_type",addrType);
    config.endGroup();
}

void UpdateChecker::Notify()
{
    DbusMessager::SendMsgToQimpanel(QString("has_update"), 1);
}

void UpdateChecker::StartUpdateProcess()
{
    if(!m_update_process)
    {
        m_update_process = new QProcess(this);
    }
    if(m_update_process->state() != QProcess::NotRunning)
        return;
    QStringList arg_list;
    arg_list << "82717623-mhe4-0293-aduh-ku87wh6328ne";
    arg_list << UPDATE_UNISPY;
    arg_list << QString::number(m_update_tip_position);

    WriteLog("call update windows to show");
    m_update_process->start(Utils::GetBinFilePath().append("huayupy-updater-fcitx"), arg_list);
}

void UpdateChecker::RemovePackagePathContent()
{
    QString download_dir = Utils::GetConfigDirPath() + "packagedownload/";
    QDir dir(download_dir);
    QFileInfoList fileList;
    if(!dir.exists())  {return;}//文件不存，则返回false

    fileList=dir.entryInfoList(QDir::Files
                               |QDir::Readable|QDir::Writable
                               |QDir::Hidden|QDir::NoDotAndDotDot
                               ,QDir::Name);

    int file_count =fileList.size();
    for(int index=0; index<file_count;++index)
    {
        QFileInfo curFile = fileList.at(index);

        if(curFile.isFile())//如果是文件，删除文件
        {
            if(QFile::exists(curFile.filePath()))
            {
                WriteLog(QString("remove file:%1").arg(curFile.fileName()));
                QFile::remove(curFile.filePath());
            }

        }

    }
}
void UpdateChecker::WriteLog(const QString inputStr)
{
    QString logdir = Config::configDirPath().append("logs/");
    QDir dir(logdir);
    if(!dir.exists())
    {
        dir.mkpath(logdir);
    }
    QString logFileAddr = logdir + "huayupy-updatetask-log";
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

void UpdateChecker::run()
{
    QSettings config(Config::StateConfigPath(), QSettings::IniFormat);
    int update_time_interval = config.value("timer/time_interval", 6*60).toInt() * 1000 * 60;
    int update_interval_min = update_time_interval/(60*1000);
    WriteLog(QString("update task initialized time_interval:%1").arg(update_interval_min));
    do
    {
        try
        {
            WriteLog("Update task start");
            Check();
            WriteLog("Update task finished");
            QThread::msleep(update_time_interval);
        }
        catch(std::exception& e)
        {
            WriteLog(QString("when run updatecheck throw exception:%1").arg(e.what()));
        }
    }while(1);

}

bool UpdateChecker::IsTimeToCheckUpdate()
{
   QFile file(Config::GetSaveNewVersionInfoPath());
   if(!file.exists())
       return true;

   QSettings setting(Config::GetSaveNewVersionInfoPath(), QSettings::IniFormat);
   setting.beginGroup("next_check_time");
   QString str_date = setting.value("date", "").toString();
   setting.endGroup();
   if(str_date.isEmpty())
   {
       WriteLog("tochek day is empty");
       return true;
   }

   //直接比较在UOS下会崩溃采用间接的方式
   QDate current_date = QDate::currentDate();
   QDate tocheck_date = QDate::fromString(str_date, Qt::ISODate);
   WriteLog(QString("tocheck date is %1 currentdate is:%2").arg(str_date).arg(current_date.toString(Qt::ISODate)));
   int days = current_date.daysTo(tocheck_date);
   WriteLog(QString("time interval is %1 days").arg(days));
   if(days <= 0)
   {
       return true;
   }
   return false;
}


