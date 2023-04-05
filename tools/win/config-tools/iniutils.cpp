#include "iniutils.h"

IniUtils::IniUtils()
{

}

int IniUtils::GetKeyName(int nLen, TCHAR* szBuf,KEY_NAME* result)
{
    int nIndex(0),nNum(0);

    if(nLen == 0 || sizeof(szBuf) == 0)
        return 0;

    for (int nPos = 0;nPos < nLen;nPos++)
    {
        if (szBuf[nPos] != _T('\0'))
        {
            result->szKeyBuf[nNum][nIndex++] = szBuf[nPos];
        }
        else
        {
            result->szKeyBuf[nNum][nIndex] = _T('\0');
            nNum++;
            nIndex = 0;
        }
    }
    result->nKeyNum = nNum;
    return nNum;

}
