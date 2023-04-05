// 下载词库（父类）

#include "wordlibdownloader.h"

#include <QNetworkRequest>
#include <QFile>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QSettings>
#include <fstream>
#include <QProcess>
#include <QDir>
#include "../public/config.h"
#include "configbus.h"
#include "../public/utils.h"
#include "userwordlibuploader.h"

WordlibDownloader::WordlibDownloader(QObject* parent)
                   :QObject(parent)
                   ,m_network_manager(NULL)
                   ,m_is_working(false)
                   ,m_wordlib_download_path("")
{
    Init();
}

WordlibDownloader::~WordlibDownloader()
{
    if(m_network_manager)
        delete m_network_manager;
}

void WordlibDownloader::DoDownload(const QString& download_path)
{
    QNetworkRequest request;
    request.setUrl(download_path);
    request.setSslConfiguration(Utils::GetQsslConfig());
    QNetworkReply* reply = m_network_manager->get(request);
    connect(m_network_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(OnDownloadWordlibDone(QNetworkReply*)));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(OnHttpError(QNetworkReply::NetworkError)));

}

void WordlibDownloader::QueryWordlibPath()
{
    QNetworkRequest request;
    request.setUrl(GetRequestUrl());
    request.setSslConfiguration(Utils::GetQsslConfig());
    QNetworkReply* reply = m_network_manager->get(request);
    connect(m_network_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(OnQueryWordlibPathDone(QNetworkReply*)));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(OnHttpError(QNetworkReply::NetworkError)));
}

void WordlibDownloader::Start()
{
    if(!IsAutoSynchronize())
        return;

    if(GetWordlibName() == "user.uwl")
    {
        RestoreUserWordlib();
    }
    if(m_is_working)
    {
        return;
    }
    SetStatusWorking();
    QueryWordlibPath();
}

void WordlibDownloader::OnQueryWordlibPathDone(QNetworkReply *reply)
{
    disconnect(m_network_manager, SIGNAL(finished(QNetworkReply*)), 0, 0);
    QString wordlib_download_path;
    QVariant http_request_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    if(http_request_code.toInt() != 200)
    {
        SetStatisUnworking();
        return;
    }
    if(!ParseQueryPathReply(reply->readAll(), wordlib_download_path))
    {
        SetStatisUnworking();
        return;
    }

    DoDownload(wordlib_download_path);
}

void WordlibDownloader::OnDownloadWordlibDone(QNetworkReply *reply)
{
    disconnect(m_network_manager, SIGNAL(finished(QNetworkReply*)), 0, 0);
    Utils::MakeDir(GetWordlibSavePath());
    const QString wordlib_absolute_path = GetWordlibSavePath().append(GetWordlibName());
    QFile file_tmp(QString(wordlib_absolute_path).append(".tmp"));
    if(file_tmp.exists())
    {
        file_tmp.remove();
    }
    if(!file_tmp.open(QIODevice::WriteOnly))
    {
        SetStatisUnworking();
        return;
    }
    file_tmp.write(reply->readAll());
    file_tmp.close();
    if(file_tmp.size() <= 0)
    {
        SetStatisUnworking();
        return ;
    }
    QString tmp_wordlib_path = QString(wordlib_absolute_path).append(".tmp");
    if(!IsWordlibValid(tmp_wordlib_path))
    {
        QFile::remove(tmp_wordlib_path);
        return;
    }


    QFile file_bak(QString(wordlib_absolute_path).append(".bak"));
    if(file_bak.exists())
    {
        file_bak.remove();
    }
    QFile file_org(wordlib_absolute_path);
    if(file_org.exists())
    {
        file_org.rename(file_bak.fileName());
    }
    file_tmp.rename(wordlib_absolute_path);
    SetStatisUnworking();
    if(UserWordlibUploader::isUesrwordlibDownload)
    {
        emit UserWordLibDownloadDone();
        UserWordlibUploader::isUesrwordlibDownload = false;
    }
    ConfigBus::instance()->valueChanged("loadWordlib", "");
}

void WordlibDownloader::IncreatementDownloadFinished(int is_success)
{
    emit IncreatementDownloadDone(is_success);
}

void WordlibDownloader::OnQueryCompleteUpdateFinished(QNetworkReply* reply)
{
    Q_UNUSED(reply)
}

void WordlibDownloader::OnHttpError(QNetworkReply::NetworkError error_code)
{
    Q_UNUSED(error_code);
    SetStatisUnworking();
}

void WordlibDownloader::Init()
{
    if(!m_network_manager)
    {
        m_network_manager = new QNetworkAccessManager(this);        
    }
}

QString WordlibDownloader::GetWordlibSavePath()
{
    return Config::configDirPath().append("wordlib/");
}

QString WordlibDownloader::GetWordlibMd5()
{
    QFile file(GetWordlibSavePath().append(GetWordlibName()));    
    if(!file.exists())
    {
        QFile default_sys(Utils::GetHuayuPYInstallDirPath().append("wordlib/").append(GetWordlibName()));
        if(default_sys.exists())            
            default_sys.copy(GetWordlibSavePath().append(GetWordlibName()));
        else
            return "";
    }
    return Utils::GetFileMD5(file.fileName());
}

void WordlibDownloader::SetStatusWorking()
{
    m_is_working = true;
}

void WordlibDownloader::SetStatisUnworking()
{
    m_is_working = false;
}

bool WordlibDownloader::ParseQueryPathReply(const QByteArray& data, QString& wordlib_download_path)
{
    wordlib_download_path.clear();
    QJsonParseError json_error;
    QJsonDocument json_doc(QJsonDocument::fromJson(data, &json_error));
    if(json_error.error != QJsonParseError::NoError)
    {
        qDebug() << "Parse Wordlib [" << GetWordlibName() << "] Download Path Faild when WordlibDownloader!";
        SetStatisUnworking();
        return false;
    }

    //解析出来的code的值是("200"), 转成int会失败, 故转换成string
    QJsonObject json_object_root = json_doc.object();
    if(!json_object_root.contains("code") || json_object_root.value("code").toString().compare("200") != 0)
    {
        qDebug() << json_object_root.value("code").toInt() ;        
        SetStatisUnworking();
        return false;
    }
    if(!json_object_root.contains("message") || json_object_root.value("message").toString().compare("fail") == 0)
    {
        SetStatisUnworking();
        return false;
    }
    if(json_object_root.contains("addrtype"))
    {
        QString addrType = json_object_root.value("addrtype").toString();
        Utils::WriteLogToFile(QString("[ParseQueryPathReply] wordlib_download addrtype=%1").arg(addrType));
    }
    wordlib_download_path = json_object_root.value("result").toString();
    return true;
}


void WordlibDownloader::SaveWordlibVersion(const QString& wordlib_name, const QString& version)
{
    QFile file(Config::WordlibVersionPath());
    if(!file.exists())
    {
        file.open(QIODevice::WriteOnly);
        file.close();
    }

    QSettings config(Config::WordlibVersionPath(), QSettings::IniFormat);
    config.beginGroup("version");
    config.setValue(wordlib_name, version);
    config.endGroup();
}

bool WordlibDownloader::IsWordlibValid(const QString &wordlib_path)
{
    try
    {
        if(!QFile::exists(wordlib_path))
        {
            return false;
        }


        WORDLIBHEADER wordlib_info;
        std::ifstream instream(wordlib_path.toStdString());
        instream.read((char*)&wordlib_info, sizeof(WORDLIBHEADER));
        int ci_count = wordlib_info.word_count;
        QFile uwl_file(wordlib_path);
        int file_size = uwl_file.size();
        Utils::WriteLogToFile(QString("%1 filesize is: %2").arg(wordlib_path).arg(file_size));
        if((file_size % 1024) != 0)
        {
            Utils::WriteLogToFile("filesize is unvalid");
            return false;
        }
        Utils::WriteLogToFile(QString("%1 wordlib count is: %2").arg(wordlib_path).arg(ci_count));

        if(ci_count <= 0)
        {
            return false;
        }

        QProcess* wordlib_process = new QProcess(this);
        QFileInfo wordlib_file_info(wordlib_path);

        QString file_dir_path = wordlib_file_info.absolutePath();
        QString txt_file_name = wordlib_file_info.baseName() + ".txt";
        QString target_txt_path = QDir::toNativeSeparators( file_dir_path + "/" + txt_file_name);

        QStringList params;
        params << QString("-export");
        params << QString(wordlib_path);
        params << QString(target_txt_path);

        QString wordlib_tool_path = Utils::GetBinFilePath().append("huayupy-wl-tool-fcitx");
        //one minute
        wordlib_process->execute(wordlib_tool_path, params);


        QFile file(target_txt_path);
        if (!file.exists())
        {
            Utils::WriteLogToFile("txt file is not exist");
            return false;
        }

        int current_file_count = 0;
        QFile txt_file(target_txt_path);
        if (txt_file.open(QIODevice::ReadOnly))
        {
            while (!txt_file.atEnd())
            {
                QByteArray tmp = txt_file.readLine();
                {
                    current_file_count++;
                }
            }
            txt_file.close();
        }
        Utils::WriteLogToFile(QString("txt wordlib file count is: %2").arg(current_file_count));

        if(current_file_count >= 5)
        {
            QFile::remove(target_txt_path);
            return true;
        }
        else
        {
            QFile::remove(target_txt_path);
            return false;
        }

    }
    catch(std::exception& e)
    {
        Utils::WriteLogToFile("when valid throw exception");
        return false;
    }

}

int WordlibDownloader::RestoreUserWordlib()
{
    try
    {
        QString wordlib_path = GetWordlibSavePath().append(GetWordlibName());
        QString bak_wordlib_path = wordlib_path + ".bak";
        if(!QFile::exists(wordlib_path))
        {
            return 1;
        }

        QFile uwl_file(wordlib_path);
        QFile bak_wordlib_file(bak_wordlib_path);

        auto restore_bak_file = [&](){
            if(bak_wordlib_file.exists())
            {
                if(IsWordlibValid(bak_wordlib_path))
                {
                    Utils::WriteLogToFile("user.uwl is error and user.uwl.bak is ok ");
                    uwl_file.remove();
                    bak_wordlib_file.rename(wordlib_path);
                }
                else
                {
                    Utils::WriteLogToFile("user.uwl is error and user.uwl.bak is error ");
                    uwl_file.remove();
                    bak_wordlib_file.remove();
                }

            }
            else
            {
                Utils::WriteLogToFile("bak file doesn't exist");
                uwl_file.remove();
            }

        };


        int file_size = uwl_file.size();
        if((file_size % 1024) != 0)
        {
            Utils::WriteLogToFile("when restore user.uwl size is not valid");
            restore_bak_file();
            return 0;
        }
        if(!IsWordlibValid(wordlib_path))
        {
            restore_bak_file();
        }
        return 0;

    }
    catch(std::exception& e)
    {
        return 1;
    }
}

bool WordlibDownloader::IsAutoSynchronize()
{
    QSettings config(Config::configFilePath(), QSettings::IniFormat);
    return config.value("account/sysWordLib", 1).toInt();
}
