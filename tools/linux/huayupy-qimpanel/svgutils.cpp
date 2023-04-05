#include "svgutils.h"
#include <QDebug>

SvgUtils::SvgUtils()
{

}

void SvgUtils::SetSVGBackColor(QDomElement elem, QString strtagname, QString strattr, QString strattrval, int tag_name_index)
{
    QDomNodeList _list = elem.elementsByTagName(strtagname);
    QDomElement el = _list.at(tag_name_index).toElement();
    QString old_value = el.attribute("fill");
    el.setAttribute(strattr, strattrval);
}
