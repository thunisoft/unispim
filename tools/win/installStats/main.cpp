#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDir>
#include "config.h"
#include "networkhandler.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QString client = Config::Instance()->GetHostMacAddress();
    QString osVersion = Config::Instance()->GetOSVersion();
    QString cpuType = Config::Instance()->GetCPUType();
    QString version = Config::Instance()->GetIMEVersion();
    if(a.arguments().size() != 2)
    {
        return 0;
    }
    QString callType = argv[1];
    int operation = 0;
    if(callType == "-install")
    {
        operation = 1;
    }
    else if(callType == "-uninstall")
    {
        operation = 2;
    }
    else if(callType == "-upgrade")
    {
        operation = 3;
    }
    else
    {
        return 0;
    }

    NetworkHandler::Instance()->NotifySetup(client,osVersion,cpuType,version,operation);

    if(operation == 1)
    {
        Config::SetUserNameToReg(true);
        QString current_version = Config::Instance()->GetIMEVersion();
        Config::WriteVersionNumToIniFile(current_version);
        Config::SendMsgToEngine();

    }
    else if(operation == 2)
    {
        Config::SetUserNameToReg(false);
    }


    return 0;
}
