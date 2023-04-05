#ifndef CUSTOMIZEMAINWINDOW_H
#define CUSTOMIZEMAINWINDOW_H

#include <QMouseEvent>
#include <QMainWindow>

class CustomizeMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit CustomizeMainWindow(QWidget *parent = 0);
    ~CustomizeMainWindow();

private:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
private:
    QPoint m_last_mouse_position;

    bool m_move_widget_flag;
};

#endif // CUSTOMIZEMAINWINDOW_H
