#ifndef UTILS_H
#define UTILS_H

#include <QFileInfo>
#include <QFile>
#include <string>
#include <QString>

namespace OLD_VERSION {

typedef unsigned short UTF16;

#define	WORDLIB_NAME_LENGTH     16  //词库名称长度
#define	WORDLIB_AUTHOR_LENGTH   16  //词库作者名称
#define	WORDLIB_PAGE_SIZE   1024    //词库页大小
#define	WORDLIB_PAGE_DATA_LENGTH    (WORDLIB_PAGE_SIZE - 4 * sizeof(int))   //页中可用数据长度
#define MAX_WORDLIBS    32  //内存中最多的词库数目

#define HYPIM_WORDLIB_V66_SIGNATURE     0x14091994  //输入法词库V66标志

#define	CON_NUMBER 24   //合法声母的数目



enum PHRASE_STATUS {
    NONE,
    MODIFIED,
    REMOVED,
    ADDED
};

typedef struct tagPHRASE {
    QString abbr;
    QString content;
    int position;
    PHRASE_STATUS state;
}PHRASE;

typedef struct tagWORDLIB_INFO {
    QString wordlib_name;
    QString url;  //词库在服务器地址
    QString md5;
    QString type;
    QString author;

    QString file_name;
    QString file_path; //词库本地路径
    int words_count;
    QString wordlib_last_modified_date;
    bool is_cloud_wordlib;
    bool is_selected;
    bool is_local_file;
    bool can_be_edit;

}WORDLIB_INFO;
}


using namespace std;

class Utils
{
    Utils();
    ~Utils();
public:
    static bool MakeDir(const QString& dir_path);
    static QString GethostMac();
    static QString GetRealPhraseFilePath();
    static QString GetConfigDirPath();
    static QString GetHuayuPYInstallDirPath();
    static QString GetHuayuPYTmpDirPath();
    static QString GetFileMD5(const QString& file_path);
    static QString GetCategoryWordlibPath();
    static QString GetTmpDir();
    static QString GetWordlibDirPath();
    static QString GetSysPhrasePath();
    static QString GetRegedit();
    static void WriteLogToFile(QString logContent);
    static int GetCurrentDayWaitToUploadCount(QString loginid);
    static void UpdateAlreadyUploadCount(QString loginid,int uploadCount);
    static QString GetOSType();
    static bool SetAppAutoStart();

    static QString GetElidedText(QFont font, QString str, int MaxWidth);

    /**@brief 对字符串进行转编码从MB到UTF-8
   * @param[in]  mb 输入的MB的字符串
   * @return  转换之后的utf-8编码字符串
   */
    static string MB2UTF8(string mb);
    static QString GetStrMd5(QString inputStr);
};

#endif // UTILS_H
