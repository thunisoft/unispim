#include "utils.h"
#include <string>
#include <memory>
#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include "zlib/zip.h"
#include "config.h"
#include "custominfombox.h"
#include "mainwindow.h"
#include <QTimer>
#include <QSettings>
#include <QApplication>
#ifdef _WIN32
#include <Windows.h>
#endif

using namespace std;
bool Utils::depressed_zlib_file(QString zipFileFullName, QString zipFilePath)
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
#ifdef _WIN32
            std::string fileExtractPath = dirExtractpath + "/" + MB2UTF8(itemName);
#else
            std::string fileExtractPath = dirExtractpath + "/" + itemName;
#endif
            QString target_dir = QDir::toNativeSeparators(QString::fromStdString(fileExtractPath));
#ifdef _WIN32
            if(target_dir.endsWith("\\"))
#else
            if(target_dir.endsWith("/"))
#endif
            {
                QDir dir(QString::fromStdString(fileExtractPath));
                if(!dir.exists())
                {
                    dir.mkpath(QString::fromStdString(fileExtractPath));
                }
                continue;
            }
#ifdef _WIN32
            fopen_s(&fp, fileExtractPath.c_str(), "wb");
#else
            fp = fopen(fileExtractPath.c_str(), "wb");
#endif
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
#ifdef _WIN32
    int lenWC = ::MultiByteToWideChar(CP_ACP, 0, mb.c_str(), static_cast<int>(mb.length()), NULL, 0);
    std::unique_ptr<wchar_t>  wc(new wchar_t[lenWC]());
    ::MultiByteToWideChar(CP_ACP, 0, mb.c_str(), static_cast<int>(mb.length()), wc.get(), lenWC);

    int lenUTF8 = ::WideCharToMultiByte(CP_UTF8, 0, wc.get(), lenWC, NULL, 0, NULL, NULL);
    std::unique_ptr<char> utf8(new char[lenUTF8]());
    ::WideCharToMultiByte(CP_UTF8, 0, wc.get(), lenWC, utf8.get(), lenUTF8, NULL, NULL);

    return std::string(utf8.get(), lenUTF8);
#else
    return string();
#endif
}

void Utils::write_log_tofile(QString log_content)
{
    QString logFilePath = Config::get_user_config_dir_path();
#ifdef _WIN32
    logFilePath.append("Log/toolBox.log");
#else
    logFilePath.append("logs/toolBox.log");
#endif
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
        QString fullContent = currentTime + log_content + "\r\n";
        QTextStream outputStream(&file);
        outputStream << fullContent;
        file.close();
    }
}

QString Utils::get_os_type()
{
#ifdef _WIN32
    return QString("win");
#else
    QSettings inputMethodInfo(GetLinuxUpdaterFilePath(), QSettings::IniFormat);
    QString os_type = inputMethodInfo.value("package_info/os_type", "").toString();
    return os_type;
#endif
}

QString Utils::get_cpu_type()
{
#ifdef _WIN32
    return QString("x86");
#else
    QSettings inputMethodInfo(GetLinuxUpdaterFilePath(), QSettings::IniFormat);
    inputMethodInfo.beginGroup("package_info");
    QString cpu_type = inputMethodInfo.value("cpu_type", "").toString();
    inputMethodInfo.endGroup();
    return cpu_type;
#endif
}

void Utils::show_messge_box(QString content, QWidget *parent)
{
#ifdef _WIN32
    CustomInfomBox* inform_msgbox = new CustomInfomBox(parent);
    inform_msgbox->set_inform_content(content);
    inform_msgbox->show();
    QTimer *m_time = new QTimer();
    m_time->setSingleShot(true);
    m_time->start(1200);
    QObject::connect(m_time, &QTimer::timeout, [=](){
        inform_msgbox->hide();
    });
#else
    MainWindow::instance()->show_messge_box(content);
#endif
}

void Utils::set_windows_show_event()
{
#ifdef _WIN32
    HANDLE hEvent = OpenEvent(EVENT_ALL_ACCESS, NULL, L"HuayuPY.ShowToolBox");
    if(hEvent != NULL)
    {
        SetEvent(hEvent);
    }
#endif
}

bool Utils::excute_command_line(QString command_line)
{
#ifndef _WIN32
    QString check_command = command_line;
    char return_value[150];
    int count = 0;
    FILE* ptr = NULL;
    if((ptr = popen(check_command.toStdString().c_str(), "r")) == NULL)
    {
        return false;
    }
    memset(return_value, 0, sizeof(return_value));
    if((fgets(return_value, sizeof(return_value),ptr))!= NULL)
    {
        count = atoi(return_value);
    }
    pclose(ptr);
    if(count <= 0)
    {
        return false;
    }
    return true;
#else
    return true;
#endif
}

bool Utils::IsProccessOn(const QString& process_name)
{
#ifndef _WIN32
    QString check_command = QString::fromLocal8Bit(" ps -ef | grep -w %1 | grep -v grep | wc -l ").arg(process_name);
    char return_value[150];
    int count = 0;
    FILE* ptr = NULL;
    if((ptr = popen(check_command.toStdString().c_str(), "r")) == NULL)
    {
        return false;
    }
    memset(return_value, 0, sizeof(return_value));
    if((fgets(return_value, sizeof(return_value),ptr))!= NULL)
    {
        count = atoi(return_value);
    }
    pclose(ptr);
    if(count <= 0)
    {
        return false;
    }
    return true;
#else
    return false;
#endif
}

QString Utils::GetLinuxHuayuInstallPath()
{
#ifdef USE_FCITX
    return "/opt/apps/huayupy/files/";
#else /*uos*/
    return "/opt/apps/com.thunisoft.input/files/";
#endif
}

QString Utils::GetLinuxUpdaterFilePath()
{
    #ifdef USE_FCITX
        return QString("/opt/apps/huayupy/files/updater.ini");
    #else
        return QString("/opt/apps/com.thunisoft.input/files/updater.ini");
    #endif
}

void Utils::DeleteAddonByName(QString addon_name, QString addon_version)
{
    if(addon_name.isEmpty() || addon_version.isEmpty())
    {
        return;
    }
    QString dir_path = Config::Instance()->get_user_addon_dir_path();
    QDir dir(dir_path);
    if(!dir.exists())
    {
        return;
    }
    dir.setFilter(QDir::Dirs);
    QString addon_dir_name = addon_name + QString("-") + addon_version;
    QFileInfoList file_info_list = dir.entryInfoList();
    for(QFileInfo index_info : file_info_list)
    {
        QString file_name = index_info.fileName();
        if((file_name == ".") || (file_name == ".."))
        {
            continue;
        }
        if(!addon_name.isEmpty() && file_name.startsWith(addon_name) && (file_name.startsWith(addon_dir_name)))
        {
            QDir sub_addon_dir;
            QString target_path = index_info.filePath();
            sub_addon_dir.setPath(target_path);
            sub_addon_dir.removeRecursively();
        }
    }
}
