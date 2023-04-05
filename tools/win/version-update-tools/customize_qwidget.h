#ifndef CUSTOMIZE_QWIDGET_H
#define CUSTOMIZE_QWIDGET_H
#include <QDialog>
#include <QMouseEvent>

class CustomizeQWidget : public QDialog
{
    Q_OBJECT
public:
    explicit CustomizeQWidget(QWidget *parent = 0);
    ~CustomizeQWidget();
private:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
private:
    QPoint m_last_mouse_position;

    bool m_move_widget_flag;
};
#endif // CUSTOMIZE_QWIDGET_H
