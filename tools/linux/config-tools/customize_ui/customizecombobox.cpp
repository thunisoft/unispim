#include "customizecombobox.h"
#include <QListView>

CustomizeCombobox::CustomizeCombobox(QWidget*parent):
    QComboBox(parent)
{
    Init();
}

CustomizeCombobox::~CustomizeCombobox()
{

}

void CustomizeCombobox::Init()
{
     m_style_sheet =
            "QComboBox {"
            "combobox-popup: 0;"
            "border:1px;"
            //"border-color:rgb(128,128,128);"
             "border-color:#cccccc;"
            "border-style:solid;"
            "}"
            "QComboBox:hover{"
            "background-color:white;color: "
            "black;border:1px;"
            "border-color:rgb(77,156,248);"
            "border-style:solid;"
            "}"
            "QComboBox::down-arrow{"
            "width: 16px;"
            "height: 16px;"
            "image: url(:/image/combobox_arrow.png);"
            "}"
            "QComboBox::down-arrow:on{"
            "image: url(:/image/combobox_arrow_clicked.png);"
            "}"
            "QComboBox::drop-down{"
            "subcontrol-origin: padding;"
            "subcontrol-position: right;"
            "width: 20px;"
            "border: 0px;"
            "}"
            "QComboBox QAbstractItemView::item{"
            "height: 30px;"
            "min-height:30px;"
            "color:rgb(0,0,0);"
            "background-color:white;"
            "text-align:center center;"
            "}"
            "QComboBox QAbstractItemView::item:hover{"
            "color:rgb(0,0,0);"
            "background-color:rgb(77,156,248);"
            "}"
            "QComboBox QAbstractScrollArea QScrollBar::vertical{"
            "background-color:white;"
            "width: 10px;"
            "}"
            "QComboBox QAbstractScrollArea QScrollBar::handle::vertical {"
            "width: 10px;"
            "background:rgb(191,191,191);"
            "border-radius:5px;"
            "}"
            "QComboBox QAbstractScrollArea QScrollBar::handle:vertical:hover{"
            "background:rgb(128,128,128);"
            "border-radius:5px;"
            "}"
            "QComboBox QScrollBar::add-line::vertical{"
            "border:none"
            "}"
            "QComboBox QScrollBar::sub-line::vertical{"
            "border:none"
            "}";

    setStyleSheet(m_style_sheet);
    setFixedWidth(120);
    setFixedHeight(26);
    setView(new QListView());
    setAutoFillBackground(true);
    setMaxVisibleItems(10);
}

void CustomizeCombobox::wheelEvent(QWheelEvent *e)
{
    ;
}

void CustomizeCombobox::focusOutEvent(QFocusEvent *e)
{
    (void)e;
    emit focusOut();
}
