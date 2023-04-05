#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>

#include "wordlibcontroller.h"
#include "updatechecker.h"
#include "dbusmsgmanager.h"
#include "inputmethodframeworkmonitor.h"
#include "statsinfouploader.h"
#include "professwordlibupdatetask.h"
#include "bakserverchecktask.h"
#include "syswordlibincretask.h"
#include "userwordlibupdatetask.h"

class MainController : public QObject
{
    Q_OBJECT
private:
    explicit MainController(QObject* parent = 0);
    void Init();

    void RemoveErrorSysUwl();
public:
    static MainController* Instance();
    ~MainController();
    void Start();
private:
    static MainController* m_instance;
    WordlibController* m_wordlib_controller;
    DbusMsgManager* m_dbus_msg_manager;
    /*统计上传*/
    StatsInfoUploader* m_stats_info_uploader;
    InputMethodFrameworkMonitor* m_input_method_framework_monitor;
    ProfessWordlibUpdateTask* m_professwordlib_update_task;

    UpdateChecker *m_update_checker;
    BakServerCheckTask* m_bak_server_check_task = NULL;
    SysWordlibIncreTask* m_sys_wordlib_update_task = NULL;
    UserWordlibUpdateTask* m_user_wordlib_update_task = NULL;
};

#endif // MAINCONTROLLER_H
