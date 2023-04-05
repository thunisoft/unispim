#ifndef WORDLIBNETWORKER_H
#define WORDLIBNETWORKER_H

#include <QObject>
#include <QNetworkAccessManager>
#include "../public/utils.h"
#include "../public/defines.h"

class QJsonValueRef;

enum DOWNLOAD_STATE{
    Ok,
    Failed
};
class WordlibNetworker : public QObject
{
    Q_OBJECT
public:
    explicit WordlibNetworker(QObject* parent = 0);
    int GetWordlibInfoFromServer();
    QMap<QString, WORDLIB_INFO> GetWordlibInfo();
    void DownloadWordlib(const WORDLIB_INFO& wordlib_info);
    void ClearUserWordlib(const QString& user_id);

signals:
    void GetWordlibInfoDone();
    void WordlibDownloadDone(int state);

private slots:
    void OnGetWordlibInfoDone(QNetworkReply *reply);
    void OnDownloadWordlibDone(QNetworkReply *reply);

private:
    void Init();
    bool PhraseJsonInfo(const QByteArray& data);
    bool FillDataToWordlibInfoEntity(WORDLIB_INFO& wordlib_info, QJsonValueRef json_value);
    bool VerifyMD5();
    void SendSignal(int state);

    WORDLIB_INFO m_one_wordlib_info;
    QMap<QString, WORDLIB_INFO> m_wordlib_info;
    QNetworkAccessManager* m_network_manager;
};

#endif // WORDLIBNETWORKER_H
