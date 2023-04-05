#ifndef UTILS_H
#define UTILS_H
#include <QString>
#include <QWidget>
class Utils
{
public:
    //对目文件进行解压缩
    static bool depressed_zlib_file(QString zipFileFullName, QString zipFilePath);
    static std::string MB2UTF8(std::string mb);
    static void write_log_tofile(QString log_content);
    static QString get_os_type();
    static QString get_cpu_type();
    static void show_messge_box(QString content, QWidget* parent = 0);
    static void set_windows_show_event();
    static bool excute_command_line(QString command_line);
    static bool IsProccessOn(const QString &process_name);
    static QString GetLinuxHuayuInstallPath();
    static QString GetLinuxUpdaterFilePath();
    static void DeleteAddonByName(QString addon_name, QString addon_version);
};

#endif // UTILS_H
