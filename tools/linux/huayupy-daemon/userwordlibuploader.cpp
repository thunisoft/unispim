// 用户词库上传

#include "userwordlibuploader.h"
#include <QSettings>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QHttpMultiPart>
#include <QDataStream>
#include <QDir>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QEventLoop>
#include "configbus.h"
#include "networkhandler.h"
#include "../public/config.h"
#include "../public/utils.h"

#define TIME_INTERVAL  4 * 60


typedef unsigned short UTF16;

#define	WORDLIB_NAME_LENGTH     16  //词库名称长度
#define	WORDLIB_AUTHOR_LENGTH   16  //词库作者名称
#define	WORDLIB_PAGE_SIZE   1024    //词库页大小
#define	WORDLIB_PAGE_DATA_LENGTH    (WORDLIB_PAGE_SIZE - 4 * sizeof(int))   //页中可用数据长度
#define MAX_WORDLIBS    32  //内存中最多的词库数目

#define HYPIM_WORDLIB_V66_SIGNATURE     0x14091994  //输入法词库V66标志

#define	CON_NUMBER 24   //合法声母的数目

bool UserWordlibUploader::isUesrwordlibDownload = false;

UserWordlibUploader::UserWordlibUploader(QObject* parent)
                    :QObject(parent)
                    ,m_ptr_network_manager(nullptr)
                    ,m_http_multi_part(nullptr)
                    ,m_is_working(false)
                    ,m_ptr_file_user_wordlib(nullptr)
                    ,m_upload_timer(nullptr)
                    ,m_upload_user_wordlib_interval(TIME_INTERVAL * 60 * 1000)                    
                    ,m_word_count(0)
{
    Init();
}

UserWordlibUploader::~UserWordlibUploader()
{
    if(m_ptr_network_manager)
        delete m_ptr_network_manager;
    if(m_http_multi_part)
        delete m_http_multi_part;
    if(m_upload_timer)
        delete m_upload_timer;
}

void UserWordlibUploader::Init()
{
    m_ptr_network_manager = new QNetworkAccessManager(this);
    m_wait_write_to_uwl_timer.setSingleShot(true);
    //引擎将用户词库写入文件需要时间，所以停1秒
    connect(&m_wait_write_to_uwl_timer, SIGNAL(timeout()), this, SLOT(AfterSavedUserWordlib()));
    SetUploadUserWorlibInterval();
}

bool UserWordlibUploader::ParseUploadFeedBack(const QByteArray& data)
{
    QJsonParseError json_error;
    QJsonDocument json_doc(QJsonDocument::fromJson(data, &json_error));
    if(json_error.error != QJsonParseError::NoError)
    {
        qDebug() << "Parse Upload User Wordlib Feedback Error";
        return false;
    }
    QJsonObject json_object = json_doc.object();
    if(!json_object.contains("code") || json_object.value("code").toString().isEmpty())
    {
        qDebug() << "Parse Upload User Wordlib Feedback [code] Error";
        return false;
    }
    if(!json_object.contains("message") || json_object.value("message").toString().isEmpty())
    {
        qDebug() << "Parse Upload User Wordlib Feedback [message] Error";
        return false;
    }
    qDebug() << "Upload User Wordlib Feedback Message [ "<< json_object.value("message").toString() << " ] ";
    return true;
}

bool UserWordlibUploader::IsAutoUploadUserWordlib()
{
    QSettings config(Config::configFilePath(), QSettings::IniFormat);
    return config.value("account/userWordLib", 1).toInt();
}

QString UserWordlibUploader::GetUploadUrl()
{
    QString url = Config::Instance()->GetServerUrl().append("/testapi/requesturl");
    Utils::AddSystemExtraParamer(url,OPType::AUTO);
    return url;
}

QString UserWordlibUploader::GetUserWordlibMd5Url()
{
    QString Url = Config::Instance()->GetServerUrl().append("/testapi/requesturl");
    Url.append("?loginid=").append(m_loginid);
    Url.append("&clientid=").append(m_macid);
    Utils::AddSystemExtraParamer(Url,OPType::AUTO);
    return Url;
}

QString UserWordlibUploader::GetUserWordlibPath()
{
    if(Config::Instance()->GetLoginId().isEmpty())
    {
        return Config::configDirPath().append("wordlib/").append("user.uwl");
    }
    return Config::configDirPath().append("wordlib/").append(Config::Instance()->GetLoginId()).append("/user.uwl");
}

void UserWordlibUploader::UploadWordlib()
{
    if(m_is_working)
        return;
    SetStatusWorking();
    ConfigBus::instance()->valueChanged("saveUserWordlib", "");
    m_wait_write_to_uwl_timer.start(1000);
}

void UserWordlibUploader::AfterSavedUserWordlib()
{
    if(Config::GetTodayInputCount() <= 0)
    {
        SetStatusUnWorkingAndNodownlod();
        return;
    }
    m_ptr_file_user_wordlib = new QFile(GetUserWordlibPath());
    if(!m_ptr_file_user_wordlib->exists() || !m_ptr_file_user_wordlib->open(QIODevice::ReadOnly))
    {
        qDebug() << "打开用户词库失败 或 用户词库不存在！";
        m_ptr_file_user_wordlib->close();
        delete m_ptr_file_user_wordlib;
        m_ptr_file_user_wordlib = nullptr;
        SetStatusUnWorking();
        return;
    }

    int check_md5_ret = CompareLocalUserWordlibWithServer();
    if(check_md5_ret == 0)
    {
        if(m_http_multi_part)
        {
            delete m_http_multi_part;
            m_http_multi_part = nullptr;
        }
        m_is_working = false;
        StartTimer();
        return;
    }

    /*
        * 用户词库上传优化
        * 统计此时段输入量，输入量为零直接退出且不下载
        * 向服务器请求当前用户词库的md5值
        * md5值相同：退出，无需更新且不下载
        * md5值不相同，检测是否登录，未登录直接上传
        * md5值不相同，检测是否登录，登录，判断是否本机，是，直接上传
        * md5值不相同，检测是否登录，登录，判断是否本机，否，下载合并用户词库后上传词库
       */

    if(!AnalysisUwl(GetUserWordlibPath(), m_word_count))
    {
        SetStatusUnWorkingAndNodownlod();
        return;
    }

    QString serverUseWordlibMd5,severClientId;
    if(GetUserWordlibMd5(serverUseWordlibMd5,severClientId))
    {
        QString loadUserWordlibMd5 = Utils::GetFileMD5(GetUserWordlibPath()).toLower();
        if(serverUseWordlibMd5.isEmpty())
        {
            qDebug()<<"[OnDownLoadFinished] UseWordlibMd5 is empty";
            SetStatusUnWorking();
            return;
        }
        if(loadUserWordlibMd5 != serverUseWordlibMd5)
        {
            if(m_loginid.isEmpty())
                DoHttp();
            else
            {
                if(severClientId == m_macid)
                {
                    DoHttp();
                }
                else
                {
                    isUesrwordlibDownload = true;
                    SetStatusUnWorking();
                }
            }
        }
        else
        {
            SetStatusUnWorkingAndNodownlod();
            return;
        }
    }

}

void UserWordlibUploader::DoHttpSlot()
{
    isMultiport = true;
    DoHttp();
}


int UserWordlibUploader::CompareLocalUserWordlibWithServer()
{
    try
    {
        QString user_wordlib_path = GetUserWordlibPath();
        NetworkHandler request_handler;

        //用户词库存在的时候
        if(QFile::exists(user_wordlib_path))
        {
            WriteLogToFile(QString("when compare user wordlib md5 local user.uwl  exist"));
            QString server_user_wordlib_md5, server_user_wordlib_url,perv_clientid;
            QString loginid = Config::Instance()->GetLoginId();
            QString clientid = Utils::GethostMac();
            if(loginid.isEmpty())
                loginid = Utils::GethostMac();
            int getmd5_ret = request_handler.GetCurrentUserUserwordlibMD5(loginid,clientid,server_user_wordlib_md5,server_user_wordlib_url,perv_clientid);
            if(getmd5_ret != 0)
            {
                WriteLogToFile(QString("get server user wordlib md5 error:%1:%2").arg(getmd5_ret).arg(Errorcode_Map[getmd5_ret]));
                return 1;
            }
            if(server_user_wordlib_md5.isEmpty())
            {
                WriteLogToFile("server user wordlib md5 is empty");
                return 2;
            }

            QString local_user_md5 = Utils::GetFileMD5(user_wordlib_path);
            if(server_user_wordlib_md5 == local_user_md5)
            {
                WriteLogToFile("user.wordlib server md5 is same with local file md5");
                return 0;
            }

        }
        else
        {
            WriteLogToFile(QString("when compare user wordlib md5 local user.uwl doesn't exist"));
            return 3;
        }
        return 4;
    }
    catch(std::exception& e)
    {
        WriteLogToFile(QString("%1 throw exception:%2").arg(QString(__FUNCTION__).arg(QString(e.what()))));
        return 5;
    }
}

bool UserWordlibUploader::AnalysisUwl(const QString& uwl_file_path, int& word_count)
{
    QFileInfo file_info(uwl_file_path);

    int length = file_info.size();

    char *buffer = new char[length];
    memset(buffer, 0, length);

    QFile file(uwl_file_path);
    if (!file.open(QFile::ReadOnly))
    {
        delete buffer;
        return false;
    }
    file.read(buffer, length);
    file.close();

    WORDLIB* wl = (WORDLIB*)buffer;
    word_count = wl->header.word_count;

    delete buffer;
    if(word_count <= 0)
        return false;
    return true;
}

void UserWordlibUploader::SetStatusWorking()
{
    m_is_working = true;
}

void UserWordlibUploader::SetStatusUnWorking()
{
    if(m_http_multi_part)
    {
        delete m_http_multi_part;
        m_http_multi_part = nullptr;
    }
    m_is_working = false;
    if(!isMultiport)
    {
        emit Done();
        isMultiport = false;
    }

    StartTimer();
}

void UserWordlibUploader::SetStatusUnWorkingAndNodownlod()
{
    if(m_http_multi_part)
    {
        delete m_http_multi_part;
        m_http_multi_part = nullptr;
    }
    m_is_working = false;
    StartTimer();
}

void UserWordlibUploader::DoHttp()
{
    m_http_multi_part = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart dictPart;
    dictPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    dictPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"dict\";filename=\"user.uwl\""));
    dictPart.setBodyDevice(m_ptr_file_user_wordlib);
    m_http_multi_part->append(dictPart);
    QString local_id = Config::Instance()->GetLoginId();
    if(local_id.isEmpty())
        local_id = Utils::GethostMac();
    QHttpPart loginidPart;
    loginidPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"loginid\""));
    loginidPart.setBody(local_id.toLocal8Bit());
    m_http_multi_part->append(loginidPart);

    QHttpPart wordCountPart;
    wordCountPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"wordCount\""));
    wordCountPart.setBody(QString::number(m_word_count).toUtf8());
    m_http_multi_part->append(wordCountPart);

    QHttpPart clientIdPart;
    clientIdPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"clientid\""));
    clientIdPart.setBody(Utils::GethostMac().toLocal8Bit());
    m_http_multi_part->append(clientIdPart);

    QHttpPart osPart;
    osPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"fromos\""));
    osPart.setBody(QString::number(Utils::GetOSTypeIndex()).toUtf8());
    m_http_multi_part->append(osPart);

    QNetworkRequest request;
    request.setUrl(QUrl(GetUploadUrl()));
    request.setSslConfiguration(Utils::GetQsslConfig());
    QNetworkReply *reply = m_ptr_network_manager->post(request, m_http_multi_part);
    qDebug() << "post the upload request!";


    QEventLoop loop;
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(UploadError(QNetworkReply::NetworkError)));
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    loop.exec();
    // 获取数据
    if (QNetworkReply::NoError == reply->error())
    {
        OnUploadDone(reply);
    }
    else
    {
        qDebug() << reply->errorString() ;
        SetStatusUnWorking();
    }
}

void UserWordlibUploader::OnUploadDone(QNetworkReply* reply)
{
    disconnect(m_ptr_network_manager, SIGNAL(finished(QNetworkReply*)), 0, 0);
    QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (status_code.isValid())
    {
        qDebug() << "Upload User Wordlib Status Code: " << status_code.toInt();
    }
    ParseUploadFeedBack(reply->readAll());
    SetStatusUnWorking();
}

void UserWordlibUploader::CloseUserWordlib()
{
    if(m_ptr_file_user_wordlib && m_ptr_file_user_wordlib->isOpen())
        m_ptr_file_user_wordlib->close();
    delete m_ptr_file_user_wordlib;
    m_ptr_file_user_wordlib = nullptr;
}

void UserWordlibUploader::UploadError(QNetworkReply::NetworkError error_code)
{
    CloseUserWordlib();
    SetStatusUnWorking();
}

void UserWordlibUploader::StartTimer()
{
    if(!m_upload_timer)
    {
        m_upload_timer = new QTimer();        
        m_upload_timer->start(m_upload_user_wordlib_interval);
        connect(m_upload_timer, SIGNAL(timeout()), this, SLOT(OnTimeUp()));
    }
}

bool UserWordlibUploader::GetUserWordlibMd5(QString& md5Value, QString& clientIdValue)
{
    QNetworkRequest request;
    request.setUrl(QUrl(GetUserWordlibMd5Url()));
    request.setSslConfiguration(Utils::GetQsslConfig());
    QNetworkAccessManager network_manager;
    QNetworkReply *reply = network_manager.get(request);

    QReplyTimeout *pTimeout = new QReplyTimeout(reply,20000);
    QEventLoop eventLoop;
    connect(reply, SIGNAL(finished()),&eventLoop, SLOT(quit()));
    connect(pTimeout, SIGNAL(timeout()),&eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    if(pTimeout->GetTimeoutFlag())
    {
        qDebug() <<"get server UserWordlibMd5 timeout";
        return false;
    }

    if (QNetworkReply::NoError == reply->error())
    {
        qDebug() <<"get server UserWordlibMd5 succeed";

        QByteArray data = reply->readAll();
        QJsonParseError json_error;
        QJsonDocument json_doc(QJsonDocument::fromJson(data, &json_error));
        if(json_error.error != QJsonParseError::NoError)
            return false;

        //解析出来的message的值是("200"), 转成int会失败, 故转换成string
        QJsonObject json_object_root = json_doc.object();
        if(!json_object_root.contains("code") ||
           json_object_root.value("code").toString().compare("200") != 0)
        {
              return false;
        }

        if(!json_object_root.contains("newinfo"))
        {
             return false;
        }
        QJsonObject json_newinfo = json_object_root.value("newinfo").toObject();
        if(!json_newinfo.contains("md5") || json_newinfo.value("md5").toString().isEmpty()
                ||!json_newinfo.contains("clientid") || json_newinfo.value("clientid").toString().isEmpty())
        {
            return false;
        }
        md5Value = json_newinfo.value("md5").toString();
        clientIdValue = json_newinfo.value("clientid").toString();
    }
    else
    {
        qDebug() <<"get server UserWordlibMd5 fail! return error:"<<reply->error();
        return false;
    }
    return true;
}

void UserWordlibUploader::OnTimeUp()
{
   UploadWordlib();
}

void UserWordlibUploader::Start()
{
    UploadWordlib();
}

void UserWordlibUploader::SetUploadUserWorlibInterval()
{
    QSettings config(Config::StateConfigPath(), QSettings::IniFormat);
    m_upload_user_wordlib_interval = config.value("timer/time_interval", TIME_INTERVAL).toInt() * 1000 * 60;
}

void UserWordlibUploader::WriteLogToFile(const QString &content)
{
    QString logdir = Config::configDirPath().append("logs/");
    QDir dir(logdir);
    if(!dir.exists())
    {
        dir.mkpath(logdir);
    }
    QString logFileAddr = logdir + "huayupy-userwordlibupdate-log";
    QString timeStamp = QDateTime::currentDateTime().toString("yyyy-M-d hh:mm:ss");
    QFile file(logFileAddr);
    bool isSucess = file.open(QIODevice::WriteOnly | QIODevice::Append);
    if(!isSucess)
    {
        return;
    }
    QTextStream output(&file);
    QString output_content = timeStamp + ":" + content + "\n" ;
    output << output_content;
    file.close();
}

