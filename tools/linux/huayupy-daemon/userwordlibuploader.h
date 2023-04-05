#ifndef USERWORDLIBUPLOADER_H
#define USERWORDLIBUPLOADER_H

#include <QObject>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <atomic>

class UserWordlibUploader : public QObject
{
    Q_OBJECT
public:
    explicit UserWordlibUploader(QObject *parent = NULL);
    ~UserWordlibUploader();
    void Start();
    //判断多端登录用户词库下载
    static bool isUesrwordlibDownload;

private slots:
    void OnUploadDone(QNetworkReply* reply);
    void UploadError(QNetworkReply::NetworkError error_code);
    void OnTimeUp();
    void AfterSavedUserWordlib();
    void DoHttpSlot();
	int CompareLocalUserWordlibWithServer();

signals:
    void Done();

private:
    void Init();
    QString GetUserWordlibPath();
    QString GetUploadUrl();
    QString GetUserWordlibMd5Url();
    bool ParseUploadFeedBack(const QByteArray& data);
    bool IsAutoUploadUserWordlib();
    void SetStatusWorking();
    void SetStatusUnWorking();
    void SetStatusUnWorkingAndNodownlod();
    void UploadWordlib();
    void DoHttp();
    void StartTimer();
    // 获取服务器上用户词库md5
    bool GetUserWordlibMd5(QString &md5Value, QString &clientIdValue);
    //设置用户词库上传间隙
    void SetUploadUserWorlibInterval();    
    bool AnalysisUwl(const QString& uwl_file_path, int& word_count);
    void CloseUserWordlib();
    void WriteLogToFile(const QString &content);

    QNetworkAccessManager* m_ptr_network_manager;
    QHttpMultiPart* m_http_multi_part;
    QFile* m_ptr_file_user_wordlib;
    std::atomic_bool m_is_working;
    QTimer* m_upload_timer;
    QTimer m_wait_write_to_uwl_timer;
    int m_upload_user_wordlib_interval;    
    int m_word_count;
    QString m_loginid;
    QString m_macid;
    bool isMultiport;

};

#endif // USERWORDLIBUPLOADER_H
