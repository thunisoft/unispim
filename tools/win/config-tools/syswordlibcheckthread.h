/**@file  syswordlibcheckthread.h
* @brief       此文件主要是后台检查系统词库是否有更新的线程
* @details     系统词库更新检查线程
* @date        2020年9月8日
* @version     V1.0
* @copyright    Copyright (c) 2018-2020  华宇信息
**********************************************************************************
* @attention
* 注意事项1:  线程请求的时候需要联网
* 注意事项2： 采用协议是https协议
* @par 修改日志:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* </table>
*
**********************************************************************************
*/

#ifndef SYSWORDLIBCHECKTHREAD_H
#define SYSWORDLIBCHECKTHREAD_H

#include <QThread>
#include <Windows.h>

class SysWordlibCheckThread : public QThread
{

    Q_OBJECT
public:
    SysWordlibCheckThread() = default;
    ~SysWordlibCheckThread();

protected:
    virtual void run() override;

public:

signals:
    //有更新对应的信号
    void updatecoming(QString version_num);
    void nowordlibupdate();
    void current_date_version_update(QString date_version);

public:
    void set_enable_flag(bool flag);

private:
    bool m_check_enable_flag = true;   //检查的线程
};

#endif // SYSWORDLIBCHECKTHREAD_H
