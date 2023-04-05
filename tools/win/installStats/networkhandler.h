#ifndef NETWORKHANDLER_H
#define NETWORKHANDLER_H

#include <QObject>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QTimer>


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

class NetworkHandler : public QObject
{
    Q_OBJECT
private:
    NetworkHandler(QObject* parent = 0);
    ~NetworkHandler();

public:
    int NotifySetup(QString clientID, QString osType, QString cpuType, QString version, int operation);

private:
    //获得下载请求头(IP地址和端口号)
    bool GetRequesetHeader(QString & requestHeader);
    bool GenerateRequest(QString inputUrlAddr, QNetworkRequest& inputRequest);
    int ParserRequestReply(QNetworkReply*inpuRely,QString replyType, QJsonObject& returnedJson);
    /**@brief 向请求中添加对应的https的证书
   * @param[in]  inputRequest 网络请求
   * @param[in]  certificatePath 证书的位置
   * @return  函数执行结果
   * - true   添加成功
   * - false  添加失败
   */
    bool SetSSlConfigToRequest(QNetworkRequest& inputRequest,QString certificatePath);

private:

public:
    static NetworkHandler* Instance();
    static NetworkHandler* m_ginstance;


};

#endif // NETWORKHANDLER_H
