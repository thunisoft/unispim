#ifndef SVGUTILS_H
#define SVGUTILS_H

#include <QDomElement>
#include <QString>


class SvgUtils
{
public:
    SvgUtils();

    static void SetSVGBackColor(QDomElement elem, QString strtagname, QString strattr, QString strattrval, int tag_name_index = 0);

};

#endif // SVGUTILS_H
