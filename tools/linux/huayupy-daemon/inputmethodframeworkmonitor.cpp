// 在中标麒麟下可能有两种框架，避免冲突

#include "inputmethodframeworkmonitor.h"
#include <stdlib.h>
#include <QProcess>
#include <QApplication>
#include "../public/utils.h"

InputMethodFrameworkMonitor::InputMethodFrameworkMonitor():                              
                                m_timer(NULL)
                              , m_is_monitor_started(false)

{

}

bool InputMethodFrameworkMonitor::IsProccessOn(const QString& process_name)
{
    QString check_command = QString::fromLocal8Bit(" ps -ef | grep -w %1 | grep -v grep | wc -l ").arg(process_name);
    char return_value[150];
    int count = 0;
    FILE* ptr = NULL;
    if((ptr = popen(check_command.toStdString().c_str(), "r")) == NULL)
    {
        return false;
    }
    memset(return_value, 0, sizeof(return_value));
    if((fgets(return_value, sizeof(return_value),ptr))!= NULL)
    {
        count = atoi(return_value);
    }
    pclose(ptr);
    if(count <= 0)
    {
        return false;
    }
    return true;
}

void InputMethodFrameworkMonitor::Monitor()
{
    if(m_timer)
        return;

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(TimeIsUp()));
    m_timer->start(5000);

}

void InputMethodFrameworkMonitor::StartMonitor()
{
    if(m_is_monitor_started)
         return;

    m_is_monitor_started = true;
    Monitor();
}


void InputMethodFrameworkMonitor::TimeIsUp()
{
    //如果框架退出之后,守护进程退出
    if(!IsProccessOn(INPUT_METHOD_FRAME_NAME))
    {
#ifdef USE_UOS
        QString daemon_name = "huayupy-qimpanel";
        Utils::kill_process_by_name(daemon_name);
        QApplication::exit(0);
#endif
        return;
    }
}

void InputMethodFrameworkMonitor::StartProcess(const QString& process_name)
{
    QProcess::startDetached(process_name);
}

void InputMethodFrameworkMonitor::MonitorQimPanelIsOn()
{
#ifndef USE_IBUS
    if(!IsProccessOn(Utils::GetBinFilePath()))
    {
        StartProcess(Utils::GetBinFilePath());
    }
#endif
}
