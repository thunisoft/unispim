#ifndef SYSCLOUDWORDLIBDOWNLOADER_H
#define SYSCLOUDWORDLIBDOWNLOADER_H

#include "wordlibdownloader.h"

class SyscloudWordlibDownloader : public WordlibDownloader
{
public:
    SyscloudWordlibDownloader();
    ~SyscloudWordlibDownloader();
    QString GetRequestUrl();
    QString GetWordlibName();
private:
    QString GetWordlibVersion();
};

#endif // SYSCLOUDWORDLIBDOWNLOADER_H
