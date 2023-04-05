#include "resetpasswdstackedwidget.h"
#include "ui_resetpasswdstackedwidget.h"
#include "../public/configmanager.h"
#include "../public/utils.h"
#include "msgboxutils.h"

#include <QAction>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QDebug>
#include <QRegExpValidator>

ResetPasswdStackedWidget::ResetPasswdStackedWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ResetPasswdStackedWidget)
{
    SetUnShow();
    ui->setupUi(this);
    InitWidget();
    ConnectSignalToSlot();
}

ResetPasswdStackedWidget::~ResetPasswdStackedWidget()
{
    delete ui;
}

void ResetPasswdStackedWidget::InitWidget()
{
    ui->confirm_btn->setText("确认");
    ui->old_passwd_lineedit->setPlaceholderText("旧密码");
    ui->new_password_lineedit->setPlaceholderText(" 密码");
    ui->confirm_password_lineedit->setPlaceholderText(" 二次确认密码");


    ui->confirm_btn->setProperty("type","radiusBtn");
    ui->confirm_btn->setFlat(true);
    ui->confirm_btn->setFocusPolicy(Qt::NoFocus);

    ui->return_btn->setFlat(true);
    ui->return_btn->setText("返回");
    ui->return_btn->setFocusPolicy(Qt::NoFocus);
    ui->return_btn->setStyleSheet("QPushButton{border:none; color:rgb(77,156,248);}QPushButton:pressed{border:none;}"
                                  "QPushButton:hover{color:rgb(85,118,189);}");

    ui->title_label->setText("密码修改");
    ui->title_label->setStyleSheet("color:rgb(128,128,128);font-size:24px;");

    ui->line->setStyleSheet("background:rgb(77,156,248);max-height:1px;border:none;");
    ui->line_2->setStyleSheet("background:rgb(77,156,248);max-height:1px;border:none;");


    QRegExp reg(".{5,64}");
    //QRegExp account_reg("[A-Za-z0-9_@\\.]{5,64}");
    ui->old_passwd_lineedit->setValidator(new QRegExpValidator(reg));
    ui->new_password_lineedit->setValidator(new QRegExpValidator(reg));
    ui->confirm_password_lineedit->setValidator(new QRegExpValidator(reg));
    ui->old_passwd_lineedit->setEchoMode(QLineEdit::Password);
    ui->new_password_lineedit->setEchoMode(QLineEdit::Password);
    ui->confirm_password_lineedit->setEchoMode(QLineEdit::Password);


    QAction* oldpasswordAction = new QAction(ui->old_passwd_lineedit);
    oldpasswordAction->setIcon(QIcon(":/image/password.png"));
    ui->old_passwd_lineedit->addAction(oldpasswordAction,QLineEdit::LeadingPosition);

    QAction* passwordAction = new QAction(ui->new_password_lineedit);
    passwordAction->setIcon(QIcon(":/image/password.png"));
    ui->new_password_lineedit->addAction(passwordAction,QLineEdit::LeadingPosition);

    QAction* confirmPasswordAction = new QAction(ui->confirm_password_lineedit);
    confirmPasswordAction->setIcon(QIcon(":/image/password.png"));
    ui->confirm_password_lineedit->addAction(confirmPasswordAction,QLineEdit::LeadingPosition);

}

void ResetPasswdStackedWidget::ConnectSignalToSlot()
{
    connect(ui->return_btn, SIGNAL(clicked(bool)),this ,SLOT(OnReturnButtonClicked()));
    connect(ui->confirm_btn, SIGNAL(clicked(bool)),this ,SLOT(OnConfirmButtonClicked()));
    connect(ui->old_passwd_lineedit, SIGNAL(textChanged(QString)),this,SLOT(CheckOldPasswdPattern(QString)));
    connect(ui->new_password_lineedit, SIGNAL(textChanged(QString)),this, SLOT(CheckPasswordPattern(QString)));
    connect(ui->confirm_password_lineedit, SIGNAL(textChanged(QString)),this, SLOT(CheckConfirmPasswordPattern(QString)));
}

void ResetPasswdStackedWidget::OnConfirmButtonClicked()
{
    QString old_passwd = ui->old_passwd_lineedit->text();
    QString password = ui->new_password_lineedit->text();
    QString confirmStr = ui->confirm_password_lineedit->text();

    if(!CheckOldPasswdPattern(old_passwd) || !CheckPasswordPattern(password))
        return;
    if(confirmStr.compare(password) != 0)
    {
        SetTipInfo("两次输入的密码不一样");
        return;
    }
    SendRequestToSever();

}

void ResetPasswdStackedWidget::keyReleaseEvent(QKeyEvent *ev)
{
    if(ev->key() == Qt::Key_Return || ev->key() == Qt::Key_Enter) /*回车键 小键盘enter*/
    {
        OnConfirmButtonClicked();
        return;
    }
    QWidget::keyPressEvent(ev);
}

bool ResetPasswdStackedWidget::CheckOldPasswdPattern(const QString&  inputStr)
{
    QRegExp password_reg(".{5,64}");
    bool isMatch = password_reg.exactMatch(inputStr);
    if(isMatch)
    {
        QPixmap pixmap(":/image/right.png");
        ui->old_passwd_tip_label->setPixmap(pixmap.scaledToWidth(16,Qt::SmoothTransformation));
        SetTipInfo("");
    }
    else
    {
        QPixmap pixmap(":/image/wrong.png");
        ui->old_passwd_tip_label->setPixmap(pixmap.scaledToWidth(16,Qt::SmoothTransformation));
        SetTipInfo("密码长度应大于5位小于64位");
    }
    return isMatch;
}

bool ResetPasswdStackedWidget::CheckPasswordPattern(QString inputStr)
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

bool ResetPasswdStackedWidget::CheckConfirmPasswordPattern(QString inputStr)
{
    QString password = ui->new_password_lineedit->text();
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

void ResetPasswdStackedWidget::SendRequestToSever()
{

    QString login_id;
    ConfigManager::Instance()->GetStrConfigItem("loginid",login_id);
    QString old_passwd = ui->old_passwd_lineedit->text();
    QString new_passwd = ui->new_password_lineedit->text();
    ui->confirm_btn->setEnabled(false);

    // post request
    QNetworkRequest request;
    QString url = QString("%1/testapi/requesturl").arg(ConfigManager::Instance()->GetServerUrl());
    request.setUrl(QUrl(url));
    request.setSslConfiguration(Utils::GetQsslConfig());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QByteArray data;
    data.append("loginid=");
    data.append(login_id.toUtf8());
    data.append("&oldPassword=");
    data.append(QCryptographicHash::hash(old_passwd.toUtf8(), QCryptographicHash::Md5).toHex());
    data.append("&newPassword=");
    data.append(QCryptographicHash::hash(new_passwd.toUtf8(), QCryptographicHash::Md5).toHex());
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
        ui->confirm_btn->setEnabled(true);
        if(reply->error() != QNetworkReply::TimeoutError )
        {
            onRegisterReplyFinished(reply);
        }
        else
        {
            SetTipInfo("修改失败，请检测服务器及网络配置");
            return;
        }
    }
    else
    {
        ui->confirm_btn->setEnabled(true);

        disconnect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        reply->abort();
        reply->close();
        reply->deleteLater();
        SetTipInfo("修改失败，请检测服务器及网络配置");
    }
}

void ResetPasswdStackedWidget::SetTipInfo(const QString inputStr)
{
    ui->tipinfo_label->setText(QString("<style>a{color:red; font-size:14px;}</style> <a>%1</a>").arg(inputStr));
}

void ResetPasswdStackedWidget::onRegisterReplyFinished(QNetworkReply *reply)
{
//    ui->register_btn->setEnabled(true);
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
        SetTipInfo("修改失败，请检查服务器及网络配置");
        return;
    }

    QJsonObject rootObj = jsonDoc.object();

    if(!rootObj.contains("code"))
    {
        SetTipInfo("修改失败，原密码错误");
    }
    else if(rootObj.value("code").toString().compare("200") == 0) {
        MsgBoxUtils::NoticeMsgBox("修改成功!", "系统提示",0,this);
        AfterResetPasswd();
    }
    else
    {
        if(rootObj.value("code").toString().compare("605") == 0)
            SetTipInfo("修改失败，原密码和新密码相同");
        else
            SetTipInfo("修改失败，原密码错误");
    }
}

void ResetPasswdStackedWidget::ResetWidget()
{
    ui->old_passwd_lineedit->clear();
    ui->new_password_lineedit->clear();
    ui->confirm_password_lineedit->clear();
    ui->old_passwd_tip_label->clear();
    ui->password_tip_label->clear();
    ui->confirm_password_tip_label->clear();
    SetTipInfo("");
}

void ResetPasswdStackedWidget::AfterResetPasswd()
{
    ResetWidget();
    ConfigManager::Instance()->SetIntConfigItem("login",0);
    SetUnShow();
    emit switchToLogin();
}

void ResetPasswdStackedWidget::OnReturnButtonClicked()
{
    SetUnShow();
    emit switchToAccountWidget();
}

void ResetPasswdStackedWidget::SetShow()
{
    m_is_resetting_passwd = 1;
}

void ResetPasswdStackedWidget::SetUnShow()
{
    m_is_resetting_passwd = 0;
}

int ResetPasswdStackedWidget::IsShow()
{
    return m_is_resetting_passwd;
}
