#ifndef STYLESHEETABLEWIDGET_H
#define STYLESHEETABLEWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QStyleOption>

class StyleSheetableWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StyleSheetableWidget(QWidget *parent = 0);

protected:
    // If you subclass from QWidget, you need to provide a paintEvent for your custom QWidget as below:
    void paintEvent(QPaintEvent *)
    {
        QStyleOption opt;
        opt.init(this);
        QPainter p(this);
        style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    }
};

#endif // STYLESHEETABLEWIDGET_H
