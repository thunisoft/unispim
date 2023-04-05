#ifndef VERIFIFYCODETHREAD_H
#define VERIFIFYCODETHREAD_H
#include <QThread>


class VerifyCodeCheckThread : public QThread
{
    Q_OBJECT
protected:
    virtual void run() override;

signals:
    void hasthemodules();

};

#endif // VERIFIFYCODETHREAD_H
