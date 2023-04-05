#ifndef DOWNLOADTHREAD_H
#define DOWNLOADTHREAD_H

#include <QThread>

class DownloadThread : public QThread
{
    Q_OBJECT
protected:
    virtual void run() override;


public:
    static bool downloadState;
    void SetSourceAndDestFilePath(QString sourceFilePath, QString destDir);
    void SetPackageMd5(QString fileMd5);
    QString GetDownloadFilePath();
    void SetDownloadUrlAndDestPath(QString input_url, QString file_download_path);
    void SetDownloadType(QString download_type);

signals:
    void downloadFinished(int);
    void packageCorrupted();

private:
    //目标文件地址和下载地址
    QString m_sourceFilePath;
    QString m_destDir;
    QString m_downloadFilePath;
    QString m_package_md5;
    QString m_downloadType;
};

#endif // DOWNLOADTHREAD_H
