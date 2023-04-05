#include <QApplication>
#include <qtsingleapplication.h>
#include <QTextCodec>
#include <QIODevice>
#include <signal.h>
#include "maincontroller.h"
#include "../public/utils.h"

int main(int argc, char *argv[])
{
    //QApplication a(argc, argv);
    QtSingleApplication a(argc, argv);
    if(a.sendMessage(QString("has other process exist")))
        return 0;
    signal(SIGSEGV, SegvHandler); // SIGSEGV    11       Core Invalid memory reference
    signal(SIGABRT, SegvHandler); // SIGABRT     6       Core Abort signal from
#ifdef QT5
#else
    QTextCodec *codec=QTextCodec::codecForName("utf-8");
    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForCStrings(codec);
    QTextCodec::setCodecForTr(codec);
#endif

    QFile file(":/basicStyleSheet.qss");
    if(file.open(QIODevice::ReadOnly))
    {
        QByteArray content = file.readAll();
        a.setStyleSheet(content);
    }

    MainController* mainController = MainController::Instance();   
    int ret = a.exec();

    delete mainController;

    return ret;
}
