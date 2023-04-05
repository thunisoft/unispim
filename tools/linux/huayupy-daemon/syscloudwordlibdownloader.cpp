// 云词库下载

#include "syscloudwordlibdownloader.h"
#include <QFile>
#include <QSettings>
#include "../public/config.h"
#include "../public/utils.h"


SyscloudWordlibDownloader::SyscloudWordlibDownloader(){}

SyscloudWordlibDownloader::~SyscloudWordlibDownloader(){}

QString SyscloudWordlibDownloader::GetRequestUrl()
{
    QString md5 = GetWordlibMd5();
    QString url = Config::Instance()->GetServerUrl().append("/testapi/requesturl");

    Utils::AddSystemExtraParamer(url,OPType::AUTO);
    return url;
}

QString SyscloudWordlibDownloader::GetWordlibName()
{
    return "syscloud.uwl";
}

QString SyscloudWordlibDownloader::GetWordlibVersion()
{
    QSettings config(Config::wordLibDirPath(WLDPL_DATA).append("wordlib.ini"), QSettings::IniFormat);
    return config.value("version/syscloud", "").toString();
}
