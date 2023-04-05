// TrayIcon 菜单

#include "systemtraymenu.h"
#include <fcitx-config/fcitx-config.h>
#include <QDesktopServices>
#include <QProcess>
#include <QDateTime>
#include <QMouseEvent>
#include <QFile>
#include <QMessageBox>
#include <QApplication>

#include "kimpanelagent.h"
#include "myaction.h"
#include "maincontroller.h"
#include "../public/utils.h"
#include "skin/skinbase.h"
#include "skin/skinfcitx.h"
#include "../public/config.h"

#define ACTION_TEXT_VIRTUAL_KEYBOARD "虚拟键盘"
#define ACTION_TEXT_SKIN "皮肤"
#define ACTION_TEXT_ONLINE_HELP "在线帮助"
#define ACTION_TEXT_CONFIGURE_FICTX "输入法管理"
#define ACTION_TEXT_CONFIGURE_IM "输入法配置"

#define ACTION_TEXT_SHOW_TOOL_BAR "显示工具栏"
#define ACTION_TEXT_HIDE_TOOL_BAR "隐藏工具栏"

SystemTrayMenu::SystemTrayMenu(KimpanelAgent* agent, const bool hide_tray)
    : QMenu()
    , mAgent(agent), mVKListMenu(NULL), mMozcToolMenu(NULL)
    , mMozcHiraganaMenu(NULL), mSkinMenu(NULL), mTimerToDelayShow(NULL)
    , mHideTray(hide_tray), mIsShowMenu(false)
{    
    this->setMouseTracking(true);
    //this->setWindowFlags(Qt::WindowDoesNotAcceptFocus /*| Qt::X11BypassWindowManagerHint*/);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus | Qt::Tool);
}

SystemTrayMenu::~SystemTrayMenu()
{
    if (mVKListMenu)
    {
        delete mVKListMenu;
        mVKListMenu = NULL;
    }
    if (mMozcToolMenu)
    {
        delete mMozcToolMenu;
        mMozcToolMenu = NULL;
    }
    if (mMozcHiraganaMenu)
    {
        delete mMozcHiraganaMenu;
        mMozcHiraganaMenu = NULL;
    }
    if (mSkinMenu)
    {
        delete mSkinMenu;
        mSkinMenu = NULL;
    }
}

void SystemTrayMenu::Init()
{
    mVKListMenu = new QMenu(ACTION_TEXT_VIRTUAL_KEYBOARD, this);
    mMozcToolMenu = new QMenu("Mozc Tool", this);
    mMozcHiraganaMenu = new QMenu("Mozc Edit mode", this);
    mSkinMenu = new QMenu(ACTION_TEXT_SKIN, this);

    connect(this, SIGNAL(aboutToHide()), this, SLOT(ClearMenu()));
    //connect(this, SIGNAL(aboutToShow()), this, SLOT(DoAboutToShow()));
    connect(this, SIGNAL(triggered(QAction*)), this, SLOT(MenuItemOnClick(QAction*)));

    connect(mAgent, SIGNAL(RegisterPropertiesSignal(const QList<KimpanelProperty>)),
            this, SLOT(RegisterProperties(const QList<KimpanelProperty>)));
    connect(mAgent, SIGNAL(ExecMenuSignal(const QList<KimpanelProperty>)),
            this, SLOT(ExecMenu(const QList<KimpanelProperty>)));
    connect(mAgent, SIGNAL(UpdatePropertySignal(KimpanelProperty)),
            this, SLOT(UpdateProperty(KimpanelProperty)));

    DoAboutToShow();    
}

void SystemTrayMenu::TriggerUpdateVKListMenu()
{
    mAgent->TriggerProperty("/Fcitx/vk");
}

void SystemTrayMenu::TriggerUpdateIMListMenu()
{
    mAgent->TriggerProperty("/Fcitx/im");
}

void SystemTrayMenu::TriggerUpdateMozcHiraganaMenu()
{
    mAgent->TriggerProperty("/Fcitx/mozc-composition-mode");
}

void SystemTrayMenu::TriggerUpdateMozcToolMenu()
{
    mAgent->TriggerProperty("/Fcitx/mozc-tool");
}

void SystemTrayMenu::SetToolStatus(bool hide_tray)
{
    mHideTray = hide_tray;
    for(int i = 0; i < this->actions().size(); i++)
    {
        if(!mHideTray && 0 == this->actions().at(i)->text().compare(ACTION_TEXT_SHOW_TOOL_BAR))
        {
            this->actions().at(i)->setText(ACTION_TEXT_HIDE_TOOL_BAR);
            return;
        }
        else if(mHideTray && 0 == this->actions().at(i)->text().compare(ACTION_TEXT_HIDE_TOOL_BAR)) {
            this->actions().at(i)->setText(ACTION_TEXT_SHOW_TOOL_BAR);
            return;
        }
        else{}
    }
}

void SystemTrayMenu::ClearMenu()
{

}

void SystemTrayMenu::DoAboutToShow()
{
    if(this->isVisible())
    {
        return;
    }
    TriggerUpdateVKListMenu();
    TriggerUpdateIMListMenu();
    TriggerUpdateMozcHiraganaMenu();
    TriggerUpdateMozcToolMenu();
    UpdateMainMenu();
}

void SystemTrayMenu::UpdateMainMenu()
{
    if (!isEmpty())
        clear();
    addAction("在线帮助");
    addSeparator();
    DoUpdateIMListMenu(mIMList);
    addSeparator();
    DoUpdateVKListMenu(mVKList);
    addAction("属性设置");
    addSeparator();
    addAction("输入法配置");
    addAction("重新启动");
}

void SystemTrayMenu::MenuItemOnClick(QAction *action)
{
    if (!action)
    {
        return;
    }

    if (action->text() == "在线帮助")
    {
        QDesktopServices::openUrl(QUrl("http://fcitx-im.org/"));
    }
    else if (action->text() == "输入法配置")
    {
#ifdef USE_UOS
        Config::Instance()->OpenUOSFcitxConfig();
#else
        QFile fcitx_config("/usr/bin/fcitx-configtool");
        if(!fcitx_config.exists())
        {
            QMessageBox::warning(this,"警告","fcitx配置工具不存在！");
            return;
        }
        QProcess::startDetached("/usr/bin/fcitx-configtool");
#endif
    }
    else if (action->text() == "属性设置")
    {
        OpenConfigTool();
    }
    else if (action->text() == "重新启动")
    {
        RestartFcitx();
    }
    else if (action->text() == "退出")
    {
        ExitTheFcitx();
        exit(0);
    }
    else
    {
        MyAction* myAction = (MyAction*)action;
        if (SkinAction == myAction->GetMyActionType())
            SkinMenuItemOnClick(myAction);
        else
        {
            mAgent->TriggerProperty(myAction->GetProp().key);
            if(myAction->isCheckable() && IsVKList(myAction->GetProp().key))
            {
                myAction->setChecked(true);
                mCurrentVkName = myAction->GetProp().label;
            }
        }
    }
}

//前两个用不到，见后端KimpanelRegisterAllStatus()函数
#define StatusMenuSkip (2)
// 第一个是Fcitx Logo
// 第二个是当前的输入法
void SystemTrayMenu::RegisterProperties(const QList<KimpanelProperty> &props)
{
    if (props.size() < 2)
        return;

    int count = 0;
    mStatusMenuList.clear();
    mCurrentIMLable = props[1].label;

    foreach(const KimpanelProperty &prop, props)
    {
        qDebug() << QString("SystemTrayMenu::RegisterProperties(key:%1 label:%2 icon:%3 tip:%4 state:%5 menu:%6)\n").arg(prop.key)
            .arg(prop.label).arg(prop.icon).arg(prop.tip).arg(prop.state).arg(prop.menu);
        if (count ++ < StatusMenuSkip)
            continue;
        if (prop.key == "/Fcitx/vk")
            continue;
        mStatusMenuList.append(prop);
    }
}

void SystemTrayMenu::ExecMenu(const QList<KimpanelProperty> &props)
{
    qDebug() << "ExecMenu--------------" ;
    if(props.isEmpty())
            return;
    QList<KimpanelProperty>::const_iterator iter = props.begin();
    if(iter->key.isNull())
        return;

    if (IsIMList(iter->key))
        mIMList = props;
    else if(IsVKList(iter->key))
    {
        mVKList = props;
    }
    else if (IsMozcHiraganaList(iter->key))
        mMozcHiraganaList=props;
    else if (IsMozcToolList(iter->key))
        mMozcToolList = props;
    else
        qDebug() << "SystemTrayMenu: unknown prop type";
    UpdateMainMenu();

}

void SystemTrayMenu::UpdateProperty(const KimpanelProperty& prop)
{
    if (prop.label == "")
        return;

    mCurrentIMLable = prop.label;
}

void SystemTrayMenu::OnDelayShowTimeOut()
{
    qDebug() << "OnDelayShowTimeOut was called at: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QMenu::setVisible(mIsShowMenu);
}

bool SystemTrayMenu::eventFilter(QObject * object, QEvent * event)
{
#ifdef QT5
    if(object == this && event->type() == QEvent::MouseMove)
    {
            QPointF lp = ((QMouseEvent*)event)->localPos();
            int actionSize = this->actions().size();

            bool flag = false;
            for (int i = 0; i < actionSize; ++i)
            {
                QRect rc = this->actionGeometry(this->actions()[i]);
                if (rc.contains(lp.toPoint()))
                {
                    // 鼠标滑动选中action时，更改该action的icon...
                    flag = true;
                }
                else
                {
                    // 鼠标滑出action时，更改该action的icon...
                }
            }

            if (!flag)
                this->hide();

    }
#endif
    return false;
}

void SystemTrayMenu::leaveEvent(QEvent* event)
{
    event->accept();
    if(!mVKListMenu->isVisible())
    {
        this->hide();
    }
}

bool SystemTrayMenu::IsIMList(const QString &key)
{
    return key.startsWith("/Fcitx/im");
}

bool SystemTrayMenu::IsVKList(const QString &key)
{
    return key.startsWith("/Fcitx/vk");
}

bool SystemTrayMenu::IsMozcHiraganaList(const QString &key)
{
    return key.startsWith("/Fcitx/mozc-composition-mode");
}

bool SystemTrayMenu::IsMozcToolList(const QString &key)
{
    return key.startsWith("/Fcitx/mozc-tool");
}

bool SystemTrayMenu::IsUnity()
{
    const char* desktop = getenv("XDG_CURRENT_DESKTOP");
    return desktop && !strcmp(desktop, "Unity");
}

QString SystemTrayMenu::DoUpdateIMListMenu(const QList<KimpanelProperty> props)
{
    MyAction* firstAction = NULL;
    MyAction* action = NULL;
    MyAction* actionChecked = NULL;
    QString value = "fcitx-kbd";
    bool checked = false;
    for (QList<KimpanelProperty>::const_iterator iter=props.begin(); iter!=props.end(); ++iter)
    {
        action = new MyAction(iter->label, this);
        action->SetProp(*iter);
        action->SetMyActionType(IMAction);
        this->addAction(action);
        if (firstAction == NULL)
            firstAction = action;

        if (iter->label == "华宇输入法")
        {
            checked = true;
            actionChecked = action;
            action->setCheckable(true);
            action->setChecked(true);
        }
    }

    if (!checked && firstAction)
    {
        actionChecked = firstAction;
        firstAction->setChecked(true);
    }

    if (actionChecked == NULL)
        value = "fcitx-kbd";

    return value;
}

void SystemTrayMenu::DoUpdateVKListMenu(const QList<KimpanelProperty> props)
{
    MyAction* menu = NULL;
    QList<KimpanelProperty>::const_iterator iter;
    mVKListMenu->clear();
    QActionGroup* actionGroup = new QActionGroup(this);
     actionGroup->setExclusive(true);
    for (iter = props.begin(); iter != props.end(); ++ iter)
    {
        menu = new MyAction(QIcon::fromTheme(iter->icon), iter->label, this);
        menu->SetProp(*iter);
        actionGroup->addAction(menu);
        menu->setCheckable(true);
        if(!mCurrentVkName.isEmpty() && (mCurrentVkName == iter->label))
        {
            menu->setChecked(true);
        }

        mVKListMenu->addAction(menu);
    }
    addMenu(mVKListMenu);
}

void SystemTrayMenu::DoUpdateMozcHiraganaListMenu(const QList<KimpanelProperty> props)
{
    MyAction* menu = NULL;
    QList<KimpanelProperty>::const_iterator iter;
    mMozcToolMenu->clear();
    for (iter = props.begin(); iter != props.end(); ++ iter)
    {
        menu =new MyAction(QIcon(iter->icon), iter->label, this);
        menu->SetProp(*iter);
        mMozcToolMenu->addAction(menu);
    }
    addMenu(mMozcToolMenu);
}

void SystemTrayMenu::DoUpdateMozcToolListMenu(const QList<KimpanelProperty> props)
{
    MyAction* menu = NULL;
    QList<KimpanelProperty>::const_iterator iter;
    mMozcHiraganaMenu->clear();
    for (iter = props.begin(); iter != props.end(); ++ iter) {
        menu = new MyAction(QIcon(iter->icon), iter->label, this);
        menu->SetProp(*iter);
        mMozcHiraganaMenu->addAction(menu);
    }
    addMenu(mMozcHiraganaMenu);
}


void SystemTrayMenu::DoUpdateStatusMenu(const QList<KimpanelProperty> props)
{
    foreach(const KimpanelProperty& prop, props)
    {
        MyAction* menu = NULL;
        if(prop.icon.contains("/share/fcitx"))
            menu = new MyAction(QIcon(prop.icon), prop.label, this);
        else
            menu = new MyAction(QIcon::fromTheme(prop.icon), prop.label, this);
        menu->SetMyActionType(StatusAction);
        menu->SetProp(prop);
        addAction(menu);
    }
}

bool SystemTrayMenu::IsKbdIcon(const QString &icon)
{
    return (icon=="fcitx-kbd" || icon=="" || icon.indexOf("indicator-keyboard")!=-1);
}

void SystemTrayMenu::SkinMenuItemOnClick(QAction* action)
{
    if (!action)
    {
        qDebug() << "parameter action is null.";
        return;
    }

    SkinBase* skin;
    MyAction* myAction = (MyAction*)action;
    MainController::Instance()->SetSkinName(myAction->text());

    if (FCITX == myAction->GetSkinClass())
        skin = new SkinFcitx;
    else
        qDebug() << "Load skin failed!";

    skin->loadSkin(myAction->GetSkinPath());
    MainController::Instance()->SetSkinBase(skin, myAction->GetSkinClass());
}

void SystemTrayMenu::OpenConfigTool()
{
    QStringList pramList;
    pramList << "14760634-a01a-4919-872a-f5844996b1d2";
    QProcess::startDetached(Utils::GetBinFilePath().append("huayupy-config-tools-fcitx"), pramList);
}

void SystemTrayMenu::ShowORHideToolBar()
{
    QString configFilePath = Config::Instance()->configFilePath();
    QSettings setting(configFilePath,QSettings::IniFormat);
    bool hide_tray = setting.value("skin/hide_tray", 0).toBool();
    int _value = hide_tray ? 0 : 1;
    setting.setValue("skin/hide_tray", _value);
    emit hideTray(_value);
}

void SystemTrayMenu::RestartFcitx()
{
    FILE* command_ptr = NULL;
    string command = "fcitx -r";
    if((command_ptr = popen(command.c_str(), "r")) != NULL)
    {
        pclose(command_ptr);
    }
    QApplication::exit();
}

void SystemTrayMenu::ExitTheFcitx()
{
    FILE* command_ptr = NULL;
    string command = "pkill -9 fcitx";
    if((command_ptr = popen(command.c_str(), "r")) != NULL)
    {
        pclose(command_ptr);
    }
}
