#ifndef SHAREDMEMORYCHECKTHREAD_H
#define SHAREDMEMORYCHECKTHREAD_H
#include <QThread>
#include "mainwindow.h"

class SharedMemoryCheckThread: public QThread
{
    Q_OBJECT

protected:
    virtual void run() override;

signals:
    void ShowNormal();
    void ShowVersionCheck();
    void to_index_page(int page_type);
    void ShowEditHotkey();

private:
    bool m_thread_flag = true;
};

#endif // SHAREDMEMORYCHECKTHREAD_H
