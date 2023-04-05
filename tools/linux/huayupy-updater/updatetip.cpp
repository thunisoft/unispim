#include "updatetip.h"
#include "../public/inisetting.h"
#include "../public/config.h"
#include "../public/utils.h"
#include <QSettings>
#include <QDate>
#include <QPushButton>

UpdateTip::UpdateTip(QWidget *parent, UpdateTipPosition position):QWidget(parent),
     ui(new Ui::UpdateTip)
{
    ui->setupUi(this);

    QVector<QPushButton*> button_vector;
    button_vector << ui->not_display_button << ui->update_button << ui->cancel_button;
    for(QPushButton*index : button_vector)
    {
        index->setProperty("type","normal");
        index->setCursor(Qt::PointingHandCursor);
        index->setFocusPolicy(Qt::NoFocus);

    }


    QString style_sheet = ""
                          " QPushButton { "
                          " image : url(:/image/close_white.png); "
                          " border : 0px solid;"
                          " width: 25px;"
                          " height:25px;"
                          " } ";
    ui->close_button->setFlat(true);
    ui->close_button->setStyleSheet(style_sheet);
    ui->close_button->setFocusPolicy(Qt::NoFocus);
    ui->close_button->setToolTip("关闭");
    ui->close_button->setCursor(Qt::PointingHandCursor);

    connect(ui->cancel_button,&QPushButton::clicked,this, &UpdateTip::OnCancelButtonClicked);
    connect(ui->update_button, SIGNAL(clicked(bool)), this, SLOT(OnUpdateButtonClicked()));
    connect(ui->not_display_button,&QPushButton::clicked,this,&UpdateTip::OnNotDisplayRecentClicked);
    connect(ui->close_button,&QPushButton::clicked,this, &UpdateTip::OnCancelButtonClicked);

}

void UpdateTip::OnCancelButtonClicked()
{
    QSettings setting(Config::GetSaveNewVersionInfoPath(), QSettings::IniFormat);
    setting.beginGroup("next_check_time");
    setting.setValue("date",QDate::currentDate().addDays(1).toString(Qt::ISODate));
    setting.endGroup();

    emit SignalCancel();
}

void UpdateTip::OnUpdateButtonClicked()
{
    emit SignalUpdate();
}

void UpdateTip::OnNotDisplayRecentClicked()
{
    QSettings setting(Config::GetSaveNewVersionInfoPath(), QSettings::IniFormat);
    setting.beginGroup("next_check_time");
    setting.setValue("date",QDate::currentDate().addDays(7).toString(Qt::ISODate));
    setting.endGroup();
    emit SignalCancel();
}

void UpdateTip::OnThisVersionNotNotify()
{
    QSettings settings(Utils::GetSaveNewVersionInfoPath(), QSettings::IniFormat);
    settings.beginGroup("new_version_info");
    QString version_str = settings.value("version","").toString();
    settings.endGroup();
    if(!version_str.isEmpty())
    {
        settings.beginGroup("next_check_time");
        settings.setValue("notcheckversion",version_str);
        settings.endGroup();
    }
    emit SignalCancel();
}
