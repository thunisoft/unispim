#include "newversionchecker.h"
#include "../public/configmanager.h"
#include "../public/utils.h"
#include "../public/defines.h"
#include "wordlibpage/myutils.h"
#include <QSettings>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include "aboutstackedwidget.h"

NewVersionChecker::NewVersionChecker(QObject *parent) : QObject(parent)
{
    if(!m_network_manager)
    {
        m_network_manager = new QNetworkAccessManager(this);
    }
}

int NewVersionChecker::CheckTheVersion()
{
    try
    {
        QString request_url = GetVersionCheckUrl();
        QNetworkRequest request;
        request.setUrl(QUrl(request_url));
        request.setSslConfiguration(Utils::GetQsslConfig());

        QNetworkReply* reply = m_network_manager->get(request);
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        timer.setInterval(4000);
        timer.start();
        connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
        connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();

        if(timer.isActive())
        {
            timer.stop();
            OnCheckFinished(reply);
        }
        else
        {
            AboutStackedWidget::WriteLog("check new version timeout");
            emit checkError(REQUEST_TIME_OUT);
        }
        return 0;
    }
    catch(...)
    {
        qDebug() << "check the version throw exception";
        return 1;
    }

}

QString NewVersionChecker::GetVersionCheckUrl()
{
    //combine the request url
    QSettings config(TOOLS::PathUtils::GetUpdaterIniPath(), QSettings::IniFormat);
    config.beginGroup("package_info");
    QString version = config.value("version", "").toString();
    QString cpu_type = config.value("cpu_type", "").toString();
    QString os_type = config.value("os_type", "").toString();
    config.endGroup();

    QString request_url = ConfigManager::Instance()->GetServerUrl().append("/testapi/requesturl");
    return request_url;
}



void NewVersionChecker::OnCheckFinished(QNetworkReply *reply)
{
    QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (!status_code.isValid() || reply->error() != QNetworkReply::NoError)
    {
        AboutStackedWidget::WriteLog("check new version reply status_code error");
        emit checkError(UNKNOWN_ERROR);
        return;
    }
    AboutStackedWidget::WriteLog("version check finished");
    ParseCheckResult(reply->readAll(), m_new_version_url, m_new_version);
}

void NewVersionChecker::ParseCheckResult(const QByteArray& data, QString& new_version_download_path, QString& version)
{
    QJsonParseError json_error;
    QJsonDocument json_doc(QJsonDocument::fromJson(data, &json_error));
    if(json_error.error != QJsonParseError::NoError)
    {
        emit checkError(REUQEST_RETURN_CODE_ERROR);
        return;
    }

    QJsonObject json_object_root = json_doc.object();
    if(!json_object_root.contains("code") || json_object_root.value("code").toString().compare("200") != 0 || !json_object_root.contains("result"))
    {
        emit checkError(REUQEST_RETURN_CODE_ERROR);
        return;
    }

    QJsonObject json_object_result = json_object_root.value("result").toObject();
    if(!json_object_result.contains("update") || json_object_result.value("update").toString().compare("false") == 0)
    {
        emit checkSucceed(VERSION_UPTODATE);
        return;
    }
    else
    {
        if(!json_object_result.contains("version") || json_object_result.value("version").toString().isEmpty()
                ||!json_object_result.contains("addr") || json_object_result.value("addr").toString().isEmpty())
        {
            emit checkError(REUQEST_RETURN_CODE_ERROR);
            return;
        }
        m_new_version_url = json_object_result.value("addr").toString();
        m_new_version = json_object_result.value("version").toString();
        m_package_md5 = json_object_result.value("md5").toString();
        if(!m_new_version_url.isEmpty() && !m_new_version.isEmpty() && !m_package_md5.isEmpty())
        {
            AboutStackedWidget::WriteLog(QString("has new verison:%1").arg(m_new_version));
            emit checkSucceed(HAS_NEW_VERSION);
        }
    }
}
