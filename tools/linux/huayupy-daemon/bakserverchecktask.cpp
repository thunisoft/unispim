#include "bakserverchecktask.h"
#include <QSettings>
#include <QDir>
#include <QJsonParseError>
#include <QJsonObject>
#include "configbus.h"


void BakServerCheckTask::run()
{
    do
    {
        if(is_able_to_check())
        {
            check_to_get_valid_addr();
            RecordCheckTime();
        }
        else
        {
            WriteLog("today has checked the valid server addr");
        }
        task_sleep();
    }
    while(1);
}


void BakServerCheckTask::check_to_get_valid_addr()
{
    //get addr list info
    QString addr_list_file_path = Config::Instance()->GetServerAddrListFilePath();
    if(!QFile::exists(addr_list_file_path))
    {
        WriteLog(QString("%1 file doesn't exist").arg(addr_list_file_path));
        return;
    }

    //get valid addr_info
    QVector<ADDR_INFO> addrinfo_list = parser_addr_list(addr_list_file_path);
    if(addrinfo_list.size() > 0)
    {
        int size_count = addrinfo_list.size();
        for(int index=0; index<size_count; ++index)
        {
            ADDR_INFO current_info = addrinfo_list.at(index);
            QString url = QString("%1://%2:%3").arg(current_info.scheme).arg(current_info.addr).arg(current_info.port);

            NetworkHandler handler;
            QString version = Utils::GetVersion();
            int os_index = Utils::GetOSTypeIndex();
            QString clientid = "ecde826abef3d7a147e234d49620d11c";
            QString timestamp = QDateTime::currentDateTime().toString("yyyy-M-d hh:mm:ss");
            int ret_code = handler.test_network_connection(url,version,os_index,clientid,timestamp);
            if(ret_code == 0)
            {
                WriteLog(QString("domain:%1 request succeed").arg(url));
                QString server_url = Config::Instance()->GetServerUrl();
                if(server_url != url)
                {
                    //record the finded server addr including scheme addr and port
                    Config::Instance()->set_domain(current_info.scheme,current_info.addr,current_info.port);
                    Config::Instance()->SaveLocalServerAddr();

                    //record current type fayuantype or standardtype
                    Config::Instance()->SetCurrentClientType(current_info.name);
                    ConfigBus::instance()->valueChanged("loadWordlib", ""); //通知引擎加载词库
                    Config::Instance()->LoadConfig();
                }
                return;
            }
        }
        WriteLog(QString("doesn't have valid server addr"));
    }
    else
    {
        WriteLog(QString("addr_list.json don't have addr"));
    }
}



void BakServerCheckTask::task_sleep()
{
    QSettings config(Config::StateConfigPath(), QSettings::IniFormat);
    int min_time_interval = config.value("timer/time_interval", 60*6).toInt();
    WriteLog(QString("time interval is %1 min").arg(min_time_interval));
    QThread::msleep(min_time_interval*60*1000);
}

void BakServerCheckTask::WriteLog(const QString inputStr)
{
    QString logdir = Config::configDirPath().append("logs/");
    QDir dir(logdir);
    if(!dir.exists())
    {
        dir.mkpath(logdir);
    }
    QString logFileAddr = logdir + "huayupy-bakchecktask-log";
    QString timeStamp = QDateTime::currentDateTime().toString("yyyy-M-d hh:mm:ss");
    QFile file(logFileAddr);
    bool isSucess = file.open(QIODevice::WriteOnly | QIODevice::Append);
    if(!isSucess)
    {
        return;
    }
    QTextStream output(&file);
    QString content = timeStamp + ":" + inputStr + "\n" ;
    output << content;
    file.close();
}

void BakServerCheckTask::RecordCheckTime()
{
    QString file_path = Config::configDirPath().append("server.ini");
    QSettings ip_config(file_path,QSettings::IniFormat);
    ip_config.beginGroup("check");
    QString  time_str = QDateTime::currentDateTime().toString("yyyy-M-d");
    ip_config.setValue("lastchecktime",time_str);
    ip_config.endGroup();
}

bool BakServerCheckTask::is_able_to_check()
{
    QString file_path = Config::configDirPath().append("server.ini");
    QSettings addr_config(file_path,QSettings::IniFormat);
    addr_config.beginGroup("check");
    QString record_time_str = addr_config.value("lastchecktime","").toString();
    QString current_time_str = QDateTime::currentDateTime().toString("yyyy-M-d");
    addr_config.endGroup();
    if(current_time_str == record_time_str)
    {
        return false;
    }
    else
    {
        return true;
    }
}

QVector<ADDR_INFO> BakServerCheckTask::parser_addr_list(QString file_path)
{
    QVector<ADDR_INFO> addr_vector;
    if(!QFile::exists(file_path))
    {
        return addr_vector;
    }

    QFile file(file_path);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream inputStream(&file);
    inputStream.setCodec("UTF-8");
    QString value = inputStream.readAll();
    file.close();
    QJsonParseError parseJsonErr;
    QJsonDocument document = QJsonDocument::fromJson(value.toUtf8(),&parseJsonErr);
    if(!(parseJsonErr.error == QJsonParseError::NoError))
    {
        return addr_vector;
    }

    QJsonObject jsonObject = document.object();
    QStringList keyList = jsonObject.keys();
    int key_count = keyList.size();
    for(int index=0; index<key_count; ++index)
    {
        QString key_str = QString::number(index);
        if(!keyList.contains(key_str))
        {
            continue;
        }
        QJsonObject child_object = jsonObject.value(key_str).toObject();
        QString addrname = child_object.value("addrname").toString();
        QString scheme = child_object.value("scheme").toString();
        int port = child_object.value("port").toInt();
        QString addr = QByteArray::fromBase64(child_object.value("addr").toString().toUtf8());
        addr_vector.push_back(ADDR_INFO(addrname,scheme,port,addr));

    }

    return addr_vector;
}


