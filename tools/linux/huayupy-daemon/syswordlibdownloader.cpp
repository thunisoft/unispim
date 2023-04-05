// 系统词库下载，全量或增量

#include "syswordlibdownloader.h"

#include <QSettings>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonArray>
#include <QNetworkRequest>
#include "configbus.h"
#include "../public/config.h"
#include "../public/utils.h"
#include "../public/defines.h"

/*
 * 更新系统词库流程：
 * 1.请求更新，并获取结果
 * 2.增量更新，调用 update_helper, 下载更新包 (zip格式)。解压，合并，将合并完成的uwl放到 config 目录下
 * 3.全量更新，请求 要更新到sys_uwl的md5，下载新的uwl文件
 */

SyswordlibDownloader::SyswordlibDownloader():                    
                      m_update_type(_COMPLETE)
                     ,m_blocks("")
{
    //m_wordlib_update_helper = new SyswordlibUpdateHelper();
    Init();
}


SyswordlibDownloader::~SyswordlibDownloader()
{
    if(m_wordlib_update_helper)
        delete m_wordlib_update_helper;
}


QString SyswordlibDownloader::GetRequestUrl()
{
    QString md5 = GetWordlibMd5();
    QString url = Config::Instance()->GetServerUrl().append("/testapi/requesturl");

    Utils::AddSystemExtraParamer(url,OPType::AUTO);
    return url;
}

QString SyswordlibDownloader::GetWordlibName()
{
    return QString("sys.uwl");
}


QString SyswordlibDownloader::GetWordlibVersion()
{
    QSettings config(Config::wordLibDirPath(WLDPL_DATA).append("wordlib.ini"), QSettings::IniFormat);
    return config.value("version/sys", "").toString();
}

void SyswordlibDownloader::SetUpdateType(UpdateType update_type)
{
    m_update_type = update_type;
}

bool SyswordlibDownloader::ParseQueryPathReply(const QByteArray& data, QString& wordlib_download_path)
{
    wordlib_download_path.clear();
    QJsonParseError json_error;
    QJsonDocument json_doc(QJsonDocument::fromJson(data, &json_error));
    if(json_error.error != QJsonParseError::NoError)
    {
        qDebug() << "Parse Wordlib [" << GetWordlibName() << "] Download Path Faild when ParseQueryPathReply!";
        SetStatisUnworking();
        return false;
    }

    //解析出来的code的值是("200"), 转成int会失败, 故转换成string
    QJsonObject json_object_root = json_doc.object();
    if(!json_object_root.contains("code")  )
    {
        qDebug() << json_object_root.value("code").toString();
        SetStatisUnworking();
        return false;
    }
    QString code = json_object_root.value("code").toString();
    if(!code.compare("1101"))
    {
        //全量更新
        SetUpdateType(_COMPLETE);
    }
    else if(!code.compare("200"))
    {
        m_blocks.clear();
        //增量更新
        SetUpdateType(_INCREMENT);        
        if(!json_object_root.contains("result") || !json_object_root.value("result").isArray())
        {
            SetStatisUnworking();
            return false;
        }
        QJsonArray result_array = json_object_root.value("result").toArray();        
        if( result_array.size() <= 0)
        {
            SetStatisUnworking();
            return false;
        }
        if(json_object_root.contains("addrtype"))
        {
            QString addrType = json_object_root.value("addrtype").toString();
            Utils::WriteLogToFile(QString("[ParseQueryPathReply] SyswordlibDownload addrtype=%1").arg(addrType));
        }
        //QJsonArray::iterator it;
        m_block_list.clear();
        for(int i = 0; i < result_array.size(); i++)
        {
            QJsonObject object_uwl_info = result_array.at(i).toObject();
            if(!object_uwl_info.contains("fileName") || !object_uwl_info.contains("block")
                    ||!object_uwl_info.contains("fulldictmd5"))
            {
                m_block_list.clear();
                SetStatisUnworking();
                return false;
            }
            else
            {
                UwlInfo uwl_info;
                uwl_info.block_id = object_uwl_info.value("block").toInt();
                uwl_info.file_name = object_uwl_info.value("fileName").toString();
                uwl_info.md5 = object_uwl_info.value("fulldictmd5").toString();
                m_block_list.insert(uwl_info.block_id, uwl_info);
                m_blocks.append(std::to_string(uwl_info.block_id).c_str()).append(",");
            }
        }
        //去掉最后一个 ,
        m_blocks.remove(m_blocks.length() - 1, 1);
        m_wordlib_update_helper->SetUwlInfo(&m_block_list);
    }
    else
    {
        //增量更新服务器返回错误，打印错误信息，调用全量更新接口
        QString message = json_object_root.value("message").toString();
        Utils::WriteLogToFile(QString("[SyswordlibDownload-ERROR] code=%1 errmsg=%2")
                              .arg(code).arg(message));
        SetUpdateType(_COMPLETE);
    }
    return true;
}

void SyswordlibDownloader::DoDownload(const QString& download_path)
{
    download_path;
    /*
    qDebug() << "sys wordlib do download";
    Q_UNUSED(download_path);
    if(m_update_type == _COMPLETE)
    {
        QueryCompletedownloadPath();
    }
    else
    {
        DownloadIncrement();
    }*/
}

void SyswordlibDownloader::DownloadComplete()
{
//    QNetworkRequest request;
//    request.setUrl(GetIncrementDownloadPath());
//    request.setSslConfiguration(Utils::GetQsslConfig());
//    m_network_manager->get(request);
}

void SyswordlibDownloader::DownloadIncrement()
{
    connect(m_wordlib_update_helper, SIGNAL(DownloadFinished(int)), this, SLOT(IncreatementDownloadFinished(int)));
    m_wordlib_update_helper->SetDownloadPath(GetIncrementDownloadPath());
    m_wordlib_update_helper->DownloadIncrement();
}

void SyswordlibDownloader::IncreatementDownloadFinished(int is_success)
{
    disconnect(m_wordlib_update_helper, SIGNAL(DownloadFinished(int)), 0, 0);
    if(is_success)
        ConfigBus::instance()->valueChanged("loadWordlib", "");
    SetStatisUnworking();
}

void SyswordlibDownloader::OnQueryCompleteUpdateFinished(QNetworkReply* reply)
{
    disconnect(m_network_manager, SIGNAL(finished(QNetworkReply*)), 0, 0);
    QByteArray data = reply->readAll();
    QJsonParseError json_error;
    QJsonDocument json_doc(QJsonDocument::fromJson(data, &json_error));
    if(json_error.error != QJsonParseError::NoError)
    {
        qDebug() << "Parse Wordlib [" << GetWordlibName() << "] Download Path Faild when OnQueryCompleteUpdateFinished!";
        SetStatisUnworking();
        return;
    }

    //解析出来的code的值是("200"), 转成int会失败, 故转换成string
    QJsonObject json_object_root = json_doc.object();
    if(!json_object_root.contains("code") || json_object_root.value("code").toString().compare("200"))
    {
        qDebug() << "Query wordlib completed path return with a wrong code value!";
        SetStatisUnworking();
        return;
    }
    if(!json_object_root.contains("result"))
    {
        qDebug() << "Query wordlib completed path return without [result] item!";
        SetStatisUnworking();
        return;
    }
    QJsonObject json_result_object = json_object_root.value("result").toObject();
    if(!json_result_object.contains("md5") || !json_result_object.contains("update")
            || json_result_object.value("update").toString().compare("true")
            || !json_result_object.contains("id"))
    {
        qDebug() << "Query wordlib completed path return without [md5] item or [update] is false!";
        SetStatisUnworking();
        return;
    }
    qDebug() << "sys call parent download";
    WordlibDownloader::DoDownload(GetCompeleteDownloadPath(json_result_object.value("id").toString()));
}

QString SyswordlibDownloader::GetIncrementDownloadPath()
{
    QString url = Config::Instance()->GetServerUrl().append("/testapi/requesturl");

    Utils::AddSystemExtraParamer(url,OPType::AUTO);
    return url;
}

QString SyswordlibDownloader::GetCompeleteDownloadPath(const QString& id)
{
    QString url = Config::Instance()->GetServerUrl().append("/testapi/requesturl");

    Utils::AddSystemExtraParamer(url,OPType::AUTO);
    return url;
}

void SyswordlibDownloader::QueryCompletedownloadPath()
{
    qDebug() << "QueryCompletedownloadPath";
    QString sys_uwl_path = Config::wordLibDirPath(WLDPL_CONFIG).append("sys.uwl");
    QString md5 = Utils::GetFileMD5(sys_uwl_path);
    QString url = Config::Instance()->GetServerUrl().append("/testapi/requesturl?");

    QNetworkRequest request;
    request.setUrl(url);
    request.setSslConfiguration(Utils::GetQsslConfig());
    QNetworkReply* reply = m_network_manager->get(request);

    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(OnHttpError(QNetworkReply::NetworkError)));
    connect(m_network_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(OnQueryCompleteUpdateFinished(QNetworkReply*)));
}

void SyswordlibDownloader::Init()
{
    m_wordlib_update_helper = new SyswordlibUpdateHelper();
}
