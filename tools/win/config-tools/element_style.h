#ifndef ELEMENT_STYLE_H
#define ELEMENT_STYLE_H

#include <QObject>

class ElementStyle : public QObject
{
    Q_OBJECT
public:
    static QString combo_styleSheet;
public:
    explicit ElementStyle(QObject *parent = nullptr);

signals:

};

#endif // ELEMENT_STYLE_H
