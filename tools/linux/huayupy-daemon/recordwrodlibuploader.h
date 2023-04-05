#ifndef RECORDWRODLIBUPLOADER_H
#define RECORDWRODLIBUPLOADER_H

#include <QObject>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>

class DateHelper;

class RecordWrodlibUploader : public QObject
{
    Q_OBJECT
public:
    RecordWrodlibUploader(QObject *parent = NULL);
    ~RecordWrodlibUploader();

private slots:
    void UploadError(QNetworkReply::NetworkError error_code);
    void OnTimeUp();

private:
    void Init();
    QString GetWordlibDir();
    void SetStatusWorking();
    void SetStatusUnworking();
    void UploadWordlib();
    void DoHttp(const QString& name);
    void TimingUploadWordlib();
    void SetUploadInterval();
    void RemoveFilesOutOfDate();
    void RemoveTodayRecord();
    QString GetUrl();

    QNetworkAccessManager* m_ptr_network_manager;
    QHttpMultiPart* m_http_multi_part;
    QFile* m_ptr_file_record_wordlib;
    std::atomic_bool m_is_working;
    int m_upload_interval;
    DateHelper* m_date_helper;
    QStringList m_to_upload;

};

#endif // RECORDWRODLIBUPLOADER_H
