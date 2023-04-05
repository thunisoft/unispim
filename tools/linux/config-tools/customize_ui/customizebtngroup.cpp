#include "customizebtngroup.h"
#include "ui_customizebtngroup.h"

CustomizeBtnGroup::CustomizeBtnGroup(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CustomizeBtnGroup)
{
    ui->setupUi(this);
    InitWidget();
}

CustomizeBtnGroup::~CustomizeBtnGroup()
{
    delete ui;
}

void CustomizeBtnGroup::InitWidget()
{
   m_rowNum = -1;
   ui->delete_btn->setProperty("type","btngroup");
   ui->delete_btn->setToolTip("删除");
   connect(ui->delete_btn, SIGNAL(clicked()),this,SLOT(SlotDeletebtnClicked()));
   this->setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::Tool);
}

void CustomizeBtnGroup::SlotDeletebtnClicked()
{
    emit deleteclicked();
}





















