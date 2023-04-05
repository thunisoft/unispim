#ifndef UPLOADSTATSINFO_H
#define UPLOADSTATSINFO_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QThread>
#include "networkhandler.h"

class StatsInfoUploader : public QThread
{
    Q_OBJECT
public:
    StatsInfoUploader() = default;
    ~StatsInfoUploader() = default;   

protected:
    virtual void run() override;

private slots:
    int GetStatsResult(QString inputTime);

private:
    QString GetStatsFileAddr(QString loginID);
    void WriteLog(const QString inputStr);

    void UpdateServerInfo();
    bool HasUploadTodayInput();
    QString GetRecordFilePath();
    int GetTodayAlreadyInputCount();
    int GetTodayInputCount();
    int UploadInputInfo(QString timeStamp, int inputCount);
    void UpdateAlreadyUploadCount(int uploadCount);
    int check_history_input();

    int UploadMultiHistoryRecord(std::map<std::string, int> history_result);
    int UploadHistoryInputInfo(std::map<std::string, int> history_result);
    QString GetCurrentDate();

    QString GetlocalIpAddr();
    void task_sleep();

private:
    QString m_loginID;   

};

#endif // UPLOADSTATSINFO_H
