#include "log.h"
#include <QMutex>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>
#include <QDir>

Log::Log()
{

}

void Log::LogConfig(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static QMutex mutex;
    mutex.lock();

    QString text;
    switch(type)
    {
        case QtDebugMsg:
            text = QString("Debug:");
            break;
        case QtWarningMsg:
            text = QString("Warning:");
            break;
        case QtCriticalMsg:
            text = QString("Critical:");
            break;
        case QtFatalMsg:
            text = QString("Fatal:");
            break;
        default:
            text = QString("Debug:");
            break;
    }

    QString context_info = QString("%1:%2").arg(QString(context.file)).arg(context.line);
    QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString message = QString("%1 %2 %3 %4").arg(text).arg(context_info).arg(current_date_time).arg(msg);
    QFile file("/tmp/huayupy/config-tools-log.txt");

    QDir dir("/tmp/huayupy");
    if(!dir.exists())
    {
        dir.mkdir("/tmp/huayupy");
    }
    file.open(QFile::WriteOnly | QFile::Append);
    QTextStream text_stream(&file);
    text_stream << message << "\n";
    file.flush();
    file.close();
    mutex.unlock();
}
