#ifndef TOOLBARCONFIGWIDGET_H
#define TOOLBARCONFIGWIDGET_H

#include <QWidget>
#include <QCheckBox>
#include <QShowEvent>
#include <QMap>
#include "customize_qwidget.h"


namespace Ui {
class ToolbarConfigWidget;
}

class ToolbarConfigWidget : public CustomizeQWidget
{
    Q_OBJECT
public:
    explicit ToolbarConfigWidget(QWidget *parent = 0);
    ~ToolbarConfigWidget();
    void LoadConfig();
public:
    void SetShowFlag(bool flag);
    void MoveToNewPos(QPoint clickPoint);

private:
    void InitWidget();

signals:
    void item_changed();

protected:
   void showEvent(QShowEvent* event);

private slots:
    void slot_on_item_checked(int state);
    void slot_on_reset_item();
    void slot_on_login_clicked();
private:
    Ui::ToolbarConfigWidget *ui;
    QMap<QCheckBox*,int> m_checkbox_value_map;
};

#endif // TOOLBARCONFIGWIDGET_H
