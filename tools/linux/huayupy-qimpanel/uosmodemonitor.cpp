// Uos 有高性能模式与特效模式
// 高性能模式不允许使用圆角和阴影

#include "uosmodemonitor.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QVariant>
#include "candidatewindow.h"

UosModeMonitor::UosModeMonitor(CandidateWindow* candidate_window):
                              m_candiadte_window(candidate_window)
{
    Init();
}

// 是否是特效模式
bool UosModeMonitor::IsSpecialMode()
{
    QDBusInterface remoteApp( _SERVICE_, _PATH_,
                              "org.freedesktop.DBus.Properties", QDBusConnection::sessionBus() );
    QDBusReply<QVariant> reply = remoteApp.call( "Get", _INTERFACE_, "active");


    do
    {
        if(!reply.isValid())
        {
            qDebug() << "get uos current mode error! "<< reply.error().message();
            break;
        }
        else if(!(reply.value().type() == QVariant::Bool))
        {
            qDebug() << "get uos current mode error! " << QString("return type is not boolean!");
            break;
        }
        else
            return reply.value().toBool();

    }while(false);

    return false;
}

// 监控模式变化
void UosModeMonitor::ModeChanged(bool value)
{
    qDebug() << "show mode changed " << value;
    m_candiadte_window->SetSystemMode(value);
}

// 注册监听接口
void UosModeMonitor::Init()
{
    QDBusConnection::sessionBus().unregisterService(_SERVICE_);
    QDBusConnection::sessionBus().registerService(_SERVICE_);
    QDBusConnection::sessionBus().registerObject(_PATH_, this,QDBusConnection :: ExportAllSlots | QDBusConnection :: ExportAllSignals);
    QDBusConnection::sessionBus().connect("", _PATH_, _INTERFACE_, "compositingToggled", this, SLOT(ModeChanged(bool)));

}
