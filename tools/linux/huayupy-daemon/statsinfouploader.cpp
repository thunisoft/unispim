#include "statsinfouploader.h"
#include "../public/config.h"
#include "uploadcache.h"
#include "../public/utils.h"

#include <QSettings>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkReply>
#include <QUrl>
#include <QJsonObject>
#include <QVector>
#include <QHttpPart>
#include <QEventLoop>
#include <QDir>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>
#include <QNetworkInterface>
#include "networkhandler.h"


QString StatsInfoUploader::GetStatsFileAddr(QString loginID)
{
    if(loginID.isEmpty())
    {
        loginID = "default";
    }

    QString stats_file_addr = Config::GetStatsDir().append("huayupy-stats-%1").arg(loginID);
    return stats_file_addr;
}


void StatsInfoUploader::run()
{
    WriteLog(QString("<<<---------StatsInfo Update Task Start--------->>>"));
    QSettings config(Config::StateConfigPath(), QSettings::IniFormat);
    int time_interval = config.value("timer/time_interval", 15).toInt();
    WriteLog(QString("statisinofuploadertask initialized time:%1min").arg(time_interval));
    QString clientid = Utils::GethostMac();
    QString server_addr = Config::Instance()->GetServerUrl();
    WriteLog(QString("current clientid:%1 server addr is:%2").arg(clientid).arg(server_addr));

    do
    {
        try
        {
             WriteLog("-----------------------------------");

             int today_input_total_count = GetTodayInputCount();
             int already_input_count = GetTodayAlreadyInputCount();
             int wait_to_upload_count = today_input_total_count - already_input_count;
             WriteLog(QString("today input is:%1 already upload:%2 wait to uploadcount is:%3").arg(today_input_total_count).arg(already_input_count)
                                   .arg(wait_to_upload_count));

             bool has_uploaded_flag = HasUploadTodayInput();
             if(has_uploaded_flag)
             {
                 WriteLog("today has already uploaded input count");
             }
             else
             {
                 if((wait_to_upload_count > 0))
                 {
                     QString timestamp = QDateTime::currentDateTime().toString("yyyy-M-d hh:mm:ss");
                     int ret = UploadInputInfo(timestamp,wait_to_upload_count);
                     if(ret == 0)
                     {
                         UpdateAlreadyUploadCount(wait_to_upload_count);
                         WriteLog(QString("Update Local bak Record timeStamp is:%1,").arg(timestamp));
                     }
                     else
                     {
                         WriteLog(QString("upload statistic info error returncode:%1 Msg:%2").arg(ret).arg(Errorcode_Map[ret]));
                     }

                 }
                 else
                 {
                    WriteLog("today don't have input count to upload");
                 }
             }

             check_history_input();
        }
        catch(std::exception& e)
        {
            WriteLog(QString("uploadstatsinfotask throw exception:%1").arg(QString(e.what())));
        }
        task_sleep();

    }while(1);

}


int StatsInfoUploader::GetStatsResult(QString inputTime)
{
    //get stats time
    QString loginid;
    QDateTime tempTime = QDateTime::fromString(inputTime, "yyyy-M-d-H");
    if(!tempTime.isValid())
    {
        return 0;
    }
    int search_hour = tempTime.toString("H").toInt();
    QString search_day = tempTime.toString("yyyy-M-d");

    if(!Config::GetClientInfo(loginid))
    {
        loginid = "default";
    }
    QString statsFileAddr = GetStatsFileAddr(loginid);

    QSettings setting(statsFileAddr, QSettings::IniFormat);
    setting.beginGroup("InputInfo");
    QStringList keyList = setting.childKeys();
    if(!keyList.contains(search_day))
    {
        return 0;
    }
    QString today_input = setting.value(search_day).toString();
    setting.endGroup();
    QStringList today_record = today_input.split("#");
    if(today_record.size() != 25)
    {
        return 0;
    }

    QString hourStr = today_record.at(search_hour + 1);
    int input_num = hourStr.toInt();
    return input_num;
}



void StatsInfoUploader::WriteLog(const QString inputStr)
{
    QString logdir = Config::configDirPath().append("logs/");
    QString loginid;
    bool is_login = Config::Instance()->IsLogin();
    if(is_login)
    {
        loginid = Config::Instance()->GetLoginId();
    }

    QDir dir(logdir);
    if(!dir.exists())
    {
        dir.mkpath(logdir);
    }
    QString logFileAddr = logdir + "huayupy-statstask-log";
    QString timeStamp = QDateTime::currentDateTime().toString("yyyy-M-d hh:mm:ss");
    QFile file(logFileAddr);
    bool isSucess = file.open(QIODevice::WriteOnly | QIODevice::Append);
    if(!isSucess)
    {
        return;
    }

    QTextStream output(&file);
    QString content = timeStamp + "-" + loginid + ":" + inputStr + "\n" ;
    output << content;
    file.close();
}

bool StatsInfoUploader::HasUploadTodayInput()
{
    int today_input = GetTodayAlreadyInputCount();
    if(today_input > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

QString StatsInfoUploader::GetRecordFilePath()
{
    bool is_login = Config::Instance()->IsLogin();
    QString loginid;
    if(is_login)
    {
        loginid = Config::Instance()->GetLoginId();
    }
    else
    {
        loginid = "default";
    }
    QString recordIniFileAddr = Config::configDirPath().append("upload-record-%1.ini").arg(loginid);
    return recordIniFileAddr;
}

int StatsInfoUploader::GetTodayAlreadyInputCount()
{
    QString record_file_path = GetRecordFilePath();
    QSettings setting(record_file_path,QSettings::IniFormat);
    setting.beginGroup("bakinputcount");
    QString currentTimeStr = QDateTime::currentDateTime().toString("yyyy-M-d");
    int already_inputcount = setting.value(currentTimeStr,0).toInt();
    setting.endGroup();
    return already_inputcount;
}

int StatsInfoUploader::GetTodayInputCount()
{
    //get stats time
    QString loginid;
    QString search_day = QDateTime::currentDateTime().toString("yyyy-M-d");
    if(!Config::GetClientInfo(loginid))
    {
        loginid = "default";
    }
    QString statsFileAddr = GetStatsFileAddr(loginid);

    QSettings setting(statsFileAddr, QSettings::IniFormat);
    setting.beginGroup("InputInfo");
    QStringList keyList = setting.childKeys();
    if(!keyList.contains(search_day))
    {
        return 0;
    }
    QString today_input = setting.value(search_day).toString();
    setting.endGroup();
    QStringList today_record = today_input.split("#");
    if(today_record.size() != 25)
    {
        return 0;
    }

    QString hourStr = today_record.at(0);
    int input_num = hourStr.toInt();
    return input_num;
}

int StatsInfoUploader::UploadInputInfo(QString timeStamp, int inputCount)
{
    QString loginid;
    Config::GetClientInfo(loginid);

    QString version = Utils::GetVersion();
    int os_index = Utils::GetOSTypeIndex();
    QString clientid = Utils::GethostMac();
    QString server_url = Config::Instance()->GetServerUrl();
    QNetworkAccessManager *uploadmanager = new QNetworkAccessManager(this);
    QString url = QString("%1//testapi/requesturl").arg(server_url);
    Utils::AddSystemExtraParamer(url,OPType::AUTO);

    QNetworkRequest request;
    request.setUrl((QUrl(url)));
    request.setSslConfiguration(Utils::GetQsslConfig());
    QNetworkReply *reply = uploadmanager->get(request);

    QReplyTimeout *pTimeout = new QReplyTimeout(reply,60000);
    QEventLoop loop;
    connect(uploadmanager, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
    connect(pTimeout, SIGNAL(timeout()),&loop, SLOT(quit()));
    loop.exec();

    if(pTimeout->GetTimeoutFlag())
    {
        return 1;
    }

    QNetworkReply::NetworkError error = reply->error();
    if(QNetworkReply::NoError != error)
    {
        QString errorStr = QString(" %1 Upload input info send Request Error: %2").arg(loginid).arg(error);
        WriteLog(errorStr);
        return 2;
    }
    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (statusCode.isValid())
    {
        if(statusCode.toInt() == 200)
        {
            WriteLog(QString("user:%1 upload:%2 at%3 succeed").arg(loginid).arg(inputCount).arg(timeStamp));
            return 0;
        }
        return 99;
    }
    else
    {
        QString errorStr = QString("%1 Upload failed statusCode:%2").arg(loginid).arg(statusCode.toInt());
        WriteLog(errorStr);
        return 7;
    }

}

void StatsInfoUploader::UpdateAlreadyUploadCount(int uploadCount)
{
    QString record_file_path = GetRecordFilePath();
    QSettings setting(record_file_path,QSettings::IniFormat);
    setting.beginGroup("bakinputcount");
    QString currentTimeStr = QDateTime::currentDateTime().toString("yyyy-M-d");
    int already_inputcount = setting.value(currentTimeStr,0).toInt();
    int new_inputcount = already_inputcount + uploadCount;
    setting.setValue(currentTimeStr,new_inputcount);
    setting.endGroup();

}

int StatsInfoUploader::check_history_input()
{
    std::map<std::string, int> input_map;
    std::map<std::string, int > already_input_map;
    std::map<std::string, int> to_upload_map;
    input_map.clear();
    already_input_map.clear();
    to_upload_map.clear();

    QString loginid;
    if(!Config::GetClientInfo(loginid))
    {
        loginid = "default";
    }
    QString statsFileAddr = GetStatsFileAddr(loginid);

    QFile statsFile(statsFileAddr);
    if (!statsFile.exists())
    {
        WriteLog(QString("%1:%2 file doesn't exist").arg(QString(__FUNCTION__)).arg(statsFileAddr));
        return 1;
    }

    QSettings input_ini_config(statsFileAddr,QSettings::IniFormat);

    input_ini_config.beginGroup("InputInfo");
    QStringList subkeys = input_ini_config.childKeys();
    int input_key_count = subkeys.size();
    for (int index = 0; index < input_key_count; ++index)
    {
        QString subindex_key = subkeys.at(index);
        QString today_input = input_ini_config.value(subindex_key).toString();
        QStringList today_record = today_input.split("#");
        if(today_record.size() != 25)
        {
             input_map.insert(std::make_pair(subindex_key.toStdString(),0));
        }
        else
        {
            QString hourStr = today_record.at(0);
            int input_num = hourStr.toInt();
            input_map.insert(std::make_pair(subindex_key.toStdString(),input_num));
        }

    }

    QString record_file_path = GetRecordFilePath();
    QFile already_upload_file(record_file_path);
    QSettings already_upload_iniconfig(record_file_path,QSettings::IniFormat);
    if (already_upload_file.exists())
    {
        already_upload_iniconfig.beginGroup("bakinputcount");
        QStringList subkeys = already_upload_iniconfig.childKeys();

        int input_key_count = subkeys.size();
        for (int index = 0; index < input_key_count; ++index)
        {
            QString subindex_key = subkeys.at(index);
            int input_value = already_upload_iniconfig.value(subindex_key,0).toInt();
            already_input_map.insert(std::make_pair(subindex_key.toStdString(),input_value));
        }
        already_upload_iniconfig.endGroup();
    }

    //差值上传
    auto input_iterator = input_map.begin();
    for (input_iterator; input_iterator != input_map.end(); input_iterator++)
    {
        string date_key = input_iterator->first;
        int input_value = input_iterator->second;
        if (already_input_map.count(date_key) == 1)
        {
            int already_upload_count = already_input_map.at(date_key);
            if (input_value > already_upload_count)
            {
                int to_upload_count = input_value - already_upload_count;
                to_upload_map.insert(std::make_pair(date_key,to_upload_count));
            }
        }
        else
        {
            if (input_value > 0)
            {
                to_upload_map.insert(std::make_pair(date_key,input_value));
            }
        }

    }
    auto to_upload_interator = to_upload_map.begin();
    string to_upload_days_set_str;
    for (to_upload_interator; to_upload_interator != to_upload_map.end(); to_upload_interator++)
    {
        string date_key = to_upload_interator->first;
        string date_value = std::to_string(to_upload_interator->second);
        to_upload_days_set_str += date_key;
        to_upload_days_set_str += string("-");
        to_upload_days_set_str += date_value;
        to_upload_days_set_str += ";";
    }
    int to_upload_size = to_upload_map.size();

    WriteLog(QString("%1:have:%2days history input to upload:%3").arg(loginid).arg(to_upload_size).arg(QString::fromStdString(to_upload_days_set_str)));
    UploadMultiHistoryRecord(to_upload_map);
    return 0;
}

int StatsInfoUploader::UploadMultiHistoryRecord(std::map<string, int> history_result)
{
    //没有数据不上传
    QString loginID = Config::Instance()->GetLoginId();
    if(loginID.isEmpty())
    {
        loginID = "default";
    }

    if (history_result.size() == 0)
    {
        WriteLog(QString("%1 don't have history input data").arg(loginID));
        return 0;
    }

    //只有今天的数据不上传
    if (history_result.size() == 1)
    {
        QString currentDate = GetCurrentDate();
        if (history_result.begin()->first == currentDate.toStdString())
        {
            WriteLog("only have today input don't upload history input");
            return 0;
        }
    }

    QString upload_count_record_file_path = GetRecordFilePath();
    QSettings already_upload_iniconfig(upload_count_record_file_path, QSettings::IniFormat);
    already_upload_iniconfig.beginGroup("bakinputcount");

    std::map<string, int> upload_paramer;
    upload_paramer.clear();

    auto begin_iterator = history_result.begin();
    for (begin_iterator; begin_iterator != history_result.end(); ++begin_iterator)
    {
        std::string date_key = begin_iterator->first;
        int input_value = begin_iterator->second;
        string upload_time_key = date_key + QDateTime::currentDateTime().toString(" hh:mm:ss").toStdString();
        upload_paramer.insert(std::make_pair(upload_time_key,input_value));
    }

    int paramer_size = upload_paramer.size();
    if(paramer_size > 0)
    {
        WriteLog(QString("%1 have history input data:%2").arg(loginID).arg(paramer_size));
    }
    else
    {
        WriteLog(QString("paraerm_size is unvalid 0"));
        return 1;
    }

    int ret = UploadHistoryInputInfo(upload_paramer);
    if (ret == 0)
    {
        auto iterator = history_result.begin();
        for (iterator; iterator != history_result.end(); ++iterator)
        {
            std::string date_key = iterator->first;
            int input_value = iterator->second;

            int prev_number = already_upload_iniconfig.value(QString::fromStdString(date_key),0).toInt();
            already_upload_iniconfig.setValue(QString::fromStdString(date_key), input_value + prev_number);
        }
        already_upload_iniconfig.endGroup();
        WriteLog("update local history inputcount record succeed");
    }
    else
    {
        WriteLog(QString("upload history input data error:%1").arg(ret).arg(Errorcode_Map[ret]));
        return 2;
    }
    return 0;
}

QString StatsInfoUploader::GetCurrentDate()
{
    QString currentTimeStr = QDateTime::currentDateTime().toString("yyyy-M-d");
    return currentTimeStr;
}

int StatsInfoUploader::UploadHistoryInputInfo(std::map<string, int> history_result)
{
    QJsonArray input_arry;
    auto iter = history_result.begin();
    for (iter; iter != history_result.end(); ++iter)
    {
        QJsonObject input_count_obj;
        input_count_obj.insert("date", QString::fromStdString(iter->first));
        input_count_obj.insert("inputcount", QString::number(iter->second));
        input_arry.append(input_count_obj);
    }

    QJsonDocument document(input_arry);
    QString inputCount_str = QString(document.toJson());
    QString mac = Utils::GethostMac();
    QString version = Utils::GetVersion();
    QString loginID;
    if(Config::Instance()->IsLogin())
    {
        loginID = Config::Instance()->GetLoginId();
    }
    QString localIPAddr = GetlocalIpAddr();
    QString server_domain = Config::Instance()->GetServerUrl();
    int os_index = Utils::GetOSTypeIndex();

    QString  request_url = QString("%1/testapi/requesturl").arg(server_domain);
    Utils::AddSystemExtraParamer(request_url,OPType::AUTO);
    try
    {
        QNetworkRequest request;
        request.setUrl((QUrl(request_url)));
        request.setSslConfiguration(Utils::GetQsslConfig());
        QNetworkAccessManager* uploadManager = new QNetworkAccessManager(this);
        QNetworkReply *reply = uploadManager->get(request);

        QReplyTimeout *pTimeout = new QReplyTimeout(reply,60000);
        QEventLoop loop;
        connect(uploadManager, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
        connect(pTimeout, SIGNAL(timeout()),&loop, SLOT(quit()));
        loop.exec();
        if(pTimeout->GetTimeoutFlag())
        {
            return 1;
        }

       QNetworkReply::NetworkError error = reply->error();
       if(QNetworkReply::NoError != error)
       {
           return 2;
       }

       QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
       if (statusCode.isValid())
       {
           if(statusCode.toInt() == 200)
           {
               WriteLog("upload history input succeed");
               return 0;
           }
       }
       else
       {
           WriteLog(QString("upload history input reply statuscode is unvalid:%1").arg(statusCode.toInt()));
           return 7;
       }


    }
    catch (const std::exception &ex)
    {
        WriteLog(QString("upload history statistic info error throw exception:%1").arg(ex.what()));
        return 5;
    }
    return 99;
}

QString StatsInfoUploader::GetlocalIpAddr()
{

    QString strIpAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // 获取第一个本主机的IPv4地址
    int nListSize = ipAddressesList.size();
    for (int i = 0; i < nListSize; ++i)
    {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
                ipAddressesList.at(i).toIPv4Address()) {
            strIpAddress = ipAddressesList.at(i).toString();
            break;
        }
    }
    // 如果没有找到，则以本地IP地址为IP
    if (strIpAddress.isEmpty())
        strIpAddress = QHostAddress(QHostAddress::LocalHost).toString();
    return strIpAddress;


}

void StatsInfoUploader::task_sleep()
{
    QSettings config(Config::StateConfigPath(), QSettings::IniFormat);
    int min_time_interval = config.value("timer/time_interval", 15).toInt();
    QThread::msleep(min_time_interval*60*1000);
}

