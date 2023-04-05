#ifndef ABOUTSTACKEDWIDGET_H
#define ABOUTSTACKEDWIDGET_H

#include <QWidget>

namespace Ui {
class AboutStackedWidget;
}

class CheckUpdateFaild;
class NewVersionChecker;
class UpdateWidget;
class AboutStackedWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AboutStackedWidget(QWidget *parent = 0);
    ~AboutStackedWidget();

     static void WriteLog(const QString inputStr);
     void LoadConfigInfo();


signals:
    void checkfailed();
private:
    void InitDbus();
    void InitWidget();
    void ConnectSignalToSlot();
    void InitConfig();
    void SetNormalStatus();
    void DownloadAndInstallNewPackage(QString destUrl,QString file_md5);

public slots:
    void OnReceiveMessage(int receivedValue);
    bool CheckDaemonStatus();
    bool IsDaemonRunning();
    void ResetConfig();
    
private slots:
    void UpdateNowSlot();
    void AutoUpdateCheckStateChangedSlot(int state);

    void OnCheckError(int errorCode);
    void OnCheckSucceed(int succeedCode);

private:
    Ui::AboutStackedWidget *ui;
    CheckUpdateFaild* m_check_update_faild;
    bool m_has_recive_msg_from_daemon;
    NewVersionChecker* m_newversion_checker = NULL;
    UpdateWidget* m_updateWidget = NULL;
};


#endif // ABOUTSTACKEDWIDGET_H
