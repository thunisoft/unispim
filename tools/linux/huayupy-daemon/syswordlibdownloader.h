#ifndef SYSWORDLIBDOWNLOADER_H
#define SYSWORDLIBDOWNLOADER_H

#include "wordlibdownloader.h"
#include "syswordlibupdatehelper.h"
#include <QMap>
#include <QObject>

enum UpdateType
{
    _COMPLETE,
    _INCREMENT
};

class SyswordlibDownloader : public WordlibDownloader
{
public:
    SyswordlibDownloader();
    ~SyswordlibDownloader();
    QString GetRequestUrl();
    QString GetWordlibName();
    bool ParseQueryPathReply(const QByteArray& data, QString& wordlib_download_path);
    void DoDownload(const QString& download_path);

public slots:
    void IncreatementDownloadFinished(int is_success);
    void OnQueryCompleteUpdateFinished(QNetworkReply* reply);
private:
    QString GetWordlibVersion();
    void SetUpdateType(UpdateType update_type);
    void DownloadComplete();
    void DownloadIncrement();
    QString GetIncrementDownloadPath();
    QString GetCompeleteDownloadPath(const QString& id);
    void QueryCompletedownloadPath();
    void Init();

    UpdateType m_update_type;
    QMap<int, UwlInfo> m_block_list;
    QString m_blocks;
    SyswordlibUpdateHelper* m_wordlib_update_helper;
};

#endif // SYSWORDLIBDOWNLOADER_H
