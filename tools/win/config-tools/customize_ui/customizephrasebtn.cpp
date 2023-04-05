#include "customizephrasebtn.h"
#include "ui_customizephrasebtn.h"

Customizephrasebtn::Customizephrasebtn(QString tip, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Customizephrasebtn)
{
    ui->setupUi(this);
    InitWidget(tip);
}

Customizephrasebtn::~Customizephrasebtn()
{
    delete ui;
}

void Customizephrasebtn::InitWidget(QString tip)
{
   m_rowNum = -1;
   ui->btn->setProperty("type",tip);
   ui->btn->setToolTip(tip);
   connect(ui->btn, SIGNAL(clicked()),this,SLOT(SlotbtnClicked()));
   this->setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::Tool);
}

void Customizephrasebtn::SlotbtnClicked()
{
    emit btnclicked();
}
