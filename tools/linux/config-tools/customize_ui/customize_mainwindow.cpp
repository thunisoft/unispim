#include "customize_mainwindow.h"

#include <QStyleOption>
#include <QPainter>
#include <QBrush>
#include <QApplication>

CustomizeMainWindow::CustomizeMainWindow(QWidget* parent):
                                      QMainWindow(parent)
                                     ,m_move_widget_flag(false)
{

    QString styleSheet = "QMainWindow {border: 1px solid rgb(128,128,128);}";
    this->setStyleSheet(styleSheet);
    this->setWindowFlags(Qt::FramelessWindowHint);
}

CustomizeMainWindow::~CustomizeMainWindow()
{
}

void CustomizeMainWindow::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void CustomizeMainWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_last_mouse_position = event->globalPos();
        m_move_widget_flag = true;
    }
}

void CustomizeMainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_move_widget_flag = false;
    }
}

void CustomizeMainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (!event->buttons().testFlag(Qt::LeftButton))
            return;
    if(!m_move_widget_flag)
            return;
    const QPoint position = pos() + event->globalPos() - m_last_mouse_position; //the position of mainfrmae + (current_mouse_position - last_mouse_position)
    move(position.x(), position.y());
    m_last_mouse_position = event->globalPos();
}

