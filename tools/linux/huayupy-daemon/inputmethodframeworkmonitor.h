#ifndef INPUTMETHODFRAMWORKMONITOR_H
#define INPUTMETHODFRAMWORKMONITOR_H

#include <QObject>
#include <QCoreApplication>
#include <QTimer>

#ifdef USE_IBUS
  #define INPUT_METHOD_FRAME_NAME " /usr/bin/ibus-daemon "
  #define OTHER_DAEMON_NAME " huayupy-daemon-fcitx "
#else //fcitx input method frame
  #define INPUT_METHOD_FRAME_NAME " /usr/share/fcitx/dbus/daemon.conf"
  #define OTHER_DAEMON_NAME " huayupy-daemon-ibus "
#endif
class InputMethodFrameworkMonitor : QObject
{
    Q_OBJECT
public:
    InputMethodFrameworkMonitor();
private:
    bool IsProccessOn(const QString& process_name);
    void StartProcess(const QString& process_name);
    void MonitorQimPanelIsOn();
private slots:
    void Monitor();
    void TimeIsUp();

signals:
    void Exit();

public:
    void StartMonitor();

private:
    QTimer *m_timer;
    bool m_is_monitor_started;
};

#endif // INPUTMETHODFRAMWORKMONITOR_H
