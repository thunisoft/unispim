#include "checkfailedstackedwidget.h"
#include "ui_checkfailedstackedwidget.h"
#include "config.h"
#include <QUrl>
#include "networkhandler.h"

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

    QString officalWeb;
    if(NetworkHandler::Instance()->GetRequesetHeader(officalWeb))
    {
        ui->download_label->setText(QString("<style> a{text-decoration:none}</style><a text-decoration:\"none\"; href=\"%1\">请到官网下载更新包</a>").arg(officalWeb));
    }
    else
    {
        ui->download_label->setText(QString("<style> a{text-decoration:none}</style><a text-decoration:\"none\"; href=\"%1\">请到官网下载更新包</a>").arg("www.thunisoft.com"));
    }

    ui->reupdate_btn->setText("再次更新");

    ui->reupdate_btn->setProperty("type","normal");
    ui->download_label->setProperty("type","normal");
    ui->title_label->setProperty("type","normal");



}

void CheckFailedStackedWidget::ConnectSignalToSlot()
{
    connect(ui->reupdate_btn, SIGNAL(clicked(bool)), this ,SLOT(ReUpdateSlot()));
}

void CheckFailedStackedWidget::ReUpdateSlot()
{
    emit reupdate();
}
