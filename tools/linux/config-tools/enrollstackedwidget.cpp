#include "enrollstackedwidget.h"
#include "ui_enrollstackedwidget.h"
#include "../public/configmanager.h"
#include "configbus.h"
#include "../public/utils.h"
#include "networkhandler.h"

#include <QPixmap>
#include <QRegExp>
#include <QRegExpValidator>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QSettings>
#include <QCryptographicHash>
#include <QDebug>
#include <QAction>
#include <QDesktopServices>
#include <QUrl>

EnrollStackedWidget::EnrollStackedWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EnrollStackedWidget)
{
    ui->setupUi(this);
    InitWidget();
    ConnectSignalToSlot();
}

EnrollStackedWidget::~EnrollStackedWidget()
{
    delete ui;
}

void EnrollStackedWidget::InitWidget()
{
    ui->login_btn->setText("登录");
    ui->account_lineedit->setPlaceholderText(" 手机号/邮箱");
    ui->password_lineedit->setPlaceholderText(" 密码");

    ui->login_btn->setProperty("type","radiusBtn");
    ui->login_btn->setFlat(true);
    ui->login_btn->setFocusPolicy(Qt::NoFocus);

    ui->register_btn->setFlat(true);
    ui->register_btn->setText("立即注册");
    ui->register_btn->setFocusPolicy(Qt::NoFocus);
    ui->register_btn->setStyleSheet("QPushButton{border:none; font-size: 15px;color:rgb(77,156,248);}QPushButton:pressed{border:none;}"
                                    "QPushButton:hover{color:rgb(85,118,189);}");

    ui->forget_password_btn->setFlat(true);
    ui->forget_password_btn->setText("忘记密码");
    ui->forget_password_btn->setFocusPolicy(Qt::NoFocus);
    ui->forget_password_btn->setStyleSheet("QPushButton{border:none; font-size: 15px;color:rgb(77,156,248);}QPushButton:pressed{border:none;}"
                                    "QPushButton:hover{color:rgb(85,118,189);}");


    QPixmap pixmap(":/image/default_user.png");
    ui->title_label->setPixmap(pixmap.scaledToWidth(100,Qt::SmoothTransformation));
    ui->title_label->setStyleSheet("color:rgb(128,128,128);font-size:24px;");

    QRegExp reg(".{5,64}");
    QRegExp accountReg("[A-Za-z0-9_@\\-\\.]{5,64}");
    ui->account_lineedit->setValidator(new QRegExpValidator(accountReg));
    ui->password_lineedit->setValidator(new QRegExpValidator(reg));
    ui->password_lineedit->setEchoMode(QLineEdit::Password);

    QAction* accountAction = new QAction(ui->account_lineedit);
    accountAction->setIcon(QIcon(":/image/username.png"));
    ui->account_lineedit->addAction(accountAction,QLineEdit::LeadingPosition);

    QAction* passwordAction = new QAction(ui->password_lineedit);
    passwordAction->setIcon(QIcon(":/image/password.png"));
    ui->password_lineedit->addAction(passwordAction,QLineEdit::LeadingPosition);
    ui->forget_password_btn->setVisible(false);
    m_vericodeCheckThread = new VerifyCodeCheckThread();
    connect(m_vericodeCheckThread,SIGNAL(hasthemodules()),this, SLOT(ShowTheForgetPasswordBtn()));
    m_vericodeCheckThread->start();
    m_login_future_watcher = new QFutureWatcher<int>(this);
    connect(m_login_future_watcher, &QFutureWatcher<int>::finished, this,&EnrollStackedWidget::SlotLoginprocessFinished);

}

void EnrollStackedWidget::ConnectSignalToSlot()
{
    connect(ui->login_btn, SIGNAL(clicked(bool)), this, SLOT(OnLoginBtnClickedSlot()));
    connect(ui->register_btn, SIGNAL(clicked(bool)), this, SLOT(OnToRegisterBtnClickedSlot()));
    connect(ui->forget_password_btn, SIGNAL(clicked(bool)),this, SLOT(OpenWebSiteLink()));
}

void EnrollStackedWidget::keyReleaseEvent(QKeyEvent *ev)
{
    if(ev->key() == Qt::Key_Return || ev->key() == Qt::Key_Enter) /*回车键 小键盘enter*/
    {
        OnLoginBtnClickedSlot();
        return;
    }
    QWidget::keyPressEvent(ev);
}

void EnrollStackedWidget::OnLoginBtnClickedSlot()
{

    QString userAccount = ui->account_lineedit->text();
    QString password = ui->password_lineedit->text();
    bool isAccountOK = CheckAccountPattern(userAccount);
    bool isPasswordOK = CheckPasswordPattern(password);
    if(!isAccountOK || !isPasswordOK)
    {
        ShowtoolTip("用户名或密码错误");
        return;
    }
    ui->login_btn->setEnabled(false);
    QFuture<int> login_future = QtConcurrent::run(this,&EnrollStackedWidget::SendRequestToServer,userAccount,password);
    m_login_future_watcher->setFuture(login_future);
}

void EnrollStackedWidget::OnToRegisterBtnClickedSlot()
{
    ResetWidget();
    emit switchToRegister();
}

bool EnrollStackedWidget::CheckAccountPattern(QString inputStr)
{
    QString pattern("[A-Za-z0-9_@\\-\\.]{5,64}");
    QRegExp rx(pattern);
    bool isMatch = rx.exactMatch(inputStr);
    return isMatch;
}

bool EnrollStackedWidget::CheckPasswordPattern(QString inputStr)
{
    QString pattern(".{5,64}");
    QRegExp rx(pattern);
    bool isMatch = rx.exactMatch(inputStr);
    return isMatch;
}

void EnrollStackedWidget::OpenWebSiteLink()
{ 
    QString domain = ConfigManager::Instance()->GetServerUrl();
    QString forget_pwd_url = QString("%1%2").arg(domain).arg("/index.html#/sy?ck=wjmm");
    QDesktopServices::openUrl(QUrl(forget_pwd_url));
}

void EnrollStackedWidget::ShowTheForgetPasswordBtn()
{
    ui->forget_password_btn->setVisible(true);
}

void EnrollStackedWidget::SlotLoginprocessFinished()
{
    try
    {
        if(m_login_future_watcher->isFinished())
        {
            QFuture<int> result = m_login_future_watcher->future();
            ui->login_btn->setEnabled(true);
            int return_code = result.resultAt(0);
            if(return_code == 0)
            {
                // save to config
                ConfigManager::Instance()->SetStrConfigItem("clientid",m_clientid);
                ConfigManager::Instance()->SetStrConfigItem("loginid",m_loginid);
                ConfigManager::Instance()->SetStrConfigItem("username",m_username.toUtf8());
                ConfigManager::Instance()->SetIntConfigItem("login",1);
                ConfigManager::Instance()->ReloadUserConfig();                

                ResetWidget();
                emit loginSucceeded();
            }
            else if(return_code==6 || return_code==1)
            {
                ShowtoolTip(m_loginErrMsg);
            }
            else
            {
                ShowtoolTip("登录失败，请检查服务器及网络配置");
            }
        }
    }
    catch(std::exception& e)
    {
        Q_UNUSED(e);
        Utils::WriteLogToFile("slot after logined throw exception");
    }

}

int EnrollStackedWidget::SendRequestToServer(QString account,QString password)
{

    // post request
    QNetworkRequest request;
    QString url = QString("").arg(ConfigManager::Instance()->GetServerUrl());
    Utils::AddSystemExtraParamer(url,OPType::MANUAL);
    request.setUrl(QUrl(url));
    request.setSslConfiguration(Utils::GetQsslConfig());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QByteArray data;
    data.append("loginid=");
    data.append(account.toUtf8().toBase64());
    data.append("&password=");
    data.append(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Md5).toHex());
    data.append("&fromos=");
    data.append(QString::number(Utils::GetOSTypeIndex()));
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QNetworkReply* reply = manager->post(request, data);
    QReplyTimeout *pTimeout = new QReplyTimeout(reply,5000);

    QEventLoop eventLoop;
    connect(manager, SIGNAL(finished(QNetworkReply*)),&eventLoop, SLOT(quit()));
    connect(pTimeout, SIGNAL(timeout()),&eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    if(pTimeout->GetTimeoutFlag())
    {
        m_loginErrMsg = "请求超时";
        return 1;
    }

    int error_code = reply->error();
    QString error_str = reply->errorString();

    if(error_code == QNetworkReply::NoError)
    {
        QString statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString();
        if (statusCode.isEmpty())
        {
            return 2;
        }
        QByteArray replyData = reply->readAll();
        QJsonParseError json_error;
        QJsonDocument jsonDoc(QJsonDocument::fromJson(replyData, &json_error));

        if(json_error.error != QJsonParseError::NoError)
        {

            return 3;
        }

        QJsonObject rootObj = jsonDoc.object();

        if(!rootObj.contains("code") || !rootObj.contains("message"))
        { 
            return 4;
        }
        QString codeStr = rootObj.value("code").toString();
        m_loginErrMsg = rootObj.value("message").toString();

        if (codeStr.compare("200") == 0)
        {
            QJsonObject resultObj = rootObj.value("result").toObject();

            if (!resultObj.isEmpty())
            {
                m_clientid = resultObj.value("clientid").toString();
                m_loginid = resultObj.value("loginid").toString();
                m_username = resultObj.value("username").toString();

                return 0;
            }
            return 5;

        }
        else
        {
            return 6;
        }
    }
    else
    {
        return 7;
    }

}


void EnrollStackedWidget::ShowtoolTip(const QString inputStr)
{
    ui->tip_info_label->setText(QString("<style>a{color:red; font-size:14px;}</style> <a>%1</a>").arg(inputStr));
}

void EnrollStackedWidget::ResetWidget()
{
    ui->password_lineedit->clear();
    ShowtoolTip("");
}
