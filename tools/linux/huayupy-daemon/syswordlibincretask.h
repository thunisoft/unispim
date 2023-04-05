#ifndef SYSWORDLIBINCRETASK_H
#define SYSWORDLIBINCRETASK_H

#include <QThread>
#include <QVector>
#include "../public/defines.h"

class SysWordlibIncreTask : public QThread
{
public:
    SysWordlibIncreTask() = default;
    ~SysWordlibIncreTask() = default;

protected:
    virtual void run() override;


private:
    void task_sleep();
    void WriteLogToFile(const QString& content);
    QString get_wordlib_update_url();
    int check_sys_wordlib_update(QVector<WORDLIB_BLOCK_INFO>& wordlib_info_vector);
    int download_full_syswordlib();
    QString GetSysWordlibMD5();
    QVector<WORDLIB_INFO> m_wordlib_info_vector;

    /*
     *合并系统词库函数
     *1、删除临时目录下所有uwl文件
     *2、解压临时目录tmp.zip的增量碎片文件
     *3、临时目录如存在sys.uwl，删除
     *4、将系统词库拷贝到临时目录
     *5、将增量碎片合并到系统词库
     *6、将合并好的词库文件拷贝到原系统词库目录中
     */
    int MergeSysWordlib(QVector<WORDLIB_BLOCK_INFO> block_vector);
    int MergeSysWordlibCdnBlock(QVector<WORDLIB_BLOCK_INFO> block_vector);
    int DownloadBlockFromCDN(QVector<WORDLIB_BLOCK_INFO> &wordlib_info_vector);
    int DownloadFileFromCdnUrl(QString& download_url, QString file_md5);
};

#endif // SYSWORDLIBINCRETASK_H
