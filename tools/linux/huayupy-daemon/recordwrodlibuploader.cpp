// 词频上传

#include "recordwrodlibuploader.h"
#include <QSettings>
#include <QDir>
#include <QHttpMultiPart>
#include <QEventLoop>
#include "../public/config.h"
#include "../public/utils.h"
#include "../public/DateHelper.h"

#define TIME_INTERVAL 4 * 60
#define RECORD_FILES_TO_SAVE_COUNT  5
#define URL "/testapi/requesturl"

RecordWrodlibUploader::RecordWrodlibUploader(QObject *parent):
                                              QObject(parent)
                                             ,m_ptr_network_manager(nullptr)
                                             ,m_http_multi_part(nullptr)                                             
                                             ,m_ptr_file_record_wordlib(nullptr)
                                             ,m_is_working(false)
                                             //,m_upload_timer(nullptr)
                                             ,m_upload_interval(TIME_INTERVAL * 60 * 1000)
                                             ,m_date_helper(NULL)
{
    Init();
}

RecordWrodlibUploader::~RecordWrodlibUploader()
{
    if(m_date_helper)
    {
        delete m_date_helper;
        m_date_helper = nullptr;
    }
}

void RecordWrodlibUploader::UploadError(QNetworkReply::NetworkError error_code)
{
    qDebug() << "upload record uwl error, error num is: " << error_code;
    m_to_upload.clear();
    SetStatusUnworking();
}

void RecordWrodlibUploader::OnTimeUp()
{
    if(m_is_working)
        return;
    SetStatusWorking();

    RemoveFilesOutOfDate();

    RemoveTodayRecord();

    UploadWordlib();
}

void RecordWrodlibUploader::Init()
{
    m_ptr_network_manager = new QNetworkAccessManager(this);
    m_date_helper = new DateHelper;
    SetUploadInterval();

    //开机的时候直接调用一次
    OnTimeUp();

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(OnTimeUp()));
    timer->start(m_upload_interval);
}

QString RecordWrodlibUploader::GetWordlibDir()
{
    QString path = Utils::GetConfigDirPath().append("wordlib/record/");
    return path;
}

void RecordWrodlibUploader::SetStatusWorking()
{
    m_is_working = true;
}

void RecordWrodlibUploader::SetStatusUnworking()
{
    m_is_working = false;
}

void RecordWrodlibUploader::UploadWordlib()
{
    if(!m_to_upload.isEmpty())
    {
        QString name = m_to_upload.at(0);
        DoHttp(name);
    }
    else
    {
        SetStatusUnworking();
    }
}

void RecordWrodlibUploader::DoHttp(const QString& name)
{
    m_http_multi_part = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart dictPart;
    dictPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    QString file_name = QString("form-data; name=\"countfile\";filename=").append(name);
    dictPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(file_name.toLocal8Bit()));
    QFile *file = new QFile(GetWordlibDir().append(name));
    file->open(QIODevice::ReadOnly);
    dictPart.setBodyDevice(file);
    m_http_multi_part->append(dictPart);
    QString local_id = Config::Instance()->GetLoginId();
    if(local_id.isEmpty())
        local_id = Utils::GethostMac();
    QHttpPart loginidPart;
    loginidPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"loginid\""));
    loginidPart.setBody(local_id.toLocal8Bit());
    m_http_multi_part->append(loginidPart);

    QHttpPart versionPart;
    versionPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"version\""));
    versionPart.setBody(Utils::GetVersion().toUtf8());
    m_http_multi_part->append(versionPart);

    QHttpPart clientIdPart;
    clientIdPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"clientid\""));
    clientIdPart.setBody(Utils::GethostMac().toLocal8Bit());
    m_http_multi_part->append(clientIdPart);

    QHttpPart osPart;
    osPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"fromos\""));
    osPart.setBody(QString::number(Utils::GetOSTypeIndex()).toUtf8());
    m_http_multi_part->append(osPart);


    std::string format_date = name.toStdString();
    format_date = m_date_helper->Format(format_date.substr(0, 8));

    QHttpPart datePart;
    datePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"filedate\""));
    datePart.setBody(QByteArray(format_date.c_str()));
    m_http_multi_part->append(datePart);

    QNetworkRequest request;
    request.setUrl(QUrl(GetUrl()));
    request.setSslConfiguration(Utils::GetQsslConfig());
    QNetworkReply *reply = m_ptr_network_manager->post(request, m_http_multi_part);
    m_http_multi_part->setParent(reply); // delete the multiPart with the reply
    qDebug() << "post the upload request!";

    QEventLoop loop;
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(UploadError(QNetworkReply::NetworkError)));
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    loop.exec();

    //获取http状态码
//    int http_code_ = _reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if(file->isOpen())
        file->close();
    // 获取数据
    if (QNetworkReply::NoError == reply->error())
    {
        if(!m_to_upload.isEmpty())
        {
            m_to_upload.removeFirst();
        }
        QString file_path = GetWordlibDir().append(name);
        QFile::remove(file_path);
        QFile bak(file_path.append(".bak"));
        if(bak.exists())
            bak.remove();
        UploadWordlib();
    }
    else
    {
        qDebug() << reply->errorString() ;
        SetStatusUnworking();
    }
}

void RecordWrodlibUploader::SetUploadInterval()
{
    QSettings config(Config::StateConfigPath(), QSettings::IniFormat);
    m_upload_interval = config.value("timer/time_interval", TIME_INTERVAL).toInt() * 1000 * 60;
    //控制时间在 12小时 和 10分钟
    if(m_upload_interval > 12 * 60 * 60 * 1000 || m_upload_interval < 10 * 60 * 1000)
        m_upload_interval = TIME_INTERVAL * 60 * 1000;
}

void RecordWrodlibUploader::RemoveFilesOutOfDate()
{
    m_to_upload.clear();

    QString wordlib_dir = GetWordlibDir();
    QDir dir(wordlib_dir);
    QStringList files;
    QStringList name_filter;
    name_filter << "*.uwl";

    files = dir.entryList(name_filter, QDir::Files);


    Date today = m_date_helper->GetToday();
    Date date_to_retain = m_date_helper->GetNDaysBeforeDate(today, RECORD_FILES_TO_SAVE_COUNT);
    QString date_str;
    foreach(auto name, files)
    {
        date_str = QString(name).left(8);
        Date date;
        if(!m_date_helper->ConvertStringToDate(date_str.toStdString(), date) ||
                m_date_helper->IsLarge(date_to_retain, date))
        {
            QString path = wordlib_dir;
            path.append(name);
            QFile wordlib(path);
            if(wordlib.exists())
                wordlib.remove();

            path.append(".bak");
            QFile bak(path);
            if(bak.exists())
                bak.remove();
        }
        else
        {
            m_to_upload.push_back(name);
        }
    }
}

void RecordWrodlibUploader::RemoveTodayRecord()
{
    string today;
    m_date_helper->GetCurrentDate(today);
    QString today_uwl = QString(today.c_str()).append(".uwl");
    foreach (auto name, m_to_upload) {
        if(today_uwl.compare(name) == 0)
        {
            m_to_upload.removeOne(name);
            return;
        }
    }

}

QString RecordWrodlibUploader::GetUrl()
{
    QString ip = Config::Instance()->GetServerUrl();
    ip.append(URL);
    Utils::AddSystemExtraParamer(ip,OPType::AUTO);
    return ip;
}
