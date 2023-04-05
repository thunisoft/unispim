#include "config.h"
#include <QFile>
#include <QSettings>
#include <QDir>
#include <QStandardPaths>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QCryptographicHash>

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
        QString clientkey = GetLoginID_MD5();
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
        QString clientkey = GetLoginID_MD5();
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

QString Config::GetLoginID_MD5()
{
    try {
        QString configPath = GetConfigFilePath();
        QFile file(configPath);
        if(!file.exists())
        {
            return "";
        }
        QString config_content;
        if(file.open(QIODevice::ReadOnly | QIODevice::Text))
        {

            config_content = file.readAll();
            file.close();
        }
        else
        {
            return "";
        }

        QJsonParseError parseJsonErr;
        QJsonDocument document = QJsonDocument::fromJson(config_content.toUtf8(),&parseJsonErr);
        if(!(parseJsonErr.error == QJsonParseError::NoError))
        {
            return "";
        }
        QJsonObject jsonObject = document.object();
        QStringList keyList = jsonObject.keys();

        if(!keyList.contains("login") || !keyList.contains("loginid"))
        {
            return "";
        }

        int islogin = jsonObject.value("login").toObject().value("currentvalue").toInt();

        if(islogin)
        {
            QString loginid = jsonObject.value("loginid").toObject().value("currentvalue").toString();
            QString loginid_Md5;
            QByteArray ba,bb;
            QCryptographicHash md5(QCryptographicHash::Md5);
            ba.append(loginid);
            md5.addData(ba);
            bb = md5.result();
            loginid_Md5.append(bb.toHex());

            return loginid_Md5;
        }
        else
        {
            return "";
        }
    } catch (...) {
        return "";
    }

}

QString Config::GetSymbolHistoryFilePth()
{
    QString dirpath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first().append("/AppData/Local/Thunisoft/HuayuPY/");
    return  dirpath.append("symbol-history.ini");
}

QString Config::GetConfigFilePath()
{
    QString dirpath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first().append("/AppData/Local/Thunisoft/HuayuPY/");
    return  dirpath.append("config.json");
}

Config* Config::m_instance = nullptr;
