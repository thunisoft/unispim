#include "checkfailedstackedwidget.h"
#include "ui_checkfailedstackedwidget.h"
#include "customize_ui/customizefont.h"
#include "../public/configmanager.h"
#include "wordlibpage/myutils.h"
#include <QUrl>
#include <QSettings>

CheckFailedStackedWidget::CheckFailedStackedWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CheckFailedStackedWidget)
{
    ui->setupUi(this);
    InitWidget();
    ConnectSignalToSlot();
}

CheckFailedStackedWidget::~CheckFailedStackedWidget()
{
    delete ui;
}

void CheckFailedStackedWidget::InitWidget()
{
    QString labelSheet = "background:transparent;"
                         "color:rgb(254,185,107);"
                         "font-weight: 400;"
                         "font-size: 28px";

    QString linkLabelSheet = "background:transparent;"
                         "color:rgb(77,156,248);"
                         "font-weight: 400;"
                         "font-size: 16px";
    QString backwidget  =
                         "background-image:url(:/image/checkfailed_bgimg.png);"
                         "background-repeat: no-repeat;"
                         "background-attachment: fixed;"
                         "background-position: center";

    ui->background_widget->setStyleSheet(backwidget);
    ui->title_label->setStyleSheet(labelSheet);
    ui->title_label->setText("检测失败");

    ui->download_label->setStyleSheet(linkLabelSheet);
    ui->download_label->setOpenExternalLinks(true);
    QString serverAddress = ConfigManager::Instance()->GetServerUrl();
    QString url = serverAddress.append("/aty/huayupy/version/download");


    QSettings inputMethodInfo(TOOLS::PathUtils::GetUpdaterIniPath(), QSettings::IniFormat);
    QString os_type = inputMethodInfo.value("package_info/os_type", "").toString();
    QString cpu_type = inputMethodInfo.value("package_info/cpu_type", "").toString();


    url.append("?os=").append(os_type.toUtf8());
    url.append("&cpu=").append(cpu_type.toUtf8());
#ifdef USE_IBUS
    url.append("&im=ibus");
#else
    url.append("&im=fcitx");
#endif

    ui->download_label->setText(QString("<style> a{text-decoration:none}</style><a text-decoration:\"none\"; href=\"%1\">请到官网下载更新包</a>").arg(url));

    ui->reupdate_btn->setProperty("type","radiusBtn");
    ui->reupdate_btn->setFlat(true);
    ui->reupdate_btn->setFocusPolicy(Qt::NoFocus);

    ui->reupdate_btn->setText("再次更新");


}

void CheckFailedStackedWidget::ConnectSignalToSlot()
{
    connect(ui->reupdate_btn, SIGNAL(clicked(bool)), this ,SLOT(ReUpdateSlot()));
}

void CheckFailedStackedWidget::ReUpdateSlot()
{
    emit reupdate();
}
