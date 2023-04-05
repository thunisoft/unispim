#ifndef PACKAGEDOWNLOADER_H
#define PACKAGEDOWNLOADER_H

#include <QObject>
#include <QString>
#include <QtNetwork/QNetworkReply>

class DownloadDlg;

class PackageDownloader : public QObject
{
    Q_OBJECT
public:
    explicit PackageDownloader(DownloadDlg* const downloadDlg = nullptr, QObject *parent = nullptr);

public:
    void StartDownload(const QString downloadUrl);
    void SetDownloadMode(const QString& download_mode);
    void SetFinishExitSlot();
    QString GetPackageSavePath();

static bool SetSSlConfigToRequest(QNetworkRequest& inputRequest,QString certificatePath);

private slots:
    void LoadProgress(qint64 currentValue ,qint64 totalValue);
    void OnDownLoadFinished(QNetworkReply* reply);
    void ExitSlot();



private:


signals:
    void DownloadFinished();

private:
    QString m_packagePath;
    QString m_downloadUrl;
    DownloadDlg* m_download_dialog; //needn't delete
    QString m_downloadMode;
};

#endif // PACKAGEDOWNLOADER_H
