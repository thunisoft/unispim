#include "customtabstyle.h"
#include <QStyleOptionTab>

QSize CustomTabStyle::sizeFromContents(ContentsType type, const QStyleOption *option,
    const QSize &size, const QWidget *widget) const
{
    QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
    if (type == QStyle::CT_TabBarTab) {
        s.transpose();
        s.rwidth() = 150; // 设置每个tabBar中item的大小
        s.rheight() = 44;
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
//                painter->setPen(0x89cfff);
//                painter->setBrush(QBrush(0x89cfff));
                painter->setPen(0x4D9CF8);
                painter->setBrush(QBrush(0x4D9CF8));
                painter->drawRect(allRect.adjusted(0, 0, 0, 0));
                painter->restore();                
            }
            else if(tab->state & QStyle::State_MouseOver)
            {
                painter->save();
                painter->setPen(0xE6E6E6);
                painter->setBrush(QBrush(0xE6E6E6));
                painter->drawRect(allRect.adjusted(0, 0, 0, 0));
                painter->restore();
            }
            else
            {
                painter->save();
                painter->setPen(0xF6F6F6);
                painter->setBrush(QBrush(0xF6F6F6));
                painter->drawRect(allRect.adjusted(0, 0, 0, 0));
                painter->restore();
            }

            //设置字的样式
            QTextOption option;
            option.setAlignment(Qt::AlignCenter);
            if (tab->state & QStyle::State_Selected) {
                painter->setPen(0xF8FCFF);
            }
            else {
                painter->setPen(0x5d5d5d);
//                painter->setBrush(QBrush(0xECF0F4));
                painter->setBrush(QBrush(0xFFFFFF));
            }

            painter->drawText(allRect, tab->text, option);
            return;
        }
    }

    if (element == CE_TabBarTab) {
        QProxyStyle::drawControl(element, option, painter, widget);
    }
}
