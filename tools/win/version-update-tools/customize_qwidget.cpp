#include "customize_qwidget.h"
#include <QStyleOption>
#include <QPainter>
#include <QBrush>

CustomizeQWidget::CustomizeQWidget(QWidget *parent):
                                    QDialog(parent),
                          m_move_widget_flag(false)
{
    this->setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::Dialog | Qt::WindowStaysOnTopHint);
}

CustomizeQWidget::~CustomizeQWidget()
{
}

void CustomizeQWidget::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void CustomizeQWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_last_mouse_position = event->globalPos();
        m_move_widget_flag = true;
    }
}

void CustomizeQWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!event->buttons().testFlag(Qt::LeftButton) || !m_move_widget_flag)
            return;
    const QPoint position = pos() + event->globalPos() - m_last_mouse_position; //the position of mainfrmae + (current_mouse_position - last_mouse_position)
    move(position.x(), position.y());
    m_last_mouse_position = event->globalPos();
}

void CustomizeQWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_move_widget_flag = false;
    }
}
