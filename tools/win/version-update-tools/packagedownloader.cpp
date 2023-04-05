#include "packagedownloader.h"

#include <QtNetwork/QNetworkAccessManager>
#include <QUrl>
#include <qurlquery.h>
#include <QtNetwork/QNetworkRequest>
#include <QFileInfo>
#include <QDesktopServices>
#include <QProcess>
#include <QRegExp>
#include <QSslConfiguration>
#include <QSslCertificate>
#include <QDir>
#include <QCoreApplication>

#include "downloaddlg.h"
#include "utils.h"
#include <windows.h>
#include <securitybaseapi.h>
#include <QFileInfo>

PackageDownloader::PackageDownloader(DownloadDlg* const downloadDlg, QObject *parent) :
    QObject(parent),
    m_download_dialog(downloadDlg)
{

}


// 当前程序是否运行在管理员权限下
BOOL IsRunAsAdmin()
{
	BOOL bIsRunAsAdmin = FALSE;
	PSID pAdministratorsGroup = nullptr;

	// Allocate and initialize a SID of the administrators group.
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdministratorsGroup))
	{
		// Determine whether the SID of administrators group is enabled in the primary access token of the process.
		if (CheckTokenMembership(nullptr, pAdministratorsGroup, &bIsRunAsAdmin) == FALSE)
			bIsRunAsAdmin = FALSE;

		if (pAdministratorsGroup)
			FreeSid(pAdministratorsGroup);
	}

	return(bIsRunAsAdmin);
}

void PackageDownloader::StartDownload(const QString downloadUrl)
{
	QString runs;
	if (IsRunAsAdmin())
		runs = " is Admin";
	else
		runs = " isn't Admin";

	Utils::WriteLogToFile(QString("StartDownload: %1").arg(runs));


    //设置对应的url对文件进行下载
    QNetworkRequest request;
    QUrl url(downloadUrl);
    m_downloadUrl = downloadUrl;
    QNetworkAccessManager *accessManager=new QNetworkAccessManager(this);
    request.setUrl(url);
    QString filePath = QDir::toNativeSeparators(QCoreApplication::applicationDirPath() + "\\server_certification.cer");
    if(!SetSSlConfigToRequest(request,filePath))
    {
        return;
    }
    QNetworkReply *reply  = accessManager->get(request);

    connect(accessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(OnDownLoadFinished(QNetworkReply*)));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(LoadProgress(qint64, qint64)));
}

void PackageDownloader::SetDownloadMode(const QString &download_mode)
{
    m_downloadMode = download_mode;
}

void PackageDownloader::LoadProgress(qint64 currentValue ,qint64 totalValue)
{
    if(m_download_dialog)
        m_download_dialog->SetProgress(currentValue, totalValue);
}

void PackageDownloader::OnDownLoadFinished(QNetworkReply*reply)
{
    //获取文件保存路径
    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (statusCode.toInt() != 200)
    {
		Utils::WriteLogToFile(QString("OnDownLoadFinished: error=%1, %2").arg(statusCode.toInt()).arg(reply->errorString()));
		Utils::WriteLogToFile(QString("OnDownLoadFinished: header"));
		const QList<QNetworkReply::RawHeaderPair>& _list = reply->rawHeaderPairs();
		for (auto it : _list)
		{
			Utils::WriteLogToFile(QString("OnDownLoadFinished: \t%1=%2").arg(QString(it.first)).arg(QString(it.second)));
		}
		exit(0);
        return;
    }

	Utils::WriteLogToFile(QString("OnDownLoadFinished OK"));

    //判断返回的请求是minIO还是字节流
    QUrl url(m_downloadUrl);
    QString fileName;
    if(m_downloadMode == "cdn")
    {
        QFileInfo url_info(m_downloadUrl);
        fileName = url_info.fileName();
    }
    else
    {
        QUrlQuery query(url.query());
        fileName = query.queryItemValue("filename");
    }

    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString fileSavePath = QString("%1/%2").arg(tempDir).arg(fileName);

    QString fileTmpSavePath = fileSavePath + ".tmp.update";
    if(QFile::exists(fileTmpSavePath))
    {
        QFile::remove(fileTmpSavePath);
    }

    QString fileBakSavePath = fileSavePath + ".bak.update";
    if(QFile::exists(fileBakSavePath))
    {
        QFile::remove(fileBakSavePath);
    }

    QFile file(fileTmpSavePath);
    if(!file.open(QIODevice::WriteOnly))
    {
		Utils::WriteLogToFile(QString("OnDownLoadFinished: Open %1 failed").arg(fileTmpSavePath));
		exit(0);
        return ;
    }
    file.write(reply->readAll());
    file.close();


    if(QFile::exists(fileSavePath))
    {
        QFile::rename(fileSavePath,fileBakSavePath);
    }
    QFile::rename(fileTmpSavePath,fileSavePath);
    m_packagePath = fileSavePath;
    emit DownloadFinished();
}

void PackageDownloader::SetFinishExitSlot()
{
    connect(this, SIGNAL(DownloadFinished()), this, SLOT(ExitSlot()));
}

void PackageDownloader::ExitSlot()
{

}

 QString PackageDownloader::GetPackageSavePath()
 {
     return m_packagePath;
 }

 bool PackageDownloader::SetSSlConfigToRequest(QNetworkRequest &inputRequest, QString certificatePath)
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
