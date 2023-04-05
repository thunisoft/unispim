#include "accountstackedwidget.h"
#include "ui_accountstackedwidget.h"
#include "customize_ui/customizefont.h"
#include "../public/configmanager.h"
#include "../public/dbusmessager.h"

#include <QDir>
#include <QTimer>
#include <QSettings>
#include <QIcon>

AccountStackedWidget::AccountStackedWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AccountStackedWidget)
{
    ui->setupUi(this);
    InitWidget();
    InitConfig();
    ConnectSignalToSlot();
    StartUpdateStatsInfo();
}

AccountStackedWidget::~AccountStackedWidget()
{
    delete ui;
}

void AccountStackedWidget::InitWidget()
{
    QString logoSheet = "QPushButton{"
                        "background-image:url(:/image/logined_boy.png);"
                        "background-repeat: no-repeat;"
                        "background-position:center;"
                        "}"
                        "QPushButton:pressed{"
                        "border:none;"
                        "}";
    ui->user_logo_btn->setStyleSheet(logoSheet);
    ui->user_logo_btn->setFlat(true);
    ui->user_logo_btn->setFocusPolicy(Qt::NoFocus);


    QString frameSheet =" QFrame { "
                        " border : 2px solid rgb(77,156,248);"
                        " padding: 2px 4px;"
                        " width: 75px;"
                        " height:25px;"
                        " border-radius:13px;"
                        " color:white;"
                        " background:rgb(77,156,248);"
                        " } ";

    QString labelSheet = "QLabel{"
                         "color:white"
                         "background:transparent;"
                         "}";

    ui->logout_btn->setText("退出登录");
    ui->logout_btn->setFocusPolicy(Qt::NoFocus);

    ui->reset_passwd_btn->setText("修改密码");
    ui->reset_passwd_btn->setFlat(true);
    ui->reset_passwd_btn->setFocusPolicy(Qt::NoFocus);
    ui->reset_passwd_btn->setStyleSheet("QPushButton{border:none; color:rgb(77,156,248);}QPushButton:pressed{border:none;}"
                                  "QPushButton:hover{color:rgb(85,118,189);}");

    ui->input_speed_frame->setStyleSheet(frameSheet);
    ui->total_days_frame->setStyleSheet(frameSheet);
    ui->total_character_frame->setStyleSheet(frameSheet);
    QPixmap speedPixmap(":/image/speed.png");
    ui->inputspeed_logo_label->setPixmap(speedPixmap);
    QPixmap total_days_pixmap(":/image/calendar.png");
    ui->total_days_logo_label->setPixmap(total_days_pixmap);
    QPixmap total_input_pixmap(":/image/keyboard.png");
    ui->total_character_logo_label->setPixmap(total_input_pixmap);
    ui->input_speed_unit_label->setText("输入速度/分");
    ui->total_character_unit_label->setText("累计打字/字");
    ui->total_days_unit_label->setText("累计使用/天");
    ui->input_speed_unit_label->setStyleSheet(labelSheet);
    ui->total_days_unit_label->setStyleSheet(labelSheet);
    ui->total_character_unit_label->setStyleSheet(labelSheet);

    ui->input_speed_label->setStyleSheet(labelSheet);
    ui->total_days_label->setStyleSheet(labelSheet);
    ui->total_character_label->setStyleSheet(labelSheet);

    ui->user_account_label->setProperty("type", "h2");
    QString loginid;
    if(ConfigManager::Instance()->GetStrConfigItem("loginid",loginid))
    {
        ui->user_account_label->setText(loginid);
    }

    ui->logout_btn->setProperty("type","radiusBtn");
    ui->logout_btn->setFlat(true);
    ui->logout_btn->setFocusPolicy(Qt::NoFocus);


}

void AccountStackedWidget::ConnectSignalToSlot()
{
    connect(ui->logout_btn, SIGNAL(clicked(bool)), this ,SLOT(LogoutSlot()));
    connect(ui->reset_passwd_btn, SIGNAL(clicked(bool)), this ,SLOT(ResetPasswdSlot()));
}

void AccountStackedWidget::InitConfig()
{
    int islogin;
    if(ConfigManager::Instance()->GetIntConfigItem("login",islogin))
    {
        if(islogin)
        {
            QString loginid;
            ConfigManager::Instance()->GetStrConfigItem("loginid",loginid);
            ui->user_account_label->setText(loginid);
        }
    }

}

void AccountStackedWidget::LogoutSlot()
{
    ConfigManager::Instance()->SetIntConfigItem("login",0);
    ConfigManager::Instance()->ReloadUserConfig();

    ConfigManager::Instance()->SenddbusMsg("logout","");
    DbusMessager::SendQimpanelIntChange("ReloadConfig",1);
    emit logoutSucceed();
}

void AccountStackedWidget::ResetPasswdSlot()
{
    emit toResetPasswd();
}

void AccountStackedWidget::UpdateShow()
{
    QString loginid;
    if(ConfigManager::Instance()->GetStrConfigItem("loginid",loginid))
    {
        ui->user_account_label->setText(loginid);
    }
}

QString AccountStackedWidget::GetStatsFileAddr()
{
    int isLogin;
    if(ConfigManager::Instance()->GetIntConfigItem("loginid",isLogin))
    {
        if(isLogin)
        {
            QString loginid;
            if(ConfigManager::Instance()->GetStrConfigItem("loginid",loginid))
            {
                QString  statsFileAddr = QDir::homePath() + QString("/.config/huayupy/huayupy-stats-%1").arg(loginid);
                return statsFileAddr;
            }

        }
    }

    return "";
}

void AccountStackedWidget::StartUpdateStatsInfo()
{
    QTimer* statsTimer = new QTimer(this);
    connect(statsTimer, SIGNAL(timeout()), this, SLOT(UpdateStatsInfoSlot()));
    statsTimer->start(2000);
}

void AccountStackedWidget::UpdateStatsInfoSlot()
{
    QString filePath = GetStatsFileAddr();
    if(filePath.isEmpty())
    {
        return;
    }

    QSettings inputSet(GetStatsFileAddr(),QSettings::IniFormat);
    inputSet.beginGroup("BasicInfo");
    ui->total_character_label->setText(inputSet.value("total_num","0").toString());
    ui->input_speed_label->setText(inputSet.value("current_speed","0").toString());
    inputSet.endGroup();
    inputSet.beginGroup("InputInfo");
    QStringList childKeys = inputSet.childKeys();
    ui->total_days_label->setText(QString::number(childKeys.size()));
    inputSet.endGroup();
}
