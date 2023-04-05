#include "customizeslider.h"

#include <QMouseEvent>
#include <QDebug>

CustomizeSlider::CustomizeSlider(QWidget*parent):
    QSlider(parent)
{
    Init();
}


CustomizeSlider::~CustomizeSlider()
{

}

void CustomizeSlider::Init()
{
    m_style_sheet =
                "QSlider::groove:horizontal{"
                "height:8px;"
                "background:rgb(255,255,255);"
                "border: 2px solid rgb(77,156,248);"
                "border-radius:4px;"
                "padding-left:-1px;"
                "padding-right:-1px;"
                "margin-left:0px;"
                "margin-right:0px;"
                "}"
                "QSlider::handle:horizontal{"
                "background:rgb(255,255,255);"
                "border: 2px solid rgb(77,156,248);"
                "width:14px;"
                "border-radius:7px;"
                "margin-top:-5px;"
                "margin-bottom:-5px;"
                "}"
                "QSlider::add-page:horizontal{"
                "background:rgb(191,191,191);"
                "border: 2px solid rgb(191,191,191);"
                "height:10px;"
                "border-radius:5px;"
                "}"
                "QSlider::sub-page:horizontal{"
                "background:rgb(77,156,248);"
                "border: 2px solid rgb(77,156,248);"
                "height:10px;"
                "border-radius:5px;"
                "}";

    setStyleSheet(m_style_sheet);
    setFocusPolicy(Qt::NoFocus);
}

void CustomizeSlider::mouseReleaseEvent(QMouseEvent *ev)
{
    if(ev->button() == Qt::LeftButton)
    {
        int dur = this->maximum() - this->minimum();
        int pos = this->minimum() + (int)(dur * ((double)ev->x() / (double)this->width()) + 0.5f);
        if(pos != sliderPosition())
        {
            this->setValue(pos);
        }
    }
}

void CustomizeSlider::mouseMoveEvent(QMouseEvent *ev)
{

    int dur = this->maximum() - this->minimum();
    int pos = this->minimum() + (int)(dur * ((double)ev->x() / (double)this->width()) + 0.5f);
    if(pos != sliderPosition())
    {
        this->setValue(pos);
    }
}

void CustomizeSlider::mousePressEvent(QMouseEvent *ev)
{
    ;
}

void CustomizeSlider::wheelEvent(QWheelEvent *e)
{
    ;
}
