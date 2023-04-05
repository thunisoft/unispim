#ifndef CONFIG_H
#define CONFIG_H

#include <QString>

#define NOTFOUND "0"
#define CHECK_UPDATE "1"
#define UPDATE_UNISPY "3"
#define CHECK_UPDATE_FCITX "1"
#define CHECK_UPDATE_IBUS "2"

#define NO "0"
#define YES "1"
#define DEFAULT_SERVER_IP   "172.16.160.9"
#define DEFAULT_SERVER_PORT "8080"

#define CHECK_FROM_SETTING "0"
#define CHECK_FROM_AUTO "1"

using namespace std;

enum WordLibDirPathLocation
{
    WLDPL_CONFIG,
    WLDPL_DATA
};

class Config
{
public:
    static QString dataDirPath();
    static QString configDirPath();
    static QString configFilePath();
    static QString wordLibDirPath(WordLibDirPathLocation location);
    static QString fcitxConfigFilePath();
    static int fcitxConfigValue_Int(const QString &key, int default_value = 0);
    static int fcitxCandidateCount();

    static string GetUpdaterIniPath();
//    static string GetServiceIp();
    static string GetServerAddress();
    static string GetSaveNewVersionInfoPath();
    static string GetSaveNewVersionInfoDir();
};


#endif // CONFIG_H
