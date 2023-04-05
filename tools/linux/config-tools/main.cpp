#include "configbus.h"
#include "mainwindow.h"
#include <QApplication>
#include <qtsingleapplication.h>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <QMutex>
#include <QDateTime>
#include <QTextStream>
#include <QTranslator>

void LogToFile(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // 加锁
    static QMutex mutex;
    mutex.lock();

    QByteArray localMsg = msg.toLocal8Bit();

    QString strMsg("");
    switch(type)
    {
    case QtDebugMsg:
        strMsg = QString("Debug:");
        break;
    case QtWarningMsg:
        strMsg = QString("Warning:");
        break;
    case QtCriticalMsg:
        strMsg = QString("Critical:");
        break;
    case QtFatalMsg:
        strMsg = QString("Fatal:");
        break;
    }

    // 设置输出信息格式
    QString strDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd");
    QString strMessage = QString("1% File:%2  Line:%3  Function:%4  DateTime:%5 Message:%6")
            .arg(strMsg).arg(context.file).arg(context.line).arg(context.function).arg(strDateTime).arg(localMsg.constData());

    // 输出信息至文件中（读写、追加形式）
    QFile file("/tmp/hauyupy-log.txt");
    file.open(QIODevice::ReadWrite | QIODevice::Append);
    QTextStream stream(&file);
    stream << strMessage << "\n";
    file.flush();
    file.close();

    // 解锁
    mutex.unlock();
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Parameter error!" << std::endl;
        return -1;
    }

    if (strcmp(argv[1], "82717623-mhe4-0293-aduh-ku87wh6328ne") != 0)
    {
        std::cerr << "Parameter error!" << std::endl;
        return -1;
    }
    qInstallMessageHandler(LogToFile);
    QtSingleApplication a(argc, argv);
    if(a.sendMessage(QString("has other process exist")))
        return 0;


    if (!ConfigBus::instance()->init())
    {
        return -1;
    }

    QFile file(":/feedback/basicstyle.qss");
    if(file.open(QIODevice::ReadOnly))
    {
        a.setStyleSheet(file.readAll());
        file.close();
    }

    QTranslator *pTranslator = new QTranslator();
    if (pTranslator->load(":/qt_zh_CN.qm"))
    {
        qApp->installTranslator(pTranslator);
    }

    MainWindow w;
    w.show();
    w.activateWindow();
    w.raise();

    //switch to index page
    if(argc == 3)
    {
        w.OnSideButtonClick(argv[2]);
    }

    int ret = a.exec();

    ConfigBus::instance()->uninit();

    return ret;
}
