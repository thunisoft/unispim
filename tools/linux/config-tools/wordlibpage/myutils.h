#ifndef WORDLIBUTILS_H
#define WORDLIBUTILS_H
#include <QObject>

#ifdef QT_OS_WIN
#elif defined(Q_OS_LINUX)
#include "../public/defines.h"
#include "../public/utils.h"
#endif

namespace  TOOLS {

class WordlibUtils
{
public:
    WordlibUtils() = default;
    ~WordlibUtils() = default;

public:
    static bool get_uwl_fileinfo(const QString& uwl_file_path, WORDLIB_INFO& wordlib_info);
    static QString get_file_md5(QString file_path);
    static bool combine_uwl_wordlib(QString dest_wordlib, QString source_wordlib);
    static bool export_uwl_wordlib_to_text(QString uwl_file_path, QString target_file_path);
    static void export_uwl_wordlib_to_uwl(QString source_path, QString target_path);
    static int create_uwl_wordlib_from_text(QString text_file_path,QString uwl_file_path);
    static int merge_uwl_wordlib(QString dest_wordlib, QString source_wordlib);
};

class Messgebox{
public:
    Messgebox() = default;
    ~Messgebox() = default;
public:
    static int notice_msg(const QString& notice_content, QWidget* parent = 0, int msgType = 0);
    static int notice_msg_without_abbre(const QString& notice_content, QWidget* parent = 0,int msgType = 0);

};

class LogUtils{
public:
    static void write_log_to_file(QString content);
};

class PathUtils{
public:
    static QString get_user_wordlib_path();
    static QString get_local_wordlib_path();
    static QString get_user_config_dir_path();
    static QString get_wordlibtool_path();
    static QString get_log_file_path();
    static QString get_wordlib_merge_tool_path();
    static QString get_install_wordlib_dir_path();

    static QString GetPackageInstallpath();
    static QString GetUpdaterIniPath();
    static QString GetSPFileDir();
    static QString get_cloud_wordlib_dir();
};
class Utils{
public:
    static QString GetElidedText(QFont font, QString str, int MaxWidth);
    static QString get_file_md5(QString file_path);
    static QString get_host_mac();
};
}

#endif // WORDLIBUTILS_H
