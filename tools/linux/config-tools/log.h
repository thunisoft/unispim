#ifndef LOG_H
#define LOG_H

#include <QObject>

class Log
{
public:
    Log();
    static void LogConfig(QtMsgType type, const QMessageLogContext &context, const QString &msg);
};

#endif // LOG_H
