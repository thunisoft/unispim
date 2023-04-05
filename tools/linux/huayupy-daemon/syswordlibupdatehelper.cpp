// 系统词库增量更新，调用 update_helper, 下载更新包 (zip格式)。解压，合并，将合并完成的uwl放到 config 目录下
#include "syswordlibupdatehelper.h"
#include <QNetworkRequest>
#include <QProcess>
#include <QFile>
#include <QStringList>
#include "../public/config.h"
#include "../public/utils.h"
#include <unistd.h>

SyswordlibUpdateHelper::SyswordlibUpdateHelper(QObject* parent):
                                               QObject(parent)
{
    m_network_manager = new QNetworkAccessManager(this);
    m_uwl_info = nullptr;
}

void SyswordlibUpdateHelper::DownloadIncrement()
{
    if(m_download_path.isEmpty() || !m_uwl_info ||m_uwl_info->empty())
        return ;

    QNetworkRequest request;
    request.setUrl(m_download_path);
    request.setSslConfiguration(Utils::GetQsslConfig());
    QNetworkReply* reply = m_network_manager->get(request);

    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(OnDownloadError(QNetworkReply::NetworkError)));
    connect(m_network_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(OnDownloadDone(QNetworkReply*)));  
}

void SyswordlibUpdateHelper::SetUwlInfo(QMap<int, UwlInfo>* uwl_info)
{
    m_uwl_info = uwl_info;
}

void SyswordlibUpdateHelper::SetDownloadPath(const QString& path)
{
    m_download_path = path;
}

void SyswordlibUpdateHelper::OnDownloadDone(QNetworkReply *reply)
{
    //解压到tmp目录下，进行合并
    disconnect(m_network_manager, SIGNAL(finished(QNetworkReply*)), 0, 0);
    Utils::MakeDir(Utils::GetTmpDir());
    const QString zip_file_path = Utils::GetTmpDir().append("tmp.zip");
    QFile file_tmp(zip_file_path);
    if(file_tmp.exists())
    {
        file_tmp.remove();
    }
    if(!file_tmp.open(QIODevice::WriteOnly))
    {
        qDebug() << "Create File [" << file_tmp.fileName() << "] Faild!";
        //SetStatisUnworking();
        return;
    }
    file_tmp.write(reply->readAll());
    file_tmp.close();

   if(MergeSysWordlib())
       emit DownloadFinished(1);
   else
       emit DownloadFinished(0);

    //ConfigBus::instance()->valueChanged("loadWordlib", "");
}

void SyswordlibUpdateHelper::OnDownloadError(QNetworkReply::NetworkError error_code)
{
    qDebug() << "下载增量包失败，失败原因 : code is " << error_code;
    disconnect(m_network_manager, SIGNAL(finished(QNetworkReply*)), 0, 0);
    emit DownloadFinished(0);
}

bool SyswordlibUpdateHelper::MergeSysWordlib()
{
    FILE* ptr2 = NULL;
    QString uwl_path = Utils::GetTmpDir().append("*.uwl");
    string command = QString(" rm -r %1").arg(uwl_path).toStdString();
    if((ptr2 = popen(command.c_str(), "r")) != NULL)
    {
        pclose(ptr2);
    }

    if(!Utils::DepressedZlibFile(Utils::GetTmpDir().append("tmp.zip"), Utils::GetTmpDir()))
        return false;

    QString sys_tmp_path = Utils::GetTmpDir().append("sys.uwl");
    QFile sys_tmp(sys_tmp_path);
    if(sys_tmp.exists())
        sys_tmp.remove();

    QString sys_wordlib_path = Config::configDirPath().append("wordlib/sys.uwl");
    auto CopySysWordlibToTmp = [&]() ->bool {
        QFile sys_uwl(sys_wordlib_path);
        if(!sys_uwl.exists())
            return false;
        if(!sys_uwl.copy(sys_tmp_path))
            return false;
        return true;
    };
    QString new_tmp(sys_tmp_path);
    new_tmp.append(".new");
    QFile new_tmp_file(new_tmp);
    if(new_tmp_file.exists())
        new_tmp_file.remove();

    if(!CopySysWordlibToTmp())
        return false;

    QProcess* _process = new QProcess();

    for(auto ite = m_uwl_info->begin(); ite!=m_uwl_info->end(); ite++)
    {
        QStringList params;
        params << QString(sys_tmp_path);
        params << QString(Utils::GetTmpDir().append(ite.value().file_name));
        params << QString(sys_tmp_path);

        //one minute
        int exit_code = _process->execute(Utils::GetBinFilePath().append("huayupy-merge-wordlib"), params);
        QString local_md5 = Utils::GetFileMD5(sys_tmp_path);
        if(local_md5.compare(ite.value().md5))
        {
            qDebug() << "合并出错: exit_code:" <<exit_code <<" local_md5 is:" <<local_md5
                     << "remote_md5 is: "<<  ite.value().md5;
            return false;
        }
    }
    Utils::CopyFileForce(sys_tmp_path, sys_wordlib_path);
    return true;
}
