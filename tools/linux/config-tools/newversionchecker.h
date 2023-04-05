#ifndef NEWVERSIONCHECKER_H
#define NEWVERSIONCHECKER_H

#include <QObject>
#include <QNetworkReply>
#include <QNetworkAccessManager>

#define REQUEST_TIME_OUT  -1
#define REQUEST_RESULT_ERROR  -2
#define REUQEST_RETURN_CODE_ERROR -3
#define UNKNOWN_ERROR -4

#define HAS_NEW_VERSION 1
#define VERSION_UPTODATE 2
class NewVersionChecker : public QObject
{
    Q_OBJECT
public:
    explicit NewVersionChecker(QObject *parent = 0);



public:
    int CheckTheVersion();

private:
    QString GetVersionCheckUrl();
    void ParseCheckResult(const QByteArray& data, QString& new_version_download_path, QString& version);


signals:
    void checkError(int Code);
    void checkSucceed(int Code);

public slots:
    void OnCheckFinished(QNetworkReply* reply);

inline QString GetNewVersionStr()
{
    return m_new_version;
}
inline QString GetDownloadUrl()
{
    return m_new_version_url;
}
inline QString GetPackageMd5()
{
    return m_package_md5;
}

private:
    bool m_isChecking = false;
    QNetworkAccessManager* m_network_manager = NULL;
    QString m_new_version;
    QString m_new_version_url;
    QString m_package_md5;
};

#endif // NEWVERSIONCHECKER_H
