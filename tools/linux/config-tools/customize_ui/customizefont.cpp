#include "customizefont.h"

CustomizeFont::CustomizeFont(QObject *parent) : QObject(parent)
{

}


QFont CustomizeFont::GetH1Font()
{
    return QFont("微软雅黑",13,420);
}

QFont CustomizeFont::GetTextFont()
{
    return QFont("微软雅黑",11,420);
}

QFont CustomizeFont::GetBigFont()
{
    return QFont("微软雅黑",20,420);
}
