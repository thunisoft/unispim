#include "customtooltip.h"
#include "ui_customtooltip.h"
#include <QGraphicsDropShadowEffect>
#include <QTimer>

CustomTooltip::CustomTooltip(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CustomTooltip)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint|Qt::Tool|Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground,true);

#ifdef USE_UOS
    this->layout()->setMargin(0);
    this->setStyleSheet("QWidget#inner_widget{border:1px solid #CCCCCC;border-radius:0px;background-color:rgb(255,255,255);}");
#else
    this->layout()->setMargin(6);
    QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
    shadow_effect->setOffset(0, 0);
    shadow_effect->setColor(QColor(150,150,150));
    shadow_effect->setBlurRadius(6);
    ui->inner_widget->setGraphicsEffect(shadow_effect);
#endif
}

CustomTooltip::~CustomTooltip()
{
    delete ui;
}

void CustomTooltip::show_text(QString content, QPoint show_pos)
{
    ui->label->setText(content);
    this->move(show_pos);
    this->show();
}

void CustomTooltip::hide_for_seconds(int hide_ms_value)
{
    this->hide();
    QTimer *m_time = new QTimer();
    m_time->setSingleShot(true);
    m_time->start(hide_ms_value);
    QObject::connect(m_time, &QTimer::timeout, [=](){
        this->show();
    });
}

