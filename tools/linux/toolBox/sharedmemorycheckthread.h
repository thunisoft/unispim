#ifndef SHAREDMEMORYCHECKTHREAD_H
#define SHAREDMEMORYCHECKTHREAD_H
#include <QThread>

class SharedMemoryCheckThread: public QThread
{
    Q_OBJECT

protected:
    virtual void run() override;

signals:
    void ShowNormal(int pos_x, int pos_y);
    void ShowVersionCheck();
    void to_index_page(int page_type);
    void enable_addon(QString addon_name);
private:
    bool m_thread_flag = true;
};

#endif // SHAREDMEMORYCHECKTHREAD_H
