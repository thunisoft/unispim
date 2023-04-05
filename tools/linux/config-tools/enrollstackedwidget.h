#ifndef ENROLLSTACKEDWIDGET_H
#define ENROLLSTACKEDWIDGET_H

#include <QWidget>
#include <QMap>
#include <QKeyEvent>
#include <QtConcurrent>
#include <QFutureWatcher>
#include "commondef.h"
#include "verififycodethread.h"

namespace Ui {
class EnrollStackedWidget;
}

class QNetworkReply;
class EnrollStackedWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EnrollStackedWidget(QWidget *parent = 0);
    ~EnrollStackedWidget();


public slots:
    void keyReleaseEvent(QKeyEvent *ev);
    void ResetWidget();

private slots:
    void OnLoginBtnClickedSlot();
    void OnToRegisterBtnClickedSlot();
    bool CheckAccountPattern(QString inputStr);
    bool CheckPasswordPattern(QString inputStr);
    void OpenWebSiteLink();
    void ShowTheForgetPasswordBtn();
    void SlotLoginprocessFinished();

signals:
    void loginSucceeded();
    void switchToRegister();



private:
    void InitWidget();
    void ConnectSignalToSlot();
    int SendRequestToServer(QString account,QString password);
    void ShowtoolTip(const QString inputStr);


private:
    Ui::EnrollStackedWidget *ui;
    VerifyCodeCheckThread* m_vericodeCheckThread;
    QFutureWatcher<int>* m_login_future_watcher;
    QString m_username;
    QString m_loginid;
    QString m_clientid;
    QString m_loginErrMsg;
};

#endif // ENROLLSTACKEDWIDGET_H
