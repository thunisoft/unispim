// 用户词库下载

#include "userwordlibdownloader.h"

#include "../public/config.h"
#include <QSettings>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonDocument>
#include "configbus.h"
#include "../public/utils.h"

UserWordlibDownloader::UserWordlibDownloader(){}

UserWordlibDownloader::~UserWordlibDownloader(){}

QString UserWordlibDownloader::GetRequestUrl()
{

    QString clientid = Utils::GethostMac();
    QString url = Config::Instance()->GetServerUrl().append("/testapi/requesturl?")
                                     .append("loginid=")
                                     .append(GetRequestParam())
                                     .append("&clientid=")
                                     .append(clientid)
                                     .append("&fromos=")
                                     .append(QString::number(Utils::GetOSTypeIndex()));
    Utils::AddSystemExtraParamer(url,OPType::AUTO);
    return url;
}

QString UserWordlibDownloader::GetWordlibName()
{
    return "user.uwl";
}

QString UserWordlibDownloader::GetRequestParam()
{
    if(Config::Instance()->GetLoginId().isEmpty())
    {
        return Utils::GethostMac();
    }
    return Config::Instance()->GetLoginId();
}

bool UserWordlibDownloader::IsAutoSynchronize()
{
    QSettings config(Config::configFilePath(), QSettings::IniFormat);
    return config.value("account/userWordLib", 1).toInt();
}


bool UserWordlibDownloader::ParseQueryPathReply(const QByteArray& data, QString& wordlib_download_path)
{
    wordlib_download_path.clear();
    QJsonParseError json_error;
    QJsonDocument json_doc(QJsonDocument::fromJson(data, &json_error));
    if(json_error.error != QJsonParseError::NoError)
    {
        qDebug() << "Parse Wordlib [" << GetWordlibName() << "] Download Path Faild!";
        SetStatisUnworking();
        return false;
    }

    QJsonObject json_object_root = json_doc.object();
    if(!json_object_root.contains("code") || json_object_root.value("code").toString().compare("200") != 0)
    {
        SetStatisUnworking();
        return false;
    }
    if(!json_object_root.contains("result") || json_object_root.value("result").toString().isEmpty())
    {
        SetStatisUnworking();
        return false;
    }

    wordlib_download_path = json_object_root.value("result").toString();
    return true;
}

QString UserWordlibDownloader::GetWordlibSavePath()
{
    QString login_id = Config::Instance()->GetLoginId();
    if(login_id.isEmpty())
    {
        return WordlibDownloader::GetWordlibSavePath();
    }
    return WordlibDownloader::GetWordlibSavePath().append(Config::Instance()->GetLoginId()).append("/");
}

void UserWordlibDownloader::OnDownloadWordlibDone(QNetworkReply *reply)
{
    WordlibDownloader::OnDownloadWordlibDone(reply);
    ConfigBus::instance()->valueChanged("loadWordlib", "");
}

void UserWordlibDownloader::NotifyEngineUserLogin()
{
    QString login_id;
    if(Config::GetClientInfo(login_id) && !login_id.isEmpty())
        ConfigBus::instance()->valueChanged("login", QString(login_id.toUtf8()));
}
