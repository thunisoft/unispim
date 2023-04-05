#ifndef CUSTOMIZESLIDER_H
#define CUSTOMIZESLIDER_H

#include <QSlider>
#include <QMouseEvent>
#include <QWheelEvent>


class CustomizeSlider : public QSlider
{
public:
    CustomizeSlider(QWidget*parent=0);
    ~CustomizeSlider();


protected:
    void mouseReleaseEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void wheelEvent(QWheelEvent *e);
private:
    void Init();
private:
    QString m_style_sheet;
};

#endif // CUSTOMIZESLIDER_H
