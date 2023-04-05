#include "updatewidget.h"
#include <QSettings>
#include <QDesktopServices>
#include <QDir>
#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QAction>
#include <QApplication>
#include <QDesktopWidget>

#include <unistd.h>
#include <stdlib.h>
#include "passworddialog.h"
#include "../public/utils.h"
#include "ui_updatewidget.h"
#include "../public/configmanager.h"
#include "wordlibpage/myutils.h"
#include "../public/defines.h"
#include "aboutstackedwidget.h"

UpdateWidget::UpdateWidget(QWidget *parent):QWidget(parent),
     m_download_path(""),
     m_file_name(""),
     m_new_version(""),
     m_is_downloading(false),
     m_user_home_location(""),
     ui(new Ui::updateWidget),
     m_has_finished(false),     
     m_first_write_pwd(true)
{

    ui->setupUi(this);
    ui->textEdit->setReadOnly(true);
    this->setWindowTitle("安装包下载");
    setWindowIcon(QIcon(":/image/logo.png"));
    move ((QApplication::desktop()->width() - width())/2,(QApplication::desktop()->height() - height())/2);
    QSettings config(TOOLS::PathUtils::GetUpdaterIniPath(), QSettings::IniFormat);
    config.beginGroup("package_info");
    m_os_type = config.value("os_type", "").toString();
    config.endGroup();
    m_file_name = *m_download_path.split("=").rbegin();

    cmd = new QProcess(this);
    connect(cmd, SIGNAL(readyReadStandardOutput()), this, SLOT(OnReadOutPut()));
    connect(cmd, SIGNAL(readyReadStandardError()), this, SLOT(OnReadError()));
    connect(ui->ok_button, SIGNAL(clicked(bool)), this, SLOT(OnConfirmButtonClicked()));
    cmd->start("bash");
    cmd->waitForStarted();
}

void UpdateWidget::SetPassword(QString &pwd)
{
    m_pwd = pwd;
}

void UpdateWidget::StartUpdate()
{
    ui->textEdit->clear();
#ifdef USE_UOS
    ui->textEdit->append(QString::fromLocal8Bit("<font size=\"12\">安装完成后，请重启电脑，使更新生效！</font>"));
#endif
    m_file_name = *m_download_path.split("=").rbegin();
    QString packagePath = GetPackageSavePath();


    if(QFile::exists(packagePath))
    {

        AboutStackedWidget::WriteLog("local exist the package");
        QString package_md5 = Utils::GetFileMD5(packagePath).toLower();
        if(package_md5 == m_package_md5)
        {
           AboutStackedWidget::WriteLog("local md5 is same with server call install process");
            DoInstall();
            return;
        }
        else
        {
            AboutStackedWidget::WriteLog(QString("local md5 is different from server,local:%1,server:%2").arg(package_md5).arg(m_package_md5));
            RemovePackagePathContent();
            DownloadPacakge();
        }
    }
    else
    {
        DownloadPacakge();
    }
}

void UpdateWidget::SetDownloadUrl(QString &downloadUrl)
{
    m_download_path = downloadUrl;
}

void UpdateWidget::SetPackageMd5(const QString &file_md5)
{
    m_package_md5 = file_md5;
}


void UpdateWidget::DownloadPacakge()
{
    QNetworkRequest request;    
    QString download_path = m_download_path;
    download_path.append("&optype=").append(QString::number(OPType::MANUAL));
    QUrl url(download_path);
    m_file_name = *m_download_path.split("=").rbegin();

    if(!m_accessManager)
    {
        m_accessManager = new QNetworkAccessManager(this);
    }

    request.setUrl(url);
    request.setSslConfiguration(Utils::GetQsslConfig());
    QNetworkReply *reply  = m_accessManager->get(request);
    AboutStackedWidget::WriteLog("Download package from server");
    connect(m_accessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(OnDownLoadFinished(QNetworkReply*)));
    connect(reply, SIGNAL(downloadProgress(qint64 ,qint64)), this, SLOT(LoadProgress(qint64 ,qint64)));
    m_is_downloading = true;
}


void UpdateWidget::SlotInstallWhenPackageIsDownloaded()
{
    m_is_downloading = false;
    int max_value = ui->progressBar->maximum();
    ui->progressBar->setValue(max_value);
    ui->label->setText(QString::fromLocal8Bit("正在安装..."));
    DoInstall();
}

void UpdateWidget::OnDownLoadFinished(QNetworkReply* reply)
{

    m_is_downloading = false;
    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (!statusCode.isValid())
    {
        AboutStackedWidget::WriteLog(QString("download finished statusCode is not valid:%1").arg(statusCode.toInt()));
        return;
    }
    QString package_save_path = GetPackageSavePath();
    if(QFile::exists(package_save_path))
    {
        AboutStackedWidget::WriteLog("local exist package file remove");
        QFile::remove(package_save_path);
    }
    QFile file(package_save_path);
    if(!file.open(QIODevice::WriteOnly))
    {
        AboutStackedWidget::WriteLog("create package file failed");
        return;
    }
    file.write(reply->readAll());
    file.close();
    QString download_packge_md5 = Utils::GetFileMD5(package_save_path).toLower();
    if(download_packge_md5 == m_package_md5)
    {
        AboutStackedWidget::WriteLog(QString("download package md5 check succeed"));
        DoInstall();
    }
    else
    {
        ui->textEdit->setText("安装包下载异常");
        AboutStackedWidget::WriteLog(QString("download package md5 check failed local:%1 server:%2").arg(download_packge_md5).arg(m_package_md5));
    }
}

void UpdateWidget::DoInstall()
{
    QByteArray install_command;
    QString package_save_path = GetPackageSavePath();
    if(strcmp(m_os_type.toStdString().c_str(), "neokylin") == 0)
    {
       m_packege_type = RPM;
       install_command = QString("sudo -S rpm -Uvh %1").arg(package_save_path).toUtf8() + '\n';
    }
    else{
       m_packege_type = DEB;
#ifdef USE_UOS
       FILE* ptr2 = NULL;
       string command = QString("deepin-deb-installer %1").arg(package_save_path).append(" &").toStdString();
       if((ptr2 = popen(command.c_str(), "r")) != NULL)
       {
           pclose(ptr2);
       }
       emit to_exit();
#else
       install_command = QString("sudo -S dpkg -i %1").arg(package_save_path).toUtf8() + '\n';
#endif
    }
    cmd->write(install_command);
}

 void UpdateWidget::LoadProgress(qint64 bytesSent, qint64 bytesTotal)
 {
     ui->progressBar->setMaximum(bytesTotal); //最大值
     ui->progressBar->setValue(bytesSent);  //当前值
     if(bytesSent < bytesTotal )
     {
         ui->label->setText(QString::fromLocal8Bit("下载中···"));
     }
     else
     {
         ui->progressBar->setMaximum(0);
         ui->progressBar->setMinimum(0);
         ui->label->setText(QString::fromLocal8Bit("正在安装..."));
         ui->ok_button->setEnabled(true);
     }
 }



 void UpdateWidget::GetUserHomeLocation()
 {
     m_user_home_location = "/tmp/huayupy/";
 }

 void UpdateWidget::CreateDir(QString &path)
 {
     QDir dir(path);
     if(!dir.exists())
     {
         dir.mkpath(path);
     }
 }

 bool UpdateWidget::IsDownLoading()
 {
    return m_is_downloading;
 }

 void UpdateWidget::OnReadOutPut()
 {
    QString tmp = QString::fromLocal8Bit(cmd->readAllStandardOutput().data());
    if(tmp == "#")
        return;
    if(tmp.contains("huayupy installed") || tmp.contains("正在设置"))
    {
        m_has_finished = true;
        InstallDone(true);
    }
    if(!m_has_finished)
    {
        ui->textEdit->append(tmp);
    }
 }

 void UpdateWidget::OnReadError()
 {
    QString tmp = QString::fromLocal8Bit(cmd->readAllStandardError().data());
    if(tmp == "#")
        return;
    if(tmp.contains("huayupy installed"))
    {
        m_has_finished = true;
        InstallDone(true);
    }
    else if (tmp.startsWith("[sudo]"))
    {
        if (m_first_write_pwd)
        {
            cmd->write(m_pwd.toLocal8Bit() + '\n');
            m_first_write_pwd = false;
        }
        return;
    }
    else if (tmp.toLower().contains("retry") || tmp.contains("请重试"))
    {
        ui->textEdit->append(tmp);
        PasswordDialog dlg;
        dlg.SetRetry(true);
        int ret = dlg.exec();
        if (ret == QDialog::Accepted)
        {
            m_pwd = dlg.GetPassword();
            cmd->write(m_pwd.toLocal8Bit() + '\n');
        }
        else
        {
            this->parentWidget()->close();
        }
        return;
    }
    else if (tmp.contains("有错误发生") || tmp.contains("conflicts with"))
    {
        InstallDone(false);
    }

    if(!m_has_finished)
    {
        ui->textEdit->append(tmp);
    }
 }

 void UpdateWidget::OnConfirmButtonClicked()
 {
     this->close();
 }

 void UpdateWidget::RestartInputFrame()
 {
    if(m_file_name.contains("fcitx"))
    {
        cmd->write(QString::fromLocal8Bit("fcitx -r ").toUtf8() + '\n');
    }
    else
    {
        cmd->write(QString::fromLocal8Bit("ibus-daemon -r -d -x ").toUtf8() + '\n');
    }
 }

 void UpdateWidget::InstallDone(bool bSuccess)
 {
    if (bSuccess)
    {
        //color=\"#FF0000\"
        ui->textEdit->append(QString::fromLocal8Bit("<font size=\"12\">华宇输入法升级完成!</font>"));
        ui->textEdit->append(QString::fromLocal8Bit("<font size=\"12\">重启电脑使输入法生效!</font>"));
        ui->ok_button->setText(QString::fromLocal8Bit("关闭"));
        ui->label->setText("安装成功");
    }
    else
    {
        ui->label->setText("安装错误");
    }

    ui->progressBar->setFormat("");
    ui->progressBar->setMaximum(100);
    ui->progressBar->setValue(100);
    ui->ok_button->setEnabled(true);
 }

 QString UpdateWidget::GetPackageSavePath()
 {
     QString download_dir = Utils::GetConfigDirPath() + "packagedownload/";
     QDir dir(download_dir);
     if(!dir.exists())
     {
         dir.mkpath(download_dir);
     }

     QString packageFilename = *m_download_path.split("=").rbegin();
     QString targetPath = download_dir + packageFilename;
     return targetPath;
 }

 void UpdateWidget::RemovePackagePathContent()
 {
     QString download_dir = Utils::GetConfigDirPath() + "packagedownload/";
     QDir dir(download_dir);
     QFileInfoList fileList;
     if(!dir.exists())  {return;}//文件不存，则返回false

     fileList=dir.entryInfoList(QDir::Files
                                |QDir::Readable|QDir::Writable
                                |QDir::Hidden|QDir::NoDotAndDotDot
                                ,QDir::Name);

     int file_count =fileList.size();
     for(int index=0; index<file_count;++index)
     {
         QFileInfo curFile = fileList.at(index);

         if(curFile.isFile())//如果是文件，删除文件
         {
             if(QFile::exists(curFile.filePath()))
             {
                 AboutStackedWidget::WriteLog(QString("remove file:%1").arg(curFile.fileName()));
                 QFile::remove(curFile.filePath());
             }

         }

     }
 }
