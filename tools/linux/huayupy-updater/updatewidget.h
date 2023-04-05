#ifndef UPDATEWIDGET_H
#define UPDATEWIDGET_H
#include <QWidget>
#include <QNetworkReply>
#include <QProcess>
#include "tabwidgetbase.h"
#include <string>
#include "define.h"
#include "ui_updatewidget.h"
using namespace std;

class UpdateWidget :public QWidget
{
    Q_OBJECT
public:
    explicit UpdateWidget(QWidget *parent = 0);

    void SetPassword(QString& pwd);

signals:
   // void CheckUpateButtonClicked();
    void to_exit();
private slots:
    void OnDownLoadFinished(QNetworkReply* reply);
    void LoadProgress(qint64 bytesSent, qint64 bytesTotal);
    void OnReadOutPut();
    void OnReadError();
    void OnConfirmButtonClicked();
    void DownloadPacakge();
    void InstallTimeout();

    void SlotInstallWhenPackageIsDownloaded();
private:
    bool GetNewVersionAndPathAndOs(QString& new_version, QString& new_version_down_path, QString& os_type,QString& addr_type);
    int FileExists(const string& filepath);    
    void GetUserHomeLocation();
    void CreateDir(QString &path);
    bool CheckHasInstalledGnomeTerminal();
    void DoInstall();
    void RestartInputFrame();
    void InstallDone(bool bSuccess);
    QString GetPackageSavePath();

public:
    bool IsDownLoading();
private:

     bool m_has_new_version;
     QString m_download_path;
     QString m_file_name;
     PackageType m_packege_type;
     QString m_new_version;
     QString m_os_type;
     QString m_addr_type;
     bool m_is_downloading;
     QString m_user_home_location;
     Ui::updateWidget *ui;
     QProcess *cmd;
     bool m_has_finished;
     QString m_pwd;
     bool m_first_write_pwd;
};

#endif // UPDATEWIDGET_H
