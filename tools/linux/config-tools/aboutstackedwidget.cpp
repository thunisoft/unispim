#include "aboutstackedwidget.h"
#include "ui_aboutstackedwidget.h"
#include "customize_ui/customizefont.h"
#include <QPalette>
#include <QImage>
#include <QPixmap>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QEventLoop>
#include <QMessageBox>
#include <QTimer>
#include <QMovie>

#include "huayupy_adaptor.h"
#include "huayupy_interface.h"
#include "../public/utils.h"
#include "../public/dbusmessager.h"
#include "../public/configmanager.h"
#include "checkupdatefaild.h"
#include "msgboxutils.h"
#include "newversionchecker.h"
#include "passworddialog.h"
#include "updatewidget.h"

AboutStackedWidget::AboutStackedWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AboutStackedWidget),
    m_has_recive_msg_from_daemon(false)
{
    ui->setupUi(this);
    InitDbus();
    InitWidget();
    InitConfig();
    ConnectSignalToSlot();

}

AboutStackedWidget::~AboutStackedWidget()
{
    m_updateWidget->deleteLater();
    m_newversion_checker->deleteLater();

    delete ui;
}

void AboutStackedWidget::WriteLog(const QString inputStr)
{
    QString logdir = ConfigManager::LocalConfigDir().append("logs/");
    QDir dir(logdir);
    if(!dir.exists())
    {
        dir.mkpath(logdir);
    }
    QString logFileAddr = logdir + "huayupy-config-tools-log";
    QString timeStamp = QDateTime::currentDateTime().toString("yyyy-M-d hh:mm:ss");
    QFile file(logFileAddr);
    bool isSucess = file.open(QIODevice::WriteOnly | QIODevice::Append);
    if(!isSucess)
    {
        return;
    }
    QTextStream output(&file);
    QString content = timeStamp + ":" + inputStr + "\n" ;
    output << content;
    file.close();
}

void AboutStackedWidget::LoadConfigInfo()
{
     ui->auto_update_checkbox->disconnect();
     int enablecheckupdateauto;
     if(ConfigManager::Instance()->GetIntConfigItem("enablecheckupdateauto",enablecheckupdateauto))
     {
         ui->auto_update_checkbox->setChecked(enablecheckupdateauto);
     }
     connect(ui->auto_update_checkbox, SIGNAL(stateChanged(int)), this, SLOT(AutoUpdateCheckStateChangedSlot(int)));
}


void AboutStackedWidget::InitDbus()
{
    new HuayupyAdaptor(this);
    new com::thunisoft::huayupy(QString(), QString(), QDBusConnection::sessionBus(), this);
    QDBusConnection::sessionBus().connect(QString(), QString(), "com.thunisoft.huayupy", "msgFromDaemonToSettings", this, SLOT(OnReceiveMessage(int)));
}

void AboutStackedWidget::InitWidget()
{
    ui->version_label->setProperty("type", "h2");
    ui->version_label->setText("V3.5");
    //ui->name_label->setFont(font);
    ui->name_label->setProperty("type", "h2");
    ui->name_label->setText("华宇输入法");

    ui->phone_content_label->setText("800 810 1866");
    ui->phone2_content_label->setText("400 810 1866");
    ui->web_address_content_label->setOpenExternalLinks(true);
    ui->web_address_content_label->setText("<style> a{text-decoration:none}</style><a href=\"http:////www.thunisoft.com\">www.thunisoft.com</a>");
    ui->ime_address_content_label->setOpenExternalLinks(true);
    ui->ime_address_content_label->setText("<style> a{text-decoration:none}</style><a href=\"http:////pinyin.thunisoft.com\">pinyin.thunisoft.com</a>");

    ui->copyRight_label->setText("Copyright@2021.Thunisoft");
    ui->update_now_btn->setText("立即更新");
    ui->auto_update_checkbox->setText("自动更新");

    ui->update_now_btn->setProperty("type","radiusBtn");
    ui->update_now_btn->setFlat(true);
    ui->update_now_btn->setFocusPolicy(Qt::NoFocus);


    QString styleSheet = "#logo_widget{"
                         "background-image:url(:/image/about_main_logo.png);"
                         "background-repeat: no-repeat;"
                         "background-position:center;"
                         "}";

    QString labelSheet = "background:transparent;"
                         "color:white;"
                         "font-weight: 400;";


    QString phone_and_web_sheet = "color:rgb(90,90,90);";
    QString copyright_sheet = "color:rgb(180,180,180);";


    ui->logo_widget->setStyleSheet(styleSheet);
    ui->version_label->setStyleSheet(labelSheet);
    ui->name_label->setStyleSheet(labelSheet);
    //ui->update_now_btn->setFixedSize(75,25);
    ui->auto_update_checkbox->autoFillBackground();
    ui->phone_content_label->setStyleSheet(phone_and_web_sheet);
    ui->web_address_content_label->setStyleSheet(phone_and_web_sheet);
    ui->copyRight_label->setStyleSheet(copyright_sheet);
    ui->phone2_content_label->setStyleSheet(phone_and_web_sheet);

    QPixmap phonePixmap(":/image/phone1.png");
    QPixmap webPixmap(":/image/web_addr.png");

    ui->phone_logo_label->setPixmap(phonePixmap);
    ui->web_address_logo->setPixmap(webPixmap);

    ui->phone2_logo_label->setPixmap(QPixmap(":/image/phone2.png").scaled(16,16));
    ui->ime_address_logo->setPixmap(QPixmap(":/image/imeWeb.png").scaled(16,16));

    ui->loading_label->setVisible(false);
    QMovie* movie_loading = new QMovie(QString(":/image/loading.gif"));
    ui->loading_label->setScaledContents(true);
    ui->loading_label->setMovie(movie_loading);
    movie_loading->start();
    ui->loading_label->setStyleSheet(labelSheet);

    QString version_file_path;
    QString version_num_str;

    version_file_path = Utils::GetHuayuPYInstallDirPath().append("version.txt");
    QFile version_file(version_file_path);
    if(!version_file.exists() || !version_file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        version_num_str = QString::fromLocal8Bit("版本未知");
    }
    QTextStream text_stream(&version_file);
    version_num_str = text_stream.readLine();
    version_file.close();
    if( version_num_str.isNull() || version_num_str.isEmpty())
    {
        version_num_str = QString::fromLocal8Bit("版本未知");
    }
    if(version_num_str.compare("版本未知") != 0)
    {
        version_num_str = "V" + version_num_str;
    }
    ui->version_label->setText(version_num_str);

    if(!m_newversion_checker)
    {
        m_newversion_checker = new NewVersionChecker(this);
    }
    if(!m_updateWidget)
    {
         m_updateWidget = new UpdateWidget();
    }

    QString typename_config;
    if(ConfigManager::Instance()->GetStrConfigItem("typename",typename_config))
    {
        if(typename_config == "private")
        {
            ui->phone2_content_label->setVisible(false);
            ui->phone2_logo_label->setVisible(false);
            ui->phone_content_label->setVisible(false);
            ui->phone_logo_label->setVisible(false);

            ui->web_address_logo->setVisible(false);
            ui->web_address_content_label->setVisible(false);
            ui->ime_address_content_label->setVisible(false);
            ui->ime_address_logo->setVisible(false);
        }
    }

}


void AboutStackedWidget::ConnectSignalToSlot()
{
    connect(ui->auto_update_checkbox, SIGNAL(stateChanged(int)), this, SLOT(AutoUpdateCheckStateChangedSlot(int)));
    connect(ui->update_now_btn, SIGNAL(clicked(bool)), this, SLOT(UpdateNowSlot()));
    connect(m_newversion_checker, SIGNAL(checkError(int)),this, SLOT(OnCheckError(int)));
    connect(m_newversion_checker, SIGNAL(checkSucceed(int)),this, SLOT(OnCheckSucceed(int)));
}

void AboutStackedWidget::InitConfig()
{
    int enablecheckupdateauto;
    if(ConfigManager::Instance()->GetIntConfigItem("enablecheckupdateauto",enablecheckupdateauto))
    {
        ui->auto_update_checkbox->setChecked(enablecheckupdateauto);
    }
}

void AboutStackedWidget::UpdateNowSlot()
{
    ui->update_now_btn->setEnabled(false);
    ui->loading_label->setVisible(true);
    WriteLog("start version check");
    m_newversion_checker->CheckTheVersion();
}

void AboutStackedWidget::AutoUpdateCheckStateChangedSlot(int state)
{
    ConfigManager::Instance()->SetIntConfigItem("enablecheckupdateauto",(state == Qt::Checked)? 1:0);
}


void AboutStackedWidget::OnCheckError(int errorCode)
{
    SetNormalStatus();
    if(errorCode == REQUEST_TIME_OUT)
    {
        MsgBoxUtils::NoticeMsgBox(QString("请求超时"),"系统提示",0,this);
    }
    else if(errorCode == REQUEST_RESULT_ERROR)
    {
        MsgBoxUtils::NoticeMsgBox(QString("请求异常"),"系统提示",0,this);
    }
    else if(errorCode == REUQEST_RETURN_CODE_ERROR)
    {
        MsgBoxUtils::NoticeMsgBox(QString("请求返回参数异常"),"系统提示",0,this);
    }
    else if(errorCode == UNKNOWN_ERROR)
    {
        MsgBoxUtils::NoticeMsgBox(QString("异常错误"),"系统提示",0,this);
    }
    else
    {
        MsgBoxUtils::NoticeMsgBox(QString("未知异常"),"系统提示",0,this);
    }
}

void AboutStackedWidget::OnCheckSucceed(int succeedCode)
{
    SetNormalStatus();

    if(succeedCode == HAS_NEW_VERSION)
    {
        QString newVersion = m_newversion_checker->GetNewVersionStr();
        int code = MsgBoxUtils::NoticeMsgBox(QString("检测到新版本%1\n是否更新?").arg(newVersion),"系统提示",1,this);
        if(code == QDialog::Accepted)
        {
            QString new_version_url = m_newversion_checker->GetDownloadUrl();
            QString package_md5 = m_newversion_checker->GetPackageMd5();
            DownloadAndInstallNewPackage(new_version_url,package_md5);
        }
        else if(code == QDialog::Rejected)
        {
            return;
        }
    }
    else if(succeedCode == VERSION_UPTODATE)
    {
        MsgBoxUtils::NoticeMsgBox(QString("已经是最新版本"),"系统提示",0,this);
    }
}


void AboutStackedWidget::OnReceiveMessage(int receivedValue)
{
    m_has_recive_msg_from_daemon = true;
    if(receivedValue == CHECKING)
    {
        SetNormalStatus();
        MsgBoxUtils::NoticeMsgBox("输入法正在升级","系统提示",0,this);

    }else if(receivedValue == UP_TO_DATE)
    {
        SetNormalStatus();
        MsgBoxUtils::NoticeMsgBox("已是最新版本，无需更新","系统提示",0,this);


    }else if(receivedValue == ALIVE)
    {
        m_has_recive_msg_from_daemon = true;
    }
    else if(receivedValue == HAS_NEW_VERSIO)
    {
        SetNormalStatus();
    }
    else
    {
        SetNormalStatus();
        emit checkfailed();
    }
}

bool AboutStackedWidget::CheckDaemonStatus()
{
    QString install_path = ConfigManager::Instance()->GetPackageInstallpath();

    QString exe_path = QDir::toNativeSeparators(install_path + QString("files/bin/huayupy-daemon-fcitx"));

#ifdef USE_IBUS
     bool isDaemonExist = QProcess::startDetached(exe_path);
#elif USE_FCITX
    bool isDaemonExist = QProcess::startDetached(exe_path);
#else
    bool isDaemonExist = QProcess::startDetached(exe_path);
#endif
   return isDaemonExist;
}

bool AboutStackedWidget::IsDaemonRunning()
{

#ifdef USE_IBUS
     QString processName = "huayupy-daemon-ibus";
#else
    QString processName = "huayupy-daemon-fcitx";

#endif
    QProcess daemonRuning;
    QStringList args;
    args << "-c";
    args << QString("ps | grep '%1'").arg(processName);
    daemonRuning.start("sh",args);

    QEventLoop loop;
    connect(&daemonRuning, SIGNAL(readyReadStandardOutput()),&loop, SLOT(quit()));
    loop.exec();

    QString  outputStr = QString(daemonRuning.readAllStandardOutput());
    if(outputStr.isEmpty())
    {
        return false;
    }
    return true;
}

void AboutStackedWidget::ResetConfig()
{
    int enablecheckupdateauto;
    if(ConfigManager::Instance()->GetDefaultIntConfig("enablecheckupdateauto",enablecheckupdateauto))
    {
        ui->auto_update_checkbox->setChecked(enablecheckupdateauto);
    }
}

void AboutStackedWidget::SetNormalStatus()
{
    ui->update_now_btn->setEnabled(true);
    ui->loading_label->setVisible(false);
}

void AboutStackedWidget::DownloadAndInstallNewPackage(QString destUrl,QString file_md5)
{
    QString pwd;
#ifndef USE_UOS
    PasswordDialog pwd_dlg;
    int ret = pwd_dlg.exec();
    pwd = pwd_dlg.GetPassword();
    if (ret == QDialog::Accepted)
    {
#endif
        m_updateWidget->SetDownloadUrl(destUrl);
        m_updateWidget->SetPassword(pwd);
        m_updateWidget->SetPackageMd5(file_md5);
        m_updateWidget->StartUpdate();
        m_updateWidget->show();
#ifndef USE_UOS
    }
    else
    {
        return;
    }
#endif
}
