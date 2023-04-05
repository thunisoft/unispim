#include "inputservice.h"
#include <QDebug>
#include <iostream>
#include <Windows.h>
#include <string>
#include <QCursor>
#include <QtConcurrent/QtConcurrent>
#include <QClipboard>
#include <QApplication>

InputService* g_input_service = NULL;

void InputService::Init(){}
void InputService::UnInit(){}
InputService::InputService(){}
InputService::~InputService(){}


void InputService::InputString(const QString& str)
{
    SendKeys(str);
}


InputService* GetInputServiceInstance()
{
	if (!g_input_service)
    {
        g_input_service = new InputService();
	}

	return g_input_service;
}

void FreeInputServiceInstance(InputService* service)
{
	delete service;
	service = NULL;
}




std::wstring InputService::stringToWstring(const std::string& str)
{
    LPCSTR pszSrc = str.c_str();
    int nLen = MultiByteToWideChar(CP_ACP, 0, pszSrc, -1, NULL, 0);
    if (nLen == 0)
        return std::wstring(L"");

    wchar_t* pwszDst = new wchar_t[nLen];
    if (!pwszDst)
        return std::wstring(L"");

    MultiByteToWideChar(CP_ACP, 0, pszSrc, -1, pwszDst, nLen);
    std::wstring wstr(pwszDst);
    delete[] pwszDst;
    pwszDst = NULL;

    return wstr;
}

void InputService::SendCopyMessage()
{
    // Create a generic keyboard event structure
    INPUT ip;
    ip.type = INPUT_KEYBOARD;
    ip.ki.wScan = 0;
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;


    // Press the "Ctrl" key
    ip.ki.wVk = VK_CONTROL;
    ip.ki.dwFlags = 0; // 0 for key press
    SendInput(1, &ip, sizeof(INPUT));

    // Press the "V" key
    ip.ki.wVk = 'V';
    ip.ki.dwFlags = 0; // 0 for key press
    SendInput(1, &ip, sizeof(INPUT));

    // Release the "V" key
    ip.ki.wVk = 'V';
    ip.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &ip, sizeof(INPUT));

    // Release the "Ctrl" key
    ip.ki.wVk = VK_CONTROL;
    ip.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &ip, sizeof(INPUT));


}

void InputService::SendUnicode(const wchar_t data)
{


    INPUT input[2];
    memset(input, 0, 2 * sizeof(INPUT));

    input[0].type = INPUT_KEYBOARD;
    input[0].ki.wVk = 0;
    input[0].ki.wScan = data;
    input[0].ki.dwFlags = 0x4;//KEYEVENTF_UNICODE;
    input[0].ki.dwExtraInfo = GetMessageExtraInfo();
    qDebug() << SendInput(1, &input[0], sizeof(INPUT));

    input[1].type = INPUT_KEYBOARD;
    input[1].ki.wVk = 0;
    input[1].ki.wScan = data;
    input[1].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP ;//KEYEVENTF_UNICODE;  这里是为了防止英文字符进入到系统输入法里面，则可以解决国内的输入法软件拦截的问题，但是国外的软件一般做了UNICODE兼容，所以还是会有问题。
    input[1].ki.dwExtraInfo = GetMessageExtraInfo();
    qDebug() << SendInput(1, &input[1], sizeof(INPUT));

}

void InputService::SendKeys(QString msg)
{
    wstring data = msg.toStdWString();

    QClipboard* clipborad = QApplication::clipboard();
    clipborad->setText(msg);
    SendCopyMessage();
}
