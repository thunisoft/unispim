#include "updatecontrol.h"
#include <QTextCodec>
#include <QDesktopWidget>
#include <QApplication>
#include <QNetworkInterface>
#include <QFileInfo>
#include <QRegExp>
#include <QSettings>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QDir>
#include <QStandardPaths>
#include <QStringList>
#include <QProcess>

#include "updatewidget.h"
#include "packagedownloader.h"
#include "utils.h"


void ShowUpdateWidget(UpdateWidget& w)
{
    //move to the center of the screen
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    QDesktopWidget* desktop = QApplication::desktop();

    int current_screen = desktop->screenNumber(&w);
    QRect rect = desktop->screenGeometry(current_screen);

    ConfigItemStruct show_position;
    bool isOK = UpdateControl::GetConfigItem("version_tools_show_position",show_position);
    if(isOK)
    {
        int show_flag = show_position.itemCurrentIntValue;
        if(show_flag == 1)
        {
            w.move((rect.width() - w.width()),0);
        }
        else
        {
            w.move((rect.width() - w.width())/2,(rect.height() - w.height())/2);
        }
    }
    else
    {
         w.move((rect.width() - w.width())/2,(rect.height() - w.height())/2);
    }

    w.show();
}

UpdateControl::UpdateControl()
{
    m_update_widget = nullptr;
    m_downloader = nullptr;
}

UpdateControl::~UpdateControl()
{

}

void UpdateControl::NotifyUpdate()
{
    if(!m_downloader)
        m_downloader = new PackageDownloader();
    if(!IsNeedToDownloadPackage())
    {
		Utils::WriteLogToFile(QString("IsNeedToDownloadPackage()=false"));
        exit(0);
    }
	if (IsDownloaded())
	{
        Utils::WriteLogToFile("local have same package");
        QString filePath = GetPackageFilePath(m_download_url);
        QFileInfo file_info(filePath);
        QString local_file_md5 = Utils::GetFileMD5(filePath).toLower();
        QString fileName = file_info.fileName();
        if(local_file_md5 != m_file_md5)
        {
            Utils::WriteLogToFile("md5 is different from server");
        }
        else
        {
            //将安装包的版本号和安装包的名称写到对应的ini配置文件中
            UpdateServerVersionNum(m_server_version,fileName);
            QStringList args;
            args.append("/verysilent");
            args.append("/norestart");
            QProcess::startDetached(filePath, args);
            m_update_widget->SaveConfigAndExit();
        }
	}

    connect(m_downloader, SIGNAL(DownloadFinished()), this, SLOT(OnDownloadFinished()));
    m_downloader->SetDownloadMode(m_download_type);
    m_downloader->StartDownload(m_download_url);
}

void UpdateControl::InstallUpdate()
{
    m_update_widget = new UpdateWidget(INSTALL);
    m_update_widget->SetPackageUrl(m_download_url);
    ShowUpdateWidget(*m_update_widget);

}

void UpdateControl::OnDownloadFinished()
{
    //通过安装包获取版本号
    QString packagePath = m_downloader->GetPackageSavePath();
    if(packagePath.isEmpty())
    {
        Utils::WriteLogToFile("on download finished get packagepath return empty");
        return;
    }
    QString local_file_md5 = Utils::GetFileMD5(packagePath).toLower();
    if(local_file_md5 != m_file_md5)
    {
        Utils::WriteLogToFile("download file md5 error");
        return;
    }

    QFileInfo fileInfo(packagePath);
    QString fileName = fileInfo.fileName();
    if(m_server_version.isEmpty())
    {
        QRegExp reg("7\\.[0-9]{1,4}\\.[0-9]{1,4}\\.[0-9]{1,4}");
        QString versionNum;
        for(int pos=0;pos=reg.indexIn(fileName,pos),pos>=0; pos+=reg.matchedLength())
        {
           versionNum  = reg.cap(0);//返回匹配到的字串
           break;
        }
        m_server_version = versionNum;
    }

    //获取当前电脑的mac地址
    QString clientID = GetClientID().toLower();
    int upload_downloadinfo_ret = UploadDownloadInfo(m_server_version, clientID);
    Utils::WriteLogToFile(QString("Upload downloadinfo return:%1").arg(upload_downloadinfo_ret));

    //将安装包的版本号和安装包的名称写到对应的ini配置文件中
    UpdateServerVersionNum(m_server_version,fileName);

    //提示更新消息
    m_update_widget = new UpdateWidget(INSTALL);
    m_update_widget->SetPackageUrl(m_download_url);

    QFile file(packagePath);
    if(file.exists())
    {
		Utils::WriteLogToFile(QString("Start Silent Install: %1").arg(packagePath));

		QStringList args;
		args.append("/verysilent");
		args.append("/norestart");
        QProcess::startDetached(packagePath, args);
        m_update_widget->SaveConfigAndExit();
    }
}

void UpdateControl::SetPackageurl(const QString& url)
{
    m_download_url = url;
}

void UpdateControl::SetPackageMd5(const QString &file_md5)
{
    m_file_md5 = file_md5;
}

void UpdateControl::SetDownloadType(const QString &download_type)
{
    m_download_type = download_type;
}

void UpdateControl::SetDownloadPackageVersion(const QString &download_version)
{
    m_server_version = download_version;
}

QString UpdateControl::GetClientID()
{
    QList<QNetworkInterface> nets = QNetworkInterface::allInterfaces();// 获取所有网络接口列表
    int nCnt = nets.count();
    QString strMacAddr = "";
    for(int i = 0; i < nCnt; i ++)
    {
        // 如果此网络接口被激活并且正在运行并且不是回环地址，则就是我们需要找的Mac地址
        if(nets[i].flags().testFlag(QNetworkInterface::IsUp) && nets[i].flags().testFlag(QNetworkInterface::IsRunning) && !nets[i].flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            strMacAddr = nets[i].hardwareAddress();
            break;
        }
    }

    strMacAddr = strMacAddr.toLower();
    //计算mc地址的md5值
    QString mcMd5;
    QByteArray ba,bb;
    QCryptographicHash md5(QCryptographicHash::Md5);
    ba.append(strMacAddr);
    md5.addData(ba);
    bb = md5.result();
    mcMd5.append(bb.toHex());

    return mcMd5;
}

int UpdateControl::UploadDownloadInfo(QString versionNum, QString clientid)
{
    //生成对应的网址请求
    QNetworkRequest request;
    QString subwebIndex = QString("/test/notifyupdate");
    QString requestheader = GetRequestScheme();
    request.setUrl(requestheader+subwebIndex);

    QString filePath = QDir::toNativeSeparators(QCoreApplication::applicationDirPath() + "\\server_certification.cer");
    if(!PackageDownloader::SetSSlConfigToRequest(request,filePath))
    {
        return 1;
    }

    //获取错误
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.get(request);
    QEventLoop eventLoop;
    connect(reply, SIGNAL(finished()),&eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
    if(reply->error() != QNetworkReply::NoError)
    {
        return 2;
    }

    QByteArray replyData = reply->readAll();
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(replyData, &json_error));

    if(json_error.error != QJsonParseError::NoError)
    {
        return 3;
    }
    QJsonObject rootObj = jsonDoc.object();
    if(!rootObj.contains("code"))
    {
        return 4;
    }
    if(rootObj.value("code").toString().toInt() == 200)
    {
        return 0;
    }
    return 5;
}

void UpdateControl::UpdateServerVersionNum(QString version_num,QString package_name)
{
    QString usr_dir = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first().append("/AppData/Local/Thunisoft/HuayuPY/");
    QString check_version_path = usr_dir + "version_check.ini";
    QSettings settings(check_version_path,QSettings::IniFormat);
    settings.beginGroup("version");
    settings.setValue("server_version",version_num);
    settings.setValue("package_name",package_name);
    settings.endGroup();
}

QString UpdateControl::GetRequestScheme()
{
    QString jsonFileFath  = QApplication::applicationDirPath() + "\\config.json";
    QFile file(jsonFileFath);
    if(!file.exists())
    {
        return "";
    }
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString value = file.readAll();
    file.close();
    QJsonParseError parseJsonErr;
    QJsonDocument document = QJsonDocument::fromJson(value.toUtf8(),&parseJsonErr);
    if(!(parseJsonErr.error == QJsonParseError::NoError))
    {
        return "";
    }
    QJsonObject jsonObject = document.object();
    QMap<QString, ConfigItemStruct> infoMap;
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

    QString serverIPAddr = infoMap["server_addr"].itemCurrentStrValue;
    QString serverPort = QString::number(infoMap["server_port"].itemCurrentIntValue);
    m_scheme = infoMap["server_scheme"].itemCurrentStrValue;

    QString requestHeader = m_scheme + QString("://") + serverIPAddr + QString(":") + serverPort;
    return requestHeader;
}

bool UpdateControl::GetConfigItem(QString key, ConfigItemStruct & value_struct)
{
    QString jsonFileFath  = QApplication::applicationDirPath() + "\\config.json";
    QFile file(jsonFileFath);
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
    QMap<QString, ConfigItemStruct> infoMap;
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
    if(!infoMap.contains(key))
    {
        return false;
    }

    value_struct = infoMap[key];
    return true;
}

QString UpdateControl::GetPackageFilePath(QString m_url)
{
    //判断返回的请求是minIO还是字节流
    QUrl url(m_url);
    QRegExp urlExp(QString("^%1://.*\\?.*=.*").arg(url.scheme()));
    QString fileName;

    //兼容了CDN的下载方式
    if(m_download_type == "cdn")
    {
        QFileInfo fileInfo(m_url);
        fileName = fileInfo.fileName();
    }
    else
    {
        if(urlExp.exactMatch(m_url))
        {
            QStringList partStrList = m_url.split("filename=");
            if(partStrList.size() != 2)
            {
                return "";
            }
            fileName = partStrList.at(1);
        }
        else
        {
            QFileInfo fileInfo(m_url);
            fileName = fileInfo.fileName();
        }
    }

    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString fileSavePath = QString("%1/%2").arg(tempDir).arg(fileName);
    return fileSavePath;
}

bool UpdateControl::IsNeedToDownloadPackage()
{
    QString filePath = GetPackageFilePath(m_download_url);
    QString fileName = QFileInfo(filePath).fileName();
    QString currentVersion = GetIMEVersion();

    if(m_server_version.isEmpty())
    {
        Utils::WriteLogToFile("extract from download addr serverversion is empty");
        return false;
    }

    if(m_server_version == currentVersion)
    {
        Utils::WriteLogToFile("local version and server version are the same");
        return false;
    }
    UpdateServerVersionNum(m_server_version,fileName);
    return true;
}

bool UpdateControl::IsDownloaded()
{
	QString filePath = GetPackageFilePath(m_download_url);
	if (QFile::exists(filePath))
	{
		Utils::WriteLogToFile(QString("File exist=%1").arg(filePath));
		return true;
	}
	return false;
}

QString UpdateControl::GetIMEVersion()
{
    QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Thunisoft\\Huayupy\\7.0",QSettings::NativeFormat);
    QString installPath = settings.value("InstallDir","").toString();
    if(installPath.isEmpty())
    {
      return QString("7.0.0.0");
    }
    else
    {
        QString fullFilePath = installPath + "\\version";
        QFile file(fullFilePath);
        if(file.open(QIODevice::ReadOnly))
        {
            QString versionNum = file.readAll();
            file.close();
            return  versionNum.trimmed();
        }
        else
        {
            return QString("7.0.0.0");
        }
    }
}
