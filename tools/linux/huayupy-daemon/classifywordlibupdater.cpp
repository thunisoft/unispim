//#include "classifywordlibupdater.h"
//#include <QNetworkAccessManager>
//#include <QFile>
//#include <QFileInfo>
//#include <QDir>
//#include <QJsonParseError>
//#include <QJsonDocument>
//#include <QJsonArray>
//#include <QJsonValueRef>
//#include <QJsonObject>
//#include <QNetworkReply>
//#include <QSettings>
//#include "../public/config.h"
//#include "../public/utils.h"

//ClassifyWordlibUpdater::ClassifyWordlibUpdater(QObject *parent):
//    QObject(parent),
//    m_network_manager(NULL),
//    m_status(UNWORK),
//    m_download_finished_count(0)
//{
//    Init();
//}

//void ClassifyWordlibUpdater::Init()
//{
//    m_network_manager = new QNetworkAccessManager(this);
//}



//void ClassifyWordlibUpdater::GetWordlibToUpdate(QList<WORDLIB_INFO>& wordlib_info_list)
//{
//    wordlib_info_list.clear();
//    QMap<QString, WORDLIB_INFO>::iterator local_wordlib = m_local_wordlibs_info.begin();
//    for(; local_wordlib != m_local_wordlibs_info.end(); local_wordlib++)
//    {
//        QMap<QString, WORDLIB_INFO>::iterator server_wordlib = m_server_wordlibs_info.begin();
//        for(; server_wordlib != m_server_wordlibs_info.end();  server_wordlib++)
//        {
//            if(local_wordlib.key() == server_wordlib.key())
//            {
//                if(local_wordlib.value().md5 != server_wordlib.value().md5)
//                {
//                     wordlib_info_list.push_back(server_wordlib.value());
//                }
//                break;
//            }
//        }
//    }
//}

//void ClassifyWordlibUpdater::SetStatusWorking()
//{
//    m_status = WORKING;
//}

//void ClassifyWordlibUpdater::SetStatusUnWork()
//{
//    m_status = UNWORK;
//}

//Status ClassifyWordlibUpdater::GetStatus()
//{
//    return m_status;
//}

//void ClassifyWordlibUpdater::OnGetWordlibInfoDone(QNetworkReply* reply)
//{
//    disconnect(m_network_manager, SIGNAL(finished(QNetworkReply*)), 0, 0);
//    QVariant http_request_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
//    if(http_request_code.isValid())
//    {
//        qDebug() << "Get Server WordlibInfo Status Code: " << http_request_code.toInt();
//    }
//    if(http_request_code.toInt() != 200)
//    {
//        qDebug() << "http request error" << http_request_code.toInt();
//        return;
//    }
//    PhraseJsonInfo(reply->readAll());
//    GetWordlibToUpdate(m_wordlibs_to_check);
//}

//void ClassifyWordlibUpdater::PhraseJsonInfo(const QByteArray& data)
//{
//    m_server_wordlibs_info.clear();
//    QJsonParseError json_error;
//    QJsonDocument json_doc(QJsonDocument::fromJson(data, &json_error));
//    if(json_error.error != QJsonParseError::NoError)
//    {
//        qDebug() << "Parse Wordlib Information Faild!";
//        return;
//    }
//    QJsonArray json_array = json_doc.array();
//    QJsonArray::iterator it = json_array.begin();
//    for(; it != json_array.end(); it++)
//    {
//        WORDLIB_INFO wordlib_info;
//        FillDataToWordlibInfoEntity(wordlib_info, *it);
//        m_server_wordlibs_info.insert(wordlib_info.file_name, wordlib_info);
//    }
//}

//void ClassifyWordlibUpdater::FillDataToWordlibInfoEntity(WORDLIB_INFO& wordlib_info, QJsonValueRef json_value)
//{
//    //md5值在测试接口中不全，待服务器加上
//    QJsonObject wordlib_object = json_value.toObject();
//    if(!wordlib_object.contains("fileName") || !wordlib_object.contains("viewName")
//            || !wordlib_object.contains("downloadUrl") || !wordlib_object.contains("type") || !wordlib_object.contains("md5"))
//        return;
//    wordlib_info.file_name = wordlib_object.value("fileName").toString();
//    wordlib_info.wordlib_name = wordlib_object.value("viewName").toString();
//    wordlib_info.url = wordlib_object.value("downloadUrl").toString();
//    wordlib_info.type = wordlib_object.value("type").toString();
//    wordlib_info.md5 = wordlib_object.value("md5").toString();
//}

//void ClassifyWordlibUpdater::DownloadWordlib()
//{
//    QString download_url = m_wordlibs_to_check[m_download_finished_count].url;
//    QNetworkRequest request;
//    request.setUrl(download_url);
//    connect(m_network_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(OnOneWordlibDownloadFinsihed(QNetworkReply*)));
//    m_network_manager->get(request);
//}

//void ClassifyWordlibUpdater::OnOneWordlibDownloadFinished(QNetworkReply* reply)
//{
//    disconnect(m_network_manager, SIGNAL(finished(QNetworkReply*)), 0, 0);

//    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
//    if (statusCode.isValid())
//    {
//        qDebug() << "status code: " << statusCode.toInt();
//    }
//    Utils::MakeDir(Utils::GetTmpDir());
//    QFile file(Utils::GetTmpDir().append(m_wordlibs_to_check[m_download_finished_count].file_name));
//    if(!file.open(QIODevice::WriteOnly))
//    {
//        qDebug() << "fileName: "<< file.fileName() << " errorInfo: " << file.errorString();
//        return;
//    }
//    file.write(reply->readAll());
//    file.close();

//    if(Utils::GetFileMD5(Utils::GetTmpDir().append(m_wordlibs_to_check[m_download_finished_count].file_name))
//                         == m_wordlibs_to_check[m_download_finished_count].md5)
//    {
//        QString file_name = m_wordlibs_to_check.at(m_download_finished_count).file_name;
//        QFile file(Utils::GetTmpDir().append(file_name));
//        file.copy(Utils::GetConfigDirPath().append("wordlib/").append(file_name));
//    }
//    ++m_download_finished_count;
//    if(m_download_finished_count < m_wordlibs_to_check.size())
//        DownloadWordlib();
//}

//void ClassifyWordlibUpdater::Update()
//{
//    QSettings config(Utils::GetConfigDirPath().append("config.ini"), QSettings::IniFormat);
//    int is_auto_update = config.value("wordlib/auto_update", 1).toInt();
//    if(!is_auto_update)
//        return;
//    if(GetStatus() == WORKING)
//        return;
//    m_download_finished_count = 0;
//    SetStatusWorking();
//    GetLocalWordlib();
//    GetServerWordlib();
//    GetWordlibToUpdate(m_wordlibs_to_check);
//    if(!m_wordlibs_to_check.isEmpty())
//    {
//        DownloadWordlib();
//    }
//    SetStatusUnWork();
//}
