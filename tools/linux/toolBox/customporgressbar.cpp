#include "customporgressbar.h"
#include "ui_customporgressbar.h"
#include <QGraphicsDropShadowEffect>

CustomPorgressBar::CustomPorgressBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CustomPorgressBar)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint |Qt::Tool|Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground,true);
    QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
    shadow_effect->setOffset(0, 0);
    shadow_effect->setColor(Qt::black);
    shadow_effect->setBlurRadius(10);
    ui->widget->setGraphicsEffect(shadow_effect);
}

CustomPorgressBar::~CustomPorgressBar()
{
    delete ui;
}
