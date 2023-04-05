#ifndef ABOUTSTACKEDWIDGET_H
#define ABOUTSTACKEDWIDGET_H

#include <QWidget>


namespace Ui {
class AboutStackedWidget;
}

class CheckUpdateFaild;
class DownloadThread;
class AboutStackedWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AboutStackedWidget(QWidget *parent = 0);
    ~AboutStackedWidget();


signals:
    void checkfailed();
private:
    void InitWidget();
    void ConnectSignalToSlot();

    void SetNormalStatus();

    int  VersionCheck(QString newVersion);

public slots:
    void OnReceiveMessage(int receivedValue);
    void LoadConfig();

    void ResetConfigInfo();
    void SlotPackageCorruptedInform();
private slots:
    void UpdateNowSlot();
    void AutoUpdateCheckStateChangedSlot(int state);
    void PackageDownloadFinished(int returnCode);

    bool IsNeedToRestartThePC();


private:
    Ui::AboutStackedWidget *ui;
    CheckUpdateFaild* m_check_update_faild;
    bool m_has_recive_msg_from_daemon;
    QString m_currentVersionNum;
    QString m_packageSavePath;

    DownloadThread* m_downloadThread;
};

#endif // ABOUTSTACKEDWIDGET_H
