#include "custominfombox.h"
#include "ui_custominfombox.h"
#include <QGraphicsDropShadowEffect>

CustomInfomBox::CustomInfomBox(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CustomInfomBox)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint|Qt::Tool|Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground,true);

#ifdef USE_UOS
    this->layout()->setMargin(0);
    setAttribute(Qt::WA_TranslucentBackground,true);
    this->setStyleSheet("QWidget#inner_widget{border:1px solid #CCCCCC;border-radius:0px;background-color:rgb(255,255,255);}");
#else
    QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
    shadow_effect->setOffset(0, 0);
    shadow_effect->setColor(QColor(150,150,150));
    shadow_effect->setBlurRadius(6);
    ui->widget->setGraphicsEffect(shadow_effect);
#endif
}

CustomInfomBox::~CustomInfomBox()
{
    delete ui;
}

void CustomInfomBox::set_inform_content(QString inputText)
{
    ui->content_label->setText(inputText);
}
