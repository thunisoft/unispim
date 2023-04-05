#ifndef UPDATETIP_H
#define UPDATETIP_H
#include <QWidget>
#include "ui_updatetip.h"
#include "define.h"

class UpdateTip : public QWidget
{
    Q_OBJECT
public:
    explicit UpdateTip(QWidget *parent = 0, UpdateTipPosition position = CENTER);
signals:
    void SignalCancel();
    void SignalUpdate();
private:
    Ui::UpdateTip *ui;
public slots:
    void OnCancelButtonClicked();
    void OnUpdateButtonClicked();
    void OnNotDisplayRecentClicked();
    void OnThisVersionNotNotify();
};

#endif // UPDATETIP_H
