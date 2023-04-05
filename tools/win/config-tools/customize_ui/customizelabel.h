#ifndef CUSTOMIZELABEL_H
#define CUSTOMIZELABEL_H
#include <QLabel>


class CustomizeLabel :public QLabel
{
public:
    CustomizeLabel(QWidget*parent=0);
    ~CustomizeLabel();
public:
    void Init();
    QString m_style_sheet;
};

#endif // CUSTOMIZELABEL_H
