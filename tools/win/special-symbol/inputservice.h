#ifndef INPUTSERVICE_H
#define INPUTSERVICE_H


#include <QObject>
#include <QDebug>
#include <iostream>
#include <Windows.h>
#include <string>


using namespace std;
class InputService
{
public:
    InputService();
    ~InputService();
    void Init();
    void UnInit();
    void InputString(const QString& str);

private:
    std::wstring stringToWstring(const std::string& str);
    void SendCopyMessage();
    void SendUnicode(const wchar_t data);
    void SendKeys(QString msg);
};

InputService* GetInputServiceInstance();
void FreeInputServiceInstance(InputService* service);

#endif // INPUTSERVICE_H
