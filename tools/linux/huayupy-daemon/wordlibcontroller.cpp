// 词库总控

#include "wordlibcontroller.h"
#include "../public/defines.h"
#include "../public/utils.h"

WordlibController::WordlibController()
                  :m_sys_wordlib_downloader(nullptr)
                  ,m_syscloud_wordlib_downloader(nullptr)
                  ,m_record_wordlib_uploader(nullptr)
{
    Init();
}

WordlibController::~WordlibController()
{
    if(m_syscloud_wordlib_downloader)
        delete m_syscloud_wordlib_downloader;
    if(m_sys_wordlib_downloader)
        delete m_sys_wordlib_downloader;
    if(m_record_wordlib_uploader)
        delete m_record_wordlib_uploader;
}

void WordlibController::Init()
{
    m_syscloud_wordlib_downloader = new SyscloudWordlibDownloader();
    m_record_wordlib_uploader = new RecordWrodlibUploader();
}

void WordlibController::Start()
{

}

