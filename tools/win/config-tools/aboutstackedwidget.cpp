#include "aboutstackedwidget.h"
#include "ui_aboutstackedwidget.h"
#include <QPalette>
#include <QImage>
#include <QPixmap>
#include <QDesktopServices>
#include <QDialog>
#include <QUrl>
#include <QProcess>
#include <QEventLoop>
#include <QTimer>
#include <QMovie>
#include <QSettings>
#include <QMovie>
#include <QFile>
#include <QTextStream>
#include "downloadthread.h"
#include "networkhandler.h"
#include "config.h"
#include "utils.h"
#include <QDir>

AboutStackedWidget::AboutStackedWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AboutStackedWidget),
    m_has_recive_msg_from_daemon(false)
{
    ui->setupUi(this);
    InitWidget();
    LoadConfig();
    ConnectSignalToSlot();

}

AboutStackedWidget::~AboutStackedWidget()
{
    delete ui;
}



void AboutStackedWidget::InitWidget()
{
    ui->version_label->setProperty("type","h2");
    ui->name_label->setProperty("type","h2");

    ui->version_label->setText("V3.5");
    ui->name_label->setText("华宇拼音输入法");
    m_downloadThread = new DownloadThread();

    //等待下载的时候的缓冲图
    QMovie* loadingGif = new QMovie(":/image/loading.gif");
    ui->loading_label->setAlignment(Qt::AlignCenter);
    loadingGif->setScaledSize(QSize(24,24));
    ui->loading_label->setMovie(loadingGif);
    ui->loading_label->movie()->start();

    ui->web_address_logo->setProperty("type","aboutLogoLabel");
    ui->bbs_logo_label->setProperty("type","aboutLogoLabel");
    ui->phone_logo_label->setProperty("type","aboutLogoLabel");
    ui->ime_web_logo_label->setProperty("type","aboutLogoLabel");
    ui->copyRight_label->setProperty("type", "copyright");

    ui->phone_content_label->setText("400-810-1866");
    ui->web_address_content_label->setOpenExternalLinks(true);
    ui->web_address_content_label->setText("<style>  a:link{color:#666666;text-decoration:none;}a:visited{}a:hover{color:;}a:active{}</style><a href=\"http://www.thunisoft.com\">www.thunisoft.com</a>");
    ui->web_address_content_label->setToolTip("官网");
    ui->phone_content_label->setToolTip("联系电话");

    ui->ime_web_content_label->setText("<style>  a:link{color:#666666;text-decoration:none;}a:visited{}a:hover{color:blue;}a:active{}</style><a href=\"http://pinyin.thunisoft.com\">pinyin.thunisoft.com</a>");
    ui->ime_web_content_label->setOpenExternalLinks(true);
    ui->ime_web_content_label->setToolTip("输入法官网");

    ui->bbs_content_label->setText("<style>  a:link{color:#666666;text-decoration:none;}a:visited{}a:hover{color:blue;}a:active{}</style><a href=\"http://bbs.pinyin.thunisoft.com/forum.php\">bbs.pinyin.thunisoft.com</a>");
    ui->bbs_content_label->setToolTip("输入法论坛");
    ui->bbs_content_label->setOpenExternalLinks(true);

    ui->copyRight_label->setText("Copyright@2021.Thunisoft");
    ui->update_now_btn->setText("版本检测");
    ui->update_now_btn->setProperty("type","login");
    ui->update_now_btn->setCursor(Qt::PointingHandCursor);
    ui->auto_update_checkbox->setText("自动更新");
    ui->loading_label->setVisible(false);

//  ui->phone_content_label->setStyleSheet("padding-bottom:1px;");
    ui->web_address_content_label->setStyleSheet("margin-bottom:2px;");
    ui->ime_web_content_label->setStyleSheet("margin-bottom:2px;");
    ui->bbs_content_label->setStyleSheet("margin-bottom:2px;");


    QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Thunisoft\\Huayupy\\7.0",QSettings::NativeFormat);
    QString installPath = settings.value("InstallDir","").toString();
    if(installPath.isEmpty())
    {
      ui->version_label->setText("V7.0.0.0");
    }
    else
    {
        QString fullFilePath = installPath + "\\version";
        QFile file(fullFilePath);
        if(file.open(QIODevice::ReadOnly))
        {
            QString versionNum = file.readAll();
            versionNum = versionNum.trimmed();
            m_currentVersionNum = versionNum;

            QString versionStr = QString("V").append(versionNum);
            ui->version_label->setText(versionStr);
        }
        else
        {
            ui->version_label->setText("V7.0");
        }
    }

}


void AboutStackedWidget::ConnectSignalToSlot()
{

    connect(ui->update_now_btn, SIGNAL(clicked(bool)), this, SLOT(UpdateNowSlot()));
}

void AboutStackedWidget::LoadConfig()
{
    ui->auto_update_checkbox->disconnect();
    ConfigItemStruct configInfo;
    int flag = 1;
    if(Config::Instance()->GetConfigItemByJson("is_auto_update",configInfo))
    {
         flag = configInfo.itemCurrentIntValue;
    }

    if(flag)
    {
        ui->auto_update_checkbox->setChecked(true);
    }
    else
    {
        ui->auto_update_checkbox->setChecked(false);
    }
    connect(ui->auto_update_checkbox, SIGNAL(stateChanged(int)), this, SLOT(AutoUpdateCheckStateChangedSlot(int)));
}

void AboutStackedWidget::UpdateNowSlot()
{

    if(IsNeedToRestartThePC())
    {
        Utils::NoticeMsgBox("等待重启中\n请重启后再进行检查",this);
        return;
    }
     //对网络进行检查
    if(!NetworkHandler::Instance()->IsNetworkConnected())
    {
        Utils::NoticeMsgBox("网络连接异常",this);
        return;
    }
    if(!NetworkHandler::Instance()->canVisitWeb())
    {
        Utils::NoticeMsgBox("网络访问异常",this);
        return;
    }

    ui->update_now_btn->setEnabled(false);
    ServerPackageStruct inputStruct;
    int returnCode = NetworkHandler::Instance()->ClientVersionDetection(m_currentVersionNum.trimmed(),inputStruct);
    if(returnCode != 0)
    {
        ui->update_now_btn->setEnabled(true);
        emit checkfailed();
        return ;
    }
    else
    {
        if(inputStruct.has_update)
        {
            int versionCheckCode = VersionCheck(inputStruct.newVersionNum);
            QString InfomContent;
            if(versionCheckCode == 1)
            {
                InfomContent = QString("检测到新版本%1\n是否更新?").arg(inputStruct.newVersionNum);
            }
            else if(versionCheckCode == -1)
            {
                InfomContent = QString("检测到版本回退%1\n是否更新?").arg(inputStruct.newVersionNum);
            }
            else
            {
                InfomContent = QString("新版本检测异常");
                Utils::NoticeMsgBox(InfomContent,this);
                ui->update_now_btn->setEnabled(true);
                return;
            }

            if(Utils::NoticeMsgBox(InfomContent,this,1) == QDialog::Rejected)
            {
                ui->update_now_btn->setEnabled(true);
                return;
            }

            //获取需要保存的文件地址
            QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

            //如果安装包已经静默下载好了的话，则不用再进行手动下载直接启动更新
            QString package_name = inputStruct.pacakge_name;
            QString fileSavePath = QString("%1/%2").arg(tempDir).arg(package_name);
            if(QFile::exists(fileSavePath))
            {
                QString local_md5 = Utils::GetFileMD5(fileSavePath);
                if(local_md5 == inputStruct.packageMd5)
                {
                    ui->update_now_btn->setEnabled(true);
                    QProcess::startDetached(fileSavePath);
                    exit(0);
                    return;
                }
            }

            if(inputStruct.download_type.isEmpty())
            {
                QString clientID = Config::Instance()->GetHostMacAddress();
                QString downloadUrl = NetworkHandler::Instance()->CombineDownloadPackageUrl(inputStruct.setupid,clientID);

                //开辟线程进行下载
                m_downloadThread->SetSourceAndDestFilePath(downloadUrl,tempDir);
                m_downloadThread->SetPackageMd5(inputStruct.packageMd5);
            }
            else
            {
                QString fileSavePath = QString("%1/%2").arg(tempDir).arg(package_name);
                m_downloadThread->SetDownloadUrlAndDestPath(inputStruct.download_url,fileSavePath);
                m_downloadThread->SetDownloadType(inputStruct.download_type);
                m_downloadThread->SetPackageMd5(inputStruct.packageMd5);
            }
            connect(m_downloadThread, SIGNAL(downloadFinished(int)), this, SLOT(PackageDownloadFinished(int)));
            connect(m_downloadThread, SIGNAL(packageCorrupted()),this, SLOT(SlotPackageCorruptedInform()));
            ui->loading_label->setVisible(true);
            m_downloadThread->start();
        }
        else
        {
            ui->update_now_btn->setEnabled(true);
            Utils::NoticeMsgBox("已经是最新版",this);
        }
    }

}

void AboutStackedWidget::AutoUpdateCheckStateChangedSlot(int state)
{
    ConfigItemStruct configInfo;
   if(state == Qt::Checked)
   {

       configInfo.itemName = "is_auto_update";
       configInfo.itemCurrentIntValue = 1;
       configInfo.itemGroupName = "state";
       Config::Instance()->SetConfigItemByJson("is_auto_update",configInfo);
   }
   else
   {
       configInfo.itemName = "is_auto_update";
       configInfo.itemCurrentIntValue = 0;
       configInfo.itemGroupName = "state";
       Config::Instance()->SetConfigItemByJson("is_auto_update",configInfo);
   }
   Config::Instance()->SaveSystemConfig();
}


void AboutStackedWidget::OnReceiveMessage(int receivedValue)
{
    Q_UNUSED(receivedValue)
}


void AboutStackedWidget::SetNormalStatus()
{
    ui->update_now_btn->setEnabled(true);
    ui->loading_label->setVisible(false);
}

void AboutStackedWidget::ResetConfigInfo()
{
    ConfigItemStruct configInfo;
    configInfo.itemName = "is_auto_update";
    configInfo.itemCurrentIntValue = 1;
    Config::Instance()->SetConfigItemByJson("is_auto_update",configInfo);
    ui->auto_update_checkbox->setChecked(true);
}

void AboutStackedWidget::SlotPackageCorruptedInform()
{
    Utils::NoticeMsgBox("安装包下载异常\n请重新下载",this);
    ui->update_now_btn->setEnabled(true);
    ui->loading_label->setVisible(false);
}

void AboutStackedWidget::PackageDownloadFinished(int returnCode)
{
    if(returnCode == 0)
    {
        ui->update_now_btn->setEnabled(true);
        ui->loading_label->setVisible(false);
        Config::Instance()->SaveConfig();
        QString downloadFilePath = m_downloadThread->GetDownloadFilePath();
        if(QFile::exists(downloadFilePath))
        {
            QProcess::startDetached(downloadFilePath);
        }

        exit(0);
    }
    else
    {
        ui->update_now_btn->setEnabled(true);
        ui->loading_label->setVisible(false);
        Utils::NoticeMsgBox("安装包打开失败",this);
    }

}

bool AboutStackedWidget::IsNeedToRestartThePC()
{
    QString installPath = Config::Instance()->GetInstallDir();
    QString tempDir = installPath + "\\HuaYuPY.new";
    QDir checkDir;
    if(checkDir.exists(tempDir))
    {
        return true;
    }
    return false;
}

int  AboutStackedWidget::VersionCheck(QString newVersion)
{
    QStringList newVersionNumList = newVersion.split(".");

    QStringList currentVersionList = m_currentVersionNum.split(".");

    if(newVersionNumList.size() != currentVersionList.size())
    {
        return 0;
    }

    int numberSize = newVersionNumList.size();
    for(int index=0; index<numberSize; ++index)
    {
        int currentIndexValue = currentVersionList.at(index).toInt();
        int newIndexValue = newVersionNumList.at(index).toInt();
        if(currentIndexValue == newIndexValue)
        {
            continue;
        }
        if(newIndexValue > currentIndexValue)
        {
            return 1;
        }
        if(newIndexValue < currentIndexValue)
        {
            return -1;
        }
    }
    //都相同的话报检测异常
    return 0;

}

