#ifndef SYSWORDLIBUPDATEHELPER_H
#define SYSWORDLIBUPDATEHELPER_H

#include <QObject>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>

typedef struct tagUwlInfo
{
    int block_id;
    QString file_name;
    QString md5;
}UwlInfo;


class SyswordlibUpdateHelper : public QObject
{
    Q_OBJECT
public:
    SyswordlibUpdateHelper(QObject* parent = NULL);

    void DownloadIncrement();
    void SetUwlInfo(QMap<int, UwlInfo>* uwl_info);
    void SetDownloadPath(const QString& path);
public slots:
    void OnDownloadDone(QNetworkReply *reply);
    void OnDownloadError(QNetworkReply::NetworkError error_code);
signals:
    void DownloadFinished(int is_success);

private:
    bool MergeSysWordlib();

    QMap<int, UwlInfo>* m_uwl_info;
    QString m_download_path;
    QNetworkAccessManager* m_network_manager;
};

#endif // SYSWORDLIBUPDATEHELPER_H
