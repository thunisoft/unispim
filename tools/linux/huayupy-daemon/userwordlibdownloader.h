#ifndef USERWORDLIBDOWNLOADER_H
#define USERWORDLIBDOWNLOADER_H

#include "wordlibdownloader.h"

class UserWordlibDownloader : public WordlibDownloader
{
public:
    UserWordlibDownloader();
    ~UserWordlibDownloader();
    QString GetRequestUrl();
    QString GetWordlibName();
    bool IsAutoSynchronize();
    bool ParseQueryPathReply(const QByteArray& data, QString& wordlib_download_path);
    QString GetWordlibSavePath();

public slots:
    void OnDownloadWordlibDone(QNetworkReply *reply);
private:
    QString GetRequestParam();
    void NotifyEngineUserLogin();
};

#endif // USERWORDLIBDOWNLOADER_H
