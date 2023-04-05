#ifndef DOWNLOADDLG_H
#define DOWNLOADDLG_H

#include <QWidget>
#include <customize_qwidget.h>
#include <QtNetwork/QNetworkReply>
#include "packagedownloader.h"

namespace Ui {
class DownloadDlg;
}

class DownloadDlg : public CustomizeQWidget
{
    Q_OBJECT

public:
    explicit DownloadDlg(QWidget *parent = nullptr);
    ~DownloadDlg();


public:
    void StartDownload(const QString downloadUrl);
    void InstallPackage(const QString filePath);
    void SetProgress(qint64 currentValue ,qint64 totalValue);
    void SetFileName(const QString file_name);

private slots:
    void OnDownLoadFinished();
    void SlotDistrubDownload();
signals:
    void exitDownload();


private:
    Ui::DownloadDlg *ui;
    QString m_file_path;
    PackageDownloader* m_package_downloader;
};

#endif // DOWNLOADDLG_H
