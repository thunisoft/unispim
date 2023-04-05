#ifndef CUSTOMIZE_QWIDGET_H
#define CUSTOMIZE_QWIDGET_H
#include <QDialog>
#include <QMouseEvent>

typedef struct tagRect {
    int x;
    int y;
    int width;
    int height;
}Rect;

class CustomizeQWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CustomizeQWidget(QWidget *parent = 0);
    ~CustomizeQWidget();

    void SetMoveFlag(bool flag);
    void SetWindowPos(int x, int y);
    int GetX();
    int GetY();
    bool IsPointInRect(const Rect& rect, const QPoint* p);
    bool IsPointInRect(const int x, const int width, const int y, const int height, const QPoint* p);
    void SoftMove(int x, int y);
    void CheckWidgetPos();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    void paintEvent(QPaintEvent *);
private:
    QPoint m_last_mouse_position;

    bool m_move_widget_flag;

    int m_pos_x;
    int m_pos_y;
};
#endif // CUSTOMIZE_QWIDGET_H
