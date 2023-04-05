#ifndef ACCOUNTSTACKEDWIDGET_H
#define ACCOUNTSTACKEDWIDGET_H

#include <QWidget>

namespace Ui {
class AccountStackedWidget;
}

class AccountStackedWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AccountStackedWidget(QWidget *parent = 0);
    ~AccountStackedWidget();


private:
    void InitWidget();
    void ConnectSignalToSlot();
    void InitConfig();
    QString GetStatsFileAddr();
    void StartUpdateStatsInfo();

public slots:
    void UpdateShow();

signals:
    void logoutSucceed();
    void toResetPasswd();


private slots:
    void LogoutSlot();
    void ResetPasswdSlot();
    void UpdateStatsInfoSlot();

private:
    Ui::AccountStackedWidget *ui;
    QString m_userAccount;
    QString m_userName;
    int m_inputSpeed;
    int m_totalDays;
    int m_totalChracter;
};

#endif // ACCOUNTSTACKEDWIDGET_H
