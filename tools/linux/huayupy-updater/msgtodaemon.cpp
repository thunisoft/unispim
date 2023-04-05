#include "msgtodaemon.h"

void MsgToDaemon::SendMsgToDaemon(QString msg)
{
    QDBusMessage dbusMsg = QDBusMessage::createSignal("/", "com.thunisoft.update", "msgTodaemon");
    dbusMsg<<msg;
    QDBusConnection::sessionBus().send(dbusMsg);
    qDebug()<<"sent message";
}
