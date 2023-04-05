#include "toolbarmenuitem.h"
#include "ui_toolbarmenuitem.h"
#include <QEvent>
#include <QHoverEvent>
#include <QDebug>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>

ToolbarMenuItem::ToolbarMenuItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ToolbarMenuItem)
{
    ui->setupUi(this);
}

ToolbarMenuItem::ToolbarMenuItem(QString icon_path, QString text_content, QString expand_icon, QWidget *parent):
    QWidget(parent),
    ui(new Ui::ToolbarMenuItem)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_Hover,true);
    this->setAutoFillBackground(true);
    set_icon(icon_path);
    set_text(text_content);
    set_expand_icon(expand_icon);
}

ToolbarMenuItem::~ToolbarMenuItem()
{
    delete ui;
}

void ToolbarMenuItem::set_icon(QString icon_path)
{
    QPixmap pixmap(icon_path);
    ui->item_icon->setPixmap(pixmap.scaled(16,16));
}

void ToolbarMenuItem::set_text(QString text_content)
{
    ui->item_text->setText(text_content);
}

void ToolbarMenuItem::set_expand_icon(QString icon_path)
{
    if(icon_path.isEmpty())
    {
        ui->item_expand_icon->setVisible(false);
    }
    else
    {

        QPixmap pixmap(icon_path);
        ui->item_expand_icon->setVisible(true);
        ui->item_expand_icon->setPixmap(pixmap.scaled(16,16));
    }
}

void ToolbarMenuItem::set_text_style(QString color_style)
{
    ui->item_text->setStyleSheet(color_style);
}

bool ToolbarMenuItem::event(QEvent *e)
{
    qDebug() << e->type();
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

void ToolbarMenuItem::hoverEnter(QHoverEvent *event)
{
    emit item_hovered_state(1);
    QWidget::event(event);
}

void ToolbarMenuItem::hoverLeave(QHoverEvent *event)
{
    emit item_hovered_state(0);
    QWidget::event(event);
}

void ToolbarMenuItem::hoverMove(QHoverEvent *event)
{
    QWidget::event(event);
}

void ToolbarMenuItem::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
}

void ToolbarMenuItem::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
}

void ToolbarMenuItem::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        emit item_clicked();
    }
}

void ToolbarMenuItem::paintEvent(QPaintEvent *event)
{

    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);//绘制样式

}
