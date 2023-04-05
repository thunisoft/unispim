#include "singlekey.h"
#include "ui_singlekey.h"
#include <QSet>

SingleKey::SingleKey(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SingleKey)
{
    ui->setupUi(this);
    InitWidget();
}

SingleKey::~SingleKey()
{
    delete ui;
}

void SingleKey::SetConStr(QString con_str)
{
    ui->con->setText(con_str);
}

void SingleKey::SetVowStr(QSet<QString> vow_str_set)
{
    ui->vow0->clear();
    ui->vow1->clear();
    int vow_size = vow_str_set.size();
    if(vow_size >= 2)
    {
        QList<QString> value_list = vow_str_set.toList();
        ui->vow0->setText(value_list.at(0));
        ui->vow1->setText(value_list.at(1));
    }
    else if(vow_size == 1)
    {
        ui->vow0->setText(*vow_str_set.begin());
    }
}

void SingleKey::SetKeyName(QString key_name)
{
    ui->key->setText(key_name);
}

void SingleKey::InitWidget()
{
    QString styleSheet =
            "QFrame#key_frame"
            "{"
            "border: 1px solid #CCCCCC;"
            "}"
            "QLabel#key"
            "{"
            "color: #999999;"
            "font-size:14px;"
            "margin-left: 1px;"
            "font-weight:400;"
            "padding-bottom: 0px;"
            "font-family:\"Arial\";"
            ""
            "}"
            "QLabel#con"
            "{"
            "font-family:\"Arial\";"
            "color: #FF783C;"
            "font-size:12px;"
            "font-weight:200;"
            "}"
            "QLabel#vow0"
            "{"
            "font-family:\"Arial\";"
            "color: #4D9CF8;"
            "font-weight:200;"
            "font-size:11px;"
            "}"
            "QLabel#vow1"
            "{"
            "font-family:\"Arial\";"
            "color: #4D9CF8;"
            "font-weight:200;"
            "font-size:11px;"
            "}";
    this->setStyleSheet(styleSheet);
}
