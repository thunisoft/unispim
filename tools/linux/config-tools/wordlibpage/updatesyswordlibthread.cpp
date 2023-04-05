#include "updatesyswordlibthread.h"
#include "myutils.h"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonParseError>
#include <QJsonArray>
#include <QProcess>
#include <QEventLoop>
#include "../public/configmanager.h"


bool SortByIndex(const WORDLIB_BLOCK_INFO& block1, const WORDLIB_BLOCK_INFO& block2)
{
    //return block1.block_index < block2.block_index;
    return true;
}


UpdateSysWordlibThread::~UpdateSysWordlibThread()
{
    if(this->isRunning())
    {
        this->exit(0);
    }
}

void UpdateSysWordlibThread::run()
{
    emit updateFinished(IncreSysWordlibUpdateTask());
}

int UpdateSysWordlibThread::IncreSysWordlibUpdateTask()
{

    //计算本地系统词库MD5
    QString local_sys_wordlib_path = TOOLS::PathUtils::get_user_config_dir_path().append("wordlib/sys.uwl");
    QString insall_wordlib_path = TOOLS::PathUtils::get_install_wordlib_dir_path().append("sys.uwl");

    if(!QFile::exists(local_sys_wordlib_path))
    {
        QFile default_sys(insall_wordlib_path);
        if(default_sys.exists())
            default_sys.copy(local_sys_wordlib_path);
    }

    QString clientid = TOOLS::Utils::get_host_mac();
    int islogin;
    ConfigManager::Instance()->GetIntConfigItem("login",islogin);
    QString loginid;
    if(islogin)
    {
        ConfigManager::Instance()->GetStrConfigItem("loginid",loginid);
    }
    QString system_wordlib_md5 = TOOLS::Utils::get_file_md5(local_sys_wordlib_path);

    QVector<WORDLIB_BLOCK_INFO> wordlib_info_vector;
    int info_code = NetworkHandler::Instance()->check_sys_wordlib_block_info(system_wordlib_md5,wordlib_info_vector);
    int wordlib_block_count = wordlib_info_vector.size();
    TOOLS::LogUtils::write_log_to_file(QString("get block succeed the wordlib block count is %1").arg(wordlib_block_count));

    if((info_code == 0)&(wordlib_block_count > 0))
    {
        //对词库碎片进行排序
        auto index_sort = [=](const WORDLIB_BLOCK_INFO& block1, const WORDLIB_BLOCK_INFO& block2)->bool
        {
            return block1.block_id < block2.block_id;
        };
        qSort(wordlib_info_vector.begin(),wordlib_info_vector.end(),index_sort);

        QString addr_type = wordlib_info_vector.at(0).addr_type;
        if(addr_type != "cdn")
        {
            int ret_code = NetworkHandler::Instance()->DownloadSysWordlibBlockFromServer(clientid,loginid,wordlib_info_vector);

            if(ret_code == 0)
            {
                int ret = MergeSysWordlib(wordlib_info_vector);
                TOOLS::LogUtils::write_log_to_file(QString("download sys wordlib block succeed mergeSyswordlib return %1").arg(ret));
                if(ret != 0)
                {

                    int download_full_code = download_full_syswordlib();
                    TOOLS::LogUtils::write_log_to_file(QString("merge sys wordlib block failed:%1,download full:%2").arg(ret).arg(download_full_code));
                }
                else
                {
                    TOOLS::LogUtils::write_log_to_file(QString("merge sys wordlib block succeed"));
                }
            }
            else
            {
                TOOLS::LogUtils::write_log_to_file(QString("download sys wordlib block errror return%1:").arg(ret_code));
            }
        }
        else
        {
            int ret = DownloadBlockFromCDN(wordlib_info_vector);
            if(ret == 0)
            {
                int ret = MergeSysWordlibCdnBlock(wordlib_info_vector);
                TOOLS::LogUtils::write_log_to_file(QString("MergeSysWordlibCdnBlock return %1").arg(ret));
                if(ret != 0)
                {
                    int download_full_code = download_full_syswordlib();
                    TOOLS::LogUtils::write_log_to_file(QString("download full wordlib return:%1").arg(download_full_code));
                }
                else
                {
                    TOOLS::LogUtils::write_log_to_file(QString("MergeSysWordlibCdnBlock succeed"));
                }

            }
            else
            {
                TOOLS::LogUtils::write_log_to_file("download wordlib block from cdn failed");
            }

        }
    }
    else if(info_code == 1101)
    {
        int ret = download_full_syswordlib();
        TOOLS::LogUtils::write_log_to_file(QString("Download full sys wordlib return code:%1").arg(ret));
    }
    else
    {
        TOOLS::LogUtils::write_log_to_file(QString("check sys wordlib block return:%1").arg(info_code));
    }
}


int UpdateSysWordlibThread::DownloadWordlibAndCombine(QVector<WORDLIB_BLOCK_INFO> block_vector)
{
    /*
    //先按照索引号对词库的碎片进行排序
    qSort(block_vector.begin(),block_vector.end(),SortByIndex);

    QString sys_wordlib_path = Config::Instance()->SystemWordlibPath();
    QString sys_wordlib_tmp_path = sys_wordlib_path + ".blcok.tmp.tools";
    QString sys_wordlib_bak_path = sys_wordlib_path + ".block.bak.tools";

    QString version = Config::Instance()->GetIMEVersion();
    QString clientid = Config::Instance()->GetHostMacAddress();
    QString loginid = Config::Instance()->GetLoginID();

    //排完序之后下载对应的碎片
    int download_code = NetworkHandler::Instance()->DownloadSysWordlibBlockFromServer(version,clientid,loginid,block_vector);
    if(download_code != 0)
    {
        return 1;
    }

    //下载完毕之后对碎片进行合并
    if(QFile::exists(sys_wordlib_tmp_path))
    {
        QFile::remove(sys_wordlib_tmp_path);
    }
    if(QFile::exists(sys_wordlib_bak_path))
    {
        QFile::remove(sys_wordlib_bak_path);
    }
    if(!QFile::exists(sys_wordlib_path))
    {
        return 2;
    }
    if(!QFile::copy(sys_wordlib_path,sys_wordlib_tmp_path))
    {
        return 3;
    }

    QString sys_wordlib_dir = Utils::GetWordlibDirPath();
    for(int index=0; index<block_vector.size(); ++index)
    {
        QString block_name = block_vector.at(index).block_filename;
        QString block_md5 = block_vector.at(index).block_md5;
        QString block_path = sys_wordlib_dir + block_name;
        if(CombineWorlibBlock(sys_wordlib_tmp_path,block_path))
        {
            QString new_file_md5 = Utils::GetFileMD5(sys_wordlib_tmp_path);
            if(new_file_md5 == block_md5)
            {
                if(QFile::exists(block_path))
                {
                    QFile::remove(block_path);
                }
                continue;

            }
            else
            {
                QFile::remove(sys_wordlib_tmp_path);
                return 2;
            }
        }
        else
        {
            return 2;
        }
    }

    bool rename_flag1 = QFile::rename(sys_wordlib_path,sys_wordlib_bak_path);
    bool rename_flag2 = QFile::rename(sys_wordlib_tmp_path,sys_wordlib_path);
    bool remove_flag = QFile::remove(sys_wordlib_bak_path);
    if(rename_flag1 && rename_flag2 && remove_flag)
    {
        return 0;
    }
    Utils::WriteLogToFile("rename operation after combined failed");
    return 9;
    */
    return 0;

}

int UpdateSysWordlibThread::update_profess_wordlib()
{
    return 0;
}

bool UpdateSysWordlibThread::CombineWorlibBlock(QString sys_wordlib_path, QString block_path)
{
    /*
    QString install_dir = Config::Instance()->GetInstallDir();
    QString wordlib_tools_path;
    if(install_dir.isEmpty())
    {
        return false;
    }
    else
    {
        wordlib_tools_path = install_dir + "\\WordlibTool.exe";
    }
    if(!QFile::exists(wordlib_tools_path))
    {
        return false;
    }
    QStringList argumentList;
    argumentList << sys_wordlib_path << block_path;

    QProcess process;
    process.setProgram(wordlib_tools_path);
    process.setArguments(argumentList);
    process.start();
    return process.waitForFinished(5000);
    */
    return true;
}

void UpdateSysWordlibThread::CleanSystmpBlock(QVector<WORDLIB_BLOCK_INFO> block_vector)
{
    /*
    QString wordlib_dir = Utils::GetWordlibDirPath();
    for(WORDLIB_BLOCK_INFO index_info : block_vector)
    {
        QString block_name = index_info.block_filename;
        QString wordlib_path = wordlib_dir + block_name;
        if(QFile::exists(wordlib_path))
        {
            QFile::remove(wordlib_path);
        }
    }
    */
}

int UpdateSysWordlibThread::check_sys_wordlib_update(QVector<WORDLIB_BLOCK_INFO>& wordlib_info_vector)
{
    /*
    QNetworkRequest request;
    QString check_url = get_wordlib_update_url();
    request.setUrl(check_url);
    qDebug() << check_url;
    request.setSslConfiguration(Utils::GetQsslConfig());
    QNetworkAccessManager* network_manager = new QNetworkAccessManager(this);
    QNetworkReply* reply = network_manager->get(request);

    QReplyTimeout *pTimeout = new QReplyTimeout(reply,20000);
    QEventLoop eventLoop;
    connect(reply, SIGNAL(finished()),&eventLoop, SLOT(quit()));
    connect(pTimeout, SIGNAL(timeout()),&eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    if(pTimeout->GetTimeoutFlag())
    {
        return 1;
    }

    QVariant http_request_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if(http_request_code.toInt() != 200)
    {
        return 2;
    }

    QJsonParseError json_error;
    QJsonDocument json_doc(QJsonDocument::fromJson(reply->readAll(), &json_error));
    if(json_error.error != QJsonParseError::NoError)
    {
        return 3;
    }

    QJsonObject json_object_root = json_doc.object();
    QString code = json_object_root.value("code").toString();
    if(code == "1101")
    {
        return 1101;
    }
    else if(code == "200")
    {
        QJsonArray result_array = json_object_root.value("result").toArray();
        if( result_array.size() <= 0)
        {
            return 0;
        }

        for(int i = 0; i < result_array.size(); i++)
        {
            QJsonObject object_uwl_info = result_array.at(i).toObject();
            if(!object_uwl_info.contains("fileName") || !object_uwl_info.contains("block")
                    ||!object_uwl_info.contains("fulldictmd5"))
            {
                WriteLogToFile("wordlib block json info is not valid");
                return 4;
            }
            else
            {
                WORDLIB_BLOCK_INFO uwl_info;
                uwl_info.block_id = object_uwl_info.value("block").toInt();
                uwl_info.file_name = object_uwl_info.value("fileName").toString();
                uwl_info.md5 = object_uwl_info.value("fulldictmd5").toString();

                if(object_uwl_info.contains("addrtype") && object_uwl_info.contains("addr") && object_uwl_info.contains("md5"))
                {
                    uwl_info.addr_type = object_uwl_info.value("addrtype").toString();
                    uwl_info.block_url = object_uwl_info.value("addr").toString();
                    uwl_info.block_md5 = object_uwl_info.value("md5").toString();
                }
                wordlib_info_vector.push_back(uwl_info);

            }
        }
        return 0;
    }
    else
    {
        QString insall_wordlib_path = Utils::GetHuayuPYInstallDirPath().append("wordlib/sys.uwl");
        if(!QFile::exists(insall_wordlib_path))
        {
           WriteLogToFile(QString("ERROR-----------------安装目录系统词库丢失！"));
           return 1101;
        }
        WriteLogToFile(QString("sys wordlib get increase info return:%1").arg(code));
        return 5;
    }*/
    return 0;
}

int UpdateSysWordlibThread::MergeSysWordlib(QVector<WORDLIB_BLOCK_INFO> block_vector)
{
    FILE* ptr2 = NULL;
    QString uwl_path = Utils::GetTmpDir().append("*.uwl");
    std::string command = QString(" rm -r %1").arg(uwl_path).toStdString();
    if((ptr2 = popen(command.c_str(), "r")) != NULL)
    {
        pclose(ptr2);
    }
    QString zip_package_path = Utils::GetTmpDir().append("tmp.zip");
    QString zip_dest_path = Utils::GetTmpDir();
    if(!Utils::DepressedZlibFile(zip_package_path,zip_dest_path))
        return 1;

    QString sys_tmp_path = Utils::GetTmpDir().append("sys.uwl");
    QFile sys_tmp(sys_tmp_path);
    if(sys_tmp.exists())
        sys_tmp.remove();

    QString sys_wordlib_path = TOOLS::PathUtils::get_user_config_dir_path().append("wordlib/sys.uwl");

    auto CopySysWordlibToTmp = [&]() ->bool {
        QFile sys_uwl(sys_wordlib_path);
        if(!sys_uwl.exists())
            return false;
        if(!sys_uwl.copy(sys_tmp_path))
            return false;
        return true;
    };

    if(!CopySysWordlibToTmp())
        return 2;

    QProcess* _process = new QProcess();

    for(auto iter = block_vector.begin(); iter!=block_vector.end(); iter++)
    {
        QStringList params;
        params << QString(sys_tmp_path);
        params << QString(Utils::GetTmpDir().append(iter->file_name));
        params << QString(sys_tmp_path);

        //one minute
        _process->execute(Utils::GetBinFilePath().append("huayupy-merge-wordlib"), params);
        QString local_md5 = Utils::GetFileMD5(sys_tmp_path);
        if(local_md5.compare(iter->md5) != 0)
        {
            return 3;
        }
    }
    Utils::CopyFileForce(sys_tmp_path, sys_wordlib_path);
    return 0;
}



int UpdateSysWordlibThread::download_full_syswordlib()
{

    QString local_sys_wordlib_path = TOOLS::PathUtils::get_user_config_dir_path().append("wordlib/sys.uwl");
    QString system_wordlib_md5 = TOOLS::Utils::get_file_md5(local_sys_wordlib_path);

    NetworkHandler network_handler;
    QString download_url, package_md5;
    int ret = network_handler.check_system_wordlib_update(system_wordlib_md5,download_url,package_md5);

    if((ret == 0) && !package_md5.isEmpty() && !download_url.isEmpty())
    {

        int ret = network_handler.download_full_sys_wordlib(download_url,package_md5);
        if(ret == 0)
        {
            return 0;
        }
        else
        {
            TOOLS::LogUtils::write_log_to_file(QString("download full wordlib error:%1:%2").arg(ret));
        }
    }
    else if(ret == 0)
    {
        return 0;
    }
    else
    {
        TOOLS::LogUtils::write_log_to_file(QString("full wordlib check error code:%1").arg(ret));
        return 2;
    }
}


int UpdateSysWordlibThread::DownloadBlockFromCDN(QVector<WORDLIB_BLOCK_INFO>& wordlib_info_vector)
{
    try
    {
        if(wordlib_info_vector.isEmpty())
        {
            return 9;
        }

        int array_size = wordlib_info_vector.size();
        for(int index=0; index<array_size; ++index)
        {
            QString block_addr_type = wordlib_info_vector.at(index).addr_type;
            if(!block_addr_type.isEmpty() && (block_addr_type == "cdn"))
            {
                QString download_url = wordlib_info_vector.at(index).block_url;
                QString block_md5 = wordlib_info_vector.at(index).block_md5;
                QString file_name = wordlib_info_vector.at(index).file_name;
                int ret = DownloadFileFromCdnUrl(download_url,block_md5);
                if(ret != 0)
                {
                    TOOLS::LogUtils::write_log_to_file(QString("download file:%1 from CND error").arg(file_name));
                    return 1;
                }
            }
        }

        return 0;
    }
    catch(std::exception& e)
    {
        return 5;
    }
}

int UpdateSysWordlibThread::DownloadFileFromCdnUrl(QString &download_url, QString file_md5)
{
    //通过返回的请求头中获得文件的名称
    QNetworkRequest request;
    QUrl url(download_url);
    request.setSslConfiguration(Utils::GetQsslConfig());

    QNetworkAccessManager *accessManager=new QNetworkAccessManager(this);
    request.setUrl(url);
    QNetworkReply *reply  = accessManager->get(request);

    QReplyTimeout *pTimeout = new QReplyTimeout(reply,20000);
    QEventLoop eventLoop;
    connect(reply, SIGNAL(finished()),&eventLoop, SLOT(quit()));
    connect(pTimeout, SIGNAL(timeout()),&eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (statusCode.toInt() != 200)
    {
        return 2;
    }
    QFileInfo url_file_info(download_url);
    QString file_name = url_file_info.fileName();
    QString file_save_path = Utils::GetTmpDir() + file_name;
    QFile file(file_save_path);
    if(file.exists())
    {
        file.remove();
    }
    if(!file.open(QIODevice::WriteOnly))
    {
        Utils::WriteLogToFile(QString("%1errorInfo:%2").arg(file_name).arg(file.errorString()));
        return 4;
    }
    file.write(reply->readAll());
    file.close();
    QString download_file_md5 = TOOLS::Utils::get_file_md5(file_save_path);
    if(download_file_md5 == file_md5)
    {
        return 0;
    }
    return 10;
}

int UpdateSysWordlibThread::MergeSysWordlibCdnBlock(QVector<WORDLIB_BLOCK_INFO> block_vector)
{

    QString sys_tmp_path = Utils::GetTmpDir().append("sys.uwl");
    QFile sys_tmp(sys_tmp_path);
    if(sys_tmp.exists())
        sys_tmp.remove();

    QString sys_wordlib_path = TOOLS::PathUtils::get_user_config_dir_path().append("wordlib/sys.uwl");

    auto CopySysWordlibToTmp = [&]() ->bool {
        QFile sys_uwl(sys_wordlib_path);
        if(!sys_uwl.exists())
            return false;
        if(!sys_uwl.copy(sys_tmp_path))
            return false;
        return true;
    };

    if(!CopySysWordlibToTmp())
        return 2;

    QProcess* _process = new QProcess();

    for(auto iter = block_vector.begin(); iter!=block_vector.end(); iter++)
    {
        QStringList params;
        params << QString(sys_tmp_path);
        params << QString(Utils::GetTmpDir().append(iter->file_name));
        params << QString(sys_tmp_path);

        //one minute
        _process->execute(Utils::GetBinFilePath().append("huayupy-merge-wordlib"), params);
        QString local_md5 = Utils::GetFileMD5(sys_tmp_path);
        if(local_md5.compare(iter->md5) != 0)
        {
            return 3;
        }
    }
    Utils::CopyFileForce(sys_tmp_path, sys_wordlib_path);
    return 0;
}
