#include "customizemessgebox.h"
#include "ui_customizemessgebox.h"

CustomizeMessgeBox::CustomizeMessgeBox(QWidget *parent) :
    CustomizeQWidget(parent),
    ui(new Ui::CustomizeMessgeBox)
{
    ui->setupUi(this);
    InitWidget();
}

CustomizeMessgeBox::~CustomizeMessgeBox()
{
    delete ui;
}

void CustomizeMessgeBox::InitWidget()
{
    ui->closebtn->setProperty("type","closebtn");
    ui->h0_label->setProperty("type","h0");
    ui->ok_btn->setProperty("type","normal");
    ui->cancel_btn->setProperty("type","normal");
    ui->label_2->setProperty("type","h0");
    this->setProperty("type","borderwidget");
    QPixmap pixmap(":/image/logo.png");
    ui->logo_label->setPixmap(pixmap.scaledToWidth(ui->logo_label->width()));

    connect(ui->closebtn, SIGNAL(clicked()),this, SLOT(SlotCloseWindow()));
    connect(ui->ok_btn, SIGNAL(clicked()),this,SLOT(SlotOk()));
    connect(ui->cancel_btn,SIGNAL(clicked()), this, SLOT(SlotCancel()));
}

void CustomizeMessgeBox::SlotCloseWindow()
{
    QDialog::reject();
}
void CustomizeMessgeBox::SlotOk()
{
    QDialog::accept();
}
void CustomizeMessgeBox::SlotCancel()
{
    QDialog::reject();
}
