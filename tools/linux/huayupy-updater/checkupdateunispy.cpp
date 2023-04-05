#include "checkupdateunispy.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <QNetworkAccessManager>
#include "update_adaptor.h"
#include "update_interface.h"
#include <unistd.h>
#include "cJSON.h"
#include "iniparser.h"
#include "define.h"
#include "../public/inisetting.h"
#include <QtDBus>
#include <QTimer>
#include <QEventLoop>
#include "mainwindow.h"

void CheckUpdateUnispy::CheckUpdate(string imFrame)
{
    // post request
    QNetworkRequest request;
    QString requestUrl = Config::Instance()->GetServerUrl().append("/testapi/requesturl?version=");
    string version;
    string cpu_type;
    if(!GetLocalVersion(version, cpu_type, m_os_type)||version.compare(NOTFOUND)==0)
    {
        SendMsgToDaemon(REASON_IS_ERROR);
        return;
    }

    requestUrl.append(version.c_str());
    requestUrl.append("&os=").append(m_os_type.c_str()).append("&cpu=").append(cpu_type.c_str());
    if(imFrame.compare(CHECK_UPDATE_FCITX) == 0)
    {
        requestUrl.append("&frame=").append("fcitx");
    }
    else
    {
        requestUrl.append("&frame=").append("ibus");
    }

    request.setUrl(QUrl(requestUrl));

    m_requestManager = new QNetworkAccessManager(this);
    QNetworkReply* reply = m_requestManager->get(request);
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(2000);
    timer.start();
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()) );
    //connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(OncheckFinished(QNetworkReply*)));
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));

    loop.exec();
    if(timer.isActive())
    {
        timer.stop();
        if(reply->error() == QNetworkReply::NoError)
        {
            OncheckFinished(reply);
        }
        else
        {
            SendMsgToDaemon(REASON_IS_ERROR);
        }
    }
    else
    {
        disconnect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        reply->abort();
        reply->close();
        reply->deleteLater();
        SendMsgToDaemon(REASON_IS_ERROR);
    }
}

void CheckUpdateUnispy::OncheckFinished(QNetworkReply *reply)
{
     QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
     if (statusCode.isValid())
     {
         qDebug() << "status code: " << statusCode.toInt();
     }
     QByteArray replyData = reply->readAll();
     cJSON *json = cJSON_Parse(replyData.data());
     if (json == NULL)
     {
         const char *error_ptr = cJSON_GetErrorPtr();
         if (error_ptr != NULL)
         {
             SendMsgToDaemon(REASON_IS_ERROR);
             qDebug() << "Error: %s\n" << error_ptr;
         }
         return;
     }

     const cJSON *code = cJSON_GetObjectItemCaseSensitive(json, "code");
     if (!code || !cJSON_IsString(code) || (code->valuestring) == NULL)
     {
         SendMsgToDaemon(REASON_IS_ERROR);
         qDebug() << "Error: code is empty";
         return;
     }

     QString codeStr = QString::fromUtf8(code->valuestring);
     qDebug() << "code: " << codeStr;

     //const cJSON *success = cJSON_GetObjectItemCaseSensitive(json, "success");

     const cJSON *message = cJSON_GetObjectItemCaseSensitive(json, "message");
     qDebug() <<"message: "<<QString::fromUtf8(message->valuestring);
     if(/*successStr == "true" &&*/ codeStr == "200" )
     {
        const cJSON *result = cJSON_GetObjectItemCaseSensitive(json, "result");
        if(result && cJSON_IsObject(result))
        {
            const cJSON *update = cJSON_GetObjectItemCaseSensitive(result, "update");
            if(update && cJSON_IsString(update))
            {
                if(strcmp(update->valuestring,"false")==0)
                {
                    SendMsgToDaemon(REASON_IS_UPTO_DATE);
                    qDebug() <<" client is up-to-date" ;

                }
                else{
                    const cJSON *version = cJSON_GetObjectItemCaseSensitive(result, "version");
                    const cJSON *addr = cJSON_GetObjectItemCaseSensitive(result, "addr");
                    QByteArray urlByteArray = addr->valuestring;
                    SaveNewVersionInfo(version->valuestring, urlByteArray.toPercentEncoding().data(), m_os_type.c_str());

                    QString package_version, url_path, os_type;

                    if(GetNewVersionAndPathAndOs(package_version,url_path,os_type))
                    {
                        DownloadInstallPackage(url_path);
                    }
                    qDebug() << "version: "<< version->valuestring;
                    qDebug() << "addr: "<< addr->valuestring;
                }
            }
        }
     }

}
void CheckUpdateUnispy::onLoginReplyReadyRead()
{
}

void CheckUpdateUnispy::onLoginReplyError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error);
}

void CheckUpdateUnispy::onLoginReplySslErrors(QList<QSslError> errors)
{
    Q_UNUSED(errors);
}

bool CheckUpdateUnispy::GetLocalVersion(string& version, string& cpu_type, string& os_type)
{
   QString filepath = Config::GetUpdaterIniPath();
   if(!FileExists(filepath.toStdString()))
   {
       return false;
   }else{
        dictionary *dict = iniparser_load(filepath.toStdString().c_str());
        if (!dict)
              return false;
        version = iniparser_getstring(dict, "package_info:version", NOTFOUND);
        cpu_type = iniparser_getstring(dict, "package_info:cpu_type", NOTFOUND);
        os_type = iniparser_getstring(dict, "package_info:os_type", NOTFOUND);
        return true;
   }
}

int CheckUpdateUnispy::FileExists(const string& filepath)
{
    if (access(filepath.c_str(), F_OK) == 0)
        return 1;

    return 0;
}

void CheckUpdateUnispy::SendMsgToDaemon(QString msg)
{
    QDBusMessage dbusMsg = QDBusMessage::createSignal("/", "com.thunisoft.update", "msgTodaemon");
    dbusMsg<<msg;
    QDBusConnection::sessionBus().send(dbusMsg);
    qDebug()<<"sent message";
}

bool CheckUpdateUnispy::SaveNewVersionInfo(const QString& new_version, const QString& new_version_down_path, const QString& os_type)
{
    if(!QFile::exists(Config::GetSaveNewVersionInfoPath()))
    {
        QDir dir(Config::configDirPath());
        if(!dir.exists(Config::configDirPath()))
        {
            if(!dir.mkdir(Config::configDirPath()))
                return false;
        }
        QFile file(Config::GetSaveNewVersionInfoPath());
        if(!file.open(QIODevice::WriteOnly))
        {
            return false;
        }
        file.close();
    }
    QSettings config(Config::GetSaveNewVersionInfoPath(), QSettings::IniFormat);
    config.beginGroup("new_version_info");
    config.setValue("version", new_version);
    config.setValue("path", new_version_down_path);
    config.setValue("os_type", os_type);
    config.endGroup();
    return true;
}

void CheckUpdateUnispy::DownloadInstallPackage(QString package_addr_url)
{

    QNetworkRequest request;
    QUrl url(package_addr_url);
    m_package_file_name = *package_addr_url.split("=").rbegin();
    request.setUrl(url);
    request.setSslConfiguration(MainWindow::GetQsslConfig());
    QNetworkReply *reply  = m_requestManager->get(request);

    connect(m_requestManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(OnDownLoadFinished(QNetworkReply*)));

}

void CheckUpdateUnispy::OnDownLoadFinished(QNetworkReply *reply)
{
    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (statusCode.isValid())
    {
        qDebug() << "status code: " << statusCode.toInt();
    }

    QString temp_save_location = "/tmp/huayupy/";
    QDir dir(temp_save_location);
    if(!dir.exists())
    {
        dir.mkpath(temp_save_location);
    }


    QFile file(QString("%1%2").arg(temp_save_location).arg(m_package_file_name));
    if(!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "fileName: "<< m_package_file_name << " errorInfo: " << file.errorString();
        return;
    }
    file.write(reply->readAll());
    file.close();
    SendMsgToDaemon(HAS_NEW_VERSION);
}

bool CheckUpdateUnispy::GetNewVersionAndPathAndOs(QString& new_version, QString& new_version_down_path, QString& os_type)
{
    QString configPath = Config::GetSaveNewVersionInfoPath();
    if(!FileExists(configPath.toStdString()))
    {
        return false;
    }else
    {
        dictionary *dict = iniparser_load(configPath.toStdString().c_str());
        if (!dict)
              return false;
        new_version = iniparser_getstring(dict, "new_version_info:version", NOTFOUND);
        //new_version_down_path = iniparser_getstring(dict, "new_version_info:path", NOTFOUND);
        //QByteArray downloadPath = iniparser_getstring(dict, "new_version_info:path", NOTFOUND);
        const char *downloadPath = iniparser_getstring(dict, "new_version_info:path", NOTFOUND);
        new_version_down_path = QByteArray::fromPercentEncoding(downloadPath);
        os_type = iniparser_getstring(dict, "new_version_info:os_type", NOTFOUND);
        return true;
    }
}
