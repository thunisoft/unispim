#ifndef NEWENROLLSTACKEDWIDGET_H
#define NEWENROLLSTACKEDWIDGET_H

#include <QWidget>
#include "customize_ui/basestackedwidget.h"
#include <QFutureWatcher>
#include <QElapsedTimer>

namespace Ui {
class NewEnrollStackedWidget;
}

class NewEnrollStackedWidget : public BaseStackedWidget
{
    Q_OBJECT

public:
    explicit NewEnrollStackedWidget(QWidget *parent = nullptr);
    ~NewEnrollStackedWidget();

    void ResetContent();

private:
    void InitWidget();
    void ConnectSignalToSlot();
public:
    void LoadConfigInfo();

signals:
    void loginSucceed();


private slots:
    void SlotRememberPassword(int checkStatus);
    void SlotAutoEnroll(int checkStatus);
    void SlotAccountLogin();
    void SlotAccountRegisterNow();
    void SlotGetVeriCode();
    void SlotPhoneEnroll();
    void SlotPhoneRegisterNow();
    void SlotSwitchToAccount();
    void SlotSwitchToPhone();

    void SlotRefreshGetVeriCodeTime();
    void slot_click_register_btn();
    void slot_register_account();
    void slot_on_account_request_completed();
    void slot_on_vericode_request_completed();
    void slot_on_getvericode_request_completed();
private:
    Ui::NewEnrollStackedWidget *ui;
    int m_vericodeCountDown;
    QTimer* m_countDownTimer;
    QFutureWatcher<int> *m_account_login_watcher;
    QFutureWatcher<int> *m_vericode_login_watcher;
    QFutureWatcher<int> *m_getvericode_watcher;
    QElapsedTimer m_elasted_timer;
};

#endif // NEWENROLLSTACKEDWIDGET_H
