#include "updatesyswordlibthread.h"
#include <QFile>
#include <QDir>
#include "utils.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonParseError>
#include <QJsonArray>
#include <QProcess>


bool SortByIndex(const WORDLIB_BLOCK_INFO& block1, const WORDLIB_BLOCK_INFO& block2)
{
    return block1.block_index < block2.block_index;
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

    // 判断系统词库存不存在
    QString sys_wordlib_path = Config::SystemWordlibPath();
    QString clientid = Config::GetHostMacAddress();
    QString loginid = Config::Instance()->GetLoginID();

    if(QFile::exists(sys_wordlib_path))
    {
        QString file_md5 = Utils::GetFileMD5(sys_wordlib_path);
        QString block_info;
        int info_code = NetworkHandler::Instance()->GetSysWordlibIncreInfo(file_md5,block_info);
        if(info_code == 0)
        {

            QString block_return_code;
            QVector<WORDLIB_BLOCK_INFO> block_vector = ParserStrToWordlibInfo(block_info,block_return_code);
            // 词库的md5值在CRC列表中不存在的时候直接下载一个全量
            if(block_return_code == "1101")
            {
                Utils::WriteLogToFile("Get system wordlib return Code 1101");
                return NetworkHandler::Instance()->DownloadFullSysWordlib(clientid,loginid,file_md5);
            }
            else if(block_return_code == "200")
            {
                if(block_vector.size() == 0)
                {
                    Utils::WriteLogToFile("the wordlib block num is 0");
                    return 0;
                }
                else
                {
                   int combine_return_code = DownloadWordlibAndCombine(block_vector);
                   if( combine_return_code == 0)
                   {
                       Utils::WriteLogToFile("combine the wordlib block succeed");
                       return 0;
                   }
                   else if(combine_return_code == 9)
                   {
                       return 9;
                   }
                   else
                   {
                       Utils::WriteLogToFile("combine the wordlib block failed");
                       CleanSystmpBlock(block_vector);
                       return NetworkHandler::Instance()->DownloadFullSysWordlib(clientid,loginid,file_md5);
                   }
                }

            }
            else
            {
                return 1;
            }
        }
        else
        {
            Utils::WriteLogToFile("Get Increase wordlibinfo failed");
            return 1;
        }


    }
    else
    {
        Utils::WriteLogToFile("the sys.uwl doesn't exist");
        //不存在直接全量更新
        return NetworkHandler::Instance()->DownloadFullSysWordlib(clientid,loginid,Utils::GetStrMd5("-1"));

    }
}

QVector<WORDLIB_BLOCK_INFO> UpdateSysWordlibThread::ParserStrToWordlibInfo(QString wordlibInfo,QString& returnCode)
{
    QJsonParseError error;
    QVector<WORDLIB_BLOCK_INFO> result;
    returnCode = "400";
    try {
        QJsonDocument document = QJsonDocument::fromJson(wordlibInfo.toUtf8(),&error);
        if(error.error != QJsonParseError::NoError)
        {
            return result;
        }
        if(document.isObject())
        {
            QJsonObject root = document.object();
            if(root.contains("code"))
            {
                returnCode = root.value("code").toString();
            }
            if(root.contains("result"))
            {
                QJsonArray resultArray = root.value("result").toArray();
                for(QJsonValue value : resultArray)
                {
                    if(!value.isNull())
                    {
                        WORDLIB_BLOCK_INFO temp_block;
                        temp_block.block_filename = value.toObject().value("fileName").toString();
                        temp_block.block_md5 = value.toObject().value("fulldictmd5").toString();
                        temp_block.block_index = value.toObject().value("block").toInt();
                        result.push_back(temp_block);
                    }
                }
            }
            return result;

        }
        else
        {
            return result;
        }

    } catch (...) {
        return result;
    }


}

int UpdateSysWordlibThread::DownloadWordlibAndCombine(QVector<WORDLIB_BLOCK_INFO> block_vector)
{
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

}

int UpdateSysWordlibThread::update_profess_wordlib()
{
    return 0;
}

bool UpdateSysWordlibThread::CombineWorlibBlock(QString sys_wordlib_path, QString block_path)
{
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
}

void UpdateSysWordlibThread::CleanSystmpBlock(QVector<WORDLIB_BLOCK_INFO> block_vector)
{
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
}


