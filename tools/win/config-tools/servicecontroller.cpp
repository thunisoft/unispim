#include "servicecontroller.h"
#include <Windows.h>
#include <processthreadsapi.h>

ServiceController::ServiceController(QString serviceName):
    serviceName(serviceName)
{
    process = new QProcess();
     connect(process,SIGNAL(readyRead()),this,SLOT(readReult()));
}


/**
 * @brief ServiceController::setStartType 设置服务的启动类型（需要管理员权限）
 * @param type 服务启动类型
 */
void ServiceController::setStartType(StartType type){
    QStringList args;
    args<<"config"<<serviceName<<"start=";
    switch (type) {
    case Auto:
        process->start("sc",args<<"auto");
        break;
    case Manual:
        process->start("sc",args<<"demand");
        break;
    case Disabled:
        process->start("sc",args<<"disabled");
        break;
    case Delayed_Auto:
        process->start("sc",args<<"delayed-auto");
        break;
    default:
        break;
    }
    process->waitForFinished();
}

void ServiceController::CheckTheServeice()
{
    query();
    if(!m_isRunning)
    {
        UpPrivilegeStart();
    }

}

bool ServiceController::UpPrivilegeStart()
{
    SHELLEXECUTEINFO sei={sizeof(SHELLEXECUTEINFO)};
    sei.lpVerb=TEXT("runas");
    sei.lpFile=TEXT("sc.exe");
    sei.nShow=SW_HIDE;
    sei.lpParameters = TEXT("start HuayuPYService");

    if(!ShellExecuteEx(&sei))
    {
      return false;
    }
    return true;
}

void ServiceController::query()
{
    process->start("sc",QStringList()<<"query"<<serviceName);
    process->waitForFinished();
}

void ServiceController::readReult()
{
    QByteArray bytes = process->readAll();
    QTextCodec *gbk = QTextCodec::codecForName("gb2312");
    QString data = gbk->toUnicode(bytes);
    if(!data.isEmpty()){
        qDebug() << data;
        QStringList splitList = data.split("\r\n");
        if(splitList.size()>3)
        {
            if(splitList.at(3).contains("STOPPED"))
            {
                m_isRunning = false;
            }
        }
    }
}
