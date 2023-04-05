#include "customize_tablewidget.h"

Customize_TableWidget::Customize_TableWidget(QWidget* parent):QTableWidget(parent)
{
    this->setMouseTracking(true);

}

void Customize_TableWidget::mouseMoveEvent(QMouseEvent* event)
{
    QPoint currentPoint = event->pos();

    emit hoverIndexChanged(currentPoint);
    event->accept();
}


void Customize_TableWidget::focusOutEvent(QFocusEvent* event)
{
    emit loseFocus();
    event->accept();

}

