#include "downloadthread.h"
#include "networkhandler.h"
#include "utils.h"

bool DownloadThread::downloadState = false;

void DownloadThread::run()
{
    try {
        downloadState = true;
        Utils::WriteLogToFile(QString("start download package:source_filePath:%1,destDir%2,m_downloadFilePath:%3")
                              .arg(m_sourceFilePath)
                              .arg(m_destDir)
                              .arg(m_downloadFilePath));

        int downloadCode = -1;

        if(m_downloadType.isEmpty())
        {
            downloadCode = NetworkHandler::Instance()->DownloadFileFromWeb(m_sourceFilePath,m_destDir,m_downloadFilePath);
        }
        else
        {
            downloadCode = NetworkHandler::Instance()->DownloadFileFromWeb(m_sourceFilePath,m_downloadFilePath);
        }

        downloadState = false;
        Utils::WriteLogToFile("emit download signal signals");
        QString downloadFileMd5 = Utils::GetFileMD5(m_downloadFilePath);
        if(m_package_md5.isEmpty())
        {
            emit downloadFinished(downloadCode);
        }
        else if(downloadFileMd5 == m_package_md5)
        {
            emit downloadFinished(downloadCode);
        }
        else {
            emit packageCorrupted();
        }


    } catch (exception e) {
        Utils::WriteLogToFile(QString(e.what()));

    }

}

void DownloadThread::SetSourceAndDestFilePath(QString sourceFilePath, QString destDir)
{
    m_sourceFilePath = sourceFilePath;
    m_destDir = destDir;
}

void DownloadThread::SetPackageMd5(QString fileMd5)
{
    m_package_md5 = fileMd5;
}

QString DownloadThread::GetDownloadFilePath()
{
    return m_downloadFilePath;
}

void DownloadThread::SetDownloadUrlAndDestPath(QString input_url, QString file_download_path)
{
    m_sourceFilePath = input_url;
    m_downloadFilePath = file_download_path;
}

void DownloadThread::SetDownloadType(QString download_type)
{
    m_downloadType = download_type;
}


