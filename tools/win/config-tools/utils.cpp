#include <iostream>
#include <QCoreApplication>
#include <fstream>
#include <Windows.h>
#include <QProcess>
#include <memory>
#include <vector>

#include "utils.h"
#include "config.h"
#include "syllableapi.h"
#include <QDir>
#include <QtNetwork/QNetworkInterface>
#include <QCryptographicHash>
#include <QStandardPaths>
#include <QSettings>
#include <QApplication>
#include <QDialog>
#include <QDateTime>
#include "dpiadaptor.h"
#include "informmsgbox.h"
#include "networkhandler.h"
#include "config.h"

#include "zlib/zip.h"



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

bool Utils::BackUpTheUserDefinedSpScheme()
{
    QString sp_ini_file_path = QDir::toNativeSeparators(Utils::GetConfigDirPath() + "SpFileSchemeList.ini");
    if(!QFile::exists(sp_ini_file_path))
    {
        return true;
    }
    QSettings settings(sp_ini_file_path,QSettings::IniFormat);
    settings.beginGroup("usersp");
    QList<QString> file_name_list = settings.childKeys();
    settings.endGroup();

    QString programDataDir = Config::Instance()->GetSPIniFileDir();
    QString userSpfileDir = Config::Instance()->GetUserSPIniFileDir();

    for(QString spSchemeName : file_name_list)
    {
        QString targetFullPath = programDataDir + spSchemeName + ".ini";
        QString destFullPath = userSpfileDir + spSchemeName + ".ini";
        if(QFile::exists(targetFullPath))
        {
            QFile::copy(targetFullPath,destFullPath);
        }
    }
    QFile::remove(sp_ini_file_path);
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

int Utils::CheckHistoryInput(QString loginid)
{
    try
    {
        QMap<QString,int> input_map;
        QMap<QString,int> already_input_map;
        QMap<QString,int> to_upload_map;
        input_map.clear();
        already_input_map.clear();
        to_upload_map.clear();

        QString stats_input_file_path = Utils::GetConfigDirPath() + loginid + "\\user_stat";
        QString upload_count_record_file_path = stats_input_file_path + ".bak";
        QFile statsFile(stats_input_file_path);
        if (!statsFile.exists())
        {
            WriteLogToFile(QString("check hisotry input stats file doesn't exist"));
            return 1;
        }

       QSettings input_ini_config(stats_input_file_path,QSettings::IniFormat);

        //read all input keys in file
        input_ini_config.beginGroup("InputCountStats");
        QStringList subkeys = input_ini_config.childKeys();
        int input_key_count = subkeys.size();
        for (int index = 0; index < input_key_count; ++index)
        {
            QString subindex_key = subkeys.at(index);

            int input_value = input_ini_config.value(subindex_key,0).toInt();
            input_map[subindex_key] = input_value;
        }
        input_ini_config.endGroup();

        //read count that has already upload
        QFile already_upload_file(upload_count_record_file_path);
        QSettings already_upload_iniconfig(upload_count_record_file_path,QSettings::IniFormat);
        if (already_upload_file.exists())
        {
            already_upload_iniconfig.beginGroup("InputCountStats");
            QStringList subkeys = already_upload_iniconfig.childKeys();
            int input_key_count = subkeys.size();
            for (int index = 0; index < input_key_count; ++index)
            {
                int input_value = already_upload_iniconfig.value(subkeys.at(index), 0).toInt();
                already_input_map[subkeys.at(index)] = input_value;
            }
            already_upload_iniconfig.endGroup();
        }

        //差值上传
        auto input_iterator = input_map.begin();
        for (input_iterator; input_iterator != input_map.end(); input_iterator++)
        {
            QString date_key = input_iterator.key();
            int input_value = input_iterator.value();
            if (already_input_map.contains(date_key))
            {
                int already_upload_count = already_input_map.value(date_key);
                if (input_value > already_upload_count)
                {
                    to_upload_map[date_key] = input_value - already_upload_count;
                }
            }
            else
            {
                to_upload_map[date_key] = input_value;
            }

        }
        int ret = UploadMultiHistoryRecord(to_upload_map,loginid);
        return ret;
    }
    catch (std::exception& e)
    {
        WriteLogToFile(QString("upload statistic info throw exception:%1").arg(e.what()));
        return 1;
    }
}

bool Utils::DepressedZlibFile(QString zipFileFullName, QString zipFilePath)
{
    string filepath = zipFileFullName.toStdString();
    string dirExtractpath = zipFilePath.toStdString();
    zip_t *zip = zip_open(filepath.data(), 0, 'r');
    FILE* fp;
    int n = zip_total_entries(zip);
    for (int i = 0; i < n; ++i)
    {
        zip_entry_openbyindex(zip, i);
        {
            const char *name = zip_entry_name(zip);
            std::string itemName(name ? name : "");

            std::string fileExtractPath = dirExtractpath + "\\" + MB2UTF8(itemName);
            fopen_s(&fp, fileExtractPath.c_str(), "wb");
            if (!fp)
            {
                continue;
            }
            char *data = NULL;
            size_t dataSize;
            zip_entry_read(zip, (void**)&data, &dataSize);
            fwrite(data, sizeof(char), dataSize, fp);
            fclose(fp);
            fp = NULL;

            free(data);
            data = NULL;
        }
        zip_entry_close(zip);
    }

    zip_close(zip);

return true;
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


int Utils::NoticeMsgBox(const QString& notice_content, QWidget* parent,int msgType)
{
    InformMsgbox msgbox(parent);
    msgbox.SetContent(notice_content);
    msgbox.SetMsgBoxType(msgType);
    return msgbox.exec();
}

int Utils::NoticeMsgBoxWithoutAbbre(const QString& notice_content, QWidget* parent,int msgType)
{
    Q_UNUSED(parent)

    InformMsgbox msgbox;
    msgbox.SetContent(notice_content,false);
    msgbox.SetMsgBoxType(msgType);
    return msgbox.exec();
}



bool Utils::IsValidWordLibFile(const QFileInfo& fileInfo)
{
    int length = fileInfo.size();
    char *buffer = new char[length];
    memset(buffer, 0, length);
    QFile file(fileInfo.absoluteFilePath());
    if (!file.open(QFile::ReadOnly))
    {
        delete [] buffer; buffer = NULL;
        return false;
    }
    file.read(buffer, length);
    file.close();
    WORDLIB* wl = (WORDLIB*)buffer;
    if (wl->header.signature != HYPIM_WORDLIB_V66_SIGNATURE)
    {
        delete [] buffer; buffer = NULL;
        return false;
    }

    delete [] buffer; buffer = NULL;
    return true;
}

void Utils::AppendFrameSuffix(QString& source_name)
{
#ifdef USE_IBUS
    source_name.append("-ibus");
#else
    source_name.append("-fcitx");
#endif
}

bool Utils::MakeDir(const QString& dir_path)
{
    QDir dir(dir_path);
    if(!dir.exists())
    {
        if(!dir.mkpath(dir_path))
        {
            NoticeMsgBox(QString::fromLocal8Bit("创建目录：[%1]失败！").arg(dir_path), NULL);
            return false;
        }
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

QString Utils::GetUserWordlibDirPath()
{
    ConfigItemStruct configInfo;
    if(Config::Instance()->GetConfigItemByJson("login",configInfo) && configInfo.itemCurrentIntValue)
    {
        if(Config::Instance()->GetConfigItemByJson("loginid",configInfo) && !configInfo.itemCurrentStrValue.isEmpty())
        {
            return GetConfigDirPath().append("wordlib/").append(configInfo.itemCurrentStrValue).append("/");
        }
    }
    return GetConfigDirPath().append("wordlib/");
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
    QCryptographicHash Hash(QCryptographicHash::Md5);
    if(file.open(QIODevice::ReadOnly)){
        while(file.atEnd() == false){
            QByteArray message = file.readAll();
            Hash.addData(message);
        }
    }
    else{
        return "";
    }
    QByteArray bytes = Hash.result();

    return QString(bytes.toHex().constData()).toLower();
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
    logFilePath.append("Log\\config_tools.log");
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
        QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ");
        QString fullContent = currentTime + logContent + "\r\n";
        QTextStream outputStream(&file);
        outputStream << fullContent;
        file.close();
    }
}

int Utils::UploadCurrentStatsCount(QString loginid)
{
    QString clientID = Config::Instance()->GetHostMacAddress();
    QString versionStr = Config::Instance()->GetIMEVersion();
    QString currentDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    int inputCount = GetCurrentDayWaitToUploadCount(loginid);
    if(inputCount > 0)
    {
       int returnCode = NetworkHandler::Instance()->UploadInputStatsCount(clientID,loginid,versionStr,currentDateTime,inputCount);
       if(returnCode == 0)
       {
           Utils::UpdateAlreadyUploadCount(loginid,inputCount);
           return 0;
       }
    }
    return 1;

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

void Utils::CopyPhraseFileToTmpDir(QString loginid)
{
    QString phrase_path;
    QString tmp_phrase_path;
    if(!loginid.isEmpty())
    {
        QString configPathDir = Utils::GetConfigDirPath() + QString("phrase\\") + loginid + QString("\\");
        QDir dir;
        if(!dir.exists(configPathDir))
        {
            dir.mkpath(configPathDir);
        }
        phrase_path = configPathDir + QString("user_phrase.txt");

        ///
        QString tmpPhrasePath = Utils::GetConfigDirPath() + QString("tmp\\") + loginid + QString("\\");
        QDir tmpdir;
        if(!tmpdir.exists(tmpPhrasePath))
        {
            tmpdir.mkpath(tmpPhrasePath);
        }
        tmp_phrase_path = tmpPhrasePath + QString("user_phrase.txt");
    }
    else
    {
         phrase_path = Utils::GetConfigDirPath() + QString("phrase\\user_phrase.txt");
         tmp_phrase_path = Utils::GetConfigDirPath().append("tmp\\user_phrase.txt");
    }
    QString real_phrase_path = phrase_path;
    if(QFile::exists(real_phrase_path))
    {
        QFileInfo tmp_phrase_fileinfo(tmp_phrase_path);
        QDir tmpDir = tmp_phrase_fileinfo.dir();
        if(!tmpDir.exists())
        {
            tmpDir.mkdir(tmpDir.path());
        }
        if(QFile::exists(tmp_phrase_path))
            QFile::remove(tmp_phrase_path);
        QFile::copy(real_phrase_path,tmp_phrase_path);
    }
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

int Utils::UploadMultiHistoryRecord(QMap<QString, int> history_result,QString loginid)
{
    if (history_result.size() == 0)
    {
        return 0;
    }
    QString stats_input_file_path = Utils::GetConfigDirPath() + loginid + "\\user_stat";
    QString upload_count_record_file_path = stats_input_file_path + ".bak";
    QSettings already_upload_iniconfig(upload_count_record_file_path,QSettings::IniFormat);

    QDateTime current_datetime = QDateTime::currentDateTime();
    QString current_time_string = current_datetime.toString(" hh:mm:ss");

    QString input_group = "InputCountStats";

    //组合需要上传的参数
    auto begin_iterator = history_result.begin();
    already_upload_iniconfig.beginGroup(input_group);
    QMap<QString, int> upload_paramer;
    upload_paramer.clear();

    for (begin_iterator; begin_iterator != history_result.end(); ++begin_iterator)
    {
        QString date_key = begin_iterator.key();
        int input_value = begin_iterator.value();

        QString upload_time_key = date_key + current_time_string;
        upload_paramer[upload_time_key] = input_value;
    }

    //进行批量的上传
    QString clientID = Config::Instance()->GetHostMacAddress();
    QString versionStr = Config::Instance()->GetIMEVersion();
    int ret = NetworkHandler::Instance()->UploadHistoryRecordInfo(upload_paramer,clientID,loginid,versionStr);

    if (ret == 0)
    {
        auto iterator = history_result.begin();
        for (iterator; iterator != history_result.end(); ++iterator)
        {
            QString date_key = iterator.key();
            int input_value = iterator.value();
            int prev_number = already_upload_iniconfig.value(date_key, 0).toInt();
            already_upload_iniconfig.setValue(date_key, input_value + prev_number);
        }
    }
    else
    {
       already_upload_iniconfig.endGroup();
       WriteLogToFile(QString("upload history input data error:%1").arg(ret));
       return 2;
    }

    already_upload_iniconfig.endGroup();
    return 0;
}

int Utils::combine_wordlib(QString dest_wordlib, QString wordlib_to_combine)
{
    try
    {
        QString install_dir = Config::Instance()->GetInstallDir();
        QString wordlib_tools_path;
        if(install_dir.isEmpty())
        {
            return false;
        }
        else
        {
            wordlib_tools_path = install_dir + "\\WordlibTool.exe";
        }
        if(!QFile::exists(wordlib_tools_path))
        {
            return false;
        }
        QStringList argumentList;
        argumentList << dest_wordlib << wordlib_to_combine;

        QProcess process;
        process.setProgram(wordlib_tools_path);
        process.setArguments(argumentList);
        process.start();
        return process.waitForFinished(5000);
    }
    catch(std::exception& e)
    {
        Utils::WriteLogToFile(QString("when combine wordlib throw exception:%1").arg(e.what()));
        return false;
    }
}
