#ifndef CUSTOMIZE_TABLEWIDGET_H
#define CUSTOMIZE_TABLEWIDGET_H

#include <QTableWidget>
#include <QMouseEvent>
#include <QModelIndex>
#include <QWidget>

class Customize_TableWidget : public QTableWidget
{
    Q_OBJECT
public:
    explicit Customize_TableWidget(QWidget* parent = nullptr);
signals:
    void hoverIndexChanged(QPoint point);
    void loseFocus();

protected:
    void mouseMoveEvent(QMouseEvent* event);
    void focusOutEvent(QFocusEvent* event);

};

#endif // CUSTOMIZE_TABLEWIDGET_H
