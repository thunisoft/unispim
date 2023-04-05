#ifndef PIPEMSGTHREAD_H
#define PIPEMSGTHREAD_H
#include <QThread>

class PipeMsgThread : public QThread
{
    Q_OBJECT
public:
    PipeMsgThread(QObject* parent=0);

protected:
    virtual void run() override;

signals:
     void EnableHotkey(bool falg);
};

#endif // PIPEMSGTHREAD_H
