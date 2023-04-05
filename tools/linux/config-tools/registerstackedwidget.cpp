#include "registerstackedwidget.h"
#include "ui_registerstackedwidget.h"
#include "../public/configmanager.h"
#include "../public/utils.h"

#include <QAction>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QRegExpValidator>
#include <QCryptographicHash>
#include <QDebug>
#include "msgboxutils.h"

RegisterStackedWidget::RegisterStackedWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RegisterStackedWidget)
{
    ui->setupUi(this);
    InitWidget();
    ConnectSignalToSlot();
}

RegisterStackedWidget::~RegisterStackedWidget()
{
    delete ui;
}

void RegisterStackedWidget::InitWidget()
{
    ui->register_btn->setText("注册");
    ui->account_lineedit->setPlaceholderText(" 手机号/邮箱");
    ui->password_lineedit->setPlaceholderText(" 密码");
    ui->confirm_password_lineedit->setPlaceholderText(" 二次确认密码");

    ui->register_btn->setProperty("type","radiusBtn");
    ui->register_btn->setFlat(true);
    ui->register_btn->setFocusPolicy(Qt::NoFocus);

    ui->login_btn->setFlat(true);
    ui->login_btn->setText("去登录");
    ui->login_btn->setFocusPolicy(Qt::NoFocus);
    ui->login_btn->setStyleSheet("QPushButton{border:none; color:rgb(77,156,248);font-size:15px;}QPushButton:pressed{border:none;}"
                                 "QPushButton:hover{color:rgb(85,118,189);}");

    ui->title_label->setText("新用户注册");
    ui->line->setStyleSheet("background:rgb(77,156,248);max-height:1px;border:none;");
    ui->line_2->setStyleSheet("background:rgb(77,156,248);max-height:1px;border:none;");

    ui->title_label->setText("新用户注册");
    ui->title_label->setStyleSheet("color:rgb(128,128,128);font-size:24px;");

    QRegExp reg(".{5,64}");
    QRegExp account_reg("[A-Za-z0-9_@\\.]{5,64}");
    ui->account_lineedit->setValidator(new QRegExpValidator(account_reg));
    ui->password_lineedit->setValidator(new QRegExpValidator(reg));
    ui->confirm_password_lineedit->setValidator(new QRegExpValidator(reg));
    ui->password_lineedit->setEchoMode(QLineEdit::Password);
    ui->confirm_password_lineedit->setEchoMode(QLineEdit::Password);


    QAction* accountAction = new QAction(ui->account_lineedit);
    accountAction->setIcon(QIcon(":/image/username.png"));
    ui->account_lineedit->addAction(accountAction,QLineEdit::LeadingPosition);

    QAction* passwordAction = new QAction(ui->password_lineedit);
    passwordAction->setIcon(QIcon(":/image/password.png"));
    ui->password_lineedit->addAction(passwordAction,QLineEdit::LeadingPosition);

    QAction* confirmPasswordAction = new QAction(ui->confirm_password_lineedit);
    confirmPasswordAction->setIcon(QIcon(":/image/password.png"));
    ui->confirm_password_lineedit->addAction(confirmPasswordAction,QLineEdit::LeadingPosition);

}

void RegisterStackedWidget::ConnectSignalToSlot()
{
    connect(ui->login_btn, SIGNAL(clicked(bool)), this, SLOT(OnLoginBtnClicked()));
    connect(ui->register_btn, SIGNAL(clicked(bool)),this ,SLOT(OnRegisterBtnClicked()));
    connect(ui->account_lineedit, SIGNAL(textChanged(QString)),this,SLOT(CheckAccountPattern(QString)));
    connect(ui->password_lineedit, SIGNAL(textChanged(QString)),this, SLOT(CheckPasswordPattern(QString)));
    connect(ui->confirm_password_lineedit, SIGNAL(textChanged(QString)),this, SLOT(CheckConfirmPasswordPattern(QString)));
}

void RegisterStackedWidget::OnLoginBtnClicked()
{
    ResetWidget();
    emit switchToLogin();
}

void RegisterStackedWidget::OnRegisterBtnClicked()
{
    QString account = ui->account_lineedit->text();
    QString password = ui->password_lineedit->text();
    QString confirmStr = ui->confirm_password_lineedit->text();

    if(!CheckAccountPattern(account) || !CheckPasswordPattern(password))
    {
        SetTipInfo("用户名或密码格式不对");
        return;
    }
    if(confirmStr.compare(password) != 0)
    {
        SetTipInfo("两次输入的密码不一样");
        return;
    }
//    if(ui->agreement_checkbox->checkState() != Qt::Checked)
//    {
//        SetTipInfo("请阅读用户协议");
//        return;
//    }
    SendRequestToSever();

}

void RegisterStackedWidget::keyReleaseEvent(QKeyEvent *ev)
{
    if(ev->key() == Qt::Key_Return || ev->key() == Qt::Key_Enter) /*回车键 小键盘enter*/
    {
        OnRegisterBtnClicked();
        return;
    }
    QWidget::keyPressEvent(ev);
}

bool RegisterStackedWidget::CheckAccountPattern(QString inputStr)
{
    QRegExp account_reg("^[a-zA-Z0-9_-]+@[a-zA-Z0-9_-]+(\\.[a-zA-Z0-9_-]+)+$|^1[3|4|5|7|8][0-9]{9}$");
    bool isMatch = account_reg.exactMatch(inputStr);
    if(isMatch)
    {
        QPixmap pixmap(":/image/right.png");
        ui->account_tip_lable->setPixmap(pixmap.scaledToWidth(16,Qt::SmoothTransformation));
        SetTipInfo("");
    }
    else
    {
        QPixmap pixmap(":/image/wrong.png");
        ui->account_tip_lable->setPixmap(pixmap.scaledToWidth(16,Qt::SmoothTransformation));
        SetTipInfo("用戶名应为手机号/邮箱");
    }
    return isMatch;
}

bool RegisterStackedWidget::CheckPasswordPattern(QString inputStr)
{
    QRegExp password_reg(".{5,64}");
    bool isMatch = password_reg.exactMatch(inputStr);
    if(isMatch)
    {
        QPixmap pixmap(":/image/right.png");
        ui->password_tip_label->setPixmap(pixmap.scaledToWidth(16,Qt::SmoothTransformation));
        SetTipInfo("");
    }
    else
    {
        QPixmap pixmap(":/image/wrong.png");
        ui->password_tip_label->setPixmap(pixmap.scaledToWidth(16,Qt::SmoothTransformation));
        SetTipInfo("密码长度应大于5位小于64位");
    }
    return isMatch;
}

bool RegisterStackedWidget::CheckConfirmPasswordPattern(QString inputStr)
{
    QString password = ui->password_lineedit->text();
    QRegExp password_reg(".{5,64}");
    bool isMatch = password_reg.exactMatch(inputStr);
    if((password.compare(inputStr) == 0) && isMatch)
    {
        QPixmap pixmap(":/image/right.png");
        ui->confirm_password_tip_label->setPixmap(pixmap.scaledToWidth(16,Qt::SmoothTransformation));
        return true;
    }
    else
    {
        QPixmap pixmap(":/image/wrong.png");
        ui->confirm_password_tip_label->setPixmap(pixmap.scaledToWidth(16,Qt::SmoothTransformation));
        return false;
    }
}

void RegisterStackedWidget::SendRequestToSever()
{

    QString account = ui->account_lineedit->text();
    QString password = ui->password_lineedit->text();
    ui->register_btn->setEnabled(false);

    // post request
    QNetworkRequest request;
    QString url = QString("%1/testapi/requesturl").arg(ConfigManager::Instance()->GetServerUrl());
    request.setUrl(QUrl(url));
    request.setSslConfiguration(Utils::GetQsslConfig());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QByteArray data;
    data.append("username=");
    data.append("");
    data.append("&loginid=");
    data.append(account.toUtf8());
    data.append("&password=");
    data.append(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Md5).toHex());
    data.append("&clientid=");
    data.append(Utils::GethostMac().toUtf8());
    data.append("&fromos=");
    data.append(QString::number(Utils::GetOSTypeIndex()));
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QNetworkReply* reply = manager->post(request, data);
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    QTimer timer;
    timer.setInterval(10000);
    timer.setSingleShot(true);
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    timer.start();
    loop.exec();
    if(timer.isActive())
    {
        ui->register_btn->setEnabled(true);
        if(reply->error() != QNetworkReply::TimeoutError )
        {
            onRegisterReplyFinished(reply);
        }
        else
        {
            SetTipInfo("注册失败，请检测服务器及网络配置");
            return;
        }
    }
    else
    {
        ui->register_btn->setEnabled(true);

        disconnect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        reply->abort();
        reply->close();
        reply->deleteLater();
        SetTipInfo("注册失败，请检测服务器及网络配置");
    }
}

void RegisterStackedWidget::SetTipInfo(const QString inputStr)
{
    ui->tipinfo_label->setText(QString("<style>a{color:red; font-size:14px;}</style> <a>%1</a>").arg(inputStr));
}

void RegisterStackedWidget::onRegisterReplyFinished(QNetworkReply *reply)
{
    ui->register_btn->setEnabled(true);
    QString statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString();
    if (!statusCode.isEmpty())
    {
        qDebug() << "status code: " << statusCode.toInt();
    }
    QByteArray replyData = reply->readAll();
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(replyData, &json_error));

    if(json_error.error != QJsonParseError::NoError)
    {
        SetTipInfo("注册失败，请检查服务器及网络配置");
        return;
    }

    QJsonObject rootObj = jsonDoc.object();

    if(!rootObj.contains("code") || !rootObj.contains("message") || !rootObj.value("code").isString() || !rootObj.value("message").isString())
    {
        SetTipInfo("注册失败，请检查服务器及网络配置");
        return;
    }
    QString codeStr = rootObj.value("code").toString();
    QString messageStr = rootObj.value("message").toString();

    if (codeStr.compare("200") == 0)
    {
        MsgBoxUtils::NoticeMsgBox("注册成功","提示",0);
        SetTipInfo("注册成功");
        ResetWidget();
        emit RegisterSucceed();
    }
    else
    {
        SetTipInfo(messageStr);
    }

}

void RegisterStackedWidget::ResetWidget()
{
    ui->account_lineedit->clear();
    ui->password_lineedit->clear();
    ui->confirm_password_lineedit->clear();
    ui->account_tip_lable->clear();
    ui->password_tip_label->clear();
    ui->confirm_password_tip_label->clear();
    SetTipInfo("");
}
