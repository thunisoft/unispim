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

const QMap<int,QString> Errorcode_Map =
{
    {0,"request succeed"},
    {1,"request timeout"},
    {2,"reply code error"},
    {3,"parser json reply error"},
    {4,"reply field error"},
    {5,"requeset throw exception"},
    {6,"make url error"},
    {99,"unknown error"},
    {10,"download file is unvalid"},
    {7,"reply statusCode is invalid"},
    {8,"certificate doesn't exist"},
    {9,"input paramer error"},
    {10,"The download file is corrupted"}
};



struct WordlibListInfo{
public:
  QString fileName;
  QString viewName;
  QString download_url;
  QString type;
  QString md5;
  bool auto_publish;
  WordlibListInfo(){
    fileName = "";
    viewName = "";
    download_url = "";
    type = "";
    md5 = "";
    auto_publish = false;
  }
  WordlibListInfo(QString inputfilename,QString inputviewname, QString inputurl,QString inputtype, QString inputmd5,bool input_publish_flag)
  {
      fileName = inputfilename;
      viewName = inputviewname;
      download_url = inputurl;
      type = inputtype;
      md5 = inputmd5;
      auto_publish = input_publish_flag;
  }
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

    //获得下载请求头(IP地址和端口号)
    bool GetRequesetHeader(QString & requestHeader);

    int DownloadFileFromWeb(QString fileUrl,QString fileSaveDir,QString fileSavePath,QString fileName = "");

    int get_profess_wordlib_list(QVector<WordlibListInfo>& wordlib_name_list);

    int test_network_connection(QString server_domain,QString version, int os_index, QString clinetid,QString timestamp);
    int download_full_sys_wordlib(QString download_url,QString packge_md5);
    int check_system_wordlib_update(QString wordlib_md5,QString& download_url,QString& download_file_md5);

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

public:
    static NetworkHandler* Instance();
    static NetworkHandler* m_ginstance;
    int DownloadSysWordlibBlockFromServer(QString clientid, QString loginid, QVector<WORDLIB_BLOCK_INFO> block_vector);
    int GetCurrentUserUserwordlibMD5(const QString &loginid, const QString &clientid, QString &md5_value, QString &download_url, QString &perv_clientid);
    int UploadCustomWordlib(QString loginid, QString clientid, QString dictFilePath, QString &md5Value);
    int DownloadFileFromWeb(QString fileUrl, QString fileSavePath);

};

#endif // NETWORKHANDLER_H
