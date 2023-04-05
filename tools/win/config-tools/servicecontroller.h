#ifndef SERVICECONTROLLER_H
#define SERVICECONTROLLER_H

#include <QObject>
#include <QProcess>
#include <QTextCodec>
#include <QStringList>
#include <QDebug>

#define mDebug(x)     (qDebug()    << "Debug   " + QString(__DATE__).replace(" ",".") + " " + QString(__TIME__) + " " + QString(__FILE__) + " " + QString(__FUNCTION__) + " " + QString::number(__LINE__) + ">>" + " " + x)

class ServiceController : public QObject
{
    Q_OBJECT

public:
    ServiceController(QString serviceName);
    typedef enum{Auto,Manual,Disabled,Delayed_Auto} StartType;//自动、手动、禁用、自动启动(延迟)

public:
    void query();
    void setStartType(StartType type);
    void CheckTheServeice();
    bool UpPrivilegeStart();

private:
    QString serviceName;
    QProcess *process;

private slots:
    void readReult();//读取运行结果

private:
    bool m_isRunning = true;

};

#endif // SERVICECONTROLLER_H
