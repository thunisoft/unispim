#ifndef CHECKUPDATEUNISPY_H
#define CHECKUPDATEUNISPY_H
#include <QNetworkReply>
#include <QSslError>
#include "../public/config.h"
#include <QString>
#include <string>
#include <QNetworkReply>
#include <QWidget>

using namespace std;

class CheckUpdateUnispy : public QObject
{
    Q_OBJECT

    public slots:
        void CheckUpdate(string imFrame);
        void OncheckFinished(QNetworkReply* reply);
    private slots:
        void onLoginReplyReadyRead();
        void onLoginReplyError(QNetworkReply::NetworkError error);
        void onLoginReplySslErrors(QList<QSslError> errors);

        bool GetLocalVersion(string& version,string& cpu_type, string& os_type);
        int FileExists(const string& filepath);

        void OnDownLoadFinished(QNetworkReply* reply);
    private:
        void SendMsgToDaemon(QString msg);
        bool SaveNewVersionInfo(const QString& new_version, const QString& new_version_down_path, const QString& os_type);


        void DownloadInstallPackage(QString package_addr_url);
private:
        QNetworkAccessManager* m_requestManager;
        string m_os_type;
        QString m_package_file_name;

        bool GetNewVersionAndPathAndOs(QString &new_version, QString &new_version_down_path, QString &os_type);
};

#endif // CHECKUPDATEUNISPY_H
