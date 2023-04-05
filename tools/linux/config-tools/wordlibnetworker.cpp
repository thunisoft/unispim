#include "wordlibnetworker.h"
#include "../public/config.h"
#include <QNetworkRequest>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValueRef>
#include <QJsonObject>
#include <QNetworkReply>
#include <QDebug>
#include <QCryptographicHash>
#include <QEventLoop>
#include "wordlibshowwidget.h"
#include "networkhandler.h"

WordlibNetworker::WordlibNetworker(QObject* parent):
    QObject(parent)
{
    Init();
}

void WordlibNetworker::Init()
{
    m_network_manager = new QNetworkAccessManager(this);
}

int WordlibNetworker::GetWordlibInfoFromServer()
{
    QString url = Config::Instance()->GetServerUrl().append("/testapi/requesturl");
    url.append("?fromos=");
    url.append(QString::number(Utils::GetOSTypeIndex()));
    Utils::AddSystemExtraParamer(url,OPType::MANUAL);
    QNetworkRequest request;
    request.setUrl(url);
    request.setSslConfiguration(Utils::GetQsslConfig());

    //添加了请求的超时操作
    QNetworkAccessManager network_manager;
    QEventLoop eventLoop;
    connect(&network_manager, SIGNAL(finished(QNetworkReply*)),&eventLoop, SLOT(quit()));
    QNetworkReply* reply = network_manager.get(request);
    QReplyTimeout *pTimeout = new QReplyTimeout(reply,5000);
    connect(pTimeout, SIGNAL(timeout()),&eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
    if(pTimeout->GetTimeoutFlag())
    {
        return 1;
    }

    QVariant http_request_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if(!http_request_code.isValid())
    {
        return 2;
    }
    if(http_request_code.toInt() != 200)
    {
        return 3;
    }
    bool is_parser_ok = PhraseJsonInfo(reply->readAll());
    if(is_parser_ok)
    {
        return 0;
    }
    return 4;
}

void WordlibNetworker::OnGetWordlibInfoDone(QNetworkReply *reply)
{
    disconnect(m_network_manager, SIGNAL(finished(QNetworkReply*)), 0, 0);
    QVariant http_request_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if(http_request_code.isValid())
    {
        qDebug() << "Get Server WordlibInfo Status Code: " << http_request_code.toInt();
    }
    if(http_request_code.toInt() != 200)
    {
        qDebug() << "http request error" << http_request_code.toInt();
        return;
    }
    PhraseJsonInfo(reply->readAll());
}

bool WordlibNetworker::PhraseJsonInfo(const QByteArray& data)
{
    m_wordlib_info.clear();
    QJsonParseError json_error;
    QJsonDocument json_doc(QJsonDocument::fromJson(data, &json_error));
    if(json_error.error != QJsonParseError::NoError)
    {
        return false;
    }
    QJsonArray json_array = json_doc.array();
    QJsonArray::iterator it = json_array.begin();
    for(; it != json_array.end(); it++)
    {
        WORDLIB_INFO wordlib_info;
        wordlib_info.is_selected = false;
        wordlib_info.is_local_file = false;
        wordlib_info.is_cloud_wordlib = true;
        if(!FillDataToWordlibInfoEntity(wordlib_info, *it))
            continue;
        m_wordlib_info.insert(wordlib_info.file_name, wordlib_info);
    }
    emit GetWordlibInfoDone();
    return true;
}

bool WordlibNetworker::FillDataToWordlibInfoEntity(WORDLIB_INFO& wordlib_info, QJsonValueRef json_value)
{
    QJsonObject wordlib_object = json_value.toObject();
    if(!wordlib_object.contains("fileName") || !wordlib_object.contains("viewName")
            || !wordlib_object.contains("downloadUrl") || !wordlib_object.contains("type")
            || !wordlib_object.contains("md5"))
        return false;
    wordlib_info.file_name = wordlib_object.value("fileName").toString();
    //从服务器上获得的文件名没有.uwl文件后缀，自行加上
    if(!wordlib_info.file_name.endsWith(".uwl"))
        wordlib_info.file_name.append(".uwl");
    //热词不在这里展示，直接后台更新
    if(wordlib_info.file_name.compare("syscloud.uwl") == 0)
        return false;
    wordlib_info.wordlib_name = wordlib_object.value("viewName").toString();
    wordlib_info.url = wordlib_object.value("downloadUrl").toString();
    wordlib_info.md5 = wordlib_object.value("md5").toString();
    return true;
}

QMap<QString, WORDLIB_INFO> WordlibNetworker::GetWordlibInfo()
{
    return m_wordlib_info;
}

void WordlibNetworker::DownloadWordlib(const WORDLIB_INFO& wordlib_info)
{
    m_one_wordlib_info = wordlib_info;
    QNetworkRequest request;
    request.setUrl(wordlib_info.url);
    request.setSslConfiguration(Utils::GetQsslConfig());
    //Utils::WriteLogToFile(QString("download url is %1").arg(wordlib_info.url));
    connect(m_network_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(OnDownloadWordlibDone(QNetworkReply*)));
    m_network_manager->get(request);    
}

void WordlibNetworker::OnDownloadWordlibDone(QNetworkReply* reply)
{
    disconnect(m_network_manager, SIGNAL(finished(QNetworkReply*)), 0, 0);

    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    Utils::WriteLogToFile(QString("download attribute code is %1").arg(statusCode.toInt()));
    if (!statusCode.isValid() || reply->error() != QNetworkReply::NoError)
    {
        Utils::WriteLogToFile(QString("statusCode is %1 ,replyError is:%2").arg(statusCode.toInt()).arg(reply->error()));
        return;
    }
    Utils::MakeDir(Utils::GetTmpDir());
    QFile file(Utils::GetTmpDir().append(m_one_wordlib_info.file_name));
    if(!file.open(QIODevice::WriteOnly))
    {
        Utils::WriteLogToFile(QString("open file error:%1").arg(Utils::GetTmpDir().append(m_one_wordlib_info.file_name)));
        qDebug() << "fileName: "<< file.fileName() << " errorInfo: " << file.errorString();
        SendSignal(Failed);
        return;
    }
    file.write(reply->readAll());
    file.close();

    if(!VerifyMD5())
    {
        Utils::WriteLogToFile(QString("verifyMd5 error"));
        qDebug() << QString::fromLocal8Bit("MD5值校验错误！");
        SendSignal(Failed);
        return;
    }
    Utils::WriteLogToFile(QString("download succeed"));
    SendSignal(Ok);
}

bool WordlibNetworker::VerifyMD5()
{

    QFile file(Utils::GetTmpDir().append(m_one_wordlib_info.file_name));
    file.open(QFile::ReadOnly);
    QByteArray bytes = QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5);
    file.close();
    QString local_md5 = QString(bytes.toHex().constData()).toLower();
    Utils::WriteLogToFile(QString("localmd5:%1,servermd5:%2").arg(local_md5).arg(m_one_wordlib_info.md5));
    if(local_md5 == m_one_wordlib_info.md5)
        return true;
    return false;
}

void WordlibNetworker::SendSignal(int state)
{
    emit WordlibDownloadDone(state);
}

void WordlibNetworker::ClearUserWordlib(const QString& user_id)
{
    QString url = Config::Instance()->GetServerUrl().append("/testapi/requesturl");
    Utils::AddSystemExtraParamer(url,OPType::MANUAL);
    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setUrl(url);
    QByteArray param;
    param.append("loginId=").append(user_id);
    param.append("&fromos=").append(QString::number(Utils::GetOSTypeIndex()));
    request.setSslConfiguration(Utils::GetQsslConfig());
    m_network_manager->post(request, param);
}
