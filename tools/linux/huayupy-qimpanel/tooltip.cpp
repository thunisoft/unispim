// 自己重新实现的 tooltip 类，避免闪烁

#include "tooltip.h"

#include <QPainter>
#include <QCursor>
#include <QDesktopWidget>
#include <QApplication>
#include "windowconfig.h"

#define TIP_FONT_HEIGHT 15
#define DOUBLE_MARGIN 2


ToolTip* ToolTip::m_tool_tip = NULL;

ToolTip::ToolTip(QWidget *parent):
        CustomizeQWidget(parent)
       ,m_font(NULL)
{
    InitFont();
}

ToolTip* ToolTip::Instance()
{
    if(!m_tool_tip)
        m_tool_tip = new ToolTip();

    return m_tool_tip;
}

void ToolTip::ShowTip(const QString& tip, int parent_bottom_, int parent_top_)
{
    m_tip_text = tip;
    CalWindowSize(tip, m_font);
    this->resize(QSize(m_text_width + DOUBLE_MARGIN, m_text_height + DOUBLE_MARGIN));
    this->show();

    QPoint p(QCursor::pos());
    this->Move(parent_bottom_ , parent_top_);
}

void ToolTip::Hide()
{
    hide();
}

void ToolTip::CalWindowSize(const QString& tip, const QFont* font)
{
    QFontMetrics font_metrics(*font);
    m_text_width = font_metrics.width(tip);
    m_text_height = font_metrics.height();
}

void ToolTip::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;

    p.setPen(QColor(0, 0, 0));
    p.setBrush(QColor(0xff, 0xff, 0xff, 0xff));
    p.fillRect(0, 0, m_text_width + DOUBLE_MARGIN, m_text_height + DOUBLE_MARGIN, p.brush());

    WindowConfig* WC = WindowConfig::Instance();
    m_font->setFamily(WC->chinese_font_name);

    p.setFont(*m_font);
    QRect _rect(QPoint(DOUBLE_MARGIN / 2, DOUBLE_MARGIN / 2), QSize(m_text_width, m_text_height));
    p.drawText(_rect, Qt::AlignCenter, m_tip_text);
}

void ToolTip::InitFont()
{
    if(m_font)
        return;
    m_font = new QFont();
   WindowConfig* WC = WindowConfig::Instance();
    m_font->setPixelSize(TIP_FONT_HEIGHT);
    m_font->setFamily(WC->chinese_font_name);
}

void ToolTip::Move(int bottom_limit, int top_limit)
{
    QDesktopWidget* desktopWidget = QApplication::desktop();
    QRect deskRect =desktopWidget->geometry();   //获取可用桌面大小
    int x = QCursor::pos().x();
    if(x < deskRect.x())
        x = deskRect.x();
    if(this->rect().width() + x > deskRect.width())
        x = deskRect.width() - this->rect().width();
    int y;
    if(this->rect().height() + bottom_limit <= deskRect.height())
        y = bottom_limit;
    else
        y =  top_limit - this->height();
    this->move(x, y);
}
