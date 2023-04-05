#include "syswordlibcheckthread.h"
#include "networkhandler.h"
#include "myutils.h"


#define CHECK_INTERVAL 2*60*1000

/**@brief 三分钟请求一次,有更新的话发送信号
*/
SysWordlibCheckThread::~SysWordlibCheckThread()
{
    if(this->isRunning())
    {
        this->exit(0);
    }
}

void SysWordlibCheckThread::run()
{

    try {
        QString server_md5;
        QString current_date_version;
        QString sys_wordlib_id;
#ifdef Q_OS_LINUX
        QString local_sys_wordlib_path = TOOLS::PathUtils::get_local_wordlib_path().append("sys.uwl");
        QString insall_wordlib_path = TOOLS::PathUtils::get_install_wordlib_dir_path().append("sys.uwl");
        if(!QFile::exists(local_sys_wordlib_path))
        {
            QFile default_sys(insall_wordlib_path);
            if(default_sys.exists())
                default_sys.copy(local_sys_wordlib_path);
        }
        QString sys_wordlib_md5 = TOOLS::WordlibUtils::get_file_md5(local_sys_wordlib_path);
#endif
        int handle_code = NetworkHandler::Instance()->CheckSystemWordlib(sys_wordlib_md5, sys_wordlib_id,server_md5,current_date_version);
        if((handle_code == 0) || (sys_wordlib_md5.isEmpty()))
        {
            emit updatecoming(server_md5);
        }
        else
        {
            emit nowordlibupdate();
        }
        if(!current_date_version.isEmpty())
        {
            emit current_date_version_update(current_date_version);
        }

    }
    catch (std::exception& e)
    {
      TOOLS::LogUtils::write_log_to_file(QString("syswordlib check throw exception:%1").arg(e.what()));
    }
}

void SysWordlibCheckThread::set_enable_flag(bool flag)
{
    m_check_enable_flag = flag;
}
