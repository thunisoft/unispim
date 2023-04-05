#include "sharedmemorycheckthread.h"
#include <QSharedMemory>


void SharedMemoryCheckThread::run()
{
    QSharedMemory shared("huayupy-special-symbol");
    while(m_thread_flag)
    {
        if(shared.attach())
        {
            QString fileContent = "show normal";
            shared.lock();
            if(strcmp((char*)shared.data(),fileContent.toStdString().c_str()) == 0)
            {
                emit ShowNormal();
                memset((char*)shared.data(),0x00,64);
            }
            shared.unlock();
            if(shared.isAttached())
            {
                shared.detach();
            }
        }

        msleep(200);
    }


}
