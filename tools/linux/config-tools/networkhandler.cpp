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
#include "../public/utils.h"
#include "../public/configmanager.h"
#include "wordlibpage/myutils.h"
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
    catch(std::exception& e)
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
     requestHeader = ConfigManager::Instance()->GetServerUrl();
     return true;
}

int NetworkHandler::UserAccountLogin(const QString account, const QString password)
{

    return 0;
}

int NetworkHandler::UserPhoneLogin(const QString phoneNum, const QString checkNum)
{

    return 0;
}

int NetworkHandler::RegisterUserAccount(const QString &account, const QString password)
{
    return 0;
}

int NetworkHandler::CheckNewVersion(bool &has_newversion, QString &version_num)
{
    try
    {
        QString request_url = GetVersionCheckUrl();
        QNetworkRequest request;
        request.setUrl(QUrl(request_url));
        request.setSslConfiguration(Utils::GetQsslConfig());

        QNetworkAccessManager network_manager;
        QNetworkReply* reply = network_manager.get(request);

        QReplyTimeout *pTimeout = new QReplyTimeout(reply,5000);
        QEventLoop eventLoop;
        connect(reply, SIGNAL(finished()),&eventLoop, SLOT(quit()));
        connect(pTimeout, SIGNAL(timeout()),&eventLoop, SLOT(quit()));
        eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

        if(pTimeout->GetTimeoutFlag())
        {
            return 1;
        }
        QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if (!status_code.isValid() || reply->error() != QNetworkReply::NoError)
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
        if(!json_object_root.contains("result"))
        {
            return 4;
        }
        QJsonObject result_object = json_object_root.value("result").toObject();

        if(result_object.contains("update") && result_object.value("update").toString().compare("true") == 0)
        {
            has_newversion = true;
        }

        if(result_object.contains("version"))
        {
            version_num = result_object.value("version").toString();
        }
        return 0;

    }
    catch(std::exception& e)
    {
        qDebug() << "check the version throw exception" << e.what();
        return 4;
    }
}

int NetworkHandler::GetVerificationCode(QString phoneNum)
{
    return 0;
}

int NetworkHandler::getTmpFile(QString fileUrl, QString fileName,QString fileSaveDir, QString &fileSavePath ,QString &fileTmpSavePath)
{
    try {
        QNetworkRequest request;
        QUrl url(fileUrl);

        QString scheme = url.scheme();
        //request.setSslConfiguration(Utils::GetQsslConfig());

        QNetworkAccessManager accessManager;
        request.setUrl(url);
        QNetworkReply *reply  = accessManager.get(request);

        QEventLoop loop;
        connect(&accessManager, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
        loop.exec(QEventLoop::ExcludeUserInputEvents);

        QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if (statusCode.toInt() != 200)
        {
            return -1;
        }

        //通过返回的请求头中获得文件的名称


        //判断返回的请求是minIO还是字节流
        QRegExp urlExp(QString("^%1://.*\\?.*=.*").arg(scheme));
        if(urlExp.exactMatch(fileUrl))
        {
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
        }
        else
        {
            if(fileName.isEmpty())
            {
                QFileInfo urlFileInfo(fileUrl);
                fileName = urlFileInfo.fileName();
            }
        }

        fileSavePath = QDir::toNativeSeparators(fileSaveDir + "//" + fileName);
        fileTmpSavePath = QDir::toNativeSeparators(fileSavePath + ".tmp.tools");
        if(QFile::exists(fileTmpSavePath))
        {
            QFile::remove(fileTmpSavePath);
        }


        QFile file(fileTmpSavePath);
        if(!file.open(QIODevice::WriteOnly))
        {
            return -2;
        }
        file.write(reply->readAll());
        file.close();
    } catch (std::exception& e) {

        return -3;
    }
}

int NetworkHandler::DownloadUserConfigFile(QString loginid,
                                           QString& serverStamp, bool & hasUpdate,
                                           QString& cfgfileAddr)
{

    return 0;
}

int NetworkHandler::get_profess_wordlib_list(QVector<WordlibListInfo>& wordlib_name_list)
{
    //生成对应的网址请求
    QNetworkRequest request;
    QString subwebIndex = QString("/testapi/requesturl");
    if(!GenerateRequest(subwebIndex,request))
    {
        return -1;
    }

    if(!SetSSlConfigToRequest(request,":/server_certification.cer"))
    {
        return -2;
    }

    //获取错误
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.get(request);

    //添加了请求的超时操作
    QReplyTimeout *pTimeout = new QReplyTimeout(reply,6000);
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
            wordlib_name_list.push_back(WordlibListInfo(fileName,viewName,url,type,md5));
        }
    }
    return 0;
}

int NetworkHandler::DownloadFileFromWeb(QString fileUrl,QString fileSaveDir,QString& fileSavePath,QString fileName)
{
    //下载服务端用户词库
    QString fileTmpSavePath;
    getTmpFile(fileUrl,fileName,fileSaveDir,fileSavePath,fileTmpSavePath);

    QString fileBakSavePath = QDir::toNativeSeparators(fileSavePath + ".bak.tools");
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

int NetworkHandler::DownloadMergerFile(QString fileUrl, QString fileSaveDir, QString &fileSavePath, QString fileName)
{
    QString fileTmpSavePath;
    //下载服务端用户词库
    getTmpFile(fileUrl,fileName,fileSaveDir,fileSavePath,fileTmpSavePath);

    FILE* ptr2 = NULL;
    QString uwl_path = Utils::GetTmpDir().append("*.uwl");
    std::string command = QString(" rm -r %1").arg(uwl_path).toStdString();
    if((ptr2 = popen(command.c_str(), "r")) != NULL)
    {
        pclose(ptr2);
    }

    auto CopyWordlibToTmp = [&](QString usrUwlbPath,QString tmpUwlPath) ->bool {
        QFile sys_uwl(usrUwlbPath);
        if(!sys_uwl.exists())
            return false;
        if(!sys_uwl.copy(tmpUwlPath))
            return false;
        return true;
    };

    QString user_tmp_tools_path = Utils::GetTmpDir().append("user.tmp.tools");
    QString user_tmp_path = Utils::GetTmpDir().append("user.uwl");

    if(QFile::exists(user_tmp_tools_path))
    {
        QFile::remove(user_tmp_tools_path);
    }

    if(!CopyWordlibToTmp(fileTmpSavePath,user_tmp_tools_path))
        return 2;
    if(!CopyWordlibToTmp(fileSavePath,user_tmp_path))
        return 2;

    QStringList params;
    params << QString(user_tmp_path);
    params << QString(Utils::GetTmpDir().append("user.tmp.tools"));
    params << QString(user_tmp_path);

    //调用合并工具合并用户词库
     QProcess* _process = new QProcess();
    _process->execute(Utils::GetBinFilePath().append("huayupy-merge-wordlib"), params);

    QString fileBakSavePath = QDir::toNativeSeparators(fileSavePath + ".bak.tools");
    if(QFile::exists(fileBakSavePath))
    {
        QFile::remove(fileBakSavePath);
    }
    if(QFile::exists(fileSavePath))
    {
        QFile::rename(fileSavePath,fileBakSavePath);
    }
    if(QFile::exists(fileTmpSavePath))
        QFile::remove(fileTmpSavePath);

    if(!CopyWordlibToTmp(user_tmp_path,fileSavePath))
        return 2;
    //删除临时目录文件
    if(QFile::exists(user_tmp_tools_path))
        QFile::remove(user_tmp_tools_path);
    if(QFile::exists(user_tmp_path))
        QFile::remove(user_tmp_path);

    return 0;
}

int NetworkHandler::UploadUserConfigFile(QString loginid, QString timeStamp,QString configJsonPath)
{
    //生成对应的网址请求
    QFileInfo configJsonFileInfo(configJsonPath);
    QString configJsonfileName =configJsonFileInfo.fileName();

    QFile* jsonFile = new QFile(configJsonPath);
    jsonFile->open(QIODevice::ReadOnly);

    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart configJsonFilePart;
    configJsonFilePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                                 QVariant(QString("form-data; name=\"configFile\";filename=\"%1\"").arg(configJsonfileName)));
    configJsonFilePart.setBodyDevice(jsonFile);
    jsonFile->setParent(multiPart);
    multiPart->append(configJsonFilePart);

    QHttpPart loginIDPart;
    loginIDPart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"loginid\""));
    loginIDPart.setBody(loginid.toUtf8());
    multiPart->append(loginIDPart);

    QHttpPart timeStampPart;
    timeStampPart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"time\""));
    timeStampPart.setBody(timeStamp.toUtf8());
    multiPart->append(timeStampPart);

    QHttpPart fromOsPart;
    fromOsPart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"fromos\""));
    fromOsPart.setBody(QString::number(Utils::GetOSTypeIndex()).toUtf8());
    multiPart->append(fromOsPart);

    QNetworkRequest request;
    if(!GenerateRequest("",request))
    {
        return -1;
    }

    QString certificate_path = "";
    if(!SetSSlConfigToRequest(request,certificate_path))
    {
        return -2;
    }

    //获取错误
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.post(request,multiPart);
    multiPart->setParent(reply);

    //添加了请求的超时操作
    QReplyTimeout *pTimeout = new QReplyTimeout(reply,5000);
    QEventLoop eventLoop;
    connect(&manager, SIGNAL(finished(QNetworkReply*)),&eventLoop, SLOT(quit()));
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
    return ParserRequestReply(reply,"上传用户配置文件",resultObject);
}

//词库上传
int NetworkHandler::UploadCustomWordlib(QString loginid, QString clientid, QString dictFilePath, QString& md5Value,int wordlib_count)
{
    if(!QFile::exists(dictFilePath) || clientid.isEmpty())
    {
        return -1;
    }

    //生成对应的网址请求
    QFileInfo uwlFileInfo(dictFilePath);
    QString uwlFileName =uwlFileInfo.fileName();

    QFile* uwlFile = new QFile(dictFilePath);
    uwlFile->open(QIODevice::ReadOnly);

    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);


    QHttpPart uwlFilePart;
    uwlFilePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    uwlFilePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                                 QVariant(QString("form-data; name=\"dict\";filename=\"%1\"").arg(uwlFileName)));
    uwlFilePart.setBodyDevice(uwlFile);
    uwlFile->setParent(multiPart);
    multiPart->append(uwlFilePart);

    QHttpPart loginIDPart;
    loginIDPart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"loginid\""));
    loginIDPart.setBody(loginid.toUtf8());
    multiPart->append(loginIDPart);

    QHttpPart wordCountPart;
    wordCountPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"wordCount\""));
    wordCountPart.setBody(QString::number(wordlib_count).toUtf8());
    multiPart->append(wordCountPart);

    QHttpPart clientIDPart;
    clientIDPart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"clientid\""));
    clientIDPart.setBody(clientid.toUtf8());
    multiPart->append(clientIDPart);

    QHttpPart fromOsPart;
    fromOsPart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"fromos\""));
    fromOsPart.setBody(QString::number(Utils::GetOSTypeIndex()).toUtf8());
    multiPart->append(fromOsPart);

    QNetworkRequest request;
    if(!GenerateRequest("/testapi/requesturl",request))
    {
        return -1;
    }

    if(!SetSSlConfigToRequest(request,":/server_certification.cer"))
    {
        return -2;
    }

    //获取错误
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.post(request,multiPart);
    multiPart->setParent(reply);

    //添加了请求的超时操作
    QReplyTimeout *pTimeout = new QReplyTimeout(reply,10000);
    QEventLoop eventLoop;
    connect(&manager, SIGNAL(finished(QNetworkReply*)),&eventLoop, SLOT(quit()));
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
    int parserCode = ParserRequestReply(reply,"上传用户词库",resultObject);

    if(parserCode == 0)
    {
        md5Value = resultObject.value("md5").toString();
    }
    return parserCode;
}

//词库下载
int NetworkHandler::DownloadCustomWordlib(QString loginid, QString clientid, QString & filePath)
{
    //生成对应的网址请求
    QString requestType = QString("/testapi/requesturl");

    QNetworkRequest request;
    if(!GenerateRequest(requestType,request))
    {
        return -1;
    }

    if(!SetSSlConfigToRequest(request,QString(":/server_certification.cer")))
    {
        return -2;
    }

    //获取信息
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

        return reply->error();
    }

    QJsonValue resultValue;
    int returnCode = ParserRequestReplyStr(reply,"下载词库",resultValue);

    if(returnCode == 0)
    {
        //下载对应的文件
        filePath = resultValue.toString();
    }

    return returnCode;
}


int NetworkHandler::ChangeUserPassword(QString loginid, QString oldPassWord, QString newPassWord)
{
    //生成对应的网址请求
    QNetworkRequest request;
    if(!GenerateRequest("/testapi/requesturl",request))
    {
        return -1;
    }

    QString certificate_path = "";
    if(!SetSSlConfigToRequest(request,certificate_path))
    {
        return -2;
    }

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QByteArray data;
    data.append("loginid=");
    data.append(loginid.toUtf8());
    data.append("&oldPassword=");
    data.append(QCryptographicHash::hash(oldPassWord.toUtf8(), QCryptographicHash::Md5).toHex());
    data.append("&newPassword=");
    data.append(QCryptographicHash::hash(newPassWord.toUtf8(), QCryptographicHash::Md5).toHex());
    data.append("&fromos=");
    data.append(QString::number(Utils::GetOSTypeIndex()));


    //获取错误
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.post(request,data);

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

        return reply->error();
    }

    QJsonObject resultObject;
    int returnCode = ParserRequestReply(reply,"修改用户密码",resultObject);

    if(returnCode == 0)
    {
        //下载对应的文件
        return 0;
    }

    return returnCode;
}

int NetworkHandler::DownloadHighFrequencyWordlib(QString wordlibVersion, QString wordName, QString& wordlibAddr)
{
    //生成对应的网址请求
    QNetworkRequest request;
    QString urlStr = QString("/testapi/requesturl");
    Utils::AddSystemExtraParamer(urlStr,OPType::MANUAL);
    QUrl url(urlStr);
    request.setUrl(url);

    QString certificate_path = "";
    if(!SetSSlConfigToRequest(request,certificate_path))
    {
        return -2;
    }

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

        return reply->error();
    }

    QJsonObject resultObject;
    int returnCode = ParserRequestReply(reply,"下载高频词库",resultObject);

    if(returnCode == 0)
    {
        //下载对应的文件
        QString updateFlag = resultObject.value("update").toString();
        if(updateFlag == "false")
        {
            return 1;
        }
        else if(updateFlag == "true")
        {
            wordlibAddr = resultObject.value("dicts").toObject().value("addr").toString();
        }
    }

    return returnCode;
}

int NetworkHandler::GetSystemWordlibVersionInfo(QString wordlibMd5,SystemWordlibInfoStru& wordlib_info)
{
    //生成对应的网址请求
    QNetworkRequest request;
    QString urlStr = QString("GET /testapi/requesturl");
    Utils::AddSystemExtraParamer(urlStr,OPType::MANUAL);
    QUrl url(urlStr);
    request.setUrl(url);

    QString certificate_path ="";
    if(!SetSSlConfigToRequest(request,certificate_path))
    {
        return -2;
    }

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

        return reply->error();
    }

    QJsonObject resultObject;
    int returnCode = ParserRequestReply(reply,"获取系统词库信息",resultObject);

    if(returnCode == 0)
    {
        //下载对应的文件
        wordlib_info.date_str = resultObject.value("date").toString();
        wordlib_info.valid = resultObject.value("valid").toInt();
        wordlib_info.name_str = resultObject.value("name").toString();
        wordlib_info.block_num = resultObject.value("date").toInt();
        wordlib_info.id_str = resultObject.value("id").toString();
    }

    return returnCode;
}

int NetworkHandler::CheckCommonWordlib(QString version, QString wordlibName, QString &wordlibPath)
{
    QNetworkRequest request;
    QString urlStr = QString("/testapi/requesturl");
    Utils::AddSystemExtraParamer(urlStr,OPType::MANUAL);
    QUrl url(urlStr);
    request.setUrl(url);

    QString certificate_path = "";
    if(!SetSSlConfigToRequest(request,certificate_path))
    {
        return -2;
    }

    //获取错误
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.get(request);
    QEventLoop eventLoop;
    connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
    if(reply->error() != QNetworkReply::NoError)
    {

        return reply->error();
    }

    QJsonObject resultObject;
    int returnCode = ParserRequestReply(reply,"检查控制台自定义词库",resultObject);

    if(returnCode == 0)
    {
        //下载对应的文件
        QString updateFlag = resultObject.value("update").toString();
        if(updateFlag == "false")
        {
            return 1;
        }
        else if(updateFlag == "true")
        {
            wordlibPath = resultObject.value("dicts").toObject().value("addr").toString();
        }
    }

    return returnCode;
}

int NetworkHandler::UploadCustomPhraseFile(QString loginid, QString timeStamp, QString customPhraseFilePath)
{
    //生成对应的网址请求
    QFileInfo customPhrasePath(customPhraseFilePath);
    QString customPhraseFileName =customPhrasePath.fileName();

    QFile* phraseFile = new QFile(customPhraseFilePath);
    phraseFile->open(QIODevice::ReadOnly);

    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart customPhrasePart;
    customPhrasePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                                 QVariant(QString("form-data; name=\"wordFile\";filename=\"%1\"").arg(customPhraseFileName)));
    customPhrasePart.setBodyDevice(phraseFile);
    phraseFile->setParent(multiPart);
    multiPart->append(customPhrasePart);

    QHttpPart loginIDPart;
    loginIDPart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"loginid\""));
    loginIDPart.setBody(loginid.toUtf8());
    multiPart->append(loginIDPart);

    QHttpPart timeStampPart;
    timeStampPart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"time\""));
    timeStampPart.setBody(timeStamp.toUtf8());
    multiPart->append(timeStampPart);

    QHttpPart fromOsPart;
    fromOsPart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"fromos\""));
    fromOsPart.setBody(QString::number(Utils::GetOSTypeIndex()).toUtf8());
    multiPart->append(fromOsPart);

    QNetworkRequest request;
    if(!GenerateRequest("/testapi/requesturl",request))
    {
        return -1;
    }

    QString certificate_path = "";
    if(!SetSSlConfigToRequest(request,certificate_path))
    {
        return -2;
    }

    //获取错误
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.post(request,multiPart);
    multiPart->setParent(reply);

    //添加了请求的超时操作
    QReplyTimeout *pTimeout = new QReplyTimeout(reply,5000);
    QEventLoop eventLoop;
    connect(&manager, SIGNAL(finished(QNetworkReply*)),&eventLoop, SLOT(quit()));
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
    return ParserRequestReply(reply,"上传用户自定义短语",resultObject);
}

int NetworkHandler::DownloadCutomPhraseFile(QString loginid, QString &serverStamp, bool &hasUpdate, QString &customPhraseFilePath)
{
    //生成对应的网址请求
    QNetworkRequest request;
    if(!GenerateRequest("/testapi/requesturl",request))
    {
        return -1;
    }

    QString certificate_path = "";
    if(!SetSSlConfigToRequest(request,certificate_path))
    {
        return -2;
    }

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QByteArray data;
    data.append("loginid=");
    data.append(loginid.toUtf8());
    data.append("&time=");
    data.append(QString("-1").toUtf8());
    data.append("&fromos=");
    data.append(QString::number(Utils::GetOSTypeIndex()));

    //获取错误
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.post(request,data);


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

        return reply->error();
    }

    QJsonObject resultObject;
    int returnCode = ParserRequestReply(reply,"下载用户自定义短语",resultObject);

    if(returnCode == 0)
    {
        //下载对应的文件
        customPhraseFilePath = resultObject.value("wordfileaddr").toString();
        serverStamp = resultObject.value("time").toString();
        QString updateStr = resultObject.value("update").toString();
        if(updateStr == "false")
        {
            hasUpdate = false;
        }
        else if (updateStr == "true")
        {
            hasUpdate = true;
        }
        else
        {
            hasUpdate = false;
        }
        return 0;
    }

    return returnCode;
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
    Utils::AddSystemExtraParamer(inputUrlAddr,OPType::MANUAL);
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
    if(!rootObj.contains("code") || !rootObj.contains("message"))
    {

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

        return -1;
    }
    QJsonObject rootObj = jsonDoc.object();
    if(!rootObj.contains("code") || !rootObj.contains("message"))
    {

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

        return codeStr.toInt();
    }
}

int NetworkHandler::UploadInputStatsCount(QString clientid, QString loginid, QString version, QString date, int inputCount)
{
    //生成对应的网址请求
    QNetworkRequest request;
    if(!GenerateRequest("/testapi/requesturl",request))
    {
        return -1;
    }
    QUrl requestUrl = request.url();

    QString certificate_path = "";
    if(!SetSSlConfigToRequest(request,certificate_path))
    {
        return -2;
    }

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QByteArray data;
    data.append("clientid=");
    data.append(clientid.toUtf8());
    if(!loginid.isEmpty())
    {
        data.append("&loginid=");
        data.append(loginid.toUtf8());
    }
    data.append("&version=");
    data.append(version.toUtf8());
    data.append("&inputcount=");
    QString inputStr = QString::number(inputCount);
    data.append(inputStr.toUtf8());
    data.append("&date=");
    data.append(date.toUtf8());
    data.append("&fromos=");
    data.append(QString::number(Utils::GetOSTypeIndex()));


    //获取错误
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.post(request,data);


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

        return reply->error();
    }

    QByteArray replyData = reply->readAll();
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(replyData, &json_error));

    if(json_error.error != QJsonParseError::NoError)
    {

        return -2;
    }
    QJsonObject rootObj = jsonDoc.object();
    if(!rootObj.contains("code"))
    {

        return -3;
    }
    QString codeStr = rootObj.value("code").toString();
    int statusCode = codeStr.toInt();
    if(statusCode == 200)
    {
        return 0;
    }
    return statusCode;
}

int NetworkHandler::UploadFeedbackInfo(QString version, QString clientid, QString loginid, QString title, QString content,QString type,QList<QString> imageList)
{
    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart versionPart;
    versionPart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"version\""));
    versionPart.setBody(version.toUtf8());
    multiPart->append(versionPart);

    QHttpPart clientidPart;
    clientidPart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"clientid\""));
    clientidPart.setBody(clientid.toUtf8());
    multiPart->append(clientidPart);

    QHttpPart loginIDPart;
    loginIDPart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"loginid\""));
    loginIDPart.setBody(loginid.toUtf8());
    multiPart->append(loginIDPart);

    QHttpPart titlePart;
    titlePart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"title\""));
    titlePart.setBody(title.toUtf8());
    multiPart->append(titlePart);

    QHttpPart contentPart;
    contentPart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"content\""));
    contentPart.setBody(content.toUtf8());
    multiPart->append(contentPart);

    QHttpPart typePart;
    typePart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"type\""));
    typePart.setBody(type.toUtf8());
    multiPart->append(typePart);

    QHttpPart fromOsPart;
    fromOsPart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"fromos\""));
    fromOsPart.setBody(QString::number(Utils::GetOSTypeIndex()).toUtf8());
    multiPart->append(fromOsPart);

    int imageIndex = 0;

    foreach(QString indexImage, imageList)
    {
        QFileInfo image_info(indexImage);
        QString file_name = image_info.fileName();
        QHttpPart attachPart;
        attachPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                                     QVariant(QString("form-data; name=\"attachList\";filename=\"%1\"").arg(file_name)));

        QFile *file = new QFile(indexImage);
        file->open(QIODevice::ReadOnly);
        file->setParent(multiPart);
        attachPart.setBodyDevice(file);
        multiPart->append(attachPart);
        ++imageIndex;
    }

    QNetworkRequest request;
    if(!GenerateRequest("/testapi/requesturl",request))
    {
        return -1;
    }

    QString certificate_path = "";
    if(!SetSSlConfigToRequest(request,certificate_path))
    {
        return -2;
    }

    //获取错误
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.post(request,multiPart);
    multiPart->setParent(reply);

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

        return reply->error();
    }

    QByteArray replyData = reply->readAll();
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(replyData, &json_error));

    if(json_error.error != QJsonParseError::NoError)
    {

        return -2;
    }
    QJsonObject rootObj = jsonDoc.object();
    if(!rootObj.contains("code"))
    {

        return -3;
    }
    QString codeStr = rootObj.value("code").toString();
    QString messageStr = rootObj.value("message").toString();
    int statusCode = codeStr.toInt();
    if(statusCode == 200)
    {
        return 0;
    }
    return statusCode;
}

int NetworkHandler::GetSysWordlibIncreInfo(QString sys_wordlib_md5, QString &result_info)
{
    //生成对应的网址请求
    QNetworkRequest request;
    QString subwebIndex = QString("/testapi/requesturl");
    if(!GenerateRequest(subwebIndex,request))
    {
        return -1;
    }

    QString certificate_path = "";
    if(!SetSSlConfigToRequest(request,certificate_path))
    {
        return -2;
    }

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

        return reply->error();
    }

    result_info = reply->readAll();

    return 0;
}

int NetworkHandler::DownloadFullSysWordlib(QString clientid, QString loginid, QString sys_wordlib_md5)
{
    return 0;
}


bool NetworkHandler::SetSSlConfigToRequest(QNetworkRequest &inputRequest, QString certificatePath)
{
    inputRequest.setSslConfiguration(Utils::GetQsslConfig());
    return true;
}

QString NetworkHandler::GetVersionCheckUrl()
{
    //combine the request url
    QSettings config(TOOLS::PathUtils::GetUpdaterIniPath(), QSettings::IniFormat);
    config.beginGroup("package_info");
    QString version = config.value("version", "").toString();
    QString cpu_type = config.value("cpu_type", "").toString();
    QString os_type = config.value("os_type", "").toString();
    config.endGroup();

    QString request_url = ConfigManager::Instance()->GetServerUrl().append("/testapi/requesturl");

    request_url.append("?version=").append(version)
               .append("&os=").append(os_type)
               .append("&cpu=").append(cpu_type);
    #ifdef USE_IBUS
    request_url.append("&frame=").append("ibus");
    #else
    request_url.append("&frame=").append("fcitx");
    #endif
    request_url.append("&optype=").append(QString::number(OPType::MANUAL));
    Utils::AddSystemExtraParamer(request_url,OPType::MANUAL);
    return request_url;
}

int NetworkHandler::CheckSystemWordlib(QString wordlib_md5,  QString& wordlibid, QString& server_md5,QString& current_date_version)
{
    //生成对应的网址请求
    QNetworkRequest request;
    QString subwebIndex = QString("/testapi/requesturl");
    if(!GenerateRequest(subwebIndex,request))
    {
        return -1;
    }
    qDebug() << request.url().toString();

    if(!SetSSlConfigToRequest(request,":/server_certification.cer"))
    {
        return -2;
    }

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
        Utils::WriteLogToFile(QString("检查系统词库失败:%1").arg(reply->error()));
        return reply->error();
    }

    QJsonObject resultObject;
    int returnCode = ParserRequestReply(reply,"检查系统词库",resultObject);

    if(returnCode == 0)
    {
        //下载对应的文件
        QString updateFlag = resultObject.value("update").toString();
        if(updateFlag == "false")
        {
            current_date_version = resultObject.value("date").toString();
            return 1;
        }
        else if(updateFlag == "true")
        {
            current_date_version = resultObject.value("date").toString();
            wordlibid = resultObject.value("id").toString();
            server_md5 = resultObject.value("md5").toString();
        }
    }

    return returnCode;
}

int NetworkHandler::check_sys_wordlib_block_info(QString wordlib_md5,QVector<WORDLIB_BLOCK_INFO>& wordlib_info_vector)
{
    QNetworkRequest request;
    QString subwebIndex = QString("/testapi/requesturl");
    if(!GenerateRequest(subwebIndex,request))
    {
        return -1;
    }
    request.setSslConfiguration(Utils::GetQsslConfig());
    QNetworkAccessManager network_manager;
    QNetworkReply* reply = network_manager.get(request);

    QString request_url = request.url().toString();

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

        return 5;
    }
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

int NetworkHandler::check_system_wordlib_update(QString wordlib_md5,QString& download_url,QString& download_file_md5)
{
    QString url = ConfigManager::Instance()->GetServerUrl().append("/testapi/requesturl");
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

    QString loginid;
    ConfigManager::Instance()->GetStrConfigItem("loginid",loginid);

    if(has_update == "true")
    {
        QString setupid = json_result_object.value("id").toString();
        QString url = ConfigManager::Instance()->GetServerUrl().append("/testapi/requesturl?");

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

    QString wordlib_path = TOOLS::PathUtils::get_local_wordlib_path();
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

int NetworkHandler::GetCurrentUserUserwordlibMD5(const QString &loginid, const QString &clientid, QString &md5_value, QString &download_url, QString &perv_clientid)
{
    //生成对应的网址请求
    QString requestType = QString("/testapi/requesturl");

    QNetworkRequest request;
    if(!GenerateRequest(requestType,request))
    {
        return 1;
    }
    request.setSslConfiguration(Utils::GetQsslConfig());
    QString url_info_str = request.url().toString();

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
        return 666;
    }

    if(reply->error() != QNetworkReply::NoError)
    {
        Utils::WriteLogToFile(QString("下载词库失败%1").arg(reply->error()));
        return reply->error();
    }

    QByteArray replyData = reply->readAll();
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(replyData, &json_error));

    if(json_error.error != QJsonParseError::NoError)
    {
        Utils::WriteLogToFile(QString("%s response parser error").arg(__FUNCTION__));
        return 3;
    }
    QJsonObject rootObj = jsonDoc.object();
    if(!rootObj.contains("code") || !rootObj.contains("message"))
    {
        Utils::WriteLogToFile(QString("返回的json数据不包含code和message"));
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
        Utils::WriteLogToFile(QString("返回值为code:%2").arg(codeStr));
        return 6;
    }

}

//词库上传
int NetworkHandler::UploadCustomWordlib(QString loginid, QString clientid, QString dictFilePath, QString& md5Value)
{
    if(!QFile::exists(dictFilePath) || clientid.isEmpty())
    {
        return -1;
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

    QHttpPart fromOsPart;
    fromOsPart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"fromos\""));
    fromOsPart.setBody(QString::number(Utils::GetOSTypeIndex()).toUtf8());
    multiPart->append(fromOsPart);
    //AddExtraPramer(multiPart);

    QNetworkRequest request;
    if(!GenerateRequest("/testapi/requesturl",request))
    {
        return -1;
    }

    request.setSslConfiguration(Utils::GetQsslConfig());

    //获取错误
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.post(request,multiPart);
    multiPart->setParent(reply);

    //添加了请求的超时操作
    QReplyTimeout *pTimeout = new QReplyTimeout(reply,5000);
    QEventLoop eventLoop;
    connect(&manager, SIGNAL(finished(QNetworkReply*)),&eventLoop, SLOT(quit()));
    connect(pTimeout, SIGNAL(timeout()),&eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
    if(pTimeout->GetTimeoutFlag())
    {
        return 666;
    }

    if(reply->error() != QNetworkReply::NoError)
    {
        Utils::WriteLogToFile(QString("上传用户词库失败%1").arg(reply->error()));
        return reply->error();
    }

    QJsonObject resultObject;
    int parserCode = ParserRequestReply(reply,"上传用户词库",resultObject);

    if(parserCode == 0)
    {
        md5Value = resultObject.value("md5").toString();
    }
    return parserCode;
}

int NetworkHandler::DownloadFileFromWeb(QString fileUrl, QString fileSavePath)
{
    //通过返回的请求头中获得文件的名称
    QNetworkRequest request;
    QUrl url(fileUrl);

    request.setSslConfiguration(Utils::GetQsslConfig());

    QNetworkAccessManager accessManager;
    request.setUrl(url);
    QNetworkReply *reply  = accessManager.get(request);

    QEventLoop loop;
    connect(&accessManager, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
    loop.exec(QEventLoop::ExcludeUserInputEvents);

    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (statusCode.toInt() != 200)
    {
        return 2;
    }

    if(QFile::exists(fileSavePath))
    {
        QFile::remove(fileSavePath);
    }


    QFile file(fileSavePath);
    if(!file.open(QIODevice::WriteOnly))
    {
        return 4;
    }
    file.write(reply->readAll());
    file.close();
    return 0;
}
