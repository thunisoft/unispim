#include "newenrollstackedwidget.h"
#include "ui_newenrollstackedwidget.h"
#include <QScreen>
#include <QTabBar>
#include <QDesktopServices>
#include <QRegExp>
#include <QRegExpValidator>
#include <QtConcurrent/QtConcurrent>
#include <QByteArray>
#include <QUrl>
#include <QTimer>
#include "utils.h"
#include "config.h"
#include "networkhandler.h"
#include "mysqlite.h"

NewEnrollStackedWidget::NewEnrollStackedWidget(QWidget *parent) :
    BaseStackedWidget(parent),
    ui(new Ui::NewEnrollStackedWidget)
{
    ui->setupUi(this);
    InitWidget();
    ConnectSignalToSlot();
    LoadConfigInfo();
}

NewEnrollStackedWidget::~NewEnrollStackedWidget()
{
    delete ui;
}


void NewEnrollStackedWidget::InitWidget()
{
    ui->get_verify_code_btn->setProperty("type","Verification_Code");
    ui->phone_enroll_btn->setProperty("type","login");
    ui->account_enroll_btn->setProperty("type","login");
    ui->phone_register_now_btn->setProperty("type","puretextbtn");
    ui->account_reigister_now_btn->setProperty("type","puretextbtn");

    QList<QPushButton*> enroll_btn_list = this->findChildren<QPushButton*>();
    for(int index=0; index<enroll_btn_list.size(); ++index)
    {
        enroll_btn_list.at(index)->setCursor(Qt::PointingHandCursor);
    }

    this->setProperty("type","stackedwidget");
    QList<QPushButton*> panelButtonList;
    panelButtonList.push_back(ui->phone_mode_btn);
    panelButtonList.push_back(ui->account_mode_btn);

    for(int index=0; index<panelButtonList.size();++index)
    {
        //panelButtonList.at(index)->setFlat(true);
        panelButtonList.at(index)->setCheckable(true);
        panelButtonList.at(index)->setAutoExclusive(true);
        panelButtonList.at(index)->setFocusPolicy(Qt::NoFocus);
        panelButtonList.at(index)->setProperty("type","loginmodebtn");
    }
    ui->account_mode_btn->setChecked(true);
    ui->stackedWidget->setCurrentIndex(0);
    ui->stackedWidget->setProperty("type","stackedWidget");

    ui->account_lineedit->setPlaceholderText("请输入账号");
    ui->password_lineedit->setPlaceholderText("请输入密码");
    ui->phone_lineeidt->setPlaceholderText("请输入手机号");
    ui->verifiCode_lineedit->setPlaceholderText("请输入验证码");

    ui->register_account_lineedit->setPlaceholderText("手机号/邮箱");
    ui->register_account_lineedit->setProperty("type","LoginInfo");
    ui->register_password_lineedit->setPlaceholderText("密码");
    ui->register_password_lineedit->setProperty("type","LoginInfo");
    ui->confirm_password_lineedit->setPlaceholderText("二次确认密码");
    ui->confirm_password_lineedit->setProperty("type","LoginInfo");
    ui->register_btn->setProperty("type","login");
    QRegExp reg(".{5,64}");
    QRegExp account_reg("[A-Za-z0-9_@\\.]{5,64}");
    ui->register_account_lineedit->setValidator(new QRegExpValidator(account_reg));
    ui->register_password_lineedit->setValidator(new QRegExpValidator(reg));
    ui->confirm_password_lineedit->setValidator(new QRegExpValidator(reg));
    ui->register_password_lineedit->setEchoMode(QLineEdit::Password);
    ui->confirm_password_lineedit->setEchoMode(QLineEdit::Password);
    ui->register_account_lineedit->setTextMargins(8,0,0,0);
    ui->register_password_lineedit->setTextMargins(8,0,0,0);
    ui->confirm_password_lineedit->setTextMargins(8,0,0,0);


    ui->account_lineedit->setProperty("type","LoginInfo");
    ui->password_lineedit->setProperty("type","LoginInfo");
    ui->phone_lineeidt->setProperty("type","LoginInfo");
    ui->verifiCode_lineedit->setProperty("type","LoginInfo");

    ui->account_lineedit->setTextMargins(8,0,0,0);
    ui->password_lineedit->setTextMargins(8,0,0,0);
    ui->phone_lineeidt->setTextMargins(8,0,0,0);
    ui->verifiCode_lineedit->setTextMargins(8,0,0,0);

    QRegExp phoneExp("^1[345678]\\d{9}$");
    //QRegExp emailExp("^[a-zA-Z0-9_-]+@[a-zA-Z0-9_-]+(\\.[a-zA-Z0-9_-]+)+$");
    QRegExp verifiCodeExp("^\\d{6}");

    QRegExp passwordRule("[a-zA-Z0-9~!@#$%^&*()]{1,20}");
    //ui->phone_lineeidt->setValidator(new QRegExpValidator(phoneExp));
    ui->password_lineedit->setEchoMode(QLineEdit::Password);
    m_vericodeCountDown = 60;
    m_countDownTimer = new QTimer(this);
    connect(m_countDownTimer, SIGNAL(timeout()), this, SLOT(SlotRefreshGetVeriCodeTime()));

#ifdef BUILD_FY_VERSION
    ui->account_reigister_now_btn->setVisible(false);
    ui->line_3->setVisible(false);
    ui->line_4->setVisible(false);
    ui->phone_mode_btn->setText("用户注册");
#endif
    m_account_login_watcher = new QFutureWatcher<int>(this);
    m_vericode_login_watcher = new QFutureWatcher<int>(this);
    m_getvericode_watcher = new QFutureWatcher<int>(this);

}

void NewEnrollStackedWidget::ConnectSignalToSlot()
{

    connect(ui->account_enroll_btn, SIGNAL(clicked()), this, SLOT(SlotAccountLogin()));
    connect(ui->account_reigister_now_btn, SIGNAL(clicked()), this, SLOT(SlotAccountRegisterNow()));
    connect(ui->get_verify_code_btn, SIGNAL(clicked()), this, SLOT(SlotGetVeriCode()));
    connect(ui->phone_enroll_btn, SIGNAL(clicked()), this, SLOT(SlotPhoneEnroll()));
    connect(ui->phone_register_now_btn, SIGNAL(clicked()), this, SLOT(SlotPhoneRegisterNow()));
    connect(ui->account_mode_btn, SIGNAL(clicked()), this, SLOT(SlotSwitchToAccount()));
    connect(ui->phone_mode_btn, SIGNAL(clicked()), this, SLOT(SlotSwitchToPhone()));

    connect(ui->account_lineedit, SIGNAL(returnPressed()), this, SLOT(SlotAccountLogin()));
    connect(ui->password_lineedit, SIGNAL(returnPressed()), this,  SLOT(SlotAccountLogin()));
    connect(ui->phone_lineeidt, SIGNAL(returnPressed()),this, SLOT(SlotPhoneEnroll()));
    connect(ui->verifiCode_lineedit, SIGNAL(returnPressed()), this, SLOT(SlotPhoneEnroll()));
    connect(ui->register_btn, &QPushButton::clicked,this, &NewEnrollStackedWidget::slot_click_register_btn);
    connect(m_account_login_watcher,&QFutureWatcher<int>::finished,this,&NewEnrollStackedWidget::slot_on_account_request_completed);
    connect(m_vericode_login_watcher,&QFutureWatcher<int>::finished,this,&NewEnrollStackedWidget::slot_on_vericode_request_completed);
    connect(m_getvericode_watcher,&QFutureWatcher<int>::finished,this,&NewEnrollStackedWidget::slot_on_getvericode_request_completed);
}


void NewEnrollStackedWidget::LoadConfigInfo()
{
    ui->auto_enroll_checkbox->disconnect();
    ui->remember_password_checkbox->disconnect();
    ConfigItemStruct configInfo;
    QString loginIDStr;
    if(Config::Instance()->GetConfigItemByJson("loginid",configInfo))
    {
       loginIDStr = configInfo.itemCurrentStrValue;
    }

    if(Config::Instance()->GetConfigItemByJson("remember_password",configInfo))
    {
        int  flag = configInfo.itemCurrentIntValue;
        flag? ui->remember_password_checkbox->setChecked(true): ui->remember_password_checkbox->setChecked(false);
        if(flag)
        {
            QString password = MySQLite::instance()->SelecteUserPassword(loginIDStr);

            ui->password_lineedit->setText(password);
            ui->account_lineedit->setText(loginIDStr);

        }
    }

    if(Config::Instance()->GetConfigItemByJson("auto_enroll",configInfo))
    {
        (configInfo.itemCurrentIntValue)? ui->auto_enroll_checkbox->setChecked(true): ui->auto_enroll_checkbox->setChecked(false);

    }
    ConfigItemStruct server_type_struct;
    bool isExist = Config::Instance()->GetConfigItemByJson("server_type", server_type_struct);
    if (isExist && (server_type_struct.itemCurrentStrValue == "private"))
    {
        ui->phone_mode_btn->setVisible(false);
    }
    else
    {
        ui->phone_mode_btn->setVisible(true);
    }


    connect(ui->remember_password_checkbox, SIGNAL(stateChanged(int)),this, SLOT(SlotRememberPassword(int)));
    connect(ui->auto_enroll_checkbox, SIGNAL(stateChanged(int)),this, SLOT(SlotAutoEnroll(int)));
}

void NewEnrollStackedWidget::SlotRememberPassword(int checkStatus)
{
    ConfigItemStruct configInfo;
    configInfo.itemName = "remember_password";
    configInfo.itemCurrentIntValue =(checkStatus == Qt::Checked);
    configInfo.itemGroupName = "state";
    Config::Instance()->SetConfigItemByJson("remember_password",configInfo);
    Config::Instance()->SaveSystemConfig();
}

void NewEnrollStackedWidget::SlotAutoEnroll(int checkStatus)
{
    ConfigItemStruct configInfo;
    configInfo.itemName = "auto_enroll";
    configInfo.itemCurrentIntValue =(checkStatus == Qt::Checked);
    configInfo.itemGroupName = "state";
    Config::Instance()->SetConfigItemByJson("auto_enroll",configInfo);
    Config::Instance()->SaveSystemConfig();
}

void NewEnrollStackedWidget::SlotAccountLogin()
{
    QRegExp passwordMatch("^(?![0-9]+$)(?![a-zA-Z]+$)[0-9A-Za-z~!@#$%^&*()]{6,20}$");
    QRegExp emailExp("^[a-zA-Z0-9_-]+@[a-zA-Z0-9_-]+(\\.[a-zA-Z0-9_-]+)+$");
    QString accountString = ui->account_lineedit->text();
    QString passwordString = ui->password_lineedit->text();
    QRegExp phoneExp("^1[345678]\\d{9}$");

    if(accountString.isEmpty() || passwordString.isEmpty())
    {
        Utils::NoticeMsgBox("账号或密码不能为空",this);
        return;
    }

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

    accountString = accountString.toLocal8Bit().toBase64();
    ui->account_enroll_btn->setEnabled(false);
    m_elasted_timer.start();
    QFuture<int> res = QtConcurrent::run(NetworkHandler::Instance(),&NetworkHandler::UserAccountLogin,accountString,passwordString);
    m_account_login_watcher->setFuture(res);

}
void NewEnrollStackedWidget::SlotAccountRegisterNow()
{

    QString requestHeader;
    if(NetworkHandler::Instance()->GetRequesetHeader(requestHeader))
    {
        QUrl url(requestHeader + QString("/index.html;jsessionid=node079fg8q35fecs1o8m0mxjeml6z23.node0#/zc"));
        QDesktopServices::openUrl(url);
    }

}
void NewEnrollStackedWidget::SlotGetVeriCode()
{
    QRegExp phoneExp("^1[345678]\\d{9}$");
    QString phoneNum = ui->phone_lineeidt->text();

    if(phoneNum.isEmpty())
    {
        Utils::NoticeMsgBox("请输入手机号",this);
        return ;
    }

    if(!phoneExp.exactMatch(phoneNum))
    {
        Utils::NoticeMsgBox("手机号格式不正确",this);
        return ;
    }
    ui->get_verify_code_btn->setEnabled(false);

   QFuture<int> res = QtConcurrent::run(NetworkHandler::Instance(),&NetworkHandler::GetVerificationCode,phoneNum);
   m_getvericode_watcher->setFuture(res);
}
void NewEnrollStackedWidget::SlotPhoneEnroll()
{
    QRegExp phoneExp("^1[345678]\\d{9}$");

    QString phoneString = ui->phone_lineeidt->text();
    QString verifyCode = ui->verifiCode_lineedit->text();

    if(phoneString.isEmpty())
    {
        Utils::NoticeMsgBox("请输入手机号",this);
        return;
    }

    if(!phoneExp.exactMatch(phoneString))
    {
        Utils::NoticeMsgBox("手机号码不正确",this);
        return;
    }

    QFuture<int> res = QtConcurrent::run(NetworkHandler::Instance(),&NetworkHandler::UserPhoneLogin,phoneString,verifyCode);
    m_vericode_login_watcher->setFuture(res);
    ui->phone_enroll_btn->setEnabled(false);
}
void NewEnrollStackedWidget::SlotPhoneRegisterNow()
{

    QString requestHeader;
    if(NetworkHandler::Instance()->GetRequesetHeader(requestHeader))
    {
        QUrl url(requestHeader + QString("/index.html;jsessionid=node079fg8q35fecs1o8m0mxjeml6z23.node0#/zc"));
        QDesktopServices::openUrl(url);
    }

}

void NewEnrollStackedWidget::SlotSwitchToAccount()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void NewEnrollStackedWidget::SlotSwitchToPhone()
{
#ifdef BUILD_FY_VERSION
     ui->stackedWidget->setCurrentIndex(2);
#else
     ui->stackedWidget->setCurrentIndex(1);
#endif

}

void NewEnrollStackedWidget::ResetContent()
{
    ui->account_lineedit->clear();
    ui->password_lineedit->clear();
    ui->phone_lineeidt->clear();
    ui->verifiCode_lineedit->clear();
}

void NewEnrollStackedWidget::SlotRefreshGetVeriCodeTime()
{
    if(m_vericodeCountDown == 0)
    {
        ui->get_verify_code_btn->setText("获取验证码");
        ui->get_verify_code_btn->setEnabled(true);
        m_vericodeCountDown = 60;
        m_countDownTimer->stop();
    }
    else
    {
        ui->get_verify_code_btn->setText(QString("重新发送%1").arg(m_vericodeCountDown));
        ui->get_verify_code_btn->setEnabled(false);
        m_vericodeCountDown--;
    }
}

void NewEnrollStackedWidget::slot_click_register_btn()
{
    auto check_account_pattern = [](QString inputStr)->bool{
        QRegExp account_reg("^[a-zA-Z0-9_-]+@[a-zA-Z0-9_-]+(\\.[a-zA-Z0-9_-]+)+$|^1[3|4|5|7|8][0-9]{9}$");
        bool isMatch = account_reg.exactMatch(inputStr);
        return isMatch;
    };
    auto check_password_pattern = [&](QString pass_word)->bool{
        QRegExp password_reg(".{5,64}");
        bool isMatch = password_reg.exactMatch(pass_word);
        return isMatch;

    };
    QString account = ui->register_account_lineedit->text();
    QString password = ui->register_password_lineedit->text();
    QString confirmStr = ui->confirm_password_lineedit->text();

    if(!check_account_pattern(account) || !check_password_pattern(password))
    {
        Utils::NoticeMsgBox("用户名或密码格式不对",this);
        return;
    }
    if(confirmStr.compare(password) != 0)
    {
        Utils::NoticeMsgBox("两次输入的密码不一样",this);
        return;
    }
    int return_code = NetworkHandler::Instance()->RegisterUserAccount(account,password);
    if(return_code == 0){
        Utils::NoticeMsgBox("注册成功",this);
        ui->register_account_lineedit->clear();
        ui->register_password_lineedit->clear();
        ui->confirm_password_lineedit->clear();
    }
    else if(return_code == 666){
        Utils::NoticeMsgBox("请求超时",this);
    }
    else if(return_code == 601)
    {
        Utils::NoticeMsgBox("用户名重复\n注册失败",this);
    }
    else
    {
         Utils::NoticeMsgBox("注册失败",this);
    }


}

void NewEnrollStackedWidget::slot_register_account()
{

}

void NewEnrollStackedWidget::slot_on_account_request_completed()
{
    qDebug() << "network request cost" << m_elasted_timer.elapsed();
    ui->account_enroll_btn->setEnabled(true);
    int returnCode = m_account_login_watcher->future().result();
    QString accountString = ui->account_lineedit->text();
    QString passwordString = ui->password_lineedit->text();

    if( returnCode == 0)
    {
        ConfigItemStruct configInfo;
        if(Config::Instance()->GetConfigItemByJson("remember_password",configInfo))
        {
            int  flag = configInfo.itemCurrentIntValue;
            if(flag)
            {
                //增加用户信息
                if(!MySQLite::instance()->IsUserExist(accountString))
                {
                    MySQLite::instance()->InsertUserInfo(accountString,passwordString);
                }
                else
                {
                    MySQLite::instance()->UpdateUserInfo(accountString,passwordString);
                }


            }
        }
        qDebug() << "emit loginsucceed cost:" << m_elasted_timer.elapsed();
        emit loginSucceed();
    }
    else if(returnCode == -1)
    {
        Utils::NoticeMsgBox("配置文件异常\n",this);
    }
    else if(returnCode == 603)
    {
       Utils::NoticeMsgBox("用户不存在",this);
    }
    else if(returnCode == 404)
    {
       Utils::NoticeMsgBox("网络连接错误",this);
    }
    else if(returnCode == 666)
    {
        Utils::NoticeMsgBox("请求超时",this);
    }
    else
    {
        Utils::NoticeMsgBox("账号或密码错误",this);
    }
}

void NewEnrollStackedWidget::slot_on_vericode_request_completed()
{
    ui->phone_enroll_btn->setEnabled(true);
    int returnCode = m_vericode_login_watcher->future().result();
    if( returnCode == 0)
    {
        emit loginSucceed();
    }
    else if(returnCode == 666)
    {
        Utils::NoticeMsgBox("请求超时",this);
    }
    else if(returnCode == -1)
    {
        Utils::NoticeMsgBox("系统配置文件异常\n",this);
    }
    else if(returnCode == 603)
    {
        Utils::NoticeMsgBox("用户不存在",this);
    }
    else
    {
        Utils::NoticeMsgBox("验证码登录失败",this);
    }
}

void NewEnrollStackedWidget::slot_on_getvericode_request_completed()
{
    int returnCode = m_getvericode_watcher->future().result();
    if(returnCode != 0)
    {
        ui->get_verify_code_btn->setEnabled(true);
    }
    if( returnCode == 0)
    {
        m_countDownTimer->start(1000);
        //Utils::NoticeMsgBox("获取验证码成功,请稍后");
    }
    else if(returnCode == 666)
    {
        Utils::NoticeMsgBox("请求超时",this);
    }
    else if(returnCode == -1)
    {
        Utils::NoticeMsgBox("配置文件异常\n",this);
    }
    else if(returnCode == 603)
    {
        Utils::NoticeMsgBox("用户不存在",this);
    }
    else if(returnCode == 604)
    {
        Utils::NoticeMsgBox("用户无效",this);
    }
    else
    {
        Utils::NoticeMsgBox("获取验证码失败",this);
    }
}
