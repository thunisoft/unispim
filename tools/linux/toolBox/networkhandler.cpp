#include "networkhandler.h"
#include <QtNetwork>
#include <QByteArray>
#include <QImageIOHandler>
#include <QImage>
#include <QImageWriter>
#include <QSslConfiguration>
#include <QSslCertificate>
#include <QRegExp>
#include <QApplication>
#include "utils.h"
#include <QNetworkConfigurationManager>
#include <QHostInfo>
#include <QElapsedTimer>

using namespace std;
NetworkHandler::NetworkHandler(QObject* parent):
    QObject(parent)
{

}

NetworkHandler::~NetworkHandler()
{

}

bool NetworkHandler::IsNetworkConnected()
{
    try{
        QNetworkConfigurationManager networkConfig;
        if(!networkConfig.isOnline())
        {
            return false;
        }
        return true;
    }
    catch(exception& e)
    {
        Q_UNUSED(e);
        return false;
    }
}


void NetworkHandler::onLookupHost(QHostInfo hostInfo)
{
    if (hostInfo.error() != QHostInfo::NoError) {
        m_canVisitWeb = false;
        emit networkStatusResult(false);
    }
    else{
        m_canVisitWeb = true;
        emit networkStatusResult(true);
    }
}

void NetworkHandler::slot_handle_download_progress(qint64 receiveBytes, qint64 totalBytes)
{
    emit download_progress(receiveBytes,totalBytes);
}

int NetworkHandler::manu_check_tools_status(QString input_client, QString loginid, QString os_type, QString cpu_arch, QString addon_list,
                                            QVector<SEVER_ADDON_INFO>& server_addon_vector)
{

    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart loginIDPart;
    loginIDPart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"loginid\""));
    loginIDPart.setBody(loginid.toUtf8());
    multiPart->append(loginIDPart);

    QHttpPart clientIDPart;
    clientIDPart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"clientid\""));
    clientIDPart.setBody(input_client.toUtf8());
    multiPart->append(clientIDPart);

    QHttpPart fromOsPart;
    fromOsPart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"os\""));
    fromOsPart.setBody(os_type.toUtf8());
    multiPart->append(fromOsPart);

    QHttpPart cpuPart;
    cpuPart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"cpu\""));
    cpuPart.setBody(cpu_arch.toUtf8());
    multiPart->append(cpuPart);

    QHttpPart addonlistPart;
    addonlistPart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"addonlist\""));
    addonlistPart.setBody(addon_list.toUtf8());
    multiPart->append(addonlistPart);

    QNetworkRequest request;
    if(!GenerateRequest("/testapi/requesturl",request))
    {
        Utils::write_log_tofile("manucheckupdate get server addrinfo error");
        return 1;
    }

    QString certificate_path = Config::Instance()->GetHttpsCertificatePath();
    if(!SetSSlConfigToRequest(request,certificate_path))
    {
        Utils::write_log_tofile("manucheckupdate config ssl config error");
        return 2;
    }
    qDebug() << request.url().toString();

    //获取错误
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.post(request,multiPart);
    multiPart->setParent(reply);


    //添加了请求的超时操作
    QReplyTimeout *pTimeout = new QReplyTimeout(reply,15000);
    QEventLoop eventLoop;
    connect(&manager, SIGNAL(finished(QNetworkReply*)),&eventLoop, SLOT(quit()));
    connect(pTimeout, SIGNAL(timeout()),&eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    if(pTimeout->GetTimeoutFlag())
    {
        Utils::write_log_tofile("manucheckupdate request timeout");
        return 3;
    }

    if(reply->error() != QNetworkReply::NoError)
    {
        qDebug() << reply->error();
        Utils::write_log_tofile(QString("manucheckupdate  reply error:%1").arg(reply->error()));
        return reply->error();
    }

    QByteArray replyData = reply->readAll();
    QString statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString();
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(replyData, &json_error));

    if(json_error.error != QJsonParseError::NoError)
    {
        Utils::write_log_tofile(QString("manucheckupdate  reply json parser error:%1").arg(json_error.error));
        return -1;
    }
    QJsonObject rootObj = jsonDoc.object();
    if(!rootObj.contains("code") || !rootObj.contains("message"))
    {
        Utils::write_log_tofile(QString("返回的json数据不包含code和message"));
        return -2;
    }
    QString codeStr = rootObj.value("code").toString();
    QString messageStr = rootObj.value("message").toString();
    Utils::write_log_tofile(QString("reply info:code:%1 message:%2").arg(codeStr).arg(messageStr));

    if (codeStr.compare("200") == 0)
    {
        QJsonArray result_array = rootObj.value("result").toArray();
        int array_size = result_array.size();
        for(int index=0; index<array_size; ++index)
        {
            SEVER_ADDON_INFO server_addon_info;
            QJsonObject indexObject = result_array.at(index).toObject();
            server_addon_info.addon_download_url = indexObject.value("addr").toString();
            server_addon_info.addon_name = indexObject.value("addonname").toString();
            server_addon_info.addon_version = indexObject.value("version").toString();
            server_addon_info.file_name = indexObject.value("filename").toString();
            server_addon_info.file_md5 = indexObject.value("md5").toString();
            server_addon_info.addon_status = indexObject.value("status").toString();
            server_addon_vector.push_back(server_addon_info);
        }
        return 0;
    }
    else
    {
        return codeStr.toInt();
    }

}

int NetworkHandler::task_check_tools_status(QString input_client, QString loginid, QString os_type, QString cpu_arch, QString addon_list,
                                            QVector<SEVER_ADDON_INFO>& server_addon_vector)
{
    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart loginIDPart;
    loginIDPart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"loginid\""));
    loginIDPart.setBody(loginid.toUtf8());
    multiPart->append(loginIDPart);

    QHttpPart clientIDPart;
    clientIDPart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"clientid\""));
    clientIDPart.setBody(input_client.toUtf8());
    multiPart->append(clientIDPart);

    QHttpPart fromOsPart;
    fromOsPart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"os\""));
    fromOsPart.setBody(os_type.toUtf8());
    multiPart->append(fromOsPart);

    QHttpPart cpuPart;
    cpuPart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"cpu\""));
    cpuPart.setBody(cpu_arch.toUtf8());
    multiPart->append(cpuPart);

    QHttpPart addonlistPart;
    addonlistPart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"addonlist\""));
    addonlistPart.setBody(addon_list.toUtf8());
    multiPart->append(addonlistPart);

    QNetworkRequest request;
    if(!GenerateRequest("/testapi/requesturl",request))
    {
        Utils::write_log_tofile("manucheckupdate get server addrinfo error");
        return 1;
    }

    QString certificate_path = Config::Instance()->GetHttpsCertificatePath();
    if(!SetSSlConfigToRequest(request,certificate_path))
    {
        Utils::write_log_tofile("manucheckupdate config ssl config error");
        return 2;
    }
    qDebug() << request.url().toString();

    //获取错误
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.post(request,multiPart);
    multiPart->setParent(reply);


    //添加了请求的超时操作
    QReplyTimeout *pTimeout = new QReplyTimeout(reply,15000);
    QEventLoop eventLoop;
    connect(&manager, SIGNAL(finished(QNetworkReply*)),&eventLoop, SLOT(quit()));
    connect(pTimeout, SIGNAL(timeout()),&eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    if(pTimeout->GetTimeoutFlag())
    {
        Utils::write_log_tofile("manucheckupdate request timeout");
        return 3;
    }

    if(reply->error() != QNetworkReply::NoError)
    {
        qDebug() << reply->error();
        Utils::write_log_tofile(QString("manucheckupdate  reply error:%1").arg(reply->error()));
        return reply->error();
    }

    QByteArray replyData = reply->readAll();
    QString statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString();
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(replyData, &json_error));

    if(json_error.error != QJsonParseError::NoError)
    {
        Utils::write_log_tofile(QString("manucheckupdate  reply json parser error:%1").arg(json_error.error));
        return -1;
    }
    QJsonObject rootObj = jsonDoc.object();
    if(!rootObj.contains("code") || !rootObj.contains("message"))
    {
        Utils::write_log_tofile(QString("返回的json数据不包含code和message"));
        return -2;
    }
    QString codeStr = rootObj.value("code").toString();
    QString messageStr = rootObj.value("message").toString();
    Utils::write_log_tofile(QString("reply info:code:%1 message:%2").arg(codeStr).arg(messageStr));

    if (codeStr.compare("200") == 0)
    {
        QJsonArray result_array = rootObj.value("result").toArray();
        int array_size = result_array.size();
        for(int index=0; index<array_size; ++index)
        {
            SEVER_ADDON_INFO server_addon_info;
            QJsonObject indexObject = result_array.at(index).toObject();
            server_addon_info.addon_download_url = indexObject.value("addr").toString();
            server_addon_info.addon_name = indexObject.value("addonname").toString();
            server_addon_info.addon_version = indexObject.value("version").toString();
            server_addon_info.file_name = indexObject.value("filename").toString();
            server_addon_info.file_md5 = indexObject.value("md5").toString();
            server_addon_info.addon_status = indexObject.value("status").toString();
            server_addon_vector.push_back(server_addon_info);
        }
        return 0;
    }
    else
    {
        return codeStr.toInt();
    }
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



int NetworkHandler::download_tool_from_url(QString fileUrl, QString fileSavePath)
{
    //通过返回的请求头中获得文件的名称
    QNetworkRequest request;
    QUrl url(fileUrl);

    QString certificate_path = Config::Instance()->GetHttpsCertificatePath();
    if(!SetSSlConfigToRequest(request,certificate_path))
    {
        return 1;
    }

    QNetworkAccessManager *accessManager=new QNetworkAccessManager(this);
    request.setUrl(url);
    QNetworkReply *reply  = accessManager->get(request);

    connect(reply,SIGNAL(downloadProgress(qint64,qint64)),this,SLOT(slot_handle_download_progress(qint64,qint64)));

    QEventLoop loop;
    connect(accessManager, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
    loop.exec(QEventLoop::ExcludeUserInputEvents);

    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (statusCode.toInt() != 200)
    {
        Utils::write_log_tofile(QString("下载安装包返回值为code:%1").arg(statusCode.toInt()));
        return 2;
    }

    if(QFile::exists(fileSavePath))
    {
        QFile::remove(fileSavePath);
    }

    QFile file(fileSavePath);
    if(!file.open(QIODevice::WriteOnly))
    {
        Utils::write_log_tofile(QString("%1errorInfo:%2").arg(fileSavePath).arg(file.errorString()));
        return 4;
    }
    file.write(reply->readAll());
    file.close();
    return 0;
}

bool NetworkHandler::GenerateRequest(QString inputUrlAddr, QNetworkRequest& inputRequest)
{
    //生成对应的网址请求
#ifdef _WIN32
    QString requestHeader;
    if(!GetRequesetHeader(requestHeader))
    {
        Utils::write_log_tofile("配置文件中的服务器地址信息不完整");
        return false;
    }
#else
    QString requestHeader = Config::Instance()->GetLinuxServerDomain();
#endif
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
        Utils::write_log_tofile(QString("%1返回的json数据失败").arg(replyType));
        return -1;
    }
    QJsonObject rootObj = jsonDoc.object();
    if(!rootObj.contains("code") || !rootObj.contains("message"))
    {
        Utils::write_log_tofile(QString("%1返回的json数据不包含code和message").arg(replyType));
        return -2;
    }
    QString codeStr = rootObj.value("code").toString();
    QString messageStr = rootObj.value("message").toString();

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
        Utils::write_log_tofile(QString("%1返回值为code:%2").arg(replyType).arg(codeStr));
        return codeStr.toInt();
    }
}

int NetworkHandler::ParserRequestReplyStr(QNetworkReply*inpuRely,QString replyType, QJsonValue& returnedJson)
{
    QByteArray replyData = inpuRely->readAll();
    QString statusCode = inpuRely->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString();
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(replyData, &json_error));

    if(json_error.error != QJsonParseError::NoError)
    {
        Utils::write_log_tofile(QString("%1返回的json数据失败").arg(replyType));
        return -1;
    }
    QJsonObject rootObj = jsonDoc.object();
    if(!rootObj.contains("code") || !rootObj.contains("message"))
    {
        Utils::write_log_tofile(QString("%1返回的json数据不包含code和message").arg(replyType));
        return -2;
    }
    QString codeStr = rootObj.value("code").toString();
    QString messageStr = rootObj.value("message").toString();

    if (codeStr.compare("200") == 0)
    {
        //登录成功之后保存对应的配置信息
        if(rootObj.contains("result"))
        {
           returnedJson = rootObj.value("result");
        }
        return 0;
    }
    else
    {
        Utils::write_log_tofile(QString("%1返回值为code:%2").arg(replyType).arg(codeStr));
        return codeStr.toInt();
    }
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
