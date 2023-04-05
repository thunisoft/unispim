#ifndef RESETPASSWDSTACKEDWIDGET_H
#define RESETPASSWDSTACKEDWIDGET_H

#include <QWidget>
#include <QKeyEvent>

namespace Ui {
class ResetPasswdStackedWidget;
}

class QNetworkReply;
class ResetPasswdStackedWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ResetPasswdStackedWidget(QWidget *parent = 0);
    ~ResetPasswdStackedWidget();
    int IsShow();
    void SetShow();


private:
    void InitWidget();
    void ConnectSignalToSlot();
    void SetTipInfo(const QString inputStr);
    void SendRequestToSever();
    void ResetWidget();
    void AfterResetPasswd();
    void SetUnShow();

public slots:
    void keyReleaseEvent(QKeyEvent *event);
private slots:
    bool CheckOldPasswdPattern(const QString&  inputStr);
    bool CheckPasswordPattern(QString inputStr);
    bool CheckConfirmPasswordPattern(QString inputStr);

    void onRegisterReplyFinished(QNetworkReply* reply);

    void OnConfirmButtonClicked();
    void OnReturnButtonClicked();


signals:
    void switchToLogin();
    void switchToAccountWidget();

private:
    Ui::ResetPasswdStackedWidget *ui;
    int m_is_resetting_passwd;
};

#endif // RESETPASSWDSTACKEDWIDGET_H
