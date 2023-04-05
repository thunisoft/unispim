#ifndef UPDATEWIDGET_H
#define UPDATEWIDGET_H
#include <QWidget>
#include <QNetworkReply>
#include <QProcess>
#include <string>


enum PackageType
{
   RPM = 0,
   DEB = 1
};

namespace Ui {
class updateWidget;
}

using namespace std;

class QNetworkAccessManager;
class UpdateWidget :public QWidget
{
    Q_OBJECT
public:
    explicit UpdateWidget( QWidget *parent = 0);

    void SetPassword(QString& pwd);
    void StartUpdate();
    void SetDownloadUrl(QString& downloadUrl);
    void SetPackageMd5(const QString& file_md5);

signals:
    void to_exit();
private slots:
    void OnDownLoadFinished(QNetworkReply* reply);
    void LoadProgress(qint64 bytesSent, qint64 bytesTotal);
    void OnReadOutPut();
    void OnReadError();
    void OnConfirmButtonClicked();
    void DownloadPacakge();

    void SlotInstallWhenPackageIsDownloaded();
private:        
    void GetUserHomeLocation();
    void CreateDir(QString &path);
    void DoInstall();
    void RestartInputFrame();
    void InstallDone(bool bSuccess);
    QString GetPackageSavePath();

public:
    bool IsDownLoading();
    void RemovePackagePathContent();
private:

     bool m_has_new_version;
     QString m_download_path;
     QString m_file_name;
     PackageType m_packege_type;
     QString m_new_version;
     QString m_os_type;
     bool m_is_downloading;
     QString m_user_home_location;
     Ui::updateWidget *ui;
     QProcess *cmd;
     bool m_has_finished;
     QString m_pwd;
     bool m_first_write_pwd;
     QNetworkAccessManager *m_accessManager = NULL;
     QString m_package_md5;
};

#endif // UPDATEWIDGET_H
