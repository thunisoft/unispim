#include "syswordlibcheckthread.h"
#include "config.h"
#include "networkhandler.h"
#include "utils.h"


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
        QString sys_wordlib_id;
        QString current_date_version;
        QString sys_wordlib_md5 = Utils::GetFileMD5(Config::Instance()->SystemWordlibPath());
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
       Utils::WriteLogToFile(QString("syswordlib check throw exception:%1").arg(e.what()));
    }
}

void SysWordlibCheckThread::set_enable_flag(bool flag)
{
    m_check_enable_flag = flag;
}
