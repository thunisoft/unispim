#include "downloaddlg.h"
#include "ui_downloaddlg.h"

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QUrl>
#include <QtNetwork/QNetworkRequest>
#include <QFileInfo>
#include <QDesktopServices>
#include <QProcess>

DownloadDlg::DownloadDlg(QWidget *parent) :
    CustomizeQWidget(parent),
    ui(new Ui::DownloadDlg),
    m_package_downloader(nullptr)
{
    ui->setupUi(this);
    ui->closebtn->setProperty("type","closebtn");
    ui->h0_label->setProperty("type","h0");
    this->setProperty("type","borderwidget");
    QPixmap pixmap(":/image/logo.png");
    ui->logo_label->setPixmap(pixmap.scaledToWidth(ui->logo_label->width()));
    connect(ui->closebtn, SIGNAL(clicked()), this, SLOT(SlotDistrubDownload()));

}

DownloadDlg::~DownloadDlg()
{
    delete ui;
    delete m_package_downloader;
}

void DownloadDlg::StartDownload(const QString downloadUrl)
{
    if(!m_package_downloader)
        m_package_downloader = new PackageDownloader(this);
    m_package_downloader->StartDownload(downloadUrl);
    connect(m_package_downloader, SIGNAL(DownloadFinished()), this, SLOT(OnDownLoadFinished()));
}

void DownloadDlg::SetProgress(qint64 currentValue ,qint64 totalValue)
{
    ui->progressBar->setMaximum((int)totalValue);
    ui->progressBar->setValue((int)currentValue);
}

void DownloadDlg::SetFileName(const QString file_path)
{
    m_file_path = file_path;
}

void DownloadDlg::OnDownLoadFinished()
{
    InstallPackage(m_file_path);
}

void DownloadDlg::InstallPackage(const QString filePath)
{
    QProcess::startDetached(filePath);
    //save the config info
    exit(0);
}

void DownloadDlg::SlotDistrubDownload()
{
    emit exitDownload();
}
