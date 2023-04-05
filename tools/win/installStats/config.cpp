#include "config.h"
#include <QSettings>
#include <QVector>
#include <QString>
#include <stdlib.h>
#include <stdio.h>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QDebug>
#include <QDir>
#include <QCoreApplication>
#include <QList>
#include <QNetworkInterface>
#include <QDebug>
#include <Windows.h>
#include <tchar.h>
#include <QCryptographicHash>
#include <QSettings>
#include <QStandardPaths>
#include <QUuid>
#include <QTime>


Config* Config::m_instance = NULL;

void Config::LoadConfig()
{
    LoadSystemConfig();
    MergeConfigInfo();
}

Config* Config::Instance()
{
    if(!m_instance)
    {
        m_instance = new Config();
    }
    return m_instance;
}


Config::Config()
{
    LoadConfig();
}

bool Config::GetConfigItemByJson(QString key,ConfigItemStruct& keyValue)
{
    if(!m_currentConfigMap.contains(key))
    {
        return false;
    }
    keyValue = m_currentConfigMap[key];
    return true;
}


void Config::MergeConfigInfo()
{
    m_currentConfigMap.clear();
    m_currentConfigMap = m_systemConfigMap;
}


void Config::LoadSystemConfig()
{
    m_systemConfigMap.clear();
    QString jsonFileFath  = QCoreApplication::applicationDirPath() + "\\config.json";
    LoadMapInfoFromJson(jsonFileFath,m_systemConfigMap);
}

//解析JSon数据到对应的Map结构
bool Config::LoadMapInfoFromJson(QString filePath, QMap<QString,ConfigItemStruct>& infoMap)
{

    QFile file(filePath);
    if(!file.exists())
    {
        return false;
    }
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString value = file.readAll();
    file.close();
    QJsonParseError parseJsonErr;
    QJsonDocument document = QJsonDocument::fromJson(value.toUtf8(),&parseJsonErr);
    if(!(parseJsonErr.error == QJsonParseError::NoError))
    {
        return false;
    }
    QJsonObject jsonObject = document.object();
    infoMap.clear();
    QStringList keyList = jsonObject.keys();
    for(int index=0; index<keyList.size(); ++index)
    {
        QJsonObject childObject = jsonObject.value(keyList.at(index)).toObject();
        QString  currentKey = keyList.at(index);
        ConfigItemStruct currentConfigItem;
        currentConfigItem.itemName = currentKey;
        currentConfigItem.itemGroupName = childObject.value("group").toString();
        currentConfigItem.itemType = childObject.value("type").toString();
        if(currentConfigItem.itemType == "int")
        {
            currentConfigItem.itemDefaultIntValue = childObject.value("defaultvalue").toInt();
            currentConfigItem.itemCurrentIntValue = childObject.value("currentvalue").toInt();
        }
        else if(currentConfigItem.itemType == "string")
        {
            currentConfigItem.itemDefaultStrValue = childObject.value("defaultvalue").toString();
            currentConfigItem.itemCurrentStrValue = childObject.value("currentvalue").toString();
        }

        infoMap[currentKey] = currentConfigItem;
    }
    return true;
}

QString Config::GetHostMacAddress()
{
    QList<QNetworkInterface> nets = QNetworkInterface::allInterfaces();
    int nCnt = nets.count();
    QString strMacAddr = "";
    for(int i = 0; i < nCnt; i ++)
    {
        if(nets[i].flags().testFlag(QNetworkInterface::IsUp) && nets[i].flags().testFlag(QNetworkInterface::IsRunning) && !nets[i].flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            strMacAddr = nets[i].hardwareAddress();
            break;
        }
    }

    strMacAddr = strMacAddr.toLower();


    QString mcMd5;
    QByteArray ba,bb;
    QCryptographicHash md5(QCryptographicHash::Md5);
    ba.append(strMacAddr);
    md5.addData(ba);
    bb = md5.result();
    mcMd5.append(bb.toHex());
    return mcMd5;
}

QString Config::GetOSVersion()

{
    QSettings regSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",QSettings::NativeFormat);
    QStringList keyList = regSettings.childKeys();
    QString osVersion = regSettings.value("ProductName","unknown osversion").toString();
    return osVersion;

}

QString Config::GetCPUType()
{
    QSettings* cpu = new QSettings("HKEY_LOCAL_MACHINE\\HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", QSettings::NativeFormat);
    QString cpuDescribe = cpu->value("ProcessorNameString").toString();
    delete cpu;
    return cpuDescribe;
}

QString Config::GetIMEVersion()
{
    QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Thunisoft\\Huayupy\\7.0",QSettings::NativeFormat);
    QString installPath = settings.value("InstallDir","").toString();
    QString versionStr;
    if(installPath.isEmpty())
    {
      versionStr = "7.0.0.0";
    }
    else
    {
        QString fullFilePath = installPath + "\\version";
        QFile file(fullFilePath);
        if(file.open(QIODevice::ReadOnly))
        {
            QString versionNum = file.readAll();
            versionStr = versionNum.trimmed();
            file.close();
        }
    }
    return versionStr;
}

bool Config::Is64BitSystem()
{
    SYSTEM_INFO si;
    GetNativeSystemInfo(&si);
    if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
            si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 )
        return true;
    else
        return false;
}

void Config::WriteVersionNumToIniFile(QString current_version)
{
    QString usr_dir = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first().append("/AppData/Local/Thunisoft/HuayuPY/");
    QString check_version_path = usr_dir + "version_check.ini";
    QSettings settings(check_version_path,QSettings::IniFormat);
    settings.beginGroup("version");
    settings.setValue("server_version",current_version);
    settings.setValue("current_version",current_version);
    settings.endGroup();
}

bool Config::SetUserNameToReg(bool flag)
{
    //judge the os is 32bit or 64 bit
    bool is_64bit_os = Is64BitSystem();
    QString os_32_key = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Thunisoft\\Huayupy\\7.0";
    QString os_64_key = "HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Thunisoft\\Huayupy\\7.0";
    QString user_path = QDir::toNativeSeparators(QDir::home().path());
    QString value_key = "ActiveWindowsUserPath";

    if(flag)
    {
        if(is_64bit_os)
        {
            QSettings setting(os_64_key,QSettings::Registry64Format);
            setting.setValue(value_key,user_path);

            QSettings wow64_32_settings(os_32_key,QSettings::Registry64Format);
            wow64_32_settings.setValue(value_key,user_path);
        }
        else
        {
            QSettings setting(os_32_key,QSettings::IniFormat);
            setting.setValue(value_key,user_path);
        }
    }
    else
    {
        if(is_64bit_os)
        {
            QSettings setting(os_64_key,QSettings::Registry64Format);
            if(setting.contains(value_key))
            {
                setting.remove(value_key);
            }
            QSettings wow64_32_settings(os_32_key,QSettings::Registry64Format);
            if(wow64_32_settings.contains(value_key))
            {
                wow64_32_settings.remove(value_key);
            }
        }
        else
        {
            QSettings setting(os_32_key,QSettings::IniFormat);
            if(setting.contains(value_key))
            {
                setting.remove(value_key);
            }
        }
    }
    return true;
}

bool Config::SendMsgToEngine()
{
    const UINT WM_RELOAD_CONFIG = ::RegisterWindowMessage(TEXT("23412343214 - VBNG - 68543 - AEDE - F69S3EWE7ACC79"));
    int timeStamp = 0;
    ::PostMessage(HWND_BROADCAST,WM_RELOAD_CONFIG,(WPARAM) timeStamp, (LPARAM)(long)timeStamp);
    return true;

}



