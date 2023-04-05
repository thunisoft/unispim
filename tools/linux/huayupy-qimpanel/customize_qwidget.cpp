// 定制化窗口（BaseWindows)

#include "customize_qwidget.h"
#include <QStyleOption>
#include <QPainter>
#include <QBrush>
#include <QDesktopWidget>
#include <QDebug>
#include <QScreen>
#include <QApplication>
#include "../public/config.h"

CustomizeQWidget::CustomizeQWidget(QWidget *parent):
    QWidget(parent),
    m_move_widget_flag(false),
    m_pos_x(0),
    m_pos_y(0)
{
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
                         Qt::WindowDoesNotAcceptFocus | Qt::Tool | Qt::X11BypassWindowManagerHint);
    this->setMouseTracking(true);      //设置为不按下鼠标键触发moveEvent
}

CustomizeQWidget::~CustomizeQWidget()
{
}

void CustomizeQWidget::SetMoveFlag(bool flag)
{
    m_move_widget_flag = flag;
}

void CustomizeQWidget::SetWindowPos(int x, int y)
{
#ifdef USE_UOS
    static double scaled_factor = Config::Instance()->GetScreenScaleFactor();
    qDebug() << "scaled factor" << scaled_factor << x << y;
    m_pos_x = static_cast<double>(x) / scaled_factor;
    m_pos_y = static_cast<double>(y) / scaled_factor;
#else
    m_pos_x = x;
    m_pos_y = y;
#endif
}

int CustomizeQWidget::GetX()
{
    return m_pos_x;
}

int CustomizeQWidget::GetY()
{
    return m_pos_y;
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
    const QPoint position = pos() + event->globalPos() - m_last_mouse_position;
    int desktop_width = QApplication::desktop()->width();
    int right_border = position.x() + this->rect().width();
    if(right_border > desktop_width)
        this->move(desktop_width - this->rect().width(), position.y());
    else
        SoftMove(position.x(), position.y());
    m_last_mouse_position = event->globalPos();
}

void CustomizeQWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_move_widget_flag = false;
    }
}

bool CustomizeQWidget::IsPointInRect(const Rect& rect, const QPoint* p)
{
    if((p->x() >= rect.x) && (p->x() <(rect.x + rect.width)) && (p->y() >= rect.y)&& (p->y() < (rect.y + rect.height)))
        return true;
    return false;
}

bool CustomizeQWidget::IsPointInRect(const int x, const int width, const int y, const int height, const QPoint* p)
{
    if(p->x() >= x && p->x() <x + width && p->y() >= y && p->y() < y + height )
        return true;
    return false;
}

void CustomizeQWidget::SoftMove(int x, int y)
{
    QList<QScreen *> list_screen =  QGuiApplication::screens();  //多显示器
    QScreen* current_screen = NULL;
    int screen_size = list_screen.size();

    QScreen* first_screen = list_screen.at(0);

    QPoint cursor_pos = QCursor::pos();

    if(screen_size == 1)
    {
        current_screen = first_screen;
    }
    else if(screen_size == 2)
    {
        QRect first_rect = list_screen.at(0)->geometry();
        QRect second_rect = list_screen.at(1)->geometry();
        int first_start = first_rect.x();
        int first_stop = first_rect.x() + first_rect.width();

        int second_start = second_rect.x();
        int second_stop = second_rect.x() + second_rect.width();
        int pos_x = cursor_pos.x();

        if(pos_x >= first_start && pos_x <= first_stop)
        {
            current_screen = list_screen.at(0);
        }
        else if(pos_x >= second_start && pos_x <= second_stop)
        {
            current_screen = list_screen.at(1);
        }
    }

    if(current_screen == NULL) return;

    //int current_index = list_screen.indexOf(current_screen);
    //int offset_when_scaled = 0;
    //if(current_index == 1){
    //  offset_when_scaled = qAbs(deskRect_geo.x() - first_screen->geometry().width());
    //  x += offset_when_scaled;
    // }

    QRect current_screen_rect  = current_screen->availableGeometry();
    int current_screen_height = current_screen_rect.height();

    int full_screen_width = QApplication::desktop()->width();

    if((x + this->width()) > full_screen_width)
        x = full_screen_width - this->width();

    if(x < 0)
        x = 0;

    if(y < 0)
        y = 0;

    if(screen_size == 1)
    {
        if(this->height() + y > current_screen_height)
            y =  current_screen_height - this->height();
    }
    this->move(x, y);
}

void CustomizeQWidget::CheckWidgetPos()
{
    const QPoint position = this->pos();
    QDesktopWidget* desktopWidget = QApplication::desktop();
    QRect deskRect = desktopWidget->availableGeometry();
    int end_pos_x = this->rect().width() + position.x();
    if(end_pos_x > deskRect.width())
        this->move(position.x() - (end_pos_x - deskRect.width()), position.y());

}
