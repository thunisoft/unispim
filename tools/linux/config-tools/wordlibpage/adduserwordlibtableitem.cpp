#include "adduserwordlibtableitem.h"
#include "ui_adduserwordlibtableitem.h"

AddUserWordlibTableItem::AddUserWordlibTableItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AddUserWordlibTableItem)
{
    ui->setupUi(this);
    ui->add_wordlib_widget->setProperty("type","add_user_wordlib");
    ui->add_wordlib_widget->setStyleSheet("QToolTip{font-size:14px;}");
    ui->add_wordlib_widget->setCursor(Qt::PointingHandCursor);
    this->setAttribute(Qt::WA_Hover,true);
    ui->add_wordlib_widget->setText("上传词库");

    ui->add_wordlib_widget->setToolTip("上传词库");
    ui->add_wordlib_widget->setIcon(QIcon(QPixmap(":/image/user_wordlib_import.png")));
    connect(ui->add_wordlib_widget, &QPushButton::clicked,this,&AddUserWordlibTableItem::import_user_wordlib_btn_clicked);
}

AddUserWordlibTableItem::~AddUserWordlibTableItem()
{
    delete ui;
}
