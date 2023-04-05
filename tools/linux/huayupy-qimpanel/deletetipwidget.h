#ifndef DELETETIPWIDGET_H
#define DELETETIPWIDGET_H

#include <QWidget>
#include <QEvent>
#include <QPaintEvent>
#include <QMouseEvent>

namespace Ui {
class DeleteTipWidget;
}

class DeleteTipWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DeleteTipWidget(QWidget *parent = 0);
    ~DeleteTipWidget();

    int ci_index() const;
    void setCi_index(int ci_index);

    void set_content_text(QString text);
signals:
    void remove_user_ci(int ci_index);

protected:
    bool event(QEvent * e);
    //悬浮进入事件
    void hoverEnter(QHoverEvent * event);
    //悬浮退出事件
    void hoverLeave(QHoverEvent * event);
    //悬浮移动事件
    void hoverMove(QHoverEvent * event);
    void paintEvent(QPaintEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

private:
    int m_ci_index;

    Ui::DeleteTipWidget *ui;
};

#endif // DELETETIPWIDGET_H
