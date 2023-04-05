#include "checkupdatefaild.h"

#include <QDesktopServices>
#include <QUrl>
#include <QSettings>

#include "../public/configmanager.h"
#include "../public/utils.h"
#include "wordlibpage/myutils.h"

CheckUpdateFaild::CheckUpdateFaild(QWidget *parent):QDialog(parent),
    ui(new Ui::openofficewebsite)
{
    ui->setupUi(this);
    connect(ui->office_website_button,SIGNAL(clicked(bool)), this, SLOT(OnOpenOfficeWebsiteClicked()));
    connect(ui->ok_button, SIGNAL(clicked(bool)), this, SLOT(OnOkButtonClicked()));
}


void CheckUpdateFaild::OnOpenOfficeWebsiteClicked()
{
    QString url = "";
    QString serverAddress = ConfigManager::Instance()->GetServerUrl();
    url = serverAddress.append("/aty/unitest/version/download");

    QSettings config(TOOLS::PathUtils::GetUpdaterIniPath(), QSettings::IniFormat);
    config.beginGroup("package_info");
    QString cpu_type = config.value("cpu_type", "").toString();
    QString os_type = config.value("os_type", "").toString();
    config.endGroup();

    if(true)
    {
        url.append("?os=").append(os_type.toUtf8());
        url.append("&cpu=").append(cpu_type.toUtf8());
    #ifdef USE_IBUS
        url.append("&im=ibus");
    #else
        url.append("&im=fcitx");
    #endif
    }
    QDesktopServices::openUrl(QUrl(url));

    emit SignalCloseCheckFaildDialog();
}

void CheckUpdateFaild::OnOkButtonClicked()
{
    emit SignalCloseCheckFaildDialog();
}

void CheckUpdateFaild::closeEvent(QCloseEvent *event)
{
   Q_UNUSED(event);
   emit SignalCloseCheckFaildDialog();
}
