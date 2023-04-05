#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <QObject>
#include <QNetworkReply>
#include <QProcess>
#include <QThread>

typedef struct tagVERSIONINFO
{
    QString version;
    QString cpu_type;
    QString os_type;
}VERSIONINFO;

enum UpdateTipPosition
{
    CENTER = 0,
    BOTTOM_RIGHT_CORNER
};

class QNetworkAccessManager;
class QNetworkReply;
class QTimer;
class UpdateChecker : public QThread
{
    Q_OBJECT


public:    
    UpdateChecker();
    void WriteLog(const QString inputStr);

    bool IsTimeToCheckUpdate();
protected:
        virtual void run() override;
private slots:
    int Check();
    void CheckError(QNetworkReply::NetworkError error_code);

public slots:
    void DownloadInstallPackage(QString package_addr_url, QString package_md5, QString addrtype);
    void OnDownLoadFinished(QNetworkReply *reply);

private:
    void Init();
    QString GetUrl();
    void GetLocalVersion();
    void AppendParam();

    void SetWorking();
    void SetUnwork();
    bool IsWorking();

    int ParseResult(QNetworkReply* reply);
    void SaveInfo(const QString& path, const QString& version, const QString &addrType);
    void Notify();
    void StartUpdateProcess();
    void RemovePackagePathContent();

    bool m_is_working;
    QTimer* m_timer;
    VERSIONINFO m_local_version;
    QString m_url;
    QString m_package_file_name;
    QString m_package_md5;
    QProcess* m_update_process = nullptr;
    UpdateTipPosition m_update_tip_position = BOTTOM_RIGHT_CORNER;
};

#endif // UPDATECHECKER_H
