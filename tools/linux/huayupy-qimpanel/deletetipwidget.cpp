#include "deletetipwidget.h"
#include "ui_deletetipwidget.h"
#include <QHoverEvent>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>

DeleteTipWidget::DeleteTipWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DeleteTipWidget)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
                         Qt::WindowDoesNotAcceptFocus | Qt::Tool | Qt::X11BypassWindowManagerHint | Qt::NoDropShadowWindowHint);
    this->setAttribute(Qt::WA_Hover);
    ui->label->setProperty("type","delete_btn");
    QGraphicsDropShadowEffect* shadow_corder_effect = new QGraphicsDropShadowEffect;
    shadow_corder_effect->setOffset(1,1);
    shadow_corder_effect->setColor(QColor(0,0,0,15));
    shadow_corder_effect->setBlurRadius(2);
    this->setGraphicsEffect(shadow_corder_effect);
    setAttribute(Qt::WA_TranslucentBackground,true);

}

DeleteTipWidget::~DeleteTipWidget()
{
    delete ui;
}



bool DeleteTipWidget::event(QEvent *e)
{
    switch(e->type())
    {
    case QEvent::HoverEnter:
        hoverEnter(static_cast<QHoverEvent*>(e));
        return true;
    case QEvent::HoverLeave:
        hoverLeave(static_cast<QHoverEvent*>(e));
        return true;
    case QEvent::HoverMove:
        hoverMove(static_cast<QHoverEvent*>(e));
        return true;
    default:
        break;
    }
    return QWidget::event(e);
}

void DeleteTipWidget::hoverEnter(QHoverEvent *event)
{
    this->setCursor(Qt::PointingHandCursor);
    this->setStyleSheet(QString("DeleteTipWidget{background-color:#E7EDF1;color:#000000;font-size:14px;border:1px solid #CCCCCC}"));
    QWidget::event(event);
}

void DeleteTipWidget::hoverLeave(QHoverEvent *event)
{
    this->setVisible(false);
    this->setStyleSheet(QString("DeleteTipWidget{background-color:#F8FCFF;color:#000000;font-size:14px;border:1px solid #CCCCCC}"));
    QWidget::event(event);
}

void DeleteTipWidget::hoverMove(QHoverEvent *event)
{
    QWidget::event(event);
}

void DeleteTipWidget::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void DeleteTipWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        emit remove_user_ci(m_ci_index);
    }
}

int DeleteTipWidget::ci_index() const
{
    return m_ci_index;
}

void DeleteTipWidget::setCi_index(int ci_index)
{
    m_ci_index = ci_index;
}

void DeleteTipWidget::set_content_text(QString text)
{
    ui->label->setText(text);
}
