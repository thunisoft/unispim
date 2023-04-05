#include "tableviewdelegate.h"
#include <QPainter>

TableViewDelegate::TableViewDelegate(QWidget*parent)
{
    Q_UNUSED(parent);
    m_hover_row_color = QColor(239,239,239);
    m_normal_color = QColor(255,255,255);
}

void TableViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{

    if(index.row() == m_hover_row)
    {
        //HERE IS HOVER COLOR
        painter->fillRect(option.rect, m_hover_row_color);
    }
    else
    {
        painter->fillRect(option.rect, m_hover_row_color);
    }

}

void TableViewDelegate::onHoverRowIndexChanged(const QModelIndex &index)
{
   m_hover_row = index.row();
}
