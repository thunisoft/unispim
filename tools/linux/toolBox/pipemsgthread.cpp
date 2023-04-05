#include "pipemsgthread.h"
#include <iostream>

#include <QDebug>
#ifdef _WIN32
#include <windows.h>
#endif
using namespace std;

PipeMsgThread::PipeMsgThread(QObject* parent):QThread(parent)
{

}

void PipeMsgThread::run()
{
#ifdef _WIN32
    while(true)
    {
        char buffer[1024];
        memset(buffer,0x00,1024);
        DWORD ReadNum;

        HANDLE pipe_handle = CreateNamedPipe(L"\\\\.\\Pipe\\toolBox",
                                      PIPE_ACCESS_DUPLEX,
                                      PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                                      1,
                                      1024,
                                      1024,
                                      1000,
                                      NULL);
        if(pipe_handle == INVALID_HANDLE_VALUE)
        {
            continue;
        }

        if(ConnectNamedPipe(pipe_handle, NULL))
        {

            if (ReadFile(pipe_handle, buffer, 1024, &ReadNum, NULL) == FALSE)
            {
                DisconnectNamedPipe(pipe_handle);
                CloseHandle(pipe_handle);
                continue;
            }
            QString input_string = QString(buffer);
            qDebug() << input_string;
            if(input_string == QString("enable-hotkey"))
            {
                emit EnableHotkey(true);
            }
            else if(input_string == QString("disable-hotkey"))
            {
                emit EnableHotkey(false);
            }
            DisconnectNamedPipe(pipe_handle);
            CloseHandle(pipe_handle);
        }
        else
        {
            CloseHandle(pipe_handle);
        }
    }
#endif

}
