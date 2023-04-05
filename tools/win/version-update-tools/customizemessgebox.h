#ifndef CUSTOMIZEMESSGEBOX_H
#define CUSTOMIZEMESSGEBOX_H

#include <QWidget>
#include "customize_qwidget.h"

namespace Ui {
class CustomizeMessgeBox;
}

class CustomizeMessgeBox : public CustomizeQWidget
{
    Q_OBJECT

public:
    explicit CustomizeMessgeBox(QWidget *parent = nullptr);
    ~CustomizeMessgeBox();

private:
    void InitWidget();
private slots:
    void SlotCloseWindow();
    void SlotOk();
    void SlotCancel();

private:
    Ui::CustomizeMessgeBox *ui;
};

#endif // CUSTOMIZEMESSGEBOX_H
