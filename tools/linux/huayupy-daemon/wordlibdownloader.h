#ifndef WORDLIBDOWNLOADER_H
#define WORDLIBDOWNLOADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <pthread.h>

class WordlibDownloader : public QObject
{
    Q_OBJECT
public:
    WordlibDownloader(QObject* parent = NULL);
    ~WordlibDownloader();
    virtual QString GetRequestUrl() = 0;
    virtual bool ParseQueryPathReply(const QByteArray& data, QString& wordlib_download_path);
    virtual QString GetWordlibName() = 0;
    virtual bool IsAutoSynchronize();
    virtual QString GetWordlibSavePath();
    QString GetWordlibMd5();
    virtual void DoDownload(const QString& download_path);
    void QueryWordlibPath();
    void Init();
    void Start();

protected:
    void SetStatusWorking();
    void SetStatisUnworking();
private:
    void SaveWordlibVersion(const QString& wordlib_name, const QString& version);
    bool IsWordlibValid(const QString& wordlib_path);
    int RestoreUserWordlib();
public slots:
    /*just for syswordlib, it's a bad designed*/
    void OnQueryWordlibPathDone(QNetworkReply *reply);
    virtual void OnDownloadWordlibDone(QNetworkReply *reply);

    virtual void IncreatementDownloadFinished(int is_success);
    virtual void OnQueryCompleteUpdateFinished(QNetworkReply* reply);
    void OnHttpError(QNetworkReply::NetworkError error_code);

signals:
    /*just for syswordlib, it's a bad designed*/
    void IncreatementDownloadDone(int is_succes);
    void QueryCompleteUpdateDone(QNetworkReply* reply);
    void UserWordLibDownloadDone();

protected:
    QNetworkAccessManager* m_network_manager;
private:
    bool m_is_working;
    QString m_wordlib_download_path;

    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
};

#endif // WORDLIBDOWNLOADER_H
