#include <QCoreApplication>
#include "stdio.h"
#include "log.h"
#include "unistd.h"
#include <sys/types.h>
#include <qtsingleapplication.h>
#include <QDir>
#include <QStandardPaths>
#include "configbus.h"
#include "maincontroller.h"

int main(int argc, char *argv[])
{
    Utils::WriteLogToFile("huayupy-daemon-fcitx-start");
    QtSingleApplication a(argc, argv);
    if(a.sendMessage(QString("has other process exist")))
        return 0;

    if (!ConfigBus::instance()->init())
    {
        return -1;
    }

    MainController::Instance()->Start();

    return a.exec();
}
