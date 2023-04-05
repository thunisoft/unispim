#ifndef UPDATECONTROL_H
#define UPDATECONTROL_H

#include <QObject>
#include "updatewidget.h"

class UpdateWidget;
class PackageDownloader;
class UpdateControl : public QObject
{
    Q_OBJECT
public:

    static bool GetConfigItem(QString key, ConfigItemStruct & value_struct);
    UpdateControl();
    ~UpdateControl();
    void NotifyUpdate();
    void InstallUpdate();
    void SetPackageurl(const QString& url);
    void SetPackageMd5(const QString& file_md5);
    void SetDownloadType(const QString& download_type);
    void SetDownloadPackageVersion(const QString& download_version);

private slots:
    void OnDownloadFinished();

private:
    QString GetClientID();


public:
    QString GetRequestScheme();
    int UploadDownloadInfo(QString versionNum, QString clientid);

    QString GetIMEVersion();
private:
    /**@brief 更新服务端的版本号通知客户端进行更新
   * @param[in]  version_num 新的版本号
   * @param[in]  package_name 安装包的名称
   */
    void UpdateServerVersionNum(QString version_num,QString package_name);

    QString GetPackageFilePath(QString m_url);

    bool IsNeedToDownloadPackage();
	bool IsDownloaded();

private:
    UpdateWidget* m_update_widget;
    PackageDownloader* m_downloader;

    QString m_download_url;
    QString m_scheme;
    QString m_file_md5;
    QString m_download_type;
    QString m_server_version;
};

#endif // UPDATECONTROL_H
