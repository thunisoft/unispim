#ifndef CUSTOMTOOLTIP_H
#define CUSTOMTOOLTIP_H

#include <QWidget>

namespace Ui {
class CustomTooltip;
}

class CustomTooltip : public QWidget
{
    Q_OBJECT

public:
    explicit CustomTooltip(QWidget *parent = 0);
    ~CustomTooltip();
public:
    void show_text(QString content,QPoint show_pos);
    void hide_for_seconds(int hide_ms_value);
private:
    Ui::CustomTooltip *ui;
};

#endif // CUSTOMTOOLTIP_H
