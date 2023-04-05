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
#include "config.h"
#include "utils.h"
#include "wordlibstackedwidget.h"
#include <QNetworkConfigurationManager>
#include <QHostInfo>
#include <QElapsedTimer>

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
//    QEventLoop loop;
//    QHostInfo::lookupHost("www.baidu.com",this,SLOT(onLookupHost(QHostInfo)));
//    connect(this,SIGNAL(networkStatusResult(bool)),&loop,SLOT(quit()));
//    loop.exec();
//    return m_canVisitWeb;
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

int NetworkHandler::UserAccountLogin(const QString account, const QString password)
{
    //生成对应的网址请求
    QElapsedTimer count_timer;
    count_timer.start();
    QNetworkRequest request;
    if(!GenerateRequest("/testapi/requesturl",request))
    {
        return -1;
    }
    QString certificate_path = Config::Instance()->GetHttpsCertificatePath();
    if(!SetSSlConfigToRequest(request,certificate_path))
    {
        return -2;
    }


    QByteArray data;
    QString clientid = Config::Instance()->GetHostMacAddress();
    data.append("userid=");
    data.append(account.toUtf8());
    data.append("&md5password=");
    data.append(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Md5).toHex());
    data.append("&clientid=");
    data.append(clientid.toUtf8());
    data.append("&ostype=");
    data.append(Utils::GetOSType().toUtf8());
    AddExtraPramer(data);

    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/x-www-form-urlencoded"));

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
    int cost_time = count_timer.elapsed();
    qDebug() << "reuest wait  cost" << cost_time << "ms";

    if(reply->error() != QNetworkReply::NoError)
    {
        Utils::WriteLogToFile(QString("登录请求异常异常码%1").arg(reply->error()));
        return reply->error();
    }

    QJsonObject resultObject;
    int returnCode = ParserRequestReply(reply,"账号登录",resultObject);

    if (returnCode == 0)
    {
        //登录成功之后保存对应的配置信息
        Config::Instance()->SaveUserConfig();
        QString userName = resultObject.value("username").toString();
        QString loginid =resultObject.value("userid").toString();
        QString clientid = resultObject.value("clientid").toString();

        //用户登录
        ConfigItemStruct configInfo;

        configInfo.itemName = "login";
        configInfo.itemGroupName = "state";
        configInfo.itemCurrentIntValue = 1;
        Config::Instance()->SetStateConfigItemByJson("login",configInfo);

        configInfo.itemName = "loginid";
        configInfo.itemGroupName = "state";
        configInfo.itemCurrentStrValue = loginid;
        configInfo.itemType = "string";
        Config::Instance()->SetStateConfigItemByJson("loginid",configInfo);

        configInfo.itemName = "clientid";
        configInfo.itemGroupName = "state";
        configInfo.itemCurrentStrValue = clientid;
        configInfo.itemType = "string";
        Config::Instance()->SetStateConfigItemByJson("clientid",configInfo);

        configInfo.itemName = "acouuntname";
        configInfo.itemGroupName = "state";
        configInfo.itemCurrentStrValue = userName;
        configInfo.itemType = "string";
        Config::Instance()->SetStateConfigItemByJson("acouuntname",configInfo);
        Config::Instance()->LoadConfig();

        int totalcost_time = count_timer.elapsed();
        Utils::WriteLogToFile(QString("login process total cost:%1ms").arg(totalcost_time));

        return 0;
    }
    return returnCode;
}

int NetworkHandler::UserPhoneLogin(const QString phoneNum, const QString checkNum)
{
    //生成对应的网址请求

    QNetworkRequest request;
    if(!GenerateRequest("/testapi/requesturl",request))
    {
        return -1;
    }

    QString certificate_path = Config::Instance()->GetHttpsCertificatePath();
    if(!SetSSlConfigToRequest(request,certificate_path))
    {
        return -2;
    }

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QString clientid = Config::Instance()->GetHostMacAddress();
    QByteArray data;
    data.append("phonenumber=");
    data.append(phoneNum.toUtf8());
    data.append("&captchal=");
    data.append(checkNum.toUtf8());
    data.append("&clientiduse=");
    data.append(clientid.toUtf8());
    data.append("&ostype=");
    data.append(Utils::GetOSType().toUtf8());
    AddExtraPramer(data);

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
        Utils::WriteLogToFile(QString("验证码登录请求异常异常码%1").arg(reply->error()));
        return reply->error();
    }

    QJsonObject resultObject;
    int returnCode = ParserRequestReply(reply,"验证码登录",resultObject);
    if(returnCode == 0)
    {
        //登录成功之后保存对应的配置信息
        Config::Instance()->SaveUserConfig();
        QString userName = resultObject.value("acouuntname").toString();
        QString loginid =resultObject.value("loginidcustom").toString();
        QString clientid = resultObject.value("clientiduse").toString();

        //用户登录
        ConfigItemStruct configInfo;
        configInfo.itemName = "login";
        configInfo.itemGroupName = "state";
        configInfo.itemCurrentIntValue = 1;
        Config::Instance()->SetStateConfigItemByJson("loginstate",configInfo);

        configInfo.itemName = "loginid";
        configInfo.itemGroupName = "state";
        configInfo.itemCurrentStrValue = loginid;
        configInfo.itemType = "string";
        Config::Instance()->SetStateConfigItemByJson("loginiduse",configInfo);

        configInfo.itemName = "clientid";
        configInfo.itemGroupName = "state";
        configInfo.itemCurrentStrValue = clientid;
        configInfo.itemType = "string";
        Config::Instance()->SetStateConfigItemByJson("clientid",configInfo);

        configInfo.itemName = "username";
        configInfo.itemGroupName = "state";
        configInfo.itemCurrentStrValue = userName;
        configInfo.itemType = "string";
        Config::Instance()->SetStateConfigItemByJson("username",configInfo);
        Config::Instance()->LoadConfig();

        return 0;
    }
    return returnCode;
}

int NetworkHandler::RegisterUserAccount(const QString &account, const QString password)
{
    QNetworkRequest request;
    if(!GenerateRequest("/testapi/requesturl",request))
    {
        return -1;
    }
    QString certificate_path = Config::Instance()->GetHttpsCertificatePath();
    if(!SetSSlConfigToRequest(request,certificate_path))
    {
        return -2;
    }
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QByteArray data;
    data.append("uuname=");
    data.append("");
    data.append("&clientid=");
    data.append(account.toUtf8());
    data.append("&password=");
    data.append(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Md5).toHex());
    data.append("&client=");
    data.append(Utils::GethostMac().toUtf8());
    data.append("&ostype=");
    data.append(Utils::GetOSType());
    AddExtraPramer(data);


    QNetworkAccessManager manager;
    QNetworkReply* reply = manager.post(request, data);

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
        Utils::WriteLogToFile(QString("注册用户信息异常:%1").arg(reply->error()));
        return reply->error();
    }

    QJsonObject resultObject;
    return ParserRequestReply(reply,"注册用户账户",resultObject);
}

int NetworkHandler::GetVerificationCode(QString phoneNum)
{
    //生成对应的网址请求
    QNetworkRequest request;
    if(!GenerateRequest("/testapi/requesturl",request))
    {
        return -1;
    }

    QString certificate_path = Config::Instance()->GetHttpsCertificatePath();
    if(!SetSSlConfigToRequest(request,certificate_path))
    {
        return -2;
    }

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QByteArray data;
    data.append("account=");
    data.append(phoneNum.toUtf8());
    data.append("&fromos=");
    data.append(Utils::GetOSType().toUtf8());
    AddExtraPramer(data);

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
        Utils::WriteLogToFile(QString("获取验证码请求异常异常码%1").arg(reply->error()));
        return reply->error();
    }

    QJsonObject resultObject;
    return ParserRequestReply(reply,"获取验证码",resultObject);

}

int NetworkHandler::DownloadUserConfigFile(QString loginid,
                                           QString& serverStamp, bool & hasUpdate,
                                           QString& cfgfileAddr)
{
    //生成对应的网址请求
    QNetworkRequest request;
    if(!GenerateRequest("/testapi/requesturl",request))
    {
        return -1;
    }

    QString certificate_path = Config::Instance()->GetHttpsCertificatePath();
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
    data.append(Utils::GetOSType().toUtf8());
    AddExtraPramer(data);

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
        Utils::WriteLogToFile(QString("下载用户配置文件异常%1").arg(reply->error()));
        return reply->error();
    }

    QJsonObject resultObject;
    int returnCode = ParserRequestReply(reply,"下载配置文件",resultObject);

    if(returnCode == 0)
    {
        //下载对应的文件
        cfgfileAddr = resultObject.value("cfgfileaddr").toString();
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

int NetworkHandler::ClientVersionDetection(QString currentVersionNum, ServerPackageStruct& package_info)
{
    //生成对应的网址请求
    QNetworkRequest request;
    QString subwebIndex = QString("/testapi/requesturl");
    if(!GenerateRequest(subwebIndex,request))
    {
        return -1;
    }

    QString certificate_path = Config::Instance()->GetHttpsCertificatePath();
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
        Utils::WriteLogToFile(QString("获取新版本异常,异常码:%1").arg(reply->error()));
        return reply->error();
    }

    QJsonObject resultObject;
    int returnCode = ParserRequestReply(reply,"获取新版本",resultObject);
    if(returnCode == 0)
    {
        //下载对应的文件
        package_info.has_update = (resultObject.value("update").toString() == "true");
        if(package_info.has_update)
        {
            package_info.newVersionNum = resultObject.value("version").toString();
            package_info.setupid = resultObject.value("setupid").toString();

            if(resultObject.contains("md5"))
            {
                package_info.packageMd5 = resultObject.value("md5").toString();
            }
            package_info.download_url = resultObject.value("addr").toString();
            package_info.download_type = resultObject.value("addrtype").toString();
            package_info.pacakge_name = resultObject.value("filename").toString();

        }
        return 0;

    }
    return returnCode;
}

QString NetworkHandler::CombineDownloadPackageUrl(const QString &setupid, const QString &clientid, int optype)
{
    QString destUrl;
    QString requestHeadder;
    GetRequesetHeader(requestHeadder);
    destUrl=QString("%1/testapi/requesturl")
            .arg(requestHeadder);
    return destUrl;
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

    QString certificate_path = Config::Instance()->GetHttpsCertificatePath();
    if(!SetSSlConfigToRequest(request,certificate_path))
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

int NetworkHandler::UploadHistoryRecordInfo(QMap<QString,int> history_data_map, QString clientid, QString loginid, QString version)
{
    //组合数组参数
    QJsonArray jsonArray;
    auto iter = history_data_map.begin();
    for (iter; iter != history_data_map.end(); ++iter)
    {
        QJsonObject input_count_obj;
        input_count_obj.insert("date",iter.key());
        input_count_obj.insert("inputcount",QString::number(iter.value()));
        jsonArray.append(input_count_obj);
    }

    QJsonDocument document;
    document.setArray(jsonArray);
    QString inputStr = QString(document.toJson());

    //生成对应的网址请求
    QNetworkRequest request;
    if(!GenerateRequest("/testapi/requesturl",request))
    {
        return -1;
    }

    QString certificate_path = Config::Instance()->GetHttpsCertificatePath();
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
    data.append("&fromos=");
    data.append(Utils::GetOSType().toUtf8());
    data.append("&inputdata=");
    data.append(inputStr);
    AddExtraPramer(data);


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
        Utils::WriteLogToFile(QString("上传历史用户统计数据异常%1").arg(reply->error()));
        return reply->error();
    }

    QByteArray replyData = reply->readAll();
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(replyData, &json_error));

    if(json_error.error != QJsonParseError::NoError)
    {
        Utils::WriteLogToFile(QString("上传历史用户统计数据返回异常"));
        return -2;
    }
    QJsonObject rootObj = jsonDoc.object();
    if(!rootObj.contains("code"))
    {
        Utils::WriteLogToFile(QString("统计上传返回的json数据不包含code"));
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

int NetworkHandler::GetCurrentUserUserwordlibMD5(const QString &loginid, const QString &clientid, QString &md5_value, QString &download_url, QString &perv_clientid)
{
    //生成对应的网址请求
    QString requestType = QString("/testapi/requesturl");

    QNetworkRequest request;
    if(!GenerateRequest(requestType,request))
    {
        return 1;
    }

    QString certificate_path = Config::Instance()->GetHttpsCertificatePath();
    if(!SetSSlConfigToRequest(request,certificate_path))
    {
        return 2;
    }

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

int NetworkHandler::DownloadFileFromWeb(QString fileUrl,QString fileSaveDir,QString& fileSavePath,QString fileName)
{   
    //通过返回的请求头中获得文件的名称
    QNetworkRequest request;
    QUrl url(fileUrl);

    QString certificate_path = Config::Instance()->GetHttpsCertificatePath();
    if(!SetSSlConfigToRequest(request,certificate_path))
    {
        return -2;
    }

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

    QString scheme = "https";
    ConfigItemStruct configInfo;
    if(Config::Instance()->GetConfigItemByJson("server_scheme",configInfo))
    {
        scheme = configInfo.itemCurrentStrValue;
    }

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

    fileSavePath = fileSaveDir + "\\" + fileName;
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

int NetworkHandler::DownloadFileFromWeb(QString fileUrl, QString fileSavePath)
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

    QEventLoop loop;
    connect(accessManager, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
    loop.exec(QEventLoop::ExcludeUserInputEvents);

    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (statusCode.toInt() != 200)
    {
        Utils::WriteLogToFile(QString("下载安装包返回值为code:%1").arg(statusCode.toInt()));
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
    fromOsPart.setBody(Utils::GetOSType().toUtf8());
    multiPart->append(fromOsPart);
    AddExtraPramer(multiPart);

    QNetworkRequest request;
    if(!GenerateRequest("/testapi/requesturl",request))
    {
        return -1;
    }

    QString certificate_path = Config::Instance()->GetHttpsCertificatePath();
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
        Utils::WriteLogToFile(QString("上传用户配置文件失败%1").arg(reply->error()));
        return reply->error();
    }

    QJsonObject resultObject;
    return ParserRequestReply(reply,"上传用户配置文件",resultObject);
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
    fromOsPart.setBody(Utils::GetOSType().toUtf8());
    multiPart->append(fromOsPart);
    AddExtraPramer(multiPart);

    QNetworkRequest request;
    if(!GenerateRequest("/testapi/requesturl",request))
    {
        return -1;
    }

    QString certificate_path = Config::Instance()->GetHttpsCertificatePath();
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

    QString certificate_path = Config::Instance()->GetHttpsCertificatePath();
    if(!SetSSlConfigToRequest(request,certificate_path))
    {
        return -2;
    }

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

    QString certificate_path = Config::Instance()->GetHttpsCertificatePath();
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
    data.append(Utils::GetOSType().toUtf8());
    AddExtraPramer(data);


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
        Utils::WriteLogToFile(QString("修改用户密码异常%1").arg(reply->error()));
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
    QUrl url(QString("/testapi/requesturl");
    request.setUrl(url);

    QString certificate_path = Config::Instance()->GetHttpsCertificatePath();
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
        Utils::WriteLogToFile(QString("下载高频词库失败:%1").arg(reply->error()));
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

int NetworkHandler::CheckSystemWordlib(QString wordlib_md5,  QString& wordlibid, QString& server_md5,QString& current_date_version)
{
    //生成对应的网址请求
    QNetworkRequest request;
    QString subwebIndex = QString("/testapi/requesturl");
    if(!GenerateRequest(subwebIndex,request))
    {
        return -1;
    }

    QString certificate_path = Config::Instance()->GetHttpsCertificatePath();
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

int NetworkHandler::GetSystemWordlibVersionInfo(QString wordlibMd5,SystemWordlibInfoStru& wordlib_info)
{
    //生成对应的网址请求
    QNetworkRequest request;
    QUrl url(QString("GET /testapi/requesturl");
    request.setUrl(url);

    QString certificate_path = Config::Instance()->GetHttpsCertificatePath();
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
        Utils::WriteLogToFile(QString("获取系统词库信息失败:%1").arg(reply->error()));
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
    QUrl url(QString("/testapi/requesturl").arg(version).arg(wordlibName).arg(Utils::GetOSType()));
    request.setUrl(url);

    QString certificate_path = Config::Instance()->GetHttpsCertificatePath();
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
        Utils::WriteLogToFile(QString("检查控制台自定义词库失败:%1").arg(reply->error()));
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
    fromOsPart.setBody(Utils::GetOSType().toUtf8());
    multiPart->append(fromOsPart);
    AddExtraPramer(multiPart);

    QNetworkRequest request;
    if(!GenerateRequest("/testapi/requesturl",request))
    {
        return -1;
    }

    QString certificate_path = Config::Instance()->GetHttpsCertificatePath();
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
        Utils::WriteLogToFile(QString("上传用户自定义短语失败%1").arg(reply->error()));
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

    QString certificate_path = Config::Instance()->GetHttpsCertificatePath();
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
    data.append(Utils::GetOSType().toUtf8());
    AddExtraPramer(data);

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
        Utils::WriteLogToFile(QString("下载用户自定义短语异常%1").arg(reply->error()));
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
        Utils::WriteLogToFile("配置文件中的服务器地址信息不完整");
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

int NetworkHandler::AddExtraPramer(QByteArray &inputByteArray)
{
    QString client_version = Config::Instance()->GetIMEVersion();
    QString clientid = Config::Instance()->GetHostMacAddress();
    QString ostype_str = Config::Instance()->GetOSVersion();
    QString cputype_str = Config::Instance()->GetCPUType();
    if(!inputByteArray.contains("version"))
    {
        inputByteArray.append("&version=");
        inputByteArray.append(client_version);
    }

    if(!inputByteArray.contains("fromos="))
    {
        inputByteArray.append("&fromos=");
        inputByteArray.append("1");
    }

    if(!inputByteArray.contains("cpu="))
    {
        inputByteArray.append("&cpu=");
        inputByteArray.append("x86");
    }

    if(!inputByteArray.contains("clientid="))
    {
        inputByteArray.append("&clientid=");
        inputByteArray.append(clientid);
    }

    if(!inputByteArray.contains("optype="))
    {
        inputByteArray.append("&optype=");
        inputByteArray.append("2");
    }

    if(!inputByteArray.contains("cputype="))
    {
        inputByteArray.append("&cputype=");
        inputByteArray.append(cputype_str);
    }

    if(!inputByteArray.contains("ostype="))
    {
        inputByteArray.append("&ostype=");
        inputByteArray.append(ostype_str);
    }
    return 0;
}

int NetworkHandler::AddExtraPramer(QHttpMultiPart *multiPart)
{
    if(!multiPart)
    {
        return 1;
    }

    QString client_version = Config::Instance()->GetIMEVersion();
    QString clientid = Config::Instance()->GetHostMacAddress();
    QString ostype_str = Config::Instance()->GetOSVersion();
    QString cputype_str = Config::Instance()->GetCPUType();

    QHttpPart clientid_httppart;
    clientid_httppart.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"clientid\""));
    clientid_httppart.setBody(clientid.toUtf8());
    multiPart->append(clientid_httppart);


    QHttpPart version_part;
    version_part.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"version\""));
    version_part.setBody(client_version.toUtf8());
    multiPart->append(version_part);

    QHttpPart ostype_part;
    ostype_part.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"ostype\""));
    ostype_part.setBody(ostype_str.toUtf8());
    multiPart->append(ostype_part);

    QHttpPart cputype_part;
    cputype_part.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"cputype\""));
    cputype_part.setBody(cputype_str.toUtf8());
    multiPart->append(cputype_part);

    QHttpPart optype_part;
    optype_part.setHeader(QNetworkRequest::ContentDispositionHeader,QVariant("form-data; name=\"optype\""));
    optype_part.setBody("2");
    multiPart->append(optype_part);

    return 0;
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

    QString certificate_path = Config::Instance()->GetHttpsCertificatePath();
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
    data.append(Utils::GetOSType().toUtf8());
    AddExtraPramer(data);


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
        Utils::WriteLogToFile(QString("上传用户统计数据异常%1").arg(reply->error()));
        return reply->error();
    }

    QByteArray replyData = reply->readAll();
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(replyData, &json_error));

    if(json_error.error != QJsonParseError::NoError)
    {
        Utils::WriteLogToFile(QString("上传用户统计数据返回异常"));
        return -2;
    }
    QJsonObject rootObj = jsonDoc.object();
    if(!rootObj.contains("code"))
    {
        Utils::WriteLogToFile(QString("统计上传返回的json数据不包含code"));
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
    version;
    clientid;
    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

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
    fromOsPart.setBody(Utils::GetOSType().toUtf8());
    multiPart->append(fromOsPart);

    AddExtraPramer(multiPart);

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

    QString certificate_path = Config::Instance()->GetHttpsCertificatePath();
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
        Utils::WriteLogToFile(QString("上传反馈信息失败%1").arg(reply->error()));
        return reply->error();
    }

    QByteArray replyData = reply->readAll();
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(replyData, &json_error));

    if(json_error.error != QJsonParseError::NoError)
    {
        Utils::WriteLogToFile(QString("上传反馈信息失败返回异常"));
        return -2;
    }
    QJsonObject rootObj = jsonDoc.object();
    if(!rootObj.contains("code"))
    {
        Utils::WriteLogToFile(QString("上传反馈信息返回值中不包含code"));
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

    QString certificate_path = Config::Instance()->GetHttpsCertificatePath();
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
        Utils::WriteLogToFile(QString("获得词库的增量更新失败:%1").arg(reply->error()));
        return reply->error();
    }

    result_info = reply->readAll();

    return 0;
}

int NetworkHandler::DownloadFullSysWordlib(QString clientid, QString loginid, QString sys_wordlib_md5)
{
    QString dictID;
    QString server_md5;
    QString current_date_version;
    int check_code = CheckSystemWordlib(sys_wordlib_md5,dictID,server_md5,current_date_version);
    if(check_code != 0)
    {
        return check_code;
    }

    //生成对应的网址请求
    QNetworkRequest request;
    QString subUrl = QString("/testapi/requesturl");
    if(!GenerateRequest(subUrl,request))
    {
        return -1;
    }

    QString certificate_path = Config::Instance()->GetHttpsCertificatePath();
    if(!SetSSlConfigToRequest(request,certificate_path))
    {
        return -2;
    }

    //获取错误
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.get(request);

    //添加了请求的超时操作
    QReplyTimeout *pTimeout = new QReplyTimeout(reply,20000);
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
        Utils::WriteLogToFile(QString("获得全量用户词库下载的请求失败:%1").arg(reply->error()));
        return reply->error();
    }

    QString sys_wordlib_path = Config::SystemWordlibPath();
    QString sys_wordlib_bak_path = sys_wordlib_path + ".bak.tools";
    QString sys_wordlib_tmp_path = sys_wordlib_path + ".tmp.tools";

    //下载完毕之后校验md5值，然后更新替换
    if(QFile::exists(sys_wordlib_bak_path))
    {
        QFile::remove(sys_wordlib_bak_path);
    }
    if(QFile::exists(sys_wordlib_tmp_path))
    {
        QFile::remove(sys_wordlib_tmp_path);
    }

    QFile file(sys_wordlib_tmp_path);
    if(file.open(QIODevice::ReadWrite))
    {
        file.write(reply->readAll());
        file.close();
    }
    else
    {
        return -1;
    }

    //系统词库条数小于400000的时候不更新
    WORDLIB_INFO wordlib_info;
    WordlibStackedWidget::AnalysisUwl(sys_wordlib_tmp_path,wordlib_info);
    if(wordlib_info.words_count < 400000)
    {
        return -2;
    }

    QString real_md5 = Utils::GetFileMD5(sys_wordlib_tmp_path);
    if(real_md5 == server_md5)
    {
        bool sys_wordlib_bak_copy = true;
        if(QFile::exists(sys_wordlib_path))
        {
            sys_wordlib_bak_copy = QFile::rename(sys_wordlib_path,sys_wordlib_bak_path);
        }

        bool sys_wordlib_copy = QFile::rename(sys_wordlib_tmp_path,sys_wordlib_path);
        if(!sys_wordlib_copy || !sys_wordlib_bak_copy)
        {
            Utils::WriteLogToFile("full wordlib update copy failed");
            return 9;
        }
    }
    else
    {
        QFile::remove(sys_wordlib_tmp_path);
    }

    return 0;
}

int NetworkHandler::DownloadSysWordlibBlockFromServer(QString version, QString clientid, QString loginid, QVector<WORDLIB_BLOCK_INFO> block_vector)
{
    try {
        QString blockList;
        for(int index=0; index<block_vector.size(); ++index)
        {
            if(index==0)
            {
                blockList += QString::number(block_vector.at(index).block_index);
            }
            else
            {
                blockList += ",";
                blockList += QString::number(block_vector.at(index).block_index);
            }
        }

        //生成对应的网址请求
        QNetworkRequest request;
        QString subUrl =QString("/testapi/requesturl");                
        if(!GenerateRequest(subUrl,request))
        {
            return -1;
        }

        QString certificate_path = Config::Instance()->GetHttpsCertificatePath();
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
            Utils::WriteLogToFile(QString("下载系统词库增量更新碎片失败:%1").arg(reply->error()));
            return reply->error();
        }

        QString contentHeader = reply->rawHeader("Content-Disposition");
        QStringList subStrList = contentHeader.split("=");
        if(subStrList.size() != 2)
        {
            return -3;
        }
        QString fileName = subStrList.at(1);

        QString target_path = Utils::GetWordlibDirPath() + fileName;
        QString target_bak_path = target_path + ".bak.tools";
        QString target_tmp_path = target_path + ".tmp.tools";

        if(QFile::exists(target_tmp_path))
        {
            QFile::remove(target_tmp_path);
        }
        if(QFile::exists(target_path))
        {
            QFile::remove(target_path);
        }
        QFile target_tmp_file(target_tmp_path);
        if(target_tmp_file.open(QIODevice::ReadWrite))
        {
            target_tmp_file.write(reply->readAll());
            target_tmp_file.close();
        }
        else
        {
            return -4;
        }
        target_tmp_file.rename(target_path);
        Utils::DepressedZlibFile(target_path,Utils::GetWordlibDirPath());
        if(QFile::exists(target_path))
        {
            QFile::remove(target_path);
        }
        return 0;


    } catch (...) {
        return 1;
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
