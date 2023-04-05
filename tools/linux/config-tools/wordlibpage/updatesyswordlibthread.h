/**@file  updatesyswordlibthread.h
* @brief      后台和并更新系统词库的线程
* @details    详细描述
* @date        2020-9-14
* @version     V1.0
* @copyright    Copyright (c) 2018-2020  华宇信息
**********************************************************************************
* @par 修改日志:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* </table>
*
**********************************************************************************
*/
#ifndef UPDATESYSWORDLIBTHREAD_H
#define UPDATESYSWORDLIBTHREAD_H

#include <QThread>
#ifdef Q_OS_WIN32
#include "config.h"
#elif defined(Q_OS_LINUX)
#endif
#include "networkhandler.h"


class UpdateSysWordlibThread : public QThread
{
    Q_OBJECT

public:
    UpdateSysWordlibThread() = default;
    ~UpdateSysWordlibThread();

    int check_sys_wordlib_update(QVector<WORDLIB_BLOCK_INFO> &wordlib_info_vector);
protected:
    virtual void run() override;

private:
    /**@brief 系统词库增量更新的函数
   * @return  函数执行结果
   * - 0  更新成功
   * - 1  更新失败
   */
    int IncreSysWordlibUpdateTask();

    /**@brief 下载增量词库的碎片并合并
   * @param[in]  block_vector 词库增量碎片的容器
   * @return  返回的状态码
   * - 0 合并成功
   * - others 合并失败
   */
    int DownloadWordlibAndCombine(QVector<WORDLIB_BLOCK_INFO> block_vector);

    int update_profess_wordlib();
    int download_full_syswordlib();


private:
    /**@brief 将下载下来的词库碎片合并到系统词库中去
   * @param[in]  sys_wordlib_path 系统词库的路径
   * @param[in]  block_path 需要合并的词库碎片的路径地址
   * @return  函数执行结果
   * - true  合并成功
   * - false  合并失败
   */
    bool CombineWorlibBlock(QString sys_wordlib_path, QString block_path);
    /**@brief 清理合并之后的词库残留碎片
   * @param[in]  block_vector 词库碎片对应的容器
   */
    void CleanSystmpBlock(QVector<WORDLIB_BLOCK_INFO> block_vector);

    //合并词库碎片
    int MergeSysWordlib(QVector<WORDLIB_BLOCK_INFO> block_vector);
    int DownloadBlockFromCDN(QVector<WORDLIB_BLOCK_INFO> &wordlib_info_vector);
    int DownloadFileFromCdnUrl(QString &download_url, QString file_md5);
    int MergeSysWordlibCdnBlock(QVector<WORDLIB_BLOCK_INFO> block_vector);
signals:
    void updateFinished(int isSuccess); ///<合并成功的信号
};

#endif // UPDATESYSWORDLIBTHREAD_H
