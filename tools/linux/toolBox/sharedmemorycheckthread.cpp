#include "sharedmemorycheckthread.h"
#include <QSharedMemory>
#include <QFile>
#ifdef _WIN32
#include <Windows.h>
#else
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif
#include "config.h"

void SharedMemoryCheckThread::run()
{
    QSharedMemory shared("huayupy-toolbox");

    while(m_thread_flag)
    {
#ifdef _WIN32
        HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, L"HuayuPY.ShowToolBox");
        WaitForSingleObject(hEvent, INFINITE);
        ResetEvent(hEvent);
        if(shared.attach())
        {
            shared.lock();
            QString input_content = QString((char*)shared.data());
            if(!input_content.isEmpty() && input_content.contains("#"))
            {
                QStringList paramer_list = input_content.split("#");
                if(paramer_list.size() == 2)
                {
                    QString pos_x_str = paramer_list.value(0);
                    QString pos_y_str = paramer_list.value(1);
                    emit ShowNormal(pos_x_str.toInt(),pos_y_str.toInt());
                }

                memset((char*)shared.data(),0x00,64);
            }
            else
            {
                enable_addon(input_content);
            }
            shared.unlock();
            if(shared.isAttached())
            {
                shared.detach();
            }
        }
#else
        int fd,ret;
        QString fifo_path = Config::Instance()->get_user_config_dir_path() + QString("huayupy_tool_box_fifo");
        if(QFile::exists(fifo_path))
        {
            QFile::remove(fifo_path);
        }
        ret = mkfifo(fifo_path.toStdString().c_str(), S_IFIFO|0666);
        if(ret == -1)
        {
            continue;
        }
        fd = open(fifo_path.toStdString().c_str(), O_RDONLY);
        if(fd < 0)
        {
            continue;
        }
        char recv[100] = {0};
        //读数据，命名管道没数据时会阻塞，有数据时就取出来
        read(fd, recv, sizeof(recv));
        QString received_argument = QString(recv);

        QStringList paramer_list = received_argument.split("#");
        if(paramer_list.size() == 2)
        {
            QString pos_x_str = paramer_list.value(0);
            QString pos_y_str = paramer_list.value(1);
            emit ShowNormal(pos_x_str.toInt(),pos_y_str.toInt());
        }
        memset(recv,0x00,sizeof(recv));
        close(fd);
#endif

    }
}
