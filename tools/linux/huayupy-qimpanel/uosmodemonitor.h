#ifndef UOSMODEMONITOR_H
#define UOSMODEMONITOR_H
#include <QObject>

/*
uos 系统下有特效模式和普通模式。

1.特效模式下 圆角和透明才会生效。
2.在普通模式下 圆角和透明部分会变成黑底。
            Wrriten by Wuyin 2021.4.19

*/
#define _SERVICE_ "org.kde.KWin"
#define _PATH_ "/Compositor"
#define _INTERFACE_ "org.kde.kwin.Compositing"

class CandidateWindow;

class UosModeMonitor : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", _SERVICE_)
public:
    UosModeMonitor(CandidateWindow* candidate_window);

    bool IsSpecialMode();

private slots:
    void ModeChanged(bool value);


private:
    void Init();

    CandidateWindow* m_candiadte_window;
};

#endif // UOSMODEMONITOR_H
