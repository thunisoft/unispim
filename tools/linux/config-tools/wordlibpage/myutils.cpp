#include <QFileInfo>
#include <QFile>
#include <QProcess>
#include <QNetworkInterface>
#include <QCryptographicHash>
#include <QDir>
#include <QApplication>
#include "myutils.h"
#include "informmsgbox.h"
#include "../public/configmanager.h"

namespace TOOLS {
//分析词库的信息
bool WordlibUtils::get_uwl_fileinfo(const QString& uwl_file_path, WORDLIB_INFO& wordlib_info)
{
    //get uwl fileinfo
    QFileInfo file_info(uwl_file_path);
    int length = file_info.size();
    char *buffer = new char[length];
    memset(buffer, 0, length);
    QFile file(uwl_file_path);
    if (!file.open(QFile::ReadOnly))
    {
        return false;
    }
    file.read(buffer, length);
    file.close();

    WORDLIB* wl = (WORDLIB*)buffer;
#ifdef Q_OS_WIN
    QString wordlib_name = QString::fromWCharArray(wl->header.name);
    wordlib_info.author = QString::fromWCharArray(wl->header.author_name);
    wordlib_info.can_be_edit = wl->header.can_be_edit;
#elif defined(Q_OS_LINUX)
    QString wordlib_name = QString::fromUtf16(wl->header.name).toLocal8Bit();
#endif

    wordlib_info.file_path = uwl_file_path;
    wordlib_info.file_name = file_info.fileName();
    wordlib_info.wordlib_name = wordlib_name;
    wordlib_info.author = QString::fromUtf16(wl->header.author_name);
    wordlib_info.words_count = wl->header.word_count;
    wordlib_info.md5 = get_file_md5(wordlib_info.file_path);
    wordlib_info.can_be_edit = wl->header.can_be_edit;
    wordlib_info.wordlib_last_modified_date = file_info.lastModified().toString("yyyy/MM/dd");
    delete buffer;
    return true;
}

QString WordlibUtils::get_file_md5(QString file_path)
{
    QFile file(file_path);
    if(!file.open(QFile::ReadOnly))
        return "";
    QByteArray bytes = QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5);
    file.close();
    return QString(bytes.toHex().constData());
}

bool WordlibUtils::combine_uwl_wordlib(QString dest_wordlib, QString source_wordlib)
{
#ifdef Q_OS_WIN
    QString install_dir;
#elif defined(Q_OS_LINUX)


    QString wordlib_tools_path = PathUtils::get_wordlib_merge_tool_path();
    if(!QFile::exists(wordlib_tools_path))
    {
        return false;
    }

    QStringList argumentList;
    argumentList << dest_wordlib << source_wordlib << dest_wordlib;
    QProcess process;
    int ret = process.execute(wordlib_tools_path,argumentList);
    if(ret == 1)
    {
        return true;
    }
    else
    {
        return false;
    }

#endif
}

bool WordlibUtils::export_uwl_wordlib_to_text(QString uwl_file_path, QString target_file_path)
{
#ifdef Q_OS_WIN
    QString install_dir;
    QString wordlib_tools_path;
#elif defined(Q_OS_LINUX)
    QString wordlib_tools_path = PathUtils::get_wordlibtool_path();
    if(!QFile::exists(wordlib_tools_path))
    {
        return false;
    }
    QStringList argumentList;
    argumentList << "-export" << uwl_file_path << target_file_path;
    QProcess process;
    process.setProgram(wordlib_tools_path);
    process.setArguments(argumentList);
    process.start();
    return process.waitForFinished(5000);
#endif
}
void WordlibUtils::export_uwl_wordlib_to_uwl(QString source_path, QString target_path)
{
    QFile uwl_file(source_path);
    uwl_file.copy(target_path);
}

int WordlibUtils::create_uwl_wordlib_from_text(QString text_file_path, QString uwl_file_path)
{

    QString wordlib_tool_path = PathUtils::get_wordlibtool_path();
    if(wordlib_tool_path.isEmpty())
    {
        return false;
    }
    if(!QFile::exists(wordlib_tool_path))
    {
        return false;
    }
    QStringList argumentList;
    argumentList << "-import" <<text_file_path << uwl_file_path;
    QProcess process;
    process.setProgram(wordlib_tool_path);
    process.setArguments(argumentList);
    process.start();
    int ret = process.waitForFinished(20000);
    if(ret != 1)
    {
        if (ret == -99)
        {
            return false;
        }
        return false;
    }
    return true;
}

int WordlibUtils::merge_uwl_wordlib(QString dest_wordlib, QString source_wordlib)
{
    QStringList params;
    params << "-merge-uwl";
    params << QString(source_wordlib);
    params << QString(dest_wordlib);

    QString wordlib_tools_path = PathUtils::get_wordlibtool_path();

    QProcess process;
    process.setProgram(wordlib_tools_path);
    process.setArguments(params);
    process.start();
    return process.waitForFinished(50000);
}

}

namespace TOOLS {
int Messgebox::notice_msg(const QString& notice_content, QWidget* parent,int msgType)
{
    InformMsgbox msgbox(parent);
    msgbox.SetContent(notice_content);
    msgbox.SetMsgBoxType(msgType);
    return msgbox.exec();
}

int Messgebox::notice_msg_without_abbre(const QString& notice_content, QWidget* parent,int msgType)
{
    Q_UNUSED(parent)
    InformMsgbox msgbox;
    msgbox.SetContent(notice_content,false);
    msgbox.SetMsgBoxType(msgType);
    return msgbox.exec();
}
}

namespace  TOOLS {

void LogUtils::write_log_to_file(QString content)
{
    QString logFilePath = PathUtils::get_log_file_path();
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
        QString fullContent = currentTime + content + "\r\n";
        QTextStream outputStream(&file);
        outputStream << fullContent;
        file.close();
    }
}
}

namespace  TOOLS {
    QString PathUtils::get_user_wordlib_path()
    {
#ifdef Q_OS_WIN
        ConfigItemStruct configInfo;
        if(Config::Instance()->GetConfigItemByJson("login",configInfo) && configInfo.itemCurrentIntValue)
        {
            if(Config::Instance()->GetConfigItemByJson("loginid",configInfo) && !configInfo.itemCurrentStrValue.isEmpty())
            {
                return GetConfigDirPath().append("wordlib/").append(configInfo.itemCurrentStrValue).append("/");
            }
        }
        return GetConfigDirPath().append("wordlib/");
#elif defined(Q_OS_LINUX)
        return ConfigManager::Instance()->GetUserUwlPath();
#endif
    }
    QString PathUtils::get_local_wordlib_path()
    {
       return get_user_config_dir_path().append("wordlib/");
    }

    QString PathUtils::get_user_config_dir_path()
    {
        QString sys_config_dir(getenv("XDG_CONFIG_HOME"));
        if(sys_config_dir.isEmpty())
            sys_config_dir = QDir::homePath() + "/.config/";
        if(!sys_config_dir.endsWith('/'))
            sys_config_dir.append('/');
        return  sys_config_dir.append("fcitx-huayupy/");
    }

    QString PathUtils::get_wordlibtool_path()
    {
        QString wordlib_tools_path = GetPackageInstallpath() +QString("files/bin/huayupy-wl-tool-fcitx");
        return wordlib_tools_path;
    }

    QString PathUtils::get_log_file_path()
    {
        return "";
    }

    QString PathUtils::get_wordlib_merge_tool_path()
    {
        QString wordlib_merge_tool = GetPackageInstallpath() +QString("files/bin/huayupy-merge-wordlib");
        return wordlib_merge_tool;
    }

    QString PathUtils::get_install_wordlib_dir_path()
    {
        QString install_wordlib_dir = GetPackageInstallpath() +QString("files/wordlib/");
        return install_wordlib_dir;
    }
    QString PathUtils::GetPackageInstallpath()
    {
    #ifdef USE_FCITX
        return QString("/opt/apps/huayupy/");
    #else
        return QString("/opt/apps/com.thunisoft.input/");
    #endif
    }

    QString PathUtils::GetUpdaterIniPath()
    {
        QString install_path = GetPackageInstallpath();
        return QDir::toNativeSeparators(install_path + QString("files/updater.ini"));
    }
    QString PathUtils::GetSPFileDir()
    {
        QString install_path = GetPackageInstallpath();
        return QDir::toNativeSeparators(install_path + QString("files/ini/"));
    }

    QString PathUtils::get_cloud_wordlib_dir()
    {
        QString cloud_dir = get_local_wordlib_path() + "cloud/";
        QDir dir(cloud_dir);
        if(!dir.exists())
        {
            dir.mkpath(cloud_dir);
        }
        return cloud_dir;
    }
}

namespace TOOLS {
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

QString Utils::get_file_md5(QString file_path)
{
    QFile file(file_path);
    if(!file.open(QFile::ReadOnly))
        return "";
    QByteArray bytes = QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5);
    file.close();
    return QString(bytes.toHex().constData());
}

QString Utils::get_host_mac()
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
    QByteArray bytes = QCryptographicHash::hash(strMacAddr.toUtf8(), QCryptographicHash::Md5).toHex();
    return bytes;
}

}

