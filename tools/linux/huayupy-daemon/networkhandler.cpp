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
#include "../public/config.h"
#include "../public/utils.h"
#include <QNetworkConfigurationManager>
#include <QHostInfo>

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
NetworkHandler::NetworkHandler(QWidget* parent):
    QWidget(parent)
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
        qDebug() << e.what();
        return false;
    }
}

bool NetworkHandler::canVisitWeb()
{

    return true;
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

bool NetworkHandler::GetRequesetHeader(QString & requestHeader)
{
    requestHeader = Config::Instance()->GetServerUrl();
    return true;
}


int NetworkHandler::get_profess_wordlib_list(QVector<WordlibListInfo>& wordlib_name_list)
{
    //生成对应的网址请求
    QNetworkRequest request;
    QString subwebIndex = QString("/testapi/requesturl");
    Utils::AddSystemExtraParamer(subwebIndex,OPType::AUTO);

    if(!GenerateRequest(subwebIndex,request))
    {
        return -1;
    }

    request.setSslConfiguration(Utils::GetQsslConfig());

    //获取错误
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.get(request);

    //添加了请求的超时操作
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
        Utils::WriteLogToFile(QString("get profess wordlib list error:%1").arg(reply->error()));
        return reply->error();
    }

    QByteArray replyData = reply->readAll();
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(replyData, &json_error));

    if(json_error.error != QJsonParseError::NoError)
    {
        Utils::WriteLogToFile(QString("%1返回的json数据失败").arg("get profess wordlib list"));
        return -1;
    }
    QJsonArray jsonArray = jsonDoc.array();
    int json_array_size = jsonArray.size();

    for(int index=0; index < json_array_size; ++index){
        QJsonObject indexObject = jsonArray.at(index).toObject();
        if(indexObject.contains("fileName")&&indexObject.contains("viewName")){
            QString fileName = indexObject.value("fileName").toString().append(".uwl");
            QString viewName = indexObject.value("viewName").toString();
            QString url = indexObject.value("downloadUrl").toString();
            QString type = indexObject.value("type").toString();
            QString md5 = indexObject.value("md5").toString();
            bool is_auto_publish = false;
            if(indexObject.contains("autopublish"))
            {
                QString auto_publish = indexObject.value("autopublish").toString();
                if(auto_publish.compare("true") == 0)
                {
                    is_auto_publish = 1;
                }
            }
            wordlib_name_list.push_back(WordlibListInfo(fileName,viewName,url,type,md5,is_auto_publish));
        }
    }
    return 0;
}

int NetworkHandler::test_network_connection(QString server_domain,QString version, int os_index, QString clinetid,QString timestamp)
{
    QString loginid;
    QNetworkAccessManager *uploadmanager = new QNetworkAccessManager(this);
    QString url = QString("%1/testapi/requesturl").arg(server_domain);
    qDebug() << url;

    QNetworkRequest request;
    request.setUrl((QUrl(url)));
    request.setSslConfiguration(Utils::GetQsslConfig());
    QNetworkReply *reply = uploadmanager->get(request);

    QReplyTimeout* timeout_watcher = new QReplyTimeout(reply,600000);
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    connect(timeout_watcher,SIGNAL(timeout()),&loop,SLOT(quit()));
    loop.exec();
    if(timeout_watcher->GetTimeoutFlag())
    {
        return 1;
    }

    QNetworkReply::NetworkError error = reply->error();

    if(QNetworkReply::NoError != error)
    {
        return 2;
    }

    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (statusCode.isValid())
    {
        if(statusCode.toInt() == 200)
        {
            return 0;
        }
    }
    return 3;
}

int NetworkHandler::download_full_sys_wordlib(QString download_url,QString packge_md5)
{
    QNetworkRequest request;
    request.setUrl(download_url);
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

    QString wordlib_path = Config::Instance()->wordLibDirPath(WLDPL_CONFIG);
    QDir dir(wordlib_path);
    if(!dir.exists())
    {
        dir.mkpath(wordlib_path);
    }

    if(reply->error() != QNetworkReply::NoError)
    {
        return 2;
    }

    const QString wordlib_absolute_path = wordlib_path + "sys.uwl";
    QString tmp_wordlib_path = wordlib_absolute_path +".tmp";
    QString bak_wordlib_path = wordlib_absolute_path +".bak";
    QFile file_tmp(tmp_wordlib_path);
    if(file_tmp.exists())
    {
        file_tmp.remove();
    }
    if(file_tmp.open(QIODevice::ReadWrite))
    {
        file_tmp.write(reply->readAll());
        file_tmp.close();
        if(file_tmp.size() <= 0)
        {
            return 99;
        }
    }

    QString tmp_file_md5 = Utils::GetFileMD5(tmp_wordlib_path);
    if(tmp_file_md5 != packge_md5)
    {
        QFile::remove(tmp_wordlib_path);
        return 10;
    }


    QFile file_bak(bak_wordlib_path);
    if(file_bak.exists())
    {
        file_bak.remove();
    }
    QFile file_org(wordlib_absolute_path);
    if(file_org.exists())
    {
        file_org.rename(bak_wordlib_path);
    }
    file_tmp.rename(wordlib_absolute_path);
    return 0;
}

int NetworkHandler::check_system_wordlib_update(QString wordlib_md5,QString& download_url,QString& download_file_md5)
{
    QString url = Config::Instance()->GetServerUrl().append("/testapi/requesturl?");
    Utils::AddSystemExtraParamer(url,OPType::AUTO);
    QNetworkRequest request;
    request.setUrl(url);
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
    if(reply->error() != QNetworkReply::NoError)
    {
        return 2;
    }

    QByteArray data = reply->readAll();
    QJsonParseError json_error;
    QJsonDocument json_doc(QJsonDocument::fromJson(data, &json_error));
    if(json_error.error != QJsonParseError::NoError)
    {
        return 3;
    }

    QJsonObject json_object_root = json_doc.object();

    QJsonObject json_result_object = json_object_root.value("result").toObject();
    QString has_update = json_result_object.value("update").toString();
    if(has_update == "true")
    {
        QString setupid = json_result_object.value("id").toString();
        QString url = Config::Instance()->GetServerUrl().append("/testapi/requesturl");

        QString addr_type = json_result_object.value("addrtype").toString();
        if(addr_type == "cdn")
        {
            QString cdn_download_url = json_result_object.value("addr").toString();
            download_url = cdn_download_url;
        }
        else
        {
            download_url = url;
        }
        Utils::AddSystemExtraParamer(download_url,OPType::AUTO);
        download_file_md5 = json_result_object.value("md5").toString();
        return 0;
    }
    else if(has_update == "false")
    {
        return 0;
    }
    else
    {
        return 99;
    }

}

int NetworkHandler::DownloadFileFromWeb(QString fileUrl,QString fileSaveDir,QString fileSavePath,QString fileName)
{   
    //通过返回的请求头中获得文件的名称
    QNetworkRequest request;
    QUrl url(fileUrl);

    request.setSslConfiguration(Utils::GetQsslConfig());

    QNetworkAccessManager *accessManager=new QNetworkAccessManager(this);
    request.setUrl(url);
    QNetworkReply *reply  = accessManager->get(request);

    QEventLoop loop;
    connect(accessManager, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
    loop.exec(QEventLoop::ExcludeUserInputEvents);

    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (statusCode.toInt() != 200)
    {
        Utils::WriteLogToFile(QString("下载安装包返回值为code:%1").arg(statusCode.toInt()));
        return -1;
    }

    //get file name
    QString contentHeader = reply->rawHeader("Content-Disposition");
    QStringList subStrList = contentHeader.split("=");
    if(subStrList.size() != 2)
    {
        return -3;
    }
    if(fileName.isEmpty())
    {
        fileName = subStrList.at(1);
    }

    fileSavePath = fileSaveDir + fileName;
    QString fileTmpSavePath = fileSavePath + ".tmp.tools";
    if(QFile::exists(fileTmpSavePath))
    {
        QFile::remove(fileTmpSavePath);
    }

    QFile file(fileTmpSavePath);
    if(!file.open(QIODevice::WriteOnly))
    {
        Utils::WriteLogToFile(QString("%1errorInfo:%2").arg(fileName).arg(file.errorString()));
        return -2;
    }
    file.write(reply->readAll());
    file.close();

    QString fileBakSavePath = fileSavePath + ".bak.tools";
    if(QFile::exists(fileBakSavePath))
    {
        QFile::remove(fileBakSavePath);
    }
    if(QFile::exists(fileSavePath))
    {
        QFile::rename(fileSavePath,fileBakSavePath);
    }
    QFile::rename(fileTmpSavePath,fileSavePath);

    return 0;
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


int NetworkHandler::ParserRequestReply(QNetworkReply*inpuRely,QString replyType, QJsonObject& returnedJson)
{
    QByteArray replyData = inpuRely->readAll();
    QString statusCode = inpuRely->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString();
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(replyData, &json_error));

    if(json_error.error != QJsonParseError::NoError)
    {
        Utils::WriteLogToFile(QString("%1返回的json数据失败").arg(replyType));
        return -1;
    }
    QJsonObject rootObj = jsonDoc.object();
    if(!rootObj.contains("code") || !rootObj.contains("message"))
    {
        Utils::WriteLogToFile(QString("%1返回的json数据不包含code和message").arg(replyType));
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
        Utils::WriteLogToFile(QString("%1返回值为code:%2").arg(replyType).arg(codeStr));
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
        Utils::WriteLogToFile(QString("%1返回的json数据失败").arg(replyType));
        return -1;
    }
    QJsonObject rootObj = jsonDoc.object();
    if(!rootObj.contains("code") || !rootObj.contains("message"))
    {
        Utils::WriteLogToFile(QString("%1返回的json数据不包含code和message").arg(replyType));
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
        Utils::WriteLogToFile(QString("%1返回值为code:%2").arg(replyType).arg(codeStr));
        return codeStr.toInt();
    }
}

bool NetworkHandler::GenerateRequest(QString inputUrlAddr, QNetworkRequest& inputRequest)
{
    //生成对应的网址请求
    QString requestHeader;
    if(!GetRequesetHeader(requestHeader))
    {
        Utils::WriteLogToFile("配置文件中的服务器地址信息不完整");
        return false;
    }
    QNetworkRequest request;
    Utils::AddSystemExtraParamer(inputUrlAddr,OPType::AUTO);
    request.setUrl(QUrl(requestHeader + inputUrlAddr));
    inputRequest = request;
    return true;
}

int NetworkHandler::DownloadSysWordlibBlockFromServer(QString clientid, QString loginid, QVector<WORDLIB_BLOCK_INFO> block_vector)
{
    try {

        //根据下载类型拼接请求字符串
        QString blockList;
        for(int index=0; index<block_vector.size(); ++index)
        {
            QString block_addr_type = block_vector.at(index).addr_type;
            if(block_addr_type.isEmpty() || (block_addr_type == "server"))
            {
                if(index==0)
                {
                    blockList += QString::number(block_vector.at(index).block_id);
                }
                else
                {
                    blockList += ",";
                    blockList += QString::number(block_vector.at(index).block_id);
                }
            }
        }


        QString current_version = Utils::GetVersion();
        //生成对应的网址请求
        QNetworkRequest request;
        QString subUrl =QString("/testapi/requesturl");
        if(!GenerateRequest(subUrl,request))
        {
            return 6;
        }
        QString full_url = request.url().toString();
        qDebug() << full_url;

        request.setSslConfiguration(Utils::GetQsslConfig());
        //获取错误
        QNetworkAccessManager manager;
        QNetworkReply *reply = manager.get(request);

        //添加了请求的超时操作
        QReplyTimeout *pTimeout = new QReplyTimeout(reply,5000);
        QEventLoop eventLoop;
        connect(reply, SIGNAL(finished()),&eventLoop, SLOT(quit()));
        connect(pTimeout, SIGNAL(timeout()),&eventLoop, SLOT(quit()));
        eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
        if(pTimeout->GetTimeoutFlag())
        {
            return 1;
        }


        if(reply->error() != QNetworkReply::NoError)
        {
            return 2;
        }

        Utils::MakeDir(Utils::GetTmpDir());
        const QString zip_file_path = Utils::GetTmpDir().append("tmp.zip");
        QFile file_tmp(zip_file_path);
        if(file_tmp.exists())
        {
            file_tmp.remove();
        }
        if(!file_tmp.open(QIODevice::WriteOnly))
        {
            return 99;
        }
        file_tmp.write(reply->readAll());
        file_tmp.close();
        return 0;

    }
    catch (std::exception& e)
    {
        Q_UNUSED(e);
        return 5;
    }
}

int NetworkHandler::GetCurrentUserUserwordlibMD5(const QString &loginid, const QString &clientid, QString &md5_value, QString &download_url, QString &perv_clientid)
{
    //生成对应的网址请求
    QString requestType = QString("/testapi/requesturl");
    Utils::AddSystemExtraParamer(requestType,OPType::AUTO);

    QNetworkRequest request;
    if(!GenerateRequest(requestType,request))
    {
        return 99;
    }

    request.setSslConfiguration(Utils::GetQsslConfig());

    //获取信息
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.get(request);

    //添加了请求的超时操作
    QReplyTimeout *pTimeout = new QReplyTimeout(reply,10000);
    QEventLoop eventLoop;
    connect(reply, SIGNAL(finished()),&eventLoop, SLOT(quit()));
    connect(pTimeout, SIGNAL(timeout()),&eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    if(pTimeout->GetTimeoutFlag())
    {
        return 1;
    }

    if(reply->error() != QNetworkReply::NoError)
    {
        return 2;
    }

    QByteArray replyData = reply->readAll();
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(replyData, &json_error));

    if(json_error.error != QJsonParseError::NoError)
    {
        return 3;
    }
    QJsonObject rootObj = jsonDoc.object();
    if(!rootObj.contains("code") || !rootObj.contains("message"))
    {
        return 4;
    }
    QString codeStr = rootObj.value("code").toString();
    QString messageStr = rootObj.value("message").toString();

    if (codeStr.compare("200") == 0)
    {
        //登录成功之后保存对应的配置信息
        if(!rootObj.contains("newinfo"))
        {
            return 5;
        }
        QJsonObject newinfo_obj = rootObj.value("newinfo").toObject();
        md5_value = newinfo_obj.value("md5").toString();
        download_url = newinfo_obj.value("addr").toString();
        perv_clientid = newinfo_obj.value("clientid").toString();
        return 0;
    }
    else
    {
        return 5;
    }

}

//词库上传
int NetworkHandler::UploadCustomWordlib(QString loginid, QString clientid, QString dictFilePath, QString& md5Value)
{
    if(!QFile::exists(dictFilePath) || clientid.isEmpty())
    {
        return 9;
    }

    //生成对应的网址请求
    QFileInfo uwlFileInfo(dictFilePath);
    QString uwlFileName =uwlFileInfo.fileName();

    QFile* uwlFile = new QFile(dictFilePath);
    uwlFile->open(QIODevice::ReadOnly);

    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart uwlFilePart;
    uwlFilePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                                 QVariant(QString("form-data; name=\"dict\";filename=\"%1\"").arg(uwlFileName)));
    uwlFilePart.setBodyDevice(uwlFile);
    uwlFile->setParent(multiPart);
    multiPart->append(uwlFilePart);

    QHttpPart loginIDPart;
    loginIDPart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"loginid\""));
    loginIDPart.setBody(loginid.toUtf8());
    multiPart->append(loginIDPart);

    QHttpPart clientIDPart;
    clientIDPart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"clientid\""));
    clientIDPart.setBody(clientid.toUtf8());
    multiPart->append(clientIDPart);

    QHttpPart fromOsPart;
    fromOsPart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"fromos\""));
    fromOsPart.setBody(QString::number(Utils::GetOSTypeIndex()).toUtf8());
    multiPart->append(fromOsPart);

    QNetworkRequest request;

    QString url = QString("/testapi/requesturl");

    if(!GenerateRequest(url,request))
    {
        return 99;
    }

    request.setSslConfiguration(Utils::GetQsslConfig());

    //获取错误
    QNetworkAccessManager* manager = new QNetworkAccessManager();
    QNetworkReply *reply = manager->post(request,multiPart);
    multiPart->setParent(reply);

    //添加了请求的超时操作
    QReplyTimeout *pTimeout = new QReplyTimeout(reply,5000);
    QEventLoop eventLoop;
    connect(manager, SIGNAL(finished(QNetworkReply*)),&eventLoop, SLOT(quit()));
    connect(pTimeout, SIGNAL(timeout()),&eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
    if(pTimeout->GetTimeoutFlag())
    {
        return 1;
    }

    int error_code = reply->error();

    if(error_code != QNetworkReply::NoError)
    {
        return 2;
    }

    QJsonObject resultObject;
    int parserCode = ParserRequestReply(reply,"上传用户词库",resultObject);

    if(parserCode == 0)
    {
        md5Value = resultObject.value("md5").toString();
    }
    manager->deleteLater();
    return parserCode;
}

int NetworkHandler::DownloadFileFromWeb(QString fileUrl, QString fileSavePath)
{
    //通过返回的请求头中获得文件的名称
    QNetworkRequest request;
    QUrl url(fileUrl);

    request.setSslConfiguration(Utils::GetQsslConfig());

    QNetworkAccessManager *accessManager=new QNetworkAccessManager(this);
    request.setUrl(url);
    QNetworkReply *reply  = accessManager->get(request);

    QEventLoop loop;
    connect(accessManager, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
    loop.exec(QEventLoop::ExcludeUserInputEvents);

    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (statusCode.toInt() != 200)
    {
        return 2;
    }

    QString scheme = "https";
    ConfigItemStruct configInfo;
    if(Config::Instance()->GetConfigItemByJson("server_scheme",configInfo))
    {
        scheme = configInfo.itemCurrentStrValue;
    }
    QFileInfo file_info(fileSavePath);
    QString fileName = file_info.fileName();

    //判断返回的请求是minIO还是字节流
    QRegExp urlExp(QString("^%1://.*\\?.*=.*").arg(scheme));
    if(urlExp.exactMatch(fileUrl))
    {
        QString contentHeader = reply->rawHeader("Content-Disposition");
        QStringList subStrList = contentHeader.split("=");
        if(subStrList.size() != 2)
        {
            return 3;
        }
        if(fileName.isEmpty())
        {
            fileName = subStrList.at(1);
        }
    }

    if(QFile::exists(fileSavePath))
    {
        QFile::remove(fileSavePath);
    }


    QFile file(fileSavePath);
    if(!file.open(QIODevice::WriteOnly))
    {
        Utils::WriteLogToFile(QString("%1errorInfo:%2").arg(fileName).arg(file.errorString()));
        return 4;
    }
    file.write(reply->readAll());
    file.close();
    return 0;
}

