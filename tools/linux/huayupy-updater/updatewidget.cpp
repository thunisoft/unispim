#include "updatewidget.h"
#include "checkupdateunispy.h"
#include "config.h"
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
//#include <QDesktopServices>
#include <QAction>

#include <unistd.h>
#include <stdlib.h>
#include "cJSON.h"
#include "iniparser.h"
#include "msgtodaemon.h"
#include "passworddialog.h"
#include "mainwindow.h"
#include "../public/utils.h"

UpdateWidget::UpdateWidget(QWidget *parent):QWidget(parent),
     ui(new Ui::updateWidget),
     m_has_finished(false),
     m_download_path(""),
     m_file_name(""),
     m_new_version(""),
     m_is_downloading(false),
     m_user_home_location(""),
     m_first_write_pwd(true)
{
    GetNewVersionAndPathAndOs(m_new_version, m_download_path, m_os_type,m_addr_type);
    m_file_name = *m_download_path.split("=").rbegin();

    ui->setupUi(this);
    ui->textEdit->setReadOnly(true);
#ifdef USE_UOS
    ui->textEdit->append(QString::fromLocal8Bit("<font size=\"12\">安装完成后，请重启电脑，使更新生效！</font>"));
#endif
//    QAction* password_action = new QAction(ui->lineEdit);
//    password_action->setIcon(QIcon("/usr/share/huayupy/image/password.png"));
//    password_action->setEnabled(false);
//    ui->lineEdit->addAction(password_action, QLineEdit::LeadingPosition);
    //connect(m_button_update_huayupy, SIGNAL(clicked(bool)), this, SLOT(UpdadateUnispy()));
    //DownloadPacakge();

    //ui->cancel_button->setVisible(false);

    cmd = new QProcess(this);
    connect(cmd, SIGNAL(readyReadStandardOutput()), this, SLOT(OnReadOutPut()));
    connect(cmd, SIGNAL(readyReadStandardError()), this, SLOT(OnReadError()));
    connect(ui->ok_button, SIGNAL(clicked(bool)), this, SLOT(OnConfirmButtonClicked()));
    //connect(ui->cancel_button,SIGNAL(clicked(bool)),this,SLOT(OnConfirmButtonClicked()));
    cmd->start("bash");
    cmd->waitForStarted();

    QString packagePath = GetPackageSavePath();

    if(QFile::exists(packagePath))
    {
        SlotInstallWhenPackageIsDownloaded();
    }
    else
    {
        connect(parent, SIGNAL(SignalDownloadPacakge()), this, SLOT(DownloadPacakge()));
    }
}

void UpdateWidget::SetPassword(QString &pwd)
{
    m_pwd = pwd;
}

bool UpdateWidget::GetNewVersionAndPathAndOs(QString& new_version, QString& new_version_down_path, QString& os_type,QString& addr_type)
{
    QString configPath = Config::GetSaveNewVersionInfoPath();
    if(!FileExists(configPath.toStdString()))
    {
        return false;
    }else
    {
        dictionary *dict = iniparser_load(configPath.toStdString().c_str());
        if (!dict)
              return false;
        new_version = iniparser_getstring(dict, "new_version_info:version", NOTFOUND);
        //new_version_down_path = iniparser_getstring(dict, "new_version_info:path", NOTFOUND);
        //QByteArray downloadPath = iniparser_getstring(dict, "new_version_info:path", NOTFOUND);
        const char *downloadPath = iniparser_getstring(dict, "new_version_info:path", NOTFOUND);
        new_version_down_path = QByteArray::fromPercentEncoding(downloadPath);
        os_type = iniparser_getstring(dict, "new_version_info:os_type", NOTFOUND);
        addr_type = iniparser_getstring(dict,"new_version_info:addr_type",NOTFOUND);
        return true;
    }
}


int UpdateWidget::FileExists(const string& filepath)
{
    if (access(filepath.c_str(), F_OK) == 0)
        return 1;

    return 0;
}

void UpdateWidget::DownloadPacakge()
{
    QNetworkRequest request;
    QUrl url(m_download_path);
    m_file_name = *m_download_path.split("=").rbegin();
    QNetworkAccessManager *accessManager=new QNetworkAccessManager(this);
    request.setUrl(url);
    request.setSslConfiguration(MainWindow::GetQsslConfig());
    QNetworkReply *reply  = accessManager->get(request);

    //m_progress_bar->show();
    connect(accessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(OnDownLoadFinished(QNetworkReply*)));
    connect(reply, SIGNAL(downloadProgress(qint64 ,qint64)), this, SLOT(LoadProgress(qint64 ,qint64)));
    //m_button_update_unispy->setVisible(false);
    m_is_downloading = true;
}

void UpdateWidget::InstallTimeout()
{
    m_has_finished = true;
    InstallDone(true);
}

void UpdateWidget::SlotInstallWhenPackageIsDownloaded()
{
    int max_value = ui->progressBar->maximum();
    ui->progressBar->setValue(max_value);
    ui->label->setText(QString::fromLocal8Bit("正在安装..."));
    DoInstall();
}

void UpdateWidget::OnDownLoadFinished(QNetworkReply* reply)
{
    //m_progress_bar->setVisible(false);
    m_is_downloading = false;
    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (statusCode.isValid())
    {
        qDebug() << "status code: " << statusCode.toInt();
    }
    GetUserHomeLocation();
    CreateDir(m_user_home_location);
    QFile file(QString("%1%2").arg(m_user_home_location).arg(m_file_name));
    if(!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "fileName: "<< m_file_name << " errorInfo: " << file.errorString();
        return;
    }
    file.write(reply->readAll());
    file.close();

    DoInstall();
}

void UpdateWidget::DoInstall()
{
    QByteArray install_command;
    QString save_path = GetPackageSavePath();
    if(strcmp(m_os_type.toStdString().c_str(), "neokylin") == 0)
    {
       m_packege_type = RPM;
       install_command = QString("sudo -S rpm -Uvh %1").arg(save_path).toUtf8() + '\n';
    }
    else{
       m_packege_type = DEB;
#ifdef USE_UOS
       FILE* ptr2 = NULL;
       string command = QString("deepin-deb-installer %1").arg(save_path).append(" &").toStdString();
       if((ptr2 = popen(command.c_str(), "r")) != NULL)
       {
           pclose(ptr2);
       }
       MsgToDaemon::SendMsgToDaemon(MSG_FROM_UPDATER_FINISH);
       emit to_exit();
#else
       install_command = QString("sudo -S dpkg -i %1").arg(save_path).toUtf8() + '\n';
#endif
    }
    cmd->write(install_command);

    MsgToDaemon::SendMsgToDaemon(MSG_FROM_UPDATER_FINISH);

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

 bool UpdateWidget::CheckHasInstalledGnomeTerminal()
 {
     FILE* fp = popen("ls /usr/bin |grep gnome-terminal |grep -v 'grep' |wc -l", "r");
     char buffer[1024] = {0};
     int read_bytes = fread(buffer, 1, sizeof(buffer), fp);
     pclose(fp);
     if(read_bytes)
     {
         if('0' == buffer[0])
         {
             return false;
         }else
         {
             return true;
         }
     }
     else
     {
         qDebug()<< "未能检测到是否安装gnome-terminal";
         return false;
     }
 }

 void UpdateWidget::GetUserHomeLocation()
 {
     m_user_home_location = "/tmp/huayupy/";
     qDebug()<<m_user_home_location;
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
        //RestartInputFrame();
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
        //RestartInputFrame();
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
     QApplication::exit();
//     if(!m_has_finished)
//     {
////         cmd->write(ui->lineEdit->text().toLocal8Bit() + '\n');
////         ui->lineEdit->clear();
//     }
//     else
//     {
//        QApplication::exit();
//     }

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
     QString packageFilename;
     //判断下载方式，cdn和sever路径返回不一致
     if(m_addr_type == "cdn")
        packageFilename = *m_download_path.split("/").rbegin();
     else if(m_addr_type == "server")
         packageFilename = *m_download_path.split("=").rbegin();
     else
         return "";

     QString download_dir = Utils::GetConfigDirPath() + "packagedownload/";
     QString targetPath = download_dir + packageFilename;

     return targetPath;
 }
