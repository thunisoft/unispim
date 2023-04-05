#ifndef CUSTOMPORGRESSBAR_H
#define CUSTOMPORGRESSBAR_H

#include <QWidget>

namespace Ui {
class CustomPorgressBar;
}

class CustomPorgressBar : public QWidget
{
    Q_OBJECT

public:
    explicit CustomPorgressBar(QWidget *parent = 0);
    ~CustomPorgressBar();

private:
    Ui::CustomPorgressBar *ui;
};

#endif // CUSTOMPORGRESSBAR_H
