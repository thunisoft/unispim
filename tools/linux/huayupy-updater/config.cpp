#include "config.h"
#include "commondef.h"

#include <QDir>
#include <QSettings>

QString Config::dataDirPath()
{
#ifdef USE_IBUS
    return QString("/usr/share/ibus-huayupy/");
#else
    return QString("/usr/share/fcitx-huayupy/");
#endif
}

QString Config::configDirPath()
{
#ifdef USE_IBUS
    return QDir::homePath() + "/.config/ibus-huayupy/";
#else
    return QDir::homePath() + "/.config/fcitx-huayupy/";
#endif
}

QString Config::configFilePath()
{
    return configDirPath() + QString("config.ini");
}

QString Config::wordLibDirPath(WordLibDirPathLocation location)
{
    switch (location)
    {
    case WLDPL_CONFIG:
        return configDirPath() + QString("wordlib/");
    case WLDPL_DATA:
        return dataDirPath() + QString("wordlib/");
    default:
        return QString("");
    }
}

string Config::GetUpdaterIniPath()
{
#ifdef USE_IBUS
    return "/usr/share/ibus-huayupy/updater.ini";
#else
    return "/usr/share/fcitx-huayupy/updater.ini";
#endif
}

string Config::GetServerAddress()
{
    QSettings config(Config::configFilePath(), QSettings::IniFormat);
    QString server_ip = config.value("server/serverip", DEFAULT_SERVER_IP).toString();
    QString server_port = config.value("server/serverport", DEFAULT_SERVER_PORT).toString();
    return QString("http://%1:%2").arg(server_ip).arg(server_port).toStdString();
}

string Config::GetSaveNewVersionInfoPath()
{
   return (configDirPath()+"new_version_info.ini" ).toStdString();
}

string Config::GetSaveNewVersionInfoDir()
{
    return "/tmp/huayupy/";
}

