#ifndef NETWORKHANDLER_H
#define NETWORKHANDLER_H

#include <QWidget>
#include <QObject>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QSslSocket>
#include <QHostInfo>
#include "../public/defines.h"
#include "wordlibpage/wordlibstackedwidget.h"



struct SystemWordlibInfoStru
{
    QString date_str;
    QString name_str;
    int valid;
    int block_num;
    QString id_str;

};

class WordlibListInfo;
class QReplyTimeout : public QObject
{
    Q_OBJECT

public:
    QReplyTimeout(QNetworkReply *reply, const int timeout) : QObject(reply),m_isTimeout(false)
    {
        Q_ASSERT(reply);
        if (reply && reply->isRunning())
        {
            // 启动单次定时器
            QTimer::singleShot(timeout, this, SLOT(onTimeout()));
        }
    }

signals:
    void timeout();  // 超时信号-供进一步处理
private:
    bool m_isTimeout;

public:
    bool GetTimeoutFlag()
    {
        return m_isTimeout;
    }

private slots:
    void onTimeout()
    {
        //处理超时
        QNetworkReply *reply = static_cast<QNetworkReply*>(parent());
        if (reply->isRunning()) {
            reply->abort();
            reply->deleteLater();
            m_isTimeout = true;
            emit timeout();
        }
    }
};


class NetworkHandler : public QWidget
{
    Q_OBJECT
public:
    NetworkHandler(QWidget* parent = 0);
    ~NetworkHandler();

public:
    bool IsNetworkConnected();
    bool canVisitWeb();

signals:
    void networkStatusResult(bool status);
public slots:
    void onLookupHost(QHostInfo hostInfo);
public:
    //邮箱账号密码登录
    int UserAccountLogin(const QString account,const QString password);
    //验证码登录
    int UserPhoneLogin(const QString phoneNum, const QString checkNum);

    int RegisterUserAccount(const QString& account, const QString password);

    int CheckNewVersion(bool& has_newversion,QString& version_num);



    //获得下载请求头(IP地址和端口号)
    bool GetRequesetHeader(QString & requestHeader);

    //请求验证码
    int GetVerificationCode(QString phoneNum);

    //请求版本更新
    int ClientVersionDetection(QString currentVersionNum,bool &isOK, QString&newVersionNum, QString& setupid,QString& packageMd5);

    int getTmpFile(QString fileUrl, QString fileName,QString fileSaveDir, QString &fileSavePath ,QString &fileTmpSavePath);
    int DownloadFileFromWeb(QString fileUrl,QString fileSaveDir,QString& fileSavePath,QString fileName = "");

    int DownloadMergerFile(QString fileUrl,QString fileSaveDir,QString& fileSavePath,QString fileName = "");
    //配置文件上传
    int UploadUserConfigFile(QString loginid, QString timeStamp,QString configJsonPath);

    //下载更新用户配置文件
    int DownloadUserConfigFile(QString loginid,
                               QString& serverStamp, bool & hasUpdate,
                               QString& cfgfileAddr);

    //修改用户密码
    int ChangeUserPassword(QString loginid, QString oldPassWord, QString newPassWord);

    //上传用户词库
    int UploadCustomWordlib(QString loginid, QString clientid, QString dictFilePath, QString& md5Value,int wordlib_count);

    //下载用户词库
    int DownloadCustomWordlib(QString loginid, QString clientid, QString & filePath);

    //高频词库下载
    int DownloadHighFrequencyWordlib(QString wordlibVersion, QString wordName, QString& wordlibAddr);

    /**@brief 向服务器请求系统词库的版本信息
   * @param[in]  wordlibMd5 当前词库的md5值
   * @param[out]  wordlib_info 最新词库的版本信息
   * @return  函数执行结果
   * - 0   查询成功
   * - other  查询失败
   */
    int GetSystemWordlibVersionInfo(QString wordlibMd5,SystemWordlibInfoStru& wordlib_info);

    //控制台自定义词库检查更新
    int CheckCommonWordlib(QString version, QString wordlibName, QString& wordlibPath);

    //上传用户自定义短语
    int UploadCustomPhraseFile(QString loginid, QString timeStamp,QString customPhraseFilePath);
    int DownloadCutomPhraseFile(QString loginid,
                                QString& serverStamp, bool & hasUpdate,
                                QString& customPhraseFilePath);

    //上传用户统计量
    int UploadInputStatsCount(QString clientid, QString loginid, QString version, QString date, int inputCount);

    //上传反馈信息
    int UploadFeedbackInfo(QString version,QString clientid, QString loginid,QString title, QString content,QString type,QList<QString> imageList);

    /**@brief 获得词库的增量更新信息
    * @param[in]  sys_wordlib_md5 词库的md5值
    * @param[out]  result_info 返回的json信息
    * @return  函数执行结果
    * - 0  请求成功
    * - 666 请求超时
    * - Others  其他错误
    */
    int GetSysWordlibIncreInfo(QString sys_wordlib_md5, QString& result_info);

    /**@brief 系统词库的全量下载
    * @return  函数执行结果
    * - 0  请求成功
    * - 666 请求超时
    * - Others  其他错误
    */
    int DownloadFullSysWordlib(QString clientid, QString loginid, QString sys_wordlib_md5);


    QString CombineDownloadPackageUrl(const QString &setupid, const QString &clientid, int optype=2);

    int get_profess_wordlib_list(QVector<WordlibListInfo>& wordlib_name_list);
    int CheckSystemWordlib(QString wordlib_md5,  QString& wordlibid, QString& server_md5,QString& current_date_version);

    //检查系统词库增量信息
    int check_sys_wordlib_block_info(QString wordlib_md5, QVector<WORDLIB_BLOCK_INFO> &wordlib_info_vector);
    int DownloadSysWordlibBlockFromServer(QString clientid, QString loginid, QVector<WORDLIB_BLOCK_INFO> block_vector);

    int check_system_wordlib_update(QString wordlib_md5, QString &download_url, QString &download_file_md5);
    int download_full_sys_wordlib(QString download_url, QString packge_md5);
    int GetCurrentUserUserwordlibMD5(const QString &loginid, const QString &clientid, QString &md5_value, QString &download_url, QString &perv_clientid);
    int UploadCustomWordlib(QString loginid, QString clientid, QString dictFilePath, QString &md5Value);

    int DownloadFileFromWeb(QString fileUrl, QString fileSavePath);
private:
    bool GenerateRequest(QString inputUrlAddr, QNetworkRequest& inputRequest);
    int ParserRequestReply(QNetworkReply*inpuRely,QString replyType, QJsonObject& returnedJson);
    int ParserRequestReplyStr(QNetworkReply*inpuRely,QString replyType, QJsonValue& returnJson);

    /**@brief 向请求中添加对应的https的证书
   * @param[in]  inputRequest 网络请求
   * @param[in]  certificatePath 证书的位置
   * @return  函数执行结果
   * - true   添加成功
   * - false  添加失败
   */
    bool SetSSlConfigToRequest(QNetworkRequest& inputRequest,QString certificatePath);



private:
    bool m_canVisitWeb = false;
    QString GetVersionCheckUrl();
public:
    static NetworkHandler* Instance();
    static NetworkHandler* m_ginstance;

};

#endif // NETWORKHANDLER_H
