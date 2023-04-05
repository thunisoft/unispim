#include <iostream>
#include <QCoreApplication>
#include <fstream>
#include <Windows.h>
#include <memory>
#include <vector>

#include <QDir>
#include <QtNetwork/QNetworkInterface>
#include <QCryptographicHash>
#include <QStandardPaths>
#include <QSettings>
#include <QApplication>
#include <QDialog>
#include <QDateTime>

#include "utils.h"

Utils::Utils(){}

Utils::~Utils(){}


QString Utils::GetOSType()
{
    return QString("1");
}

bool Utils::SetAppAutoStart()
{
    QSettings autoRunKey("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    QString strAppPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
    autoRunKey.setValue("HuayuPY-config",strAppPath);
    return true;
}

QString Utils::GetElidedText(QFont font, QString str, int MaxWidth)
{
    if (str.isEmpty())
    {
        return "";
    }

    QFontMetrics fontWidth(font);

    //计算字符串宽度
    int width = fontWidth.width(str);

    //当字符串宽度大于最大宽度时进行转换
    if (width >= MaxWidth)
    {
        //右部显示省略号
        str = fontWidth.elidedText(str, Qt::ElideRight, MaxWidth);
    }

    //返回处理后的字符串
    return str;
}

string Utils::MB2UTF8(string mb)
{
    int lenWC = ::MultiByteToWideChar(CP_ACP, 0, mb.c_str(), static_cast<int>(mb.length()), NULL, 0);
    std::unique_ptr<wchar_t>  wc(new wchar_t[lenWC]());
    ::MultiByteToWideChar(CP_ACP, 0, mb.c_str(), static_cast<int>(mb.length()), wc.get(), lenWC);

    int lenUTF8 = ::WideCharToMultiByte(CP_UTF8, 0, wc.get(), lenWC, NULL, 0, NULL, NULL);
    std::unique_ptr<char> utf8(new char[lenUTF8]());
    ::WideCharToMultiByte(CP_UTF8, 0, wc.get(), lenWC, utf8.get(), lenUTF8, NULL, NULL);

    return std::string(utf8.get(), lenUTF8);
}

bool Utils::MakeDir(const QString& dir_path)
{
    QDir dir(dir_path);
    if(!dir.exists())
    {
        if(!dir.mkpath(dir_path))
            return false;
    }
    return true;
}

QString Utils::GethostMac()
{
    QList<QNetworkInterface> nets = QNetworkInterface::allInterfaces();// 获取所有网络接口列表
    int nCnt = nets.count();
    QString strMacAddr = "";
    for(int i = 0; i < nCnt; i ++)
    {
        // 如果此网络接口被激活并且正在运行并且不是回环地址，则就是我们需要找的Mac地址
        if(nets[i].flags().testFlag(QNetworkInterface::IsUp) && nets[i].flags().testFlag(QNetworkInterface::IsRunning)
                && !nets[i].flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            strMacAddr = nets[i].hardwareAddress();
            break;
        }
    }
    return strMacAddr;
}

QString Utils::GetRealPhraseFilePath()
{
    return GetConfigDirPath().append("phrase/user_phrase.txt");
}

QString Utils::GetConfigDirPath()
{
    QStringList sls = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if (sls.isEmpty())
    {
        return "";
    }

    QString result = sls.first();
    result.append("/AppData/Local/Thunisoft/HuayuPY/");

    return result;
}

QString Utils::GetWordlibDirPath()
{
    return QString("C:\\ProgramData\\Thunisoft\\Huayupy\\wordlib\\");
}

QString Utils::GetSysPhrasePath()
{
    return QStandardPaths::locate(QStandardPaths::GenericConfigLocation, QString::fromLocal8Bit("Thunisoft/HuayuPY/phrase/系统短语库.ini"));
}

QString Utils::GetHuayuPYInstallDirPath()
{
    return GetConfigDirPath();
}

QString Utils::GetHuayuPYTmpDirPath()
{
    return GetConfigDirPath();
}

QString Utils::GetFileMD5(const QString& file_path)
{
    QFile file(file_path);
    if(!file.open(QFile::ReadOnly))
        return "";
    QByteArray bytes = QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5);
    file.close();

    return QString(bytes.toHex().constData());
}

QString Utils::GetCategoryWordlibPath()
{
    return GetConfigDirPath();
}

QString Utils::GetTmpDir()
{
    return GetConfigDirPath().append("tmp");
}

QString Utils::GetRegedit()
{
    QSettings setting("HKEY_LOCAL_MACHINE\\SOFTWARE\\Thunisoft\\Huayupy\\7.0", QSettings::NativeFormat);
    return setting.value("InstallDir", "").toString();
}

void Utils::WriteLogToFile(QString logContent)
{
    QString logFilePath = GetConfigDirPath();
    logFilePath.append("Log\\version-tools.log");
    logFilePath = QDir::toNativeSeparators(logFilePath);
    QFileInfo fileInfo(logFilePath);
    QString dir_path = fileInfo.dir().path();
    QDir dir(dir_path);
    if(!dir.exists(dir_path))
    {
       dir.mkpath(dir_path);
    }
    QFile file(logFilePath);
    if(file.open(QIODevice::Append | QIODevice::WriteOnly))
    {
        QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss");
        QString fullContent = currentTime + logContent + "\r\n";
        QTextStream outputStream(&file);
        outputStream << fullContent;
        file.close();
    }
}

int Utils::GetCurrentDayWaitToUploadCount(QString loginid)
{
    QString statsFile;
    if(!loginid.isEmpty())
    {
        statsFile = Utils::GetConfigDirPath() + loginid + "\\user_stat";
    }
    else
    {
        statsFile = Utils::GetConfigDirPath() + "user_stat";
    }

    //获得等待上传的数量
    QString currentDateKey = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    if(!QFile::exists(statsFile))
    {
        return 0;
    }
    QSettings setting(statsFile,QSettings::IniFormat);
    setting.beginGroup("InputCountStats");
    int todayInputCount = setting.value(currentDateKey,"0").toInt();
    setting.endGroup();

    //获得已经上传的数量
    QString alreadyUploadFile = statsFile.append(".bak");
    QSettings alreadyUploadSetting(alreadyUploadFile,QSettings::IniFormat);
    alreadyUploadSetting.beginGroup("InputCountStats");
    int alreadyUploadCount = alreadyUploadSetting.value(currentDateKey,"0").toInt();
    int waitToUploadCount = todayInputCount - alreadyUploadCount;
    return (waitToUploadCount >0)? waitToUploadCount:0;
}

void Utils::UpdateAlreadyUploadCount(QString loginid,int uploadCount)
{
    QString statsFile;
    if(!loginid.isEmpty())
    {
        statsFile = Utils::GetConfigDirPath() + loginid + "\\user_stat";
    }
    else
    {
        statsFile = Utils::GetConfigDirPath() + "user_stat";
    }
    QString alreadyUploadFile = statsFile.append(".bak");
    QSettings alreadyUploadSetting(alreadyUploadFile,QSettings::IniFormat);
    alreadyUploadSetting.beginGroup("InputCountStats");
    QString currentDateKey = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    int alreadyUploadCount = alreadyUploadSetting.value(currentDateKey,"0").toInt();
    int totalCount = alreadyUploadCount + uploadCount;
    alreadyUploadSetting.setValue(currentDateKey,totalCount);
    alreadyUploadSetting.endGroup();
}

QString Utils::GetStrMd5(QString inputStr)
{
    QByteArray sourceArray,destArray;
    QString str_md5;
    QCryptographicHash md5(QCryptographicHash::Md5);
    sourceArray.append(inputStr);
    md5.addData(sourceArray);
    destArray = md5.result();
    str_md5.append(destArray.toHex());

    return str_md5;
}
