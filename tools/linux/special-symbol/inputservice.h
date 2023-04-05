#ifndef INPUTSERVICE_H
#define INPUTSERVICE_H


#include <QObject>
#include <QDebug>

class InputService
{
public:
    InputService();
    virtual ~InputService();
    virtual void Init();
    virtual void UnInit();
    virtual void InputString(const QString& str);
};

InputService* GetInputServiceInstance();
void FreeInputServiceInstance(InputService* service);

#endif // INPUTSERVICE_H
