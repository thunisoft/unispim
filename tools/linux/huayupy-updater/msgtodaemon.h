#ifndef MSGTODAEMON_H
#define MSGTODAEMON_H

#include  <QtDBus>
class MsgToDaemon{
    public:
        static void SendMsgToDaemon(QString msg);
};

#endif // MSGTODAEMON_H
