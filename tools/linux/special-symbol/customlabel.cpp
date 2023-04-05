#include "customlabel.h"
#include <QMouseEvent>
#include <QApplication>
#include <QDesktopWidget>

MovableLabel::MovableLabel(QWidget* parent):QLabel(parent)
{
}

MovableLabel::~MovableLabel()
{}

bool MovableLabel::event(QEvent *event)
{
    static QPoint lastPoint;
    static bool   pressed = false;
    if (event->type() == QEvent::MouseButtonPress)
    {
        //鼠标左键按下的时候记录一下鼠标位置
        //且标记鼠标已经按下
        QMouseEvent *mouse_event = static_cast<QMouseEvent *>(event);
        if (mouse_event && mouse_event->button() == Qt::LeftButton)
        {
            lastPoint = mouse_event->globalPos();
            pressed = true;
        }
        return true;
    }
    else if (event->type() == QEvent::MouseButtonRelease)
    {
        //鼠标左键抬起的时候标记按下结束
        QMouseEvent *mouse_event = static_cast<QMouseEvent *>(event);
        if (mouse_event && mouse_event->button() == Qt::LeftButton)
        {
            pressed = false;
        }
        return true;
    }
    else if (event->type() == QEvent::MouseMove)
    {
        QMouseEvent *mouse_event = static_cast<QMouseEvent *>(event);
        if (parentWidget() != Q_NULLPTR && pressed)
        {
            //计算鼠标的偏移位置
            QPoint movePos(parentWidget()->x() + (mouse_event->globalX() - lastPoint.x()),
                           parentWidget()->y() + (mouse_event->globalY() - lastPoint.y()));

            //确保软键盘的位置不会超出屏幕
            //要考虑多屏幕的问题
            //多屏幕还没有测试

            parentWidget()->move(movePos);

            lastPoint = mouse_event->globalPos();
        }
        else
        {
            pressed = false;
        }
        return true;
    }
    else if (event->type() == QEvent::Enter)
    {
        //鼠标进入的时候修改鼠标为四个方向的鼠标
        setCursor(QCursor(Qt::SizeAllCursor));
    }
    return QWidget::event(event);
}

PinyinLabel::PinyinLabel(QWidget *parent):QLabel(parent)
{

}

PinyinLabel::~PinyinLabel()
{

}

void PinyinLabel::clear()
{
    QLabel::clear();
    emit content_text_changed("");
}

void PinyinLabel::set_text(QString content)
{
    if (content != QLabel::text()) {
        QLabel::setText(content);
        emit content_text_changed(content);
    }
}
