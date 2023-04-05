#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QMessageBox"
#include "../public/inisetting.h"
#include <QDesktopWidget>
#include <QSettings>
#include <QtConcurrent>
#include <QIcon>
#include "../public/utils.h"
#include "aboutstackedwidget.h"
#include "accountstackedwidget.h"
#include "wordlibmainwidget.h"
#include "checkfailedstackedwidget.h"
#include "registerstackedwidget.h"
#include "enrollstackedwidget.h"
#include "resetpasswdstackedwidget.h"
#include "customize_ui/generalstyledefiner.h"
#include <QDebug>
#include "feedback/feedbackstackedwidget.h"
#include "wubistackedwidght.h"
#include "huayupy_adaptor.h"
#include "huayupy_interface.h"
#include "skinstackedwidget.h"
#include "basicconfigstackedwidget.h"
#include "advancedconfigstackedwidget.h"
#include "msgboxutils.h"
#include "configbus.h"
#include "wordlibshowwidget.h"
#include "wordlibpage/wordlibstackedwidget.h"
#include "../public/configmanager.h"
#include "wordlibpage/myutils.h"
#include "personpage/personaccountstackedwidget.h"


MainWindow::MainWindow(QWidget *parent) :
                    CustomizeMainWindow(parent)
                   ,ui(new Ui::MainWindow)
{
     Init();
    ui->miniBtn->setVisible(false);

    setContentsMargins(1, 1, 1, 1);
    setStyleSheet(
                "MainWindow {"
                "border:1px solid #cccccc;"
                "}");
}

MainWindow::~MainWindow()
{
    if(m_aboutStack)
    {
        delete m_aboutStack;
        m_aboutStack=NULL;
    }
    if(m_checkFailedStack)
    {
        delete m_checkFailedStack;
        m_checkFailedStack=NULL;
    }
    if(m_registerStack)
    {
        delete m_registerStack;
        m_registerStack=NULL;
    }
    if(m_enrollStack)
    {
        delete m_enrollStack;
        m_enrollStack=NULL;
    }
    if(m_resetPasswdStack)
    {
        delete m_resetPasswdStack;
        m_resetPasswdStack=NULL;
    }
    if(m_feedbackStack)
    {
        delete m_feedbackStack;
        m_feedbackStack=NULL;
    }
    if(m_wubiStack)
    {
        delete m_wubiStack;
        m_wubiStack=NULL;
    }
    if(m_skinConfigStack)
    {
        delete m_skinConfigStack;
        m_skinConfigStack=NULL;
    }
    if(m_advancedStack)
    {
        delete m_advancedStack;
        m_advancedStack=NULL;
    }
    if(m_basicStack)
    {
        delete m_basicStack;
        m_basicStack=NULL;
    }

}

void MainWindow::Init()
{
    ui->setupUi(this);
    SetUpCustomizeUI();
    InitStackWidget();
    RegisterSlots();
}

void MainWindow::InitStackWidget()
{

    QList<QPushButton*> btnList;
    btnList.push_back(ui->config_option_button);
    btnList.push_back(ui->wubi_option_button);
    btnList.push_back(ui->advanced_option_button);
    btnList.push_back(ui->wordlib_option_button);
    btnList.push_back(ui->account_option_button);
    btnList.push_back(ui->about_option_button);
    btnList.push_back(ui->feedback_option_button);
    btnList.push_back(ui->skin_option_button);

    for(QPushButton*btn:btnList)
    {
        btn->setProperty("type","sidebtn");
        btn->setFlat(true);
        btn->setCheckable(true);
        btn->setAutoExclusive(true);
        btn->setFocusPolicy(Qt::NoFocus);
        btn->autoFillBackground();
        btn->setCursor(QCursor(Qt::PointingHandCursor));
    }

    ui->apply_button->setProperty("type","normalBtn");
    ui->confirm_button->setProperty("type","normalBtn");
    ui->cancel_button->setProperty("type","normalBtn");
    ui->miniBtn->setProperty("type","minbtn");

    ui->bottom_widget->setVisible(false);

    m_version_check_watcher = new QFutureWatcher<int>(this);
    connect(m_version_check_watcher, &QFutureWatcher<int>::finished, this, &MainWindow::OnVesionCheckFinished);

    m_user_wordlib_watcher = new QFutureWatcher<int>(this);
    connect(m_user_wordlib_watcher, &QFutureWatcher<int>::finished, this, &MainWindow::OnWordlibUpdateFinished);

    QFuture<int> version_info_check = QtConcurrent::run(this,&MainWindow::CheckNewVersion);
    m_version_check_watcher->setFuture(version_info_check);

}

void MainWindow::SetUpCustomizeUI()
{
    SetUpCloseButton();
    SetUpMinmizeWindowButton();
    SetUpLogo();
    SetUpOptionButton();
    SetUpApplyButton();
    setWindowIcon(QIcon(":/image/logo.png"));

    QDesktopWidget *desktop = QApplication::desktop();
    this->move((desktop->width() - this->width())/2,
                           (desktop->height() - this->height())/2);
    SetStyleSheet();


    ui->resetconfigbtn->setProperty("type","puretextbtn");
    ui->label_title->setProperty("type", "h2");

}

void MainWindow::SetUpCloseButton()
{
    QString style_sheet = ""
                          " QPushButton { "
                          " image : url(:/image/close.svg); "
                          " border : 0px solid;"
                          " width: 20px;"
                          " height:18px;"
                          " } "
                          " QPushButton:hover { "
                          " image : url(:/image/close_hover.svg); "
                          " } "
                          " QPushButton:pressed { "
                          " image : url(:/image/close.svg); "
                          " border : 0px solid;"
                          " } ";
    ui->close_push_button->setFlat(true);
    ui->close_push_button->setStyleSheet(style_sheet);
    ui->close_push_button->setFocusPolicy(Qt::NoFocus);
    ui->close_push_button->setToolTip("关闭");
    connect(ui->close_push_button, SIGNAL(clicked()), this, SLOT(OnCloseButtonClicked()));
}

void MainWindow::SetUpMinmizeWindowButton()
{

}

void MainWindow::SetUpStackWidgetOptionButtonSlot()
{

}


void MainWindow::SetUpOptionButton()
{
    ui->config_option_button->setIcon(QIcon(":/image/config.svg"));
    ui->config_option_button->setIconSize(QSize(16, 16));

    ui->wordlib_option_button->setIcon(QIcon(":/image/wordlib.svg"));
    ui->wordlib_option_button->setIconSize(QSize(16, 16));

    ui->advanced_option_button->setIcon(QIcon(":/image/advanced.svg"));
    ui->advanced_option_button->setIconSize(QSize(16,16));

    ui->wubi_option_button->setIcon(QIcon(":/image/wubi.png"));
    ui->wubi_option_button->setIconSize(QSize(16, 16));


    ui->account_option_button->setIcon(QIcon(":/image/account.svg"));
    ui->account_option_button->setIconSize(QSize(16, 16));

    ui->about_option_button->setIcon(QIcon(":/image/about.svg"));
    ui->about_option_button->setIconSize(QSize(16, 16));

    ui->feedback_option_button->setIcon(QIcon(":/feedback/feedback.svg"));
    ui->feedback_option_button->setIconSize(QSize(16, 16));


    ui->skin_option_button->setIcon(QIcon(":/image/skin_config.svg"));
    ui->skin_option_button->setIconSize(QSize(16, 16));


}

void MainWindow::SetUpLogo()
{
    QPixmap pixmap(":/image/logo.png");
    ui->label_logo->setPixmap(pixmap.scaledToWidth(25));
}

void MainWindow::RegisterSlots()
{
    connect(ui->config_option_button, SIGNAL(clicked()), this, SLOT(OnBasicOptionBtnClicked()));
    connect(ui->wubi_option_button, SIGNAL(clicked()), this, SLOT(OnWubiButtonClicked()));
    connect(ui->wordlib_option_button, SIGNAL(clicked()), this, SLOT(OnWordlibButtonClicked()));
    connect(ui->advanced_option_button, SIGNAL(clicked()), this, SLOT(OnAdvancedOptionBtnClicked()));
    connect(ui->account_option_button, SIGNAL(clicked()), this, SLOT(OnAccountButtonClicked()));
    connect(ui->about_option_button, SIGNAL(clicked()), this, SLOT(OnAboutButtonClicked()));
    connect(ui->feedback_option_button, SIGNAL(clicked(bool)),this,SLOT(OnFeedBackButtonClicked()));
    connect(ui->skin_option_button, SIGNAL(clicked(bool)),this, SLOT(OnSkinOptionBtnClicked()));
    connect(ui->miniBtn, SIGNAL(clicked()), this,SLOT(SlotMinimizeWindow()));

    ui->config_option_button->setChecked(true);
    OnBasicOptionBtnClicked();

    new HuayupyAdaptor(this);
    new com::thunisoft::huayupy(QString(), QString(), QDBusConnection::sessionBus(), this);
    QDBusConnection::sessionBus().connect(QString(), QString(), "com.thunisoft.huayupy", "to_show", this, SLOT(ToShow()));
}

void MainWindow::ToShow()
{
    if(!this->isVisible())
        this->setVisible(true);

    showNormal();
    activateWindow();
    raise();
}

void MainWindow::OnSkinOptionBtnClicked()
{
    if(m_skinConfigStack == nullptr)
    {
        m_skinConfigStack = new SkinStackedWidget(this);
        ui->stackedWidget->addWidget(m_skinConfigStack);
    }
    int index = ui->stackedWidget->indexOf(m_skinConfigStack);
    ChangeVisibleWidget(index);
    OnBtnStateChanged();
}

void MainWindow::OnBasicOptionBtnClicked()
{
    if(m_basicStack == nullptr)
    {
        m_basicStack = new BasicConfigStackedWidget(this);
        ui->stackedWidget->addWidget(m_basicStack);
    }
    int index = ui->stackedWidget->indexOf(m_basicStack);
    ChangeVisibleWidget(index);
    OnBtnStateChanged();
}

void MainWindow::OnAdvancedOptionBtnClicked()
{
    if(m_advancedStack == nullptr)
    {
        m_advancedStack = new AdvancedConfigStackedWidget(this);
        ui->stackedWidget->addWidget(m_advancedStack);
    }
    int index = ui->stackedWidget->indexOf(m_advancedStack);
    ChangeVisibleWidget(index);
    OnBtnStateChanged();
}

void MainWindow::OnWordlibButtonClicked()
{
    if(m_wordlibStack == nullptr)
    {
        m_wordlibStack = new WordlibStackedWidget(this);
        ui->stackedWidget->addWidget(m_wordlibStack);
    }
    int index = ui->stackedWidget->indexOf(m_wordlibStack);
    ChangeVisibleWidget(index);
    OnBtnStateChanged();
}

void MainWindow::OnAccountButtonClicked()
{
    if(m_enrollStack == nullptr)
    {
        m_enrollStack = new EnrollStackedWidget(this);
        ui->stackedWidget->addWidget(m_enrollStack);

        m_registerStack = new RegisterStackedWidget(this);
        ui->stackedWidget->addWidget(m_registerStack);

        m_personinfo_stack = new PersonAccountStackedWidget(this);
        ui->stackedWidget->addWidget(m_personinfo_stack);

        m_resetPasswdStack = new ResetPasswdStackedWidget(this);
        ui->stackedWidget->addWidget(m_resetPasswdStack);

        connect(m_personinfo_stack, SIGNAL(logoutSucceed()), this, SLOT(OnAccountButtonClicked()));
        connect(m_personinfo_stack, SIGNAL(logoutSucceed()), this, SLOT(UpdateAllStackedWidget()));
        connect(m_personinfo_stack, SIGNAL(toResetPasswd()), this, SLOT(SwitchToResetPasswdSlot()));


        connect(m_enrollStack, SIGNAL(loginSucceeded()), this, SLOT(OnUserLoginedSlot()));
        connect(m_enrollStack,SIGNAL(switchToRegister()), this, SLOT(SwitchToReigisterSlot()));
        connect(m_registerStack, SIGNAL(switchToLogin()), this, SLOT(SwitchToLoginSlot()));
        connect(m_registerStack, SIGNAL(RegisterSucceed()), this, SLOT(SwitchToLoginSlot()));
        connect(m_resetPasswdStack, SIGNAL(switchToAccountWidget()), this, SLOT(SwitchToAccountSlot()));
        connect(m_resetPasswdStack, SIGNAL(switchToLogin()), this, SLOT(SwitchToLoginSlot()));
    }
    int isUserLogin;
    ConfigManager::Instance()->GetIntConfigItem("login",isUserLogin);
    if(isUserLogin && !m_resetPasswdStack->IsShow())
    {
        int loginIndex = ui->stackedWidget->indexOf(m_personinfo_stack);
        ui->stackedWidget->setCurrentIndex(loginIndex);
    }
    else if(m_resetPasswdStack->IsShow()) {
        SwitchToResetPasswdSlot();
    }
    else
    {
        int index = ui->stackedWidget->indexOf(m_enrollStack);
        ChangeVisibleWidget(index);
    }
    OnBtnStateChanged();
}
void MainWindow::OnAboutButtonClicked()
{
    if(m_aboutStack == nullptr)
    {
        m_aboutStack = new AboutStackedWidget(this);
        ui->stackedWidget->addWidget(m_aboutStack);
        m_checkFailedStack = new CheckFailedStackedWidget(this);
        ui->stackedWidget->addWidget(m_checkFailedStack);

        connect(m_checkFailedStack, SIGNAL(reupdate()), this, SLOT(SwitchToAboutStack()));
        connect(m_aboutStack, SIGNAL(checkfailed()), this, SLOT(SwitchToCheckFailedWidget()));
    }
    int index = ui->stackedWidget->indexOf(m_aboutStack);
    ChangeVisibleWidget(index);
    OnBtnStateChanged();
}

void MainWindow::ChangeVisibleWidget(int index)
{
    ui->stackedWidget->setCurrentIndex(index);
}
void MainWindow::OnBtnStateChanged()
{
    QString configIconPath = ui->config_option_button->isChecked()? ":/image/config_pressed.svg":":/image/config.svg";
    ui->config_option_button->setIcon(QIcon(configIconPath));
    ui->config_option_button->setIconSize(QSize(16, 16));

    QString wubiIconPath = ui->wubi_option_button->isChecked()? ":/image/wubi_pressed.png":":/image/wubi.png";
    ui->wubi_option_button->setIcon(QIcon(wubiIconPath));
    ui->wubi_option_button->setIconSize(QSize(16, 16));

    QString wordlibIconPath = ui->wordlib_option_button->isChecked()?":/image/wordlib_pressed.svg":":/image/wordlib.svg";
    ui->wordlib_option_button->setIcon(QIcon(wordlibIconPath));
    ui->wordlib_option_button->setIconSize(QSize(16, 16));

    QString advancedIconPath = ui->advanced_option_button->isChecked()?":/image/advanced_pressed.svg":":/image/advanced.svg";
    ui->advanced_option_button->setIcon(QIcon(advancedIconPath));
    ui->advanced_option_button->setIconSize(QSize(16,16));


    QString accountIconPath = ui->account_option_button->isChecked()?":/image/account_pressed.svg":":/image/account.svg";
    ui->account_option_button->setIcon(QIcon(accountIconPath));
    ui->account_option_button->setIconSize(QSize(16, 16));

    QString aboutIconPath = ui->about_option_button->isChecked()? ":/image/about_pressed.svg":":/image/about.svg";
    ui->about_option_button->setIcon(QIcon(aboutIconPath));
    ui->about_option_button->setIconSize(QSize(16, 16));

    QString feedbackIconPath = ui->feedback_option_button->isChecked()? ":/feedback/feedback_pressed.svg":":/feedback/feedback.svg";
    ui->feedback_option_button->setIcon(QIcon(feedbackIconPath));
    ui->feedback_option_button->setIconSize(QSize(16, 16));

    QString skinImagePath = ui->skin_option_button->isChecked()? ":/image/skin_config_hover.svg":":/image/skin_config.svg";
    ui->skin_option_button->setIcon(QIcon(skinImagePath));
    ui->skin_option_button->setIconSize(QSize(16, 16));
}

void MainWindow::SetUpApplyButton()
{
    connect(ui->cancel_button, SIGNAL(clicked(bool)), this, SLOT(OnCancelBtnSlot()));
    connect(ui->resetconfigbtn, SIGNAL(clicked(bool)),this, SLOT(SlotResetTheWindowConfig()));
}

void MainWindow::DrawNoticeFlagToWidget(QPushButton *push_btn, bool isAddFlag)
{
    QString style_sheet =
            "QLabel"
            "{"
            "padding-left:22px;"
            "padding-right:0px;"
            "padding-top:0px;"
            "padding-bottom:0px;"
            "background-color: rgba(255, 255, 224, 0%);"
            "}";
    if(m_notice_label == nullptr)
    {
        m_notice_label = new QLabel(push_btn);
        m_notice_label->setStyleSheet(style_sheet);
        m_notice_label->resize(push_btn->width(),push_btn->height());
        m_notice_label->setAlignment(Qt::AlignCenter | Qt::AlignTop);
        QPixmap pix(5,5);
        pix.fill(Qt::transparent);
        QPainter painter(&pix);
        painter.setPen(Qt::transparent);
        painter.setBrush(Qt::red);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.drawEllipse(pix.rect());
        m_notice_label->setPixmap(pix);
    }
    if(isAddFlag)
        m_notice_label->setVisible(true);
    else
        m_notice_label->setVisible(false);
}

int MainWindow::CheckNewVersion()
{
    NetworkHandler network_handler;
    bool has_update = false;
    QString new_version_str;
    int ret_code = network_handler.CheckNewVersion(has_update,new_version_str);
    if((ret_code == 0) && (has_update))
    {
        return 1;
    }
    return 0;
}

void MainWindow::OnFeedBackButtonClicked()
{
    if(!m_feedbackStack)
    {
        m_feedbackStack = new FeedbackStackedWidget(this);
        ui->stackedWidget->addWidget(m_feedbackStack);
    }
    int index = ui->stackedWidget->indexOf(m_feedbackStack);
    ChangeVisibleWidget(index);
    OnBtnStateChanged();
}


void MainWindow::OnUserLoginedSlot()
{
    int isUserLogin;
    ConfigManager::Instance()->GetIntConfigItem("login",isUserLogin);
    if(isUserLogin)
    {
        int accountIndex = ui->stackedWidget->indexOf(m_personinfo_stack);
        ui->stackedWidget->setCurrentIndex(accountIndex);
        m_personinfo_stack->SetUserState(1);
        m_personinfo_stack->UpdateUserFile();
    }
    ConfigBus::instance()->valueChanged("loadWordlib", "");
    UpdateAllStackedWidget();
    ConfigManager::Instance()->CallQimpanelToReloadAllConfig();

}

void MainWindow::SwitchToReigisterSlot()
{
    int registerIndex = ui->stackedWidget->indexOf(m_registerStack);
    ui->stackedWidget->setCurrentIndex(registerIndex);
}

void MainWindow::SwitchToLoginSlot()
{
    int loginIndex = ui->stackedWidget->indexOf(m_enrollStack);
    ui->stackedWidget->setCurrentIndex(loginIndex);
}

void MainWindow::SwitchToAccountSlot()
{
    int accountIndex = ui->stackedWidget->indexOf(m_personinfo_stack);
    ui->stackedWidget->setCurrentIndex(accountIndex);
}

void MainWindow::SwitchToResetPasswdSlot()
{
    m_resetPasswdStack->SetShow();
    int resetPasswd = ui->stackedWidget->indexOf(m_resetPasswdStack);
    ui->stackedWidget->setCurrentIndex(resetPasswd);
}

void MainWindow::OnUserLogoutSlot()
{
    int isUserLogin ;
    ConfigManager::Instance()->GetIntConfigItem("login",isUserLogin);
    if(!isUserLogin)
    {
        int enrollIndex = ui->stackedWidget->indexOf(m_enrollStack);
        ui->stackedWidget->setCurrentIndex(enrollIndex);
    }
    NotifyReloadUserWordlib();
}

void MainWindow::SwitchToCheckFailedWidget()
{
    int checkfailedIndex = ui->stackedWidget->indexOf(m_checkFailedStack);
    ui->stackedWidget->setCurrentIndex(checkfailedIndex);
}

void MainWindow::SwitchToAboutStack()
{
    int aboutIndex = ui->stackedWidget->indexOf(m_aboutStack);
    ui->stackedWidget->setCurrentIndex(aboutIndex);
}

void MainWindow::SetStyleSheet()
{
    this->setStyleSheet(QString(""
                        "#MainWindow  { "
                        " %1 "
                        " } "
                        "").arg(GeneralStyleDefiner::BorderStyle()));

    ui->stackedWidget->setStyleSheet("#stackedWidget{background-color:#ffffff;}");
    QFile file(":/feedback/basicstyle.qss");
    if(file.open(QIODevice::ReadOnly))
    {
        this->setStyleSheet(file.readAll());
        file.close();
    }

}

void MainWindow::OnCloseButtonClicked()
{
    ExitCurrentApp();
}

void MainWindow::SlotResetTheWindowConfig()
{
    if (ui->stackedWidget->currentWidget() == m_basicStack)
    {
        m_basicStack->ResetConfigInfo();
    }
    else if (ui->stackedWidget->currentWidget() == m_advancedStack)
    {
        m_advancedStack->ResetConfigInfo();
    }
    else if (ui->stackedWidget->currentWidget() == m_skinConfigStack)
    {
        m_skinConfigStack->ResetConfigInfo();
    }
    else if (ui->stackedWidget->currentWidget() == m_wubiStack)
    {
        m_wubiStack->ResetConfigInfo();
    }
    else if (ui->stackedWidget->currentWidget() == m_wordlibStack)
    {
        m_wordlibStack->ResetConfigInfo();
    }
    else if (ui->stackedWidget->currentWidget() == m_personinfo_stack)
    {
        m_personinfo_stack->ResetConfigInfo();
    }
    else if (ui->stackedWidget->currentWidget() == m_aboutStack)
    {
        m_aboutStack->ResetConfig();
    }
    else if (ui->stackedWidget->currentWidget() == m_feedbackStack)
    {
        m_feedbackStack->ResetConfigInfo();
    }
    else if(ui->stackedWidget->currentWidget() == m_enrollStack)
    {
        m_enrollStack->ResetWidget();        
    }
}

void MainWindow::OnWubiButtonClicked()
{
    if(!m_wubiStack)
    {
        m_wubiStack = new wubistackedwidght(this);
        ui->stackedWidget->addWidget(m_wubiStack);
    }
    int index = ui->stackedWidget->indexOf(m_wubiStack);
    ChangeVisibleWidget(index);
    OnBtnStateChanged();
}

void MainWindow::SlotMinimizeWindow()
{
    this->showMinimized();
}

int MainWindow::UpdateUserWordlib(QString user_uwl_path,QString loginid)
{
    QString serverMD5Value;
    NetworkHandler network_handler;
    QString userUwlFilePath = user_uwl_path;
    QString clientid = Utils::GethostMac();

    QFileInfo fileInfo(userUwlFilePath);
    QString userUwlFileDir = fileInfo.dir().path();
    QString targetFilePath;
    QDir dir(userUwlFileDir);
    if(!dir.exists()){
        dir.mkpath(userUwlFileDir);
    }

    //用户词库存在的时候
    if(QFile::exists(userUwlFilePath))
    {
        int itemCount = 0;
        WORDLIB_INFO userUwlWordelibInfo;
        WordlibShowWidget::AnalysisUwl(userUwlFilePath,userUwlWordelibInfo);
        itemCount = userUwlWordelibInfo.words_count;
        if(itemCount == 0)
        {
            QString targetFileUrl;

            int downloadCode = network_handler.DownloadCustomWordlib(loginid,clientid,targetFileUrl);

            if(downloadCode == 0)
            {
                int downloadFromWebCode = network_handler.DownloadFileFromWeb(targetFileUrl,userUwlFileDir,targetFilePath,"user.uwl");
                return 0;
            }
            else if(downloadCode == 705)
            {
                return 1;
            }
            else if(downloadCode == 701)
            {
                return 701;
            }

            return 2;
        }

        QFile file(userUwlFilePath);
        QByteArray fileContent;
        if(file.open(QIODevice::ReadOnly))
        {
            fileContent = file.readAll();
            file.close();
        }

        int returnCode = network_handler.UploadCustomWordlib(loginid,clientid,userUwlFilePath,serverMD5Value,itemCount);
        if( returnCode == 0)
        {

            QString targetFileUrl;
            int downloadCode = network_handler.DownloadCustomWordlib(loginid,clientid,targetFileUrl);
            if(downloadCode == 0)
            {
                qDebug() << targetFileUrl << targetFilePath << userUwlFileDir;
                network_handler.DownloadFileFromWeb(targetFileUrl,userUwlFileDir,targetFilePath,"user.uwl");
                return 0;
            }
            else if(downloadCode == 705)
            {
                return 2;
            }

            return 1;
        }
        //服务端不存在则上传
        else if(returnCode == 701)
        {

            int uploadValue = network_handler.UploadCustomWordlib(loginid,clientid,userUwlFilePath,serverMD5Value,itemCount);
            if(uploadValue == 0)
            {

                return 0;
            }
        }
        else if(returnCode == 710)
        {
            QString targetFileUrl;
            int downloadCode = network_handler.DownloadCustomWordlib(loginid,clientid,targetFileUrl);
            if(downloadCode == 0)
            {

                network_handler.DownloadFileFromWeb(targetFileUrl,userUwlFileDir,targetFilePath,"user.uwl");
                return 0;
            }
        }
        else
        {
            return 1;
        }
    }
    //用户词库不存在
    else
    {

        QString tempFilePath;
        int returnCode = network_handler.DownloadCustomWordlib(loginid,clientid,tempFilePath);
        if( returnCode == 0)
        {

            int webdownload_code = network_handler.DownloadFileFromWeb(tempFilePath,userUwlFileDir,targetFilePath,"user.uwl");

            return 0;

        }
        //本地没有配置文件服务端也没有则不提示
        else if(returnCode == 701)
        {
            return 0;
        }
        else
        {

            return 1;
        }
    }
    return 1;
}

int MainWindow::quitUploadUserlib()
{
    QString serverMD5Value;
    QString loginid;
    ConfigManager::Instance()->GetStrConfigItem("loginid",loginid);
    NetworkHandler network_handler;

    QString userUwlFilePath = QDir::toNativeSeparators(ConfigManager::Instance()->GetUserUwlPath());

    QString clientid = Utils::GethostMac();
    loginid = "";
    QFileInfo fileInfo(userUwlFilePath);
    QString userUwlFileDir = fileInfo.dir().path();
    QString targetFilePath;
    QDir dir(userUwlFileDir);
    if(!dir.exists()){
        dir.mkpath(userUwlFileDir);
    }

    //用户词库存在的时候
    if(QFile::exists(userUwlFilePath))
    {
        int itemCount = 0;
        WORDLIB_INFO userUwlWordelibInfo;
        WordlibShowWidget::AnalysisUwl(userUwlFilePath,userUwlWordelibInfo);
        itemCount = userUwlWordelibInfo.words_count;

        QFile file(userUwlFilePath);
        QByteArray fileContent;
        if(file.open(QIODevice::ReadOnly))
        {
            fileContent = file.readAll();
            file.close();
        }

        int returnCode = network_handler.UploadCustomWordlib(loginid,clientid,userUwlFilePath,serverMD5Value,itemCount);
        if( returnCode == 0)
        {
            return 0;
        }
        else if(returnCode == 710)
        {
            QString targetFileUrl;
            int downloadCode = network_handler.DownloadCustomWordlib(loginid,clientid,targetFileUrl);
            if(downloadCode == 0)
            {
                int returnCode = network_handler.DownloadMergerFile(targetFileUrl,userUwlFileDir,targetFilePath,"user.uwl");
                if(returnCode == 0)
                {
                    int returnCode = network_handler.UploadCustomWordlib(loginid,clientid,userUwlFilePath,serverMD5Value,itemCount);
                    return returnCode;
                }
            }
        }
        else
        {
            return returnCode;
        }
    }
    //用户词库不存在
    else
    {
        return 1;
    }

}

void MainWindow::NotifyReloadUserWordlib()
{
    ConfigBus::instance()->valueChanged("userWordlibCanSave", 0);
    ConfigBus::instance()->valueChanged("loadWordlib", "");
    ConfigBus::instance()->valueChanged("userWordlibCanSave", 1);
}

void MainWindow::OnVesionCheckFinished()
{
    if(m_version_check_watcher->isFinished())
    {
        int has_update = m_version_check_watcher->resultAt(0);
        DrawNoticeFlagToWidget(ui->about_option_button, has_update);
    }

}

void MainWindow::OnWordlibUpdateFinished()
{
    if(m_user_wordlib_watcher->isFinished())
    {
        int update_code = m_user_wordlib_watcher->resultAt(0);
        NotifyReloadUserWordlib();
    }
}

void MainWindow::UpdateAllStackedWidget()
{
    if(m_basicStack)
    {
        m_basicStack->LoadConfigInfo();
    }
    if(m_advancedStack)
    {
        m_advancedStack->LoadConfigInfo();
    }
    if(m_skinConfigStack)
    {
        m_skinConfigStack->LoadConfigInfo();
    }
    if(m_wubiStack)
    {
        m_wubiStack->LoadConfigInfo();
    }
    if(m_wordlibStack)
    {
        m_wordlibStack->LoadConfigInfo();
    }
    if(m_aboutStack)
    {
        m_aboutStack->LoadConfigInfo();
    }

}

void MainWindow::UploadUserWorild()
{
    int returncode = QtConcurrent::run(this,&MainWindow::quitUploadUserlib);
    if(returncode == 0)
        Utils::WriteLogToFile(QString("退出登录，文件上传同步成功"));
    else if(returncode == 1)
        Utils::WriteLogToFile(QString("用户词库不存在，退出不更新"));
    else if(returncode == 666)
        Utils::WriteLogToFile(QString("请求超时"));
    else
        Utils::WriteLogToFile(QString("服务器返回失败，错误码%1").arg(returncode));
}

void MainWindow::ExitCurrentApp()
{
    //this->close();
    exit(0);
}

void MainWindow::OnSideButtonClick(QString btn_name)
{
    if(btn_name == "login_page")
    {
        ui->account_option_button->click();
    }
    else if(btn_name == "update_page")
    {
        ui->about_option_button->click();
    }
    else if(btn_name == "edit_hotkey")
    {
        ui->advanced_option_button->click();
        if(m_advancedStack != NULL)
        {
            m_advancedStack->slot_scroll_to_hotkey();
        }
    }
}
