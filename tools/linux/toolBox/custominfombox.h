#ifndef CUSTOMINFOMBOX_H
#define CUSTOMINFOMBOX_H

#include <QWidget>

namespace Ui {
class CustomInfomBox;
}

class CustomInfomBox : public QWidget
{
    Q_OBJECT

public:
    explicit CustomInfomBox(QWidget *parent = 0);
    ~CustomInfomBox();


public:
    void set_inform_content(QString inputText);
private:
    Ui::CustomInfomBox *ui;
};

#endif // CUSTOMINFOMBOX_H
