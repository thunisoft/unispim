#include "networkhandler.h"
#include <QtNetwork>
#include <QByteArray>
#include <QTextCodec>
#include <QSslConfiguration>
#include <QSslCertificate>
#include <QCoreApplication>
#include "config.h"

NetworkHandler* NetworkHandler::m_ginstance = NULL;

NetworkHandler* NetworkHandler::Instance()
{
    if(m_ginstance == NULL)
    {
        m_ginstance = new NetworkHandler;
        return m_ginstance;
    }
    return m_ginstance;
}
NetworkHandler::NetworkHandler(QObject* parent):
    QObject(parent)
{

}

NetworkHandler::~NetworkHandler()
{

}

bool NetworkHandler::GetRequesetHeader(QString & requestHeader)
{
    ConfigItemStruct configInfo;
    QString scheme;
    QString serverAddr;
    QString port;

    if(Config::Instance()->GetConfigItemByJson("server_addr",configInfo))
    {
        serverAddr = configInfo.itemCurrentStrValue;
    }
    if(Config::Instance()->GetConfigItemByJson("server_port",configInfo))
    {
        port = QString::number(configInfo.itemCurrentIntValue);
    }
    if(Config::Instance()->GetConfigItemByJson("server_scheme",configInfo))
    {
        scheme = configInfo.itemCurrentStrValue;
    }
    if(scheme.isEmpty() || serverAddr.isEmpty() || port.isEmpty())
    {
        return false;
    }
    else
    {
        requestHeader = scheme + QString("://") + serverAddr + QString(":") + port;
        return true;
    }
}


bool NetworkHandler::GenerateRequest(QString inputUrlAddr, QNetworkRequest& inputRequest)
{
    //生成对应的网址请求
    QString requestHeader;
    if(!GetRequesetHeader(requestHeader))
    {
        return false;
    }
    QNetworkRequest request;
    request.setUrl(QUrl(requestHeader + inputUrlAddr));
    inputRequest = request;
    return true;
}

int NetworkHandler::ParserRequestReply(QNetworkReply*inpuRely,QString replyType, QJsonObject& returnedJson)
{
    QByteArray replyData = inpuRely->readAll();
    QString statusCode = inpuRely->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString();
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(replyData, &json_error));

    if(json_error.error != QJsonParseError::NoError)
    {
        return -1;
    }
    QJsonObject rootObj = jsonDoc.object();
    if(!rootObj.contains("code"))
    {
        return -2;
    }
    QString codeStr = rootObj.value("code").toString();

    if (codeStr.compare("200") == 0)
    {
        //登录成功之后保存对应的配置信息
        if(rootObj.contains("result"))
        {
           returnedJson = rootObj.value("result").toObject();
        }
        return 0;
    }
    else
    {
        return codeStr.toInt();
    }
}

int NetworkHandler::NotifySetup(QString clientID, QString osType, QString cpuType, QString version, int operation)
{
    //生成对应的网址请求
    QNetworkRequest request;
    if(!GenerateRequest("/testapi/requesturl",request))
    {
        return -1;
    }

    QString filePath = QDir::toNativeSeparators(QCoreApplication::applicationDirPath() + "\\certification.cer");
    if(!SetSSlConfigToRequest(request,filePath))
    {
        return -2;
    }

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QByteArray data;
    data.append("clientid=");
    data.append(clientID.toUtf8());
    data.append("&ostype=");
    data.append(osType.toUtf8());
    data.append("&cputype=");
    data.append(cpuType.toUtf8());
    data.append("&version=");
    data.append(version.toUtf8());
    data.append("&operation=");
    data.append(QString::number(operation).toUtf8());
    data.append("&fromos=");
    data.append(QString("1").toUtf8());

    string ostype = data.toStdString();

    //获取错误
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.post(request,data);

    QReplyTimeout *pTimeout = new QReplyTimeout(reply,5000);
    QEventLoop eventLoop;
    connect(reply, SIGNAL(finished()),&eventLoop, SLOT(quit()));
    connect(pTimeout, SIGNAL(timeout()),&eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    if(pTimeout->GetTimeoutFlag())
    {
        return 666;
    }

    if(reply->error() != QNetworkReply::NoError)
    {
        return reply->error();
    }

    QJsonObject resultObject;
    int returnCode = ParserRequestReply(reply,"downloadcustomPhrase",resultObject);

    return returnCode;
}

bool NetworkHandler::SetSSlConfigToRequest(QNetworkRequest &inputRequest, QString certificatePath)
{
    QSslConfiguration config;
    if(certificatePath.isEmpty())
    {
        config.setPeerVerifyMode(QSslSocket::VerifyNone);
        config.setProtocol(QSsl::TlsV1_0);

    }
    else
    {
        if(!QFile::exists(certificatePath))
        {
            return false;
        }
        QList<QSslCertificate> certs = QSslCertificate::fromPath(certificatePath,QSsl::Der);
        config.setCaCertificates(certs);
        config.setPeerVerifyMode(QSslSocket::VerifyNone);
        config.setProtocol(QSsl::TlsV1_2);
    }
    inputRequest.setSslConfiguration(config);
    return true;
}
