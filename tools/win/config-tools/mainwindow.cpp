#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDesktopWidget>
#include <QSettings>
#include <QStyleOption>
#include <QApplication>
#include <QElapsedTimer>
#include <QDialog>
#include <QPainter>
#include <QIcon>
#include "config.h"
#include "aboutstackedwidget.h"
#include "checkfailedstackedwidget.h"
#include "advancedconfigstackedwidget.h"
#include "basicconfigstackedwidget.h"
#include "skinconfigstackedwidget.h"
#include "wordlibstackedwidget.h"
#include "newenrollstackedwidget.h"
#include "personaccountstackedwidget.h"
#include "feedbackstackedwidget.h"
#include "networkhandler.h"
#include "sharedmemorycheckthread.h"
#include <QDebug>
#include <tchar.h>
#include <QDir>
#include <QScreen>
#include <QGraphicsDropShadowEffect>
#include <QtMath>
#include <QTimer>
#include <QtConcurrent>
#include <QTabWidget>
#include <QUuid>
#include <QDateTime>
#include <Windows.h>
#include "dpiadaptor.h"
#include "utils.h"
#include "downloadthread.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
                    CustomizeMainWindow(parent)
                   ,ui(new Ui::MainWindow),
                    m_screenPos(0)
{
     Init();

}

MainWindow::~MainWindow()
{
    if(m_memoryCheckThread->isRunning())
    {
        m_memoryCheckThread->exit(0);
    }
    delete m_memoryCheckThread;
    m_memoryCheckThread = NULL;

}

void MainWindow::Init()
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint|Qt::WindowMinimizeButtonHint|Qt::WindowMaximizeButtonHint);

    SetUpCustomizeUI();
    InitStackWidget();
    RegisterSlots();

    m_memoryCheckThread = new SharedMemoryCheckThread();
    QDesktopWidget* desktopWidght = QApplication::desktop();
    connect(desktopWidght, SIGNAL(resized(int)),this, SLOT(ResolutionChanged()));
    connect(m_memoryCheckThread, SIGNAL(ShowNormal()), this , SLOT(MainWindowShowNormalSlot()));
    connect(m_memoryCheckThread, SIGNAL(ShowVersionCheck()), this , SLOT(ShowAboutVersionCheckWindow()));
    connect(m_memoryCheckThread, SIGNAL(to_index_page(int)), this , SLOT(click_index_page(int)));
    connect(m_memoryCheckThread, SIGNAL(ShowEditHotkey()),this, SLOT(slot_show_edit_hotkey()));
    m_memoryCheckThread->start();
}

void MainWindow::InitStackWidget()
{
    m_aboutStack = NULL;
    m_checkFailedStack = NULL;
    m_advancedConfigStack = NULL;
    m_basicConfigStack = NULL;
    m_skinConfigStack = NULL;
    m_wordlibStackedWidget = NULL;
    m_newEnrollStackedWidget = NULL;
    m_personAccountStackedWidget = NULL;
    m_feedbackStackedWidget = NULL;
}

void MainWindow::SetUpCustomizeUI()
{
    SetUpCloseButton();
    InitBtnIcon();


    //setWindowIcon(QIcon(":/image/logo.ico"));

    QDesktopWidget *desktop = QApplication::desktop();
    this->move((desktop->width() - this->width())/2, (desktop->height() - this->height())/2);
    QList<QPushButton*> panelButtonList;
    panelButtonList.push_back(ui->config_option_button);
    panelButtonList.push_back(ui->advanced_option_button);
    panelButtonList.push_back(ui->skin_option_button);
    panelButtonList.push_back(ui->wordlib_option_button);
    panelButtonList.push_back(ui->about_option_button);
    panelButtonList.push_back(ui->account_option_button);
    panelButtonList.push_back(ui->feedback_option_button);
    for(int index=0; index<panelButtonList.size();++index)
    {
        panelButtonList.at(index)->setFlat(true);
        panelButtonList.at(index)->setCheckable(true);
        panelButtonList.at(index)->setAutoExclusive(true);
        panelButtonList.at(index)->setFocusPolicy(Qt::NoFocus);
        panelButtonList.at(index)->setProperty("type","sidebtn");
        panelButtonList.at(index)->setCursor(Qt::PointingHandCursor);
    }
    ui->reset_btn->setCursor(Qt::PointingHandCursor);
    this->setObjectName("mainwindow");
    QList<QPushButton*> bottomBtnList;
    bottomBtnList.push_back(ui->apply_btn);
    bottomBtnList.push_back(ui->cancel_btn);

    for(int index=0; index<bottomBtnList.size();++index)
    {
        bottomBtnList.at(index)->setCheckable(true);
        bottomBtnList.at(index)->setAutoExclusive(true);
        bottomBtnList.at(index)->setFocusPolicy(Qt::NoFocus);
        bottomBtnList.at(index)->setProperty("type","normal");
    }
    ui->apply_btn->setCheckable(false);
    ui->cancel_btn->setCheckable(false);

    ui->save_btn->setCheckable(true);
    ui->save_btn->setAutoExclusive(true);
    ui->save_btn->setFocusPolicy(Qt::NoFocus);
    ui->save_btn->setProperty("type","normalChecked");

    //隐藏对应的确定应用和取消按钮
    ui->save_btn->setVisible(false);
    ui->apply_btn->setVisible(false);
    ui->cancel_btn->setVisible(false);



    ui->reset_btn->setProperty("type","puretextbtn");
    ui->close_push_button->setProperty("type","closebtn");
    ui->close_push_button->setCursor(Qt::PointingHandCursor);
    ui->miniBtn->setProperty("type","minbtn");
    ui->miniBtn->setCursor(Qt::PointingHandCursor);
    ui->logo_label->setProperty("type","main_logo_label");
    ui->main_title->setProperty("type","main_title_label");
    int has_update = Config::Instance()->HasNewVersion();
    if(has_update)
    {
        SlotDrawNoticeFlagToWidget(ui->about_option_button,true);
    }
    else
    {
        SlotDrawNoticeFlagToWidget(ui->about_option_button,false);
    }
    m_config_update_watcher = new QFutureWatcher<int>(this);
    connect(m_config_update_watcher, &QFutureWatcher<int>::finished,this,&MainWindow::slot_on_configfile_update_finished);

}

void MainWindow::SetUpCloseButton()
{
    ui->close_push_button->setObjectName("closebtn");
    ui->close_push_button->setFocusPolicy(Qt::NoFocus);
    ui->close_push_button->setToolTip("关闭");
    connect(ui->close_push_button, SIGNAL(clicked()), this, SLOT(OnCloseButtonClicked()));
}


void MainWindow::RegisterSlots()
{
    connect(ui->config_option_button, SIGNAL(clicked()), this, SLOT(OnConfigButtonClicked()));
    connect(ui->advanced_option_button, SIGNAL(clicked()), this, SLOT(OnAdvancedButtonClicked()));
    connect(ui->skin_option_button, SIGNAL(clicked()), this, SLOT(OnSkinButtonClicked()));
    connect(ui->account_option_button, SIGNAL(clicked()), this, SLOT(OnAccountButtonClicked()));
    connect(ui->about_option_button, SIGNAL(clicked()), this, SLOT(OnAboutButtonClicked()));
    connect(ui->wordlib_option_button, SIGNAL(clicked()), this, SLOT(OnWordlibButtonClicked()));
    connect(ui->feedback_option_button,SIGNAL(clicked()), this, SLOT(OnFeedbackButtonClicked()));

    connect(ui->apply_btn, SIGNAL(clicked()),this, SLOT(OnApllyBtnClicked()));
    connect(ui->save_btn, SIGNAL(clicked()),this,SLOT(OnSaveBtnClicked()));
    connect(ui->reset_btn, SIGNAL(clicked()),this, SLOT(OnResetBtnClicked()));
    connect(ui->miniBtn, SIGNAL(clicked()), this,SLOT(SlotMinimizeWindow()));

    connect(ui->cancel_btn, SIGNAL(clicked()),this,SLOT(OnCloseButtonClicked()));

    ui->config_option_button->setChecked(true);
    OnConfigButtonClicked();
    LoadChildConfigPage();
}

void MainWindow::LoadChildConfigPage()
{
    
    OnAdvancedButtonClicked();
    OnSkinButtonClicked();
    OnWordlibButtonClicked();
    OnAboutButtonClicked();
    OnAccountButtonClicked();
    OnFeedbackButtonClicked();
    OnConfigButtonClicked();
}

void MainWindow::ResolutionChanged()
{
    //do nothing
}

void MainWindow::SlotInformIMEUpdateConfig()
{
//    //释放信号量
//    if(ReleaseSemaphore(m_configCountSem,1,NULL))
//    {
//        Utils::WriteLogToFile("infom the ime to update config succeed");
//    }
//    else
//    {
//        Utils::WriteLogToFile("infom the ime to update config failed");
//    }
}

void MainWindow::OnConfigButtonClicked()
{
    OnBtnStateChanged();
    if(!m_basicConfigStack)
    {
        m_basicConfigStack = new BasicConfigStackedWidget();
        ui->stackedWidget->addWidget(m_basicConfigStack);
    }
    int index = ui->stackedWidget->indexOf(m_basicConfigStack);
    ChangeVisibleWidget(index);
    ui->bottomWidget->setVisible(false);
}
void MainWindow::OnWordlibButtonClicked()
{
    OnBtnStateChanged();
    if(!m_wordlibStackedWidget)
    {
        m_wordlibStackedWidget = new WordlibStackedWidget(this);
        QtConcurrent::run(m_wordlibStackedWidget,&WordlibStackedWidget::LoadConfigInfo);
        connect(m_wordlibStackedWidget, SIGNAL(wordlibChanged()),this,SLOT(SlotInformIMEUpdateConfig()));
        ui->stackedWidget->addWidget(m_wordlibStackedWidget);
    }
    int index = ui->stackedWidget->indexOf(m_wordlibStackedWidget);
    ChangeVisibleWidget(index);
    ui->bottomWidget->setVisible(false);
}
void MainWindow::OnSkinButtonClicked()
{
    OnBtnStateChanged();
    if(!m_skinConfigStack)
    {
        m_skinConfigStack = new SkinConfigStackedWidget(this);
        ui->stackedWidget->addWidget(m_skinConfigStack);
    }
    int index = ui->stackedWidget->indexOf(m_skinConfigStack);
    ChangeVisibleWidget(index);
    ui->bottomWidget->setVisible(false);
}
void MainWindow::OnAccountButtonClicked()
{
    OnBtnStateChanged();
    if(!m_newEnrollStackedWidget)
    {
        m_newEnrollStackedWidget = new NewEnrollStackedWidget(this);
        ui->stackedWidget->addWidget(m_newEnrollStackedWidget);
        m_personAccountStackedWidget = new PersonAccountStackedWidget(this);
        ui->stackedWidget->addWidget(m_personAccountStackedWidget);
        connect(m_newEnrollStackedWidget, SIGNAL(loginSucceed()), this, SLOT(OnUserLoginedSucceed()));
        connect(m_personAccountStackedWidget, SIGNAL(logout()), this, SLOT(OnUserLogout()));
        connect(m_personAccountStackedWidget, SIGNAL(userconfigfileupdated()), this, SLOT(OnUpdateTheUserConfigFile()));
        connect(m_personAccountStackedWidget, SIGNAL(userInfoHasUpdated()), this, SLOT(OnNotifyTheEngine()));
        connect(m_personAccountStackedWidget, SIGNAL(configChanged()),this, SLOT(SlotInformIMEUpdateConfig()));
        int index = ui->stackedWidget->indexOf(m_newEnrollStackedWidget);
        int personIndex = ui->stackedWidget->indexOf(m_personAccountStackedWidget);
        ChangeVisibleWidget(personIndex);
        ChangeVisibleWidget(index);
        ui->bottomWidget->setVisible(false);
    }
    else
    {
        int index = ui->stackedWidget->indexOf(m_newEnrollStackedWidget);
        int personIndex = ui->stackedWidget->indexOf(m_personAccountStackedWidget);
        ChangeVisibleWidget(personIndex);
        ChangeVisibleWidget(index);
        int height = ui->content_widget->geometry().height();
        ui->bottomWidget->setVisible(false);
        ui->content_widget->setFixedHeight(height);

        //直接切换到用户登录的界面
        //拼音模式
        ConfigItemStruct configInfo;
        if(Config::Instance()->GetConfigItemByJson("login",configInfo))
        {
            if(configInfo.itemCurrentIntValue)
            {
                ChangeVisibleWidget(personIndex);
            }
            else
            {
                ChangeVisibleWidget(index);
            }
        }
        else
        {
            ChangeVisibleWidget(index);
        }


    }



}
void MainWindow::OnAboutButtonClicked()
{
    OnBtnStateChanged();
    if(!m_aboutStack)
    {
        m_aboutStack = new AboutStackedWidget(this);
        ui->stackedWidget->addWidget(m_aboutStack);
        m_checkFailedStack = new CheckFailedStackedWidget(this);
        ui->stackedWidget->addWidget(m_checkFailedStack);

        connect(m_checkFailedStack, SIGNAL(reupdate()), this, SLOT(SwitchToAboutStack()));
        connect(m_aboutStack, SIGNAL(checkfailed()), this, SLOT(SwitchToCheckFailedWidget()));
        int index = ui->stackedWidget->indexOf(m_aboutStack);
        ChangeVisibleWidget(index);
        ui->bottomWidget->setVisible(false);
    }
    else
    {
        int index = ui->stackedWidget->indexOf(m_aboutStack);
        ChangeVisibleWidget(index);
        int height = ui->content_widget->geometry().height();
        ui->bottomWidget->setVisible(false);
        ui->content_widget->setFixedHeight(height);
    }
}

void MainWindow::ChangeVisibleWidget(int index)
{
    ui->stackedWidget->setCurrentIndex(index);   
}


void MainWindow::OnAdvancedButtonClicked()
{
    OnBtnStateChanged();
    if(!m_advancedConfigStack)
    {
        m_advancedConfigStack = new AdvancedConfigStackedWidget(this);
        ui->stackedWidget->addWidget(m_advancedConfigStack);
    }
    int index = ui->stackedWidget->indexOf(m_advancedConfigStack);
    ChangeVisibleWidget(index);
    ui->bottomWidget->setVisible(false);
}

void MainWindow::OnFeedbackButtonClicked()
{
    OnBtnStateChanged();
    if(!m_feedbackStackedWidget)
    {
        m_feedbackStackedWidget = new FeedbackStackedWidget(this);
        ui->stackedWidget->addWidget(m_feedbackStackedWidget);
    }
    int index = ui->stackedWidget->indexOf(m_feedbackStackedWidget);
    ChangeVisibleWidget(index);
    ui->bottomWidget->setVisible(false);
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


void MainWindow::OnCloseButtonClicked()
{
    if(DownloadThread::downloadState == true)
    {
        if(Utils::NoticeMsgBox("正在下载安装包\n是否要退出?",this,1) == QDialog::Accepted)
        {
             this->close();
            m_wordlibStackedWidget->set_check_thread_flag(false);

        }
    }
    else
    {
        this->close();
        m_wordlibStackedWidget->set_check_thread_flag(false);

    }
}


void MainWindow::OnUserLoginedSucceed()
{
    ChangeUserLoginState(true);

    int personAccountIndex = ui->stackedWidget->indexOf(m_personAccountStackedWidget);
    ChangeVisibleWidget(personAccountIndex);

    QString config_file_path = Config::Instance()->configJsonFilePath();
    QString loginid;
    ConfigItemStruct loginitem;
    if(Config::Instance()->GetConfigItemByJson("loginid",loginitem))
    {
        loginid = loginitem.itemCurrentStrValue;
    }
    QString phase_file_path = Config::Instance()->CustomPhraseFilePath();

    QtConcurrent::run(m_personAccountStackedWidget,&PersonAccountStackedWidget::asyn_update_userwordlib);
    QFuture<int> config_res = QtConcurrent::run(m_personAccountStackedWidget,&PersonAccountStackedWidget::SlotUpdateConfigFile,config_file_path,loginid);
    QtConcurrent::run(m_personAccountStackedWidget,&PersonAccountStackedWidget::SlotUpdateCustomPhraseFile,phase_file_path,loginid);
    m_config_update_watcher->setFuture(config_res);
    //Config::Instance()->SaveConfig();
    SendMsgToEngine();

    //上传用户统计量
    QtConcurrent::run(Utils::UploadCurrentStatsCount,QString(""));
    QtConcurrent::run(Utils::CheckHistoryInput,QString(""));

}

void MainWindow::OnApllyBtnClicked()
{
    QString message;
    if (!Config::Instance()->CheckValidation(message))
    {
        QMessageBox::information(this, QString::fromLocal8Bit("系统提示"), message);
        return;
    }

    Config::Instance()->SaveConfig();

    if(Config::Instance()->IsToolBarChanged())
    {
        MainWindow::SendMsgToEngine();
    }

    SlotInformIMEUpdateConfig();

    Config::Instance()->ReloadConfig();
}

void MainWindow::OnSaveBtnClicked()
{
    QString message;
    if (!Config::Instance()->CheckValidation(message))
    {
        QMessageBox::information(this, QString::fromLocal8Bit("系统提示"), message);
        return;
    }

    Config::Instance()->SaveConfig();
    Config::Instance()->ChangeLoginStateWhenExit();
    if(Config::Instance()->IsToolBarChanged())
    {
        MainWindow::SendMsgToEngine();
    }
    SlotInformIMEUpdateConfig();
    Config::Instance()->ReloadConfig();
    SlotHideTheMainWindow();
}

void MainWindow::OnResetBtnClicked()
{
    int currentIndex = ui->stackedWidget->currentIndex();
    int basicIndex = ui->stackedWidget->indexOf(m_basicConfigStack);
    int advancedIndex = ui->stackedWidget->indexOf(m_advancedConfigStack);
    int skinIndex = ui->stackedWidget->indexOf(m_skinConfigStack);
    int worlibIndex = ui->stackedWidget->indexOf(m_wordlibStackedWidget);
    int aboutIndex = ui->stackedWidget->indexOf(m_aboutStack);
    if(currentIndex == basicIndex)
    {
        m_basicConfigStack->ResetConfigInfo();
        m_basicConfigStack->LoadConfigInfo();

    }
    else if(currentIndex == advancedIndex)
    {
        m_advancedConfigStack->ResetConfigInfo();
        m_advancedConfigStack->LoadConfigInfo();
    }
    else if(currentIndex == skinIndex)
    {
        m_skinConfigStack->ResetConfigInfo();
        m_skinConfigStack->LoadConfigInfo();
    }
    else if(currentIndex == worlibIndex)
    {
        m_wordlibStackedWidget->ResetConfigInfo();
        m_wordlibStackedWidget->LoadConfigInfo();
    }
    else if(currentIndex == aboutIndex)
    {
        m_aboutStack->ResetConfigInfo();
    }

}

bool MainWindow::SendMsgToEngine()
{
    const UINT WM_RELOAD_CONFIG = ::RegisterWindowMessage(TEXT("345EWE33 - 123B - 4752 - A9A8 - F3412WEWED2"));
    HWND hWnd = ::FindWindow(TEXT("HuayuIME.ToolBarWindow"),NULL);
    QUuid unique_id = QUuid::createUuid();
    QString unique_id_str = unique_id.toString();

    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    int timeStamp = qrand()%10000000;   //随机生成0到10000000的随机数

    if(!hWnd)
    {
        ::PostMessage(HWND_BROADCAST,WM_RELOAD_CONFIG,(WPARAM) timeStamp, (LPARAM)(long)timeStamp);
        Utils::WriteLogToFile(QString("{%1}Find ToolBar HWND failed send broadcast Message").arg(timeStamp));
        return false;
    }
    if(IsWindow(hWnd))
    {
        ::PostMessage(HWND_BROADCAST,WM_RELOAD_CONFIG,(WPARAM) timeStamp, (LPARAM)(long)timeStamp);

        //::SendMessage(hWnd,WM_RELOAD_CONFIG,(WPARAM) timeStamp, (LPARAM)(long)timeStamp);
        Utils::WriteLogToFile(QString("{%1}Find ToolBar HWND succeed send register Message").arg(timeStamp));
        return true;
    }
    return false;

}



void MainWindow::SetWindowShowOnTop()
{
    //move the widget to screen center
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    int pos_x = (screenGeometry.width() - width()) / 2;
    int pos_y = (screenGeometry.height() - height()) / 2;
    this->move(pos_x, pos_y);

    if(this->isMinimized())
    {
        this->showNormal();
    }
    ::SetWindowPos(HWND(this->winId()),HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
    ::SetWindowPos(HWND(this->winId()),HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);


    this->show();

    this->activateWindow();

}



void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_last_mouse_position = event->globalPos();
        m_move_widget_flag = true;
    }
    if(event->button() == Qt::RightButton)
    {

    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_move_widget_flag = false;
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (!event->buttons().testFlag(Qt::LeftButton))
            return;
    if(!m_move_widget_flag)
            return;
    const QPoint position = pos() + event->globalPos() - m_last_mouse_position;
    move(position.x(), position.y());
    m_last_mouse_position = event->globalPos();
    QPoint windowsPos = this->pos();
    QPoint currentPoint;
    currentPoint.setX(windowsPos.x() + this->width()/2);
    currentPoint.setY(windowsPos.y() + this->height()/2);
    QList<QScreen*> screenList = QApplication::screens();
    QScreen* screen = QApplication::primaryScreen();
    int currentPos = 0;
    if(screen->geometry().contains(currentPoint) || (screenList.size() == 1) )
    {
        currentPos = 0;
    }
    else if(screenList.at(1)->geometry().contains(currentPoint))
    {
        currentPos = 1;
    }

}

void MainWindow::InitBtnIcon()
{
    ui->config_option_button->setIcon(QIcon(":/image/config.png"));
    ui->wordlib_option_button->setIcon(QIcon(":/image/wordlib.png"));
    ui->skin_option_button->setIcon(QIcon(":/image/skin.png"));
    ui->account_option_button->setIcon(QIcon(":/image/account.png"));
    ui->about_option_button->setIcon(QIcon(":/image/about.png"));
    ui->advanced_option_button->setIcon(QIcon(":/image/advanced.png"));
    ui->feedback_option_button->setIcon(QIcon(":/image/feedback.png"));

    ui->config_option_button->setIconSize(QSize(16,16));
    ui->wordlib_option_button->setIconSize(QSize(16,16));
    ui->skin_option_button->setIconSize(QSize(16,16));
    ui->account_option_button->setIconSize(QSize(16,16));
    ui->about_option_button->setIconSize(QSize(16,16));
    ui->advanced_option_button->setIconSize(QSize(16,16));
    ui->feedback_option_button->setIconSize(QSize(16,16));
}

void MainWindow::OnUserLogout()
{
    //用户退出之后更新页面显示并刷新配置
    Config::Instance()->LoadPublishWordlib();
    m_basicConfigStack->LoadConfigInfo();
    m_advancedConfigStack->LoadConfigInfo();
    m_skinConfigStack->LoadConfigInfo();

    QtConcurrent::run(m_wordlibStackedWidget,&WordlibStackedWidget::LoadConfigInfo);

    m_aboutStack->LoadConfig();
    SlotInformIMEUpdateConfig();

    m_newEnrollStackedWidget->ResetContent();
    m_newEnrollStackedWidget->LoadConfigInfo();
    int enrollIndex = ui->stackedWidget->indexOf(m_newEnrollStackedWidget);
    ChangeVisibleWidget(enrollIndex);
    ConfigItemStruct currentInfo;
    Config::Instance()->GetConfigItemByJson("loginid",currentInfo);


    //上传用户统计量
    QtConcurrent::run(Utils::UploadCurrentStatsCount,currentInfo.itemCurrentStrValue);
    QtConcurrent::run(Utils::CheckHistoryInput,currentInfo.itemCurrentStrValue);

    SendMsgToEngine();
}

void MainWindow::OnBtnStateChanged()
{
    QString configIconPath = ui->config_option_button->isChecked()? ":/image/config_pressed.png":":/image/config.png";
    ui->config_option_button->setIcon(QIcon(configIconPath));
    QString wordlibIconPath = ui->wordlib_option_button->isChecked()?":/image/wordlib_pressed.png":":/image/wordlib.png";
    ui->wordlib_option_button->setIcon(QIcon(wordlibIconPath));
    QString accountIconPath = ui->account_option_button->isChecked()?":/image/account_pressed.png":":/image/account.png";
    ui->account_option_button->setIcon(QIcon(accountIconPath));
    QString aboutIconPath = ui->about_option_button->isChecked()? ":/image/about_pressed.png":":/image/about.png";
    ui->about_option_button->setIcon(QIcon(aboutIconPath));
    QString skinIconPath = ui->skin_option_button->isChecked()? ":/image/skin_pressed.png":":/image/skin.png";
    ui->skin_option_button->setIcon(QIcon(skinIconPath));
    QString advancedIconPath = ui->advanced_option_button->isChecked()? ":/image/advanced_pressed.png":":/image/advanced.png";
    ui->advanced_option_button->setIcon(QIcon(advancedIconPath));

    QString feedbackIconPath = ui->feedback_option_button->isChecked()? ":/image/feedback_pressed.png":":/image/feedback.png";
    ui->feedback_option_button->setIcon(QIcon(feedbackIconPath));

}

void MainWindow::AddNoticeInfoToMainwindow()
{
    SlotDrawNoticeFlagToWidget(ui->about_option_button);

}

void MainWindow::SlotMinimizeWindow()
{
    this->showMinimized();
}

int MainWindow::GetScreenSizeFactor(int screenArea)
{
    //大型显示器尺寸(>17寸）
    if(screenArea > 792)
    {
        return 3;
    }
    //小于17寸大于13寸
    else if(screenArea>500 && screenArea<792)
    {
        return 2;
    }
    //小于13寸的平板电脑
    else if(screenArea>0 && screenArea<500)
    {
        return 1;
    }
    return 2;
}

void MainWindow::ChangeUserLoginState(bool flag)
{
    Q_UNUSED(flag)
    ConfigItemStruct configInfo;
    configInfo.itemName = "login";
    configInfo.itemGroupName = "state";
    configInfo.itemCurrentIntValue = 1;
    Config::Instance()->SetStateConfigItemByJson("login",configInfo);
    Config::Instance()->SaveSystemConfig();

}

void MainWindow::OnUpdateTheWidgetConfig()
{
    m_basicConfigStack->LoadConfigInfo();
    m_advancedConfigStack->LoadConfigInfo();
    m_skinConfigStack->LoadConfigInfo();
    m_personAccountStackedWidget->SetUserState(true);
    QtConcurrent::run(m_wordlibStackedWidget,&WordlibStackedWidget::LoadConfigInfo);
}

void MainWindow::OnUpdateTheUserConfigFile()
{
    Config::Instance()->SaveSystemConfig();
    Config::Instance()->LoadConfig();
    OnUpdateTheWidgetConfig();
    SlotInformIMEUpdateConfig();
}
void MainWindow::OnNotifyTheEngine()
{
    SlotInformIMEUpdateConfig();
}

void MainWindow::SlotDrawNoticeFlagToWidget(QPushButton *push_btn,bool isAddFalg)
{

    QString style_sheet =
            "QLabel"
            "{"
            "padding-left:22px;"
            "padding-right:0px;"
            "padding-top:0px;"
            "padding-bottom:0px;"
            "}";
    if(!m_notice_label)
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

    if(isAddFalg)
    {
        m_notice_label->setVisible(true);
    }
    else
    {
        m_notice_label->setVisible(false);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    SlotHideTheMainWindow();
    Config::Instance()->ChangeLoginStateWhenExit();
    int basic_index = ui->stackedWidget->indexOf(m_basicConfigStack);
    if(basic_index != -1)
    {
        ui->config_option_button->click();
    }
    ResetAllTheWidght();
    event->ignore();
}


void MainWindow::MainWindowShowNormalSlot()
{
    //显示的时候检查是否有更新从而显示红点
    ResetAllTheWidght();
    int has_update = Config::Instance()->HasNewVersion();
    if(has_update)
    {
        SlotDrawNoticeFlagToWidget(ui->about_option_button);
    }
    else
    {
        SlotDrawNoticeFlagToWidget(ui->about_option_button,false);
    }
    if(m_wordlibStackedWidget)
    {
        m_wordlibStackedWidget->ReloadWordlibInfo();
    }
    this->SetWindowShowOnTop();
    m_wordlibStackedWidget->set_check_thread_flag(true);
    QtConcurrent::run(m_wordlibStackedWidget,&WordlibStackedWidget::LoadConfigInfo);
}

void MainWindow::ResetAllTheWidght()
{
    Config::Instance()->ReloadConfig();
    m_basicConfigStack->LoadConfigInfo();
    m_advancedConfigStack->LoadConfigInfo();
    m_skinConfigStack->LoadConfigInfo();
    m_wordlibStackedWidget->LoadConfigInfo();
    m_aboutStack->LoadConfig();
    m_newEnrollStackedWidget->LoadConfigInfo();

}

void MainWindow::SlotHideTheMainWindow()
{
    this->hide();
    m_feedbackStackedWidget->ClearFeedbackMsg();
}

void MainWindow::ShowAboutVersionCheckWindow()
{
    ResetAllTheWidght();
    //显示的时候检查是否有更新从而显示红点
    int has_update = Config::Instance()->HasNewVersion();
    if(has_update)
    {
        SlotDrawNoticeFlagToWidget(ui->about_option_button);
    }
    else
    {
        SlotDrawNoticeFlagToWidget(ui->about_option_button,false);
    }
    if(m_wordlibStackedWidget)
    {
        m_wordlibStackedWidget->ReloadWordlibInfo();
    }
    click_index_page(ABOUT_PAGE);
}

void MainWindow::click_index_page(int click_page)
{
    ResetAllTheWidght();
    PAGE_TYPE index_type = (PAGE_TYPE)click_page;
    this->SetWindowShowOnTop();
    switch(index_type)
    {
    case BASIC_PAGE:
        ui->config_option_button->click();
        break;
    case ADVANCED_PAGE:
        ui->advanced_option_button->click();
        break;
    case SKIN_PAGE:
        ui->skin_option_button->click();
        break;
    case LOGIN_PAGE:
        ui->account_option_button->click();
        break;
    case ABOUT_PAGE:
        ui->about_option_button->click();
        break;
    case FEEDBACK_PAGE:
        ui->feedback_option_button->click();
        break;
    default:
        break;
    }
}

void MainWindow::slot_on_configfile_update_finished()
{
    if(m_config_update_watcher->isFinished())
    {
        Config::Instance()->ReloadConfig();
        Config::Instance()->LoadPublishWordlib();
        OnUpdateTheWidgetConfig();
    }
}
void MainWindow::slot_show_edit_hotkey()
{
    MainWindowShowNormalSlot();
    ui->advanced_option_button->click();
}

