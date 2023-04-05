#include "customizelabel.h"

CustomizeLabel::CustomizeLabel(QWidget*parent):
    QLabel(parent)
{
    Init();
}

CustomizeLabel::~CustomizeLabel()
{

}

void CustomizeLabel::Init()
{
    m_style_sheet ="color:rgb(50,50,50);";
    setStyleSheet(m_style_sheet);
}
