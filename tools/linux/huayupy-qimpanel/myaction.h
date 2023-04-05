#ifndef MYACTION_H
#define MYACTION_H

#include <QObject>
#include <QAction>

#include "agenttype.h"

enum MyActionType
{
    IMAction,
    StatusAction,
    VKAction,
    SkinAction,
    MozcHiraganaAction,
    MozcToolAction,
};

enum SkinClass
{
    FCITX,
};

class MyAction
        : public QAction
{
    Q_OBJECT

public:
    MyAction(const QString &text, QObject *parent);
    MyAction(const QIcon &icon, const QString &text, QObject *parent);
    virtual ~MyAction();

public:
    void SetProp(const KimpanelProperty &prop);
    const KimpanelProperty& GetProp();
    void SetSkinPath(const QString skinPath);
    const QString GetSkinPath();
    void SetSkinClass(const SkinClass skinClass);
    SkinClass GetSkinClass();
    void SetMyActionType(const MyActionType myActionType);
    MyActionType GetMyActionType();

private:
    KimpanelProperty mProp;
    QString mSkinPath;
    SkinClass mSkinClass;
    MyActionType mMyActionType;
};

#endif // MYACTION_H
