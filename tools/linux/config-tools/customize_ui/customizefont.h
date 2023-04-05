#ifndef CUSTOMIZEFONT_H
#define CUSTOMIZEFONT_H

#include <QObject>
#include <QFont>

class CustomizeFont : public QObject
{
    Q_OBJECT
public:
    explicit CustomizeFont(QObject *parent = 0);

    static QFont GetH1Font();
    static QFont GetTextFont();
    static QFont GetBigFont();
};

#endif // CUSTOMIZEFONT_H
