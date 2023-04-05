// 没用

#include "myaction.h"


MyAction::MyAction(const QString &text, QObject *parent) : QAction(text, parent)
{

}

MyAction::MyAction(const QIcon &icon, const QString &text, QObject *parent)
    : QAction(text, parent)
{
    setIcon(QIcon(icon.pixmap(QSize(16,16))));
}

MyAction::~MyAction()
{

}

void MyAction::SetProp(const KimpanelProperty &prop)
{
    this->mProp = prop;
}

const KimpanelProperty& MyAction::GetProp()
{
    return this->mProp;
}

void MyAction::SetSkinPath(const QString skinPath)
{
    this->mSkinPath = skinPath;
}

const QString MyAction::GetSkinPath()
{
    return this->mSkinPath;
}

void MyAction::SetSkinClass(const SkinClass skinClass)
{
    this->mSkinClass = skinClass;
}

SkinClass MyAction::GetSkinClass()
{
    return this->mSkinClass;
}

void MyAction::SetMyActionType(const MyActionType myActionType)
{
    this->mMyActionType = myActionType;
}

MyActionType MyAction::GetMyActionType()
{
    return this->mMyActionType;
}
