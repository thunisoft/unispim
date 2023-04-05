#include "verififycodethread.h"
#include "../public/configmanager.h"
#include "../public/utils.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonParseError>
#include <QJsonArray>
#include <QProcess>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QEventLoop>


void VerifyCodeCheckThread::run()
{
    QNetworkRequest request;
    QString url = QString("%1/testapi/requesturl").arg(ConfigManager::Instance()->GetServerUrl());
    Utils::AddSystemExtraParamer(url,OPType::MANUAL);
    request.setUrl(QUrl(url));
    request.setSslConfiguration(Utils::GetQsslConfig());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QNetworkReply* reply = manager->get(request);
    QTimer timer;
    timer.setSingleShot(false);
    timer.setInterval(5000);
    timer.start();
    QEventLoop loop;
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if(timer.isActive())
    {
        timer.stop();
        if(reply->error() != QNetworkReply::TimeoutError)
        {

            QString statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString();
            if (!statusCode.isEmpty())
            {
                qDebug() << "status code: " << statusCode.toInt();
            }
            QByteArray replyData = reply->readAll();
            QJsonParseError json_error;
            QJsonDocument jsonDoc(QJsonDocument::fromJson(replyData, &json_error));

            if(json_error.error != QJsonParseError::NoError)
            {
                return;
            }

            QJsonObject rootObj = jsonDoc.object();
            if(!rootObj.contains("code") || !rootObj.contains("message"))
            {
                return;
            }
            QString codeStr = rootObj.value("code").toString();
            if (codeStr.compare("200") == 0)
            {
                bool flagValue = rootObj.value("result").toBool();
                if(flagValue)
                {
                    emit hasthemodules();
                }
            }
        }
    }
    else
    {
        disconnect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        reply->abort();
        reply->close();
        reply->deleteLater();
    }
}
