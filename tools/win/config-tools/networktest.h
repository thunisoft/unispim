/**@file  networktest.h
* @brief       网络结构测试
* @details    主要用来对网络请求的接口进行测试
* @date       2020-7-31
* @version     V1.0
* @copyright    Copyright (c) 2020-2024  华宇信息
**********************************************************************************
* @attention
* 注意事项1:  网络请求的配置和配置文件关系紧密，注意修改
* 注意事项2:  配置文件和应用程序放在相同的目录下
* @par 修改日志:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* </table>
*
**********************************************************************************
*/
#ifndef NETWORKTEST_H
#define NETWORKTEST_H

#include <QObject>
#include "networkhandler.h"
#include "utils.h"

class NetWorkTest : public QObject
{
    Q_OBJECT
public:
    explicit NetWorkTest(QObject *parent = nullptr);

public:
    /**@brief 运行函数的测试
   * @par 示例:
   * @code
   *    NetWorkTest test;
   *    test.RunTest();
   * @endcode
   */
    void RunTest();

private:

    void TestUserAccountLogin();
    void TestUserPhoneLogin();
    void TestGetVerificateionCode();
    void TestClientVersionDetection();
    void TestDownloadFileFromWeb();
    void TestUploadUserConfigFile();
    void TestDownloadUserConfigFile();
    void TestChangeUserPassword();
    void TestUploadCustomWordlib();
    void TestDownloadCustomWordlib();
    void TestDownloadHighFreqWordlib();
    void TestCheckSystemWordlib();
    void TestCheckCommonWordlib();
    void TestUploadCustomPhrase();
    void TestDownloadCustomPhrase();
    void TestUploadStatsInput();
    void TestUploadFeedBack();


private:
    QString m_clientid;///< 测试用的clientid
    QString m_loginid;///< 测试用的loginid
    QString m_configFilePath; ///< 配置文件地址
    QString m_password; ///< 用户的密码
    QString m_phoneNum; ///< 用户的手机号码
    QString m_versionNum;  ///< 当前的版本号

};

#endif // NETWORKTEST_H
