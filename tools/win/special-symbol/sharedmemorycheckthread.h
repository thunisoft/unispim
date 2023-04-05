#ifndef SHAREDMEMORYCHECKTHREAD_H
#define SHAREDMEMORYCHECKTHREAD_H
#include <QThread>

class SharedMemoryCheckThread: public QThread
{
    Q_OBJECT
protected:
    virtual void run() override;

signals:
    void ShowNormal();

private:
    bool m_thread_flag = true;
};

#endif // SHAREDMEMORYCHECKTHREAD_H
