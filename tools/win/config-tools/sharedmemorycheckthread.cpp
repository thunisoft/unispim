#include "sharedmemorycheckthread.h"
#include <QSharedMemory>

void SharedMemoryCheckThread::run()
{
    QSharedMemory shared("huayupy-config-tools");

    while(m_thread_flag)
    {
        if(shared.attach())
        {
            QString fileContent = "config-tools-show-normal";
            QString checkSwitch = "newVersionCheck";
            QString login_page = "login_page";
            shared.lock();
            if(strcmp((char*)shared.data(),fileContent.toStdString().c_str()) == 0)
            {
                emit ShowNormal();
                memset((char*)shared.data(),0x00,64);
            }
            else if(strcmp((char*)shared.data(),checkSwitch.toStdString().c_str()) == 0)
            {
                emit ShowVersionCheck();
                memset((char*)shared.data(),0x00,64);
            }
            else if(strcmp((char*)shared.data(),login_page.toStdString().c_str()) == 0)
            {
                emit to_index_page((int)LOGIN_PAGE);
                memset((char*)shared.data(),0x00,64);
            }
            else if(strcmp((char*)shared.data(),"edit_hotkey") == 0)
            {
                emit ShowEditHotkey();
            }
            shared.unlock();
            if(shared.isAttached())
            {
                shared.detach();
            }
        }

        msleep(1000);
    }


}
