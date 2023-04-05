#ifndef TABLEVIEWDELEGATE_H
#define TABLEVIEWDELEGATE_H


#include <QItemDelegate>
#include <QPaintEvent>
class TableViewDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    TableViewDelegate(QWidget* parent = 0);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void onHoverRowIndexChanged(const QModelIndex& index);
private:
    int m_hover_row;
    QColor m_hover_row_color;
    QColor m_normal_color;
};

#endif // TABLEVIEWDELEGATE_H
