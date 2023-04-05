#ifndef INIUTILS_H
#define INIUTILS_H
#include <Windows.h>
#include <tchar.h>
#include <winnt.h>
#include <stdio.h>



struct KEY_NAME
{
    TCHAR szKeyBuf[100][MAX_PATH];
    int nKeyNum;

    KEY_NAME()
    {
        memset(szKeyBuf,'0x0',sizeof(szKeyBuf)/sizeof(TCHAR));
        nKeyNum = 0;
    }
};

class IniUtils
{
public:
    //获取某个Section下的所有的key值和key的总的个数
    static int GetKeyName(int nLen, TCHAR* szBuf,KEY_NAME* result);
public:
    IniUtils();
};

#endif // INIUTILS_H
