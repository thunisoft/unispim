#include "config.h"
#include <QFile>
#include <QSettings>
#include <QDir>

Config::Config()
{

}

Config *Config::Instance()
{
    if(m_instance == nullptr)
    {
        m_instance = new Config();
    }
    return m_instance;

}

QVector<QString> Config::GetHistorySymbol()
{
    try{
        QString clientkey = GetLoginID();
        QString symbolhispath = GetSymbolHistoryFilePth();
        QSettings setting(symbolhispath,QSettings::IniFormat);
        if(clientkey.isEmpty())
        {
            return setting.value("default","").toString().split(";").toVector();
        }
        else
        {
            return setting.value(clientkey,"").toString().split(";").toVector();
        }
    }
    catch(...)
    {
        return QVector<QString>();
    }
}


void Config::SetHistorySymbol(QVector<QString> &symbolVector)
{
    try{
        QString targetStr;
        for(int index=0; index<symbolVector.size(); ++index)
        {
            if(index == 0)
            {
                targetStr += symbolVector.at(index);
            }
            else{
                targetStr += ";";
                targetStr += symbolVector.at(index);
            }
        }
        QString clientkey = GetLoginID();
        QString symbolhispath = GetSymbolHistoryFilePth();
        QSettings setting(symbolhispath,QSettings::IniFormat);
        if(clientkey.isEmpty())
        {
            setting.setValue("default",targetStr);
        }
        else
        {
            setting.setValue(clientkey,targetStr);
        }
    }
    catch(...)
    {
    }
}

QString Config::GetLoginID()
{
    QString filePath = GetConfigFilePath();
    QFile file(filePath);
    if(!file.exists())
    {
        return "";
    }
    else
    {
        QSettings setting(filePath,QSettings::IniFormat);
        setting.beginGroup("account");
        int is_login = setting.value("login",0).toInt();
        if(is_login)
        {
            return setting.value("loginid","").toString();
        }
        return "";
    }
}

QString Config::GetSymbolHistoryFilePth()
{
#ifdef USE_IBUS
    return QDir::homePath() + "/.config/ibus-huayupy/";
#else
    QString sys_config_dir(getenv("XDG_CONFIG_HOME"));
    if(sys_config_dir.isEmpty())
        sys_config_dir = QDir::homePath() + "/.config/";
    if(!sys_config_dir.endsWith('/'))
        sys_config_dir.append('/');
    return  sys_config_dir.append("huayupy/symbol-history.ini");
#endif
}

QString Config::GetConfigFilePath()
{
#ifdef USE_IBUS
    return QDir::homePath() + "/.config/ibus-huayupy/";
#else
    QString sys_config_dir(getenv("XDG_CONFIG_HOME"));
    if(sys_config_dir.isEmpty())
        sys_config_dir = QDir::homePath() + "/.config/";
    if(!sys_config_dir.endsWith('/'))
        sys_config_dir.append('/');
    return  sys_config_dir.append("fcitx-huayupy/config.ini");
#endif
}

Config* Config::m_instance = nullptr;
