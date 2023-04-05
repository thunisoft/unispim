#include "maincontroller.h"
#include "../public/utils.h"
#include "professwordlibupdatetask.h"
#include "syswordlibincretask.h"
#include "userwordlibupdatetask.h"



MainController* MainController::m_instance = NULL;

MainController::MainController(QObject* parent)
                :QObject(parent)
                ,m_wordlib_controller(NULL)
                ,m_dbus_msg_manager(NULL)
                ,m_stats_info_uploader(NULL)
                ,m_input_method_framework_monitor(NULL)
                ,m_update_checker(NULL)
                ,m_professwordlib_update_task(NULL)
{
    Init();
}


MainController::~MainController()
{
    disconnect(m_dbus_msg_manager, SIGNAL(CheckUpdate()), 0, 0);    
    if(m_wordlib_controller)
        delete m_wordlib_controller;
    if(m_dbus_msg_manager)
        delete m_dbus_msg_manager;
    if(m_stats_info_uploader)
        delete m_stats_info_uploader;
    if(m_input_method_framework_monitor)
        delete m_input_method_framework_monitor;
    if(m_update_checker)
        delete m_update_checker;
    if(m_professwordlib_update_task)
    {
        delete m_professwordlib_update_task;
        m_professwordlib_update_task = NULL;
    }
    if(m_bak_server_check_task)
    {
        delete m_bak_server_check_task;
        m_bak_server_check_task = NULL;
    }
    if(m_sys_wordlib_update_task)
    {
        delete m_sys_wordlib_update_task;
        m_sys_wordlib_update_task = NULL;
    }

    if(m_user_wordlib_update_task)
    {
        delete m_user_wordlib_update_task;
        m_user_wordlib_update_task = NULL;
    }
}

MainController* MainController::Instance()
{
  if(!m_instance)
  {
      m_instance = new MainController();
  }
  return m_instance;
}

void MainController::Start()
{

}

void MainController::Init()
{
    RemoveErrorSysUwl();

    m_dbus_msg_manager = new DbusMsgManager();

    //词库控制
    m_wordlib_controller = new WordlibController();
    m_wordlib_controller->Start();

    m_stats_info_uploader = new StatsInfoUploader();
    m_stats_info_uploader->start();

    //输入法架构监控
    m_input_method_framework_monitor = new InputMethodFrameworkMonitor();
    m_input_method_framework_monitor->StartMonitor();

    //版本更新
    m_update_checker = new UpdateChecker;
    m_update_checker->start();

    //专业词库更新
    m_professwordlib_update_task = new ProfessWordlibUpdateTask;
    m_professwordlib_update_task->start();

    //备用Ip检测，用来测试网络
    m_bak_server_check_task = new BakServerCheckTask();
    m_bak_server_check_task->start();

    m_sys_wordlib_update_task = new SysWordlibIncreTask();
    m_sys_wordlib_update_task->start();


    //user wordlid update task
    m_user_wordlib_update_task = new UserWordlibUpdateTask();
    m_user_wordlib_update_task->start();

}

void MainController::RemoveErrorSysUwl()
{
    QString sys_uwl_path = Utils::GetConfigDirPath().append("wordlib/sys.uwl");
    QFile sys(sys_uwl_path);
    /*配置目录下小于 7M，认为系统词库有问题*/
    if(sys.exists() && sys.size() < 7000000)
        sys.remove();
}
