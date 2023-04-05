// com.thunisoft.huayupy.xml 定义了接口
// DBus 消息管理，这里是实现

#include "dbusmsgmanager.h"

#include "huayupy_adaptor.h"
#include "huayupy_interface.h"

DbusMsgManager::DbusMsgManager()
{
    Init();
}


void DbusMsgManager::Init()
{
    new HuayupyAdaptor(this);
    QDBusConnection::sessionBus().registerObject("/", this, QDBusConnection::ExportAllSlots);
    QDBusConnection::sessionBus().registerService("com.thunisoft.huayupy");
    new com::thunisoft::huayupy(QString(), QString(), QDBusConnection::sessionBus(), this);
    QDBusConnection::sessionBus().connect(QString(), QString(), "com.thunisoft.huayupy", "msgFromSettingsToDaemon", this, SLOT(OnReceiveMsgFromSettings(int,int)));
    QDBusConnection::sessionBus().connect(QString(), QString(), "com.thunisoft.huayupy", "msgFromEngineToDaemon", this, SLOT(OnReceiveMsgFromEngine(int,int)));
}



void DbusMsgManager::OnReceiveMsgFromSettings(int input_frame, int msg_info)
{
    qDebug() << "收到dbus消息，消息为: " << msg_info << " 框架为：" << input_frame;
    if(!JudgeInputFrame(input_frame))
        return;
    HandleMsgFromSettings(msg_info);
}

void DbusMsgManager::OnReceiveMsgFromEngine(int msg_info, int input_num)
{
    if(msg_info == 1)
    {
        qDebug() << "引擎输入消息,输入数为：" << input_num;
        emit StatisticInputInfo(input_num);
    }
}

/*判断是否是当前输入法框架发过来的消息。
·防止出现像ibus设置界面发出消息，被fcitx守护进程捕获，出现交叉处理的情况。
·这个现在已经做了处理， 当切换输入法框架时， 守护进程会判断，如果守护进程不对应当前输入法框架会自动退出，并启动对应的输入法框架*/
bool DbusMsgManager::JudgeInputFrame(int input_frame)
{
#ifdef USE_IBUS
    if(input_frame == IBUS)
        return true;
#else
    if(input_frame == FCITX)
        return true;
#endif
    return false;
}

void DbusMsgManager::HandleMsgFromSettings(int msg_info)
{
    switch(msg_info)
    {
        case IS_ALIVE :
          MsgToSettings(ALIVE);
          break;
        case CHECK_UPDATE_ :
            emit CheckUpdate();
            break;
        case USER_LOGIN :
        case USER_LOGOUT :
            emit UpdateWordlib(msg_info);
            break;
        default:
            break;
    }
}

void DbusMsgManager::MsgToSettings(int msg)
{
    QDBusMessage dbus_msg = QDBusMessage::createSignal("/", "com.thunisoft.huayupy", "msgFromDaemonToSettings");
    dbus_msg << msg;
    QDBusConnection::sessionBus().send(dbus_msg);
}


