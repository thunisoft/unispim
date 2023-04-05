#ifndef MSGBOXUTILS_H
#define MSGBOXUTILS_H

#include <QMessageBox>

class MsgBoxUtils
{
public:
    MsgBoxUtils();


public:
    static int NoticeMsgBox(const QString& notice_content, QString title="", bool isReturnValue=true,QWidget* parent = 0);
    static int NoticeMsgBoxWithReturnValue(const QString& notice_content, QWidget* parent = 0);
};

#endif // MSGBOXUTILS_H
