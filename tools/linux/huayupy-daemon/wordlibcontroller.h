#ifndef WORDLIBCONTROLLER_H
#define WORDLIBCONTROLLER_H

#include "syscloudwordlibdownloader.h"
#include "syswordlibdownloader.h"
#include "userwordlibdownloader.h"
#include "userwordlibuploader.h"
#include "recordwrodlibuploader.h"

class SyswordlibDownloader;
class WordlibController :public QObject
{
    Q_OBJECT
public:
    WordlibController();
    ~WordlibController();

public:
    void Start();

private slots:

private:
    void Init();

    SyswordlibDownloader* m_sys_wordlib_downloader;
    WordlibDownloader* m_syscloud_wordlib_downloader;
    RecordWrodlibUploader* m_record_wordlib_uploader;
};

#endif // WORDLIBCONTROLLER_H
