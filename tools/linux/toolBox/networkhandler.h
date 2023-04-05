#ifndef NETWORKHANDLER_H
#define NETWORKHANDLER_H

#include <QWidget>
#include <QObject>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QSslSocket>
#include <QHostInfo>
#include "config.h"

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
public:
    NetworkHandler(QObject* parent = 0);
    ~NetworkHandler();

public:
    bool IsNetworkConnected();
    bool canVisitWeb();

signals:
    void networkStatusResult(bool status);
    void download_progress(qint64 receivedBytes,qint64 tool_bytes);


public slots:
    void onLookupHost(QHostInfo hostInfo);
    void slot_handle_download_progress(qint64 receiveBytes,qint64 totalBytes);
public:
    //邮箱账号密码登录
    int manu_check_tools_status(QString input_client,QString loginid,QString os_type,QString cpu_arch,
                                QString addon_list,QVector<SEVER_ADDON_INFO>& server_addon_vector);
    int task_check_tools_status(QString input_client,QString loginid,QString os_type,
                                QString cpu_arch,QString addon_list,QVector<SEVER_ADDON_INFO>& server_addon_vector);

    //获得下载请求头(IP地址和端口号)
    bool GetRequesetHeader(QString & requestHeader);
    int download_tool_from_url(QString fileUrl,QString fileSavePath);

private:
    bool GenerateRequest(QString inputUrlAddr, QNetworkRequest& inputRequest);
    int ParserRequestReply(QNetworkReply*inpuRely,QString replyType, QJsonObject& returnedJson);
    int ParserRequestReplyStr(QNetworkReply*inpuRely,QString replyType, QJsonValue& returnJson);
    bool SetSSlConfigToRequest(QNetworkRequest& inputRequest,QString certificatePath);


private:
    bool m_canVisitWeb = false;
};

#endif // NETWORKHANDLER_H
