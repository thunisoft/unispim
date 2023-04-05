#ifndef REGISTERSTACKEDWIDGET_H
#define REGISTERSTACKEDWIDGET_H

#include <QWidget>
#include <QKeyEvent>

namespace Ui {
class RegisterStackedWidget;
}

class QNetworkReply;
class RegisterStackedWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterStackedWidget(QWidget *parent = 0);
    ~RegisterStackedWidget();


private:
    void InitWidget();
    void ConnectSignalToSlot();
    void SetTipInfo(const QString inputStr);
    void SendRequestToSever();
    void ResetWidget();

public slots:
    void keyReleaseEvent(QKeyEvent *ev);
private slots:
    bool CheckAccountPattern(QString inputStr);
    bool CheckPasswordPattern(QString inputStr);
    bool CheckConfirmPasswordPattern(QString inputStr);

    void onRegisterReplyFinished(QNetworkReply* reply);

    void OnRegisterBtnClicked();
    void OnLoginBtnClicked();


signals:
    void RegisterSucceed();
    void switchToLogin();

private:
    Ui::RegisterStackedWidget *ui;
};

#endif // REGISTERSTACKEDWIDGET_H
