#include "searchfailed.h"
#include "ui_searchfailed.h"
#include <QMovie>


#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

SearchFailed::SearchFailed(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SearchFailed)
{
    ui->setupUi(this);
    InitWidget();
}

SearchFailed::~SearchFailed()
{
    delete ui;
}

void SearchFailed::ReturnBtnClickSlot()
{
    emit returnMainWindow();
}

void SearchFailed::InitWidget()
{
    ui->NofindGifLabel->setProperty("type","bigNotice");
    ui->ReturnBtn->setProperty("type","normal-set-btn");
    ui->ReturnBtn->setIcon(QIcon(":/image/returnArrow.png"));
    ui->ReturnBtn->setCursor(QCursor(Qt::PointingHandCursor));
    ui->SearchResultLabel->setStyleSheet("color:#333333");
    ui->SearchResultLabel->setText("共找到0个结果");
    connect(ui->ReturnBtn, SIGNAL(clicked()), this, SLOT(ReturnBtnClickSlot()));
}
