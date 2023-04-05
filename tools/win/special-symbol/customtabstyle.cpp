#include "customtabstyle.h"
#include <QStyleOptionTab>

QSize CustomTabStyle::sizeFromContents(ContentsType type, const QStyleOption *option,
    const QSize &size, const QWidget *widget) const
{
    QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
    if (type == QStyle::CT_TabBarTab) {
        s.transpose();
        s.rwidth() = 130; // 设置每个tabBar中item的大小
        s.rheight() = 34;
    }
    return s;
}

void CustomTabStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (element == CE_TabBarTabLabel) {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            QRect allRect = tab->rect;

            if (tab->state & QStyle::State_Selected) {
                painter->save();
                painter->setPen(0x4d9cf8);
                painter->setBrush(QBrush(0x4d9cf8));
                painter->drawRect(allRect.adjusted(0, 0, 0, 0));
                painter->restore();
            }
            QTextOption option;
            option.setAlignment(Qt::AlignCenter);
            if (tab->state & QStyle::State_Selected) {
                painter->setPen(0xf8fcff);
            }
            else {
               // painter->setPen(0x5d5d5d);
                painter->setPen(QColor("#333333"));
                painter->setBrush(QBrush(0xECF0F4));
            }
            QFont font("Microsoft YaHei");
            font.setPixelSize(14);
            painter->setFont(font);
            painter->drawText(allRect, tab->text, option);
            return;
        }
    }

    if (element == CE_TabBarTab) {
        QProxyStyle::drawControl(element, option, painter, widget);
    }
}
