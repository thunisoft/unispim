#ifndef DBUSMSGMANAGER_H
#define DBUSMSGMANAGER_H

#include <QObject>

#include "../public/defines.h"

class DbusMsgManager : public QObject
{
    Q_OBJECT
public:
    DbusMsgManager();

private:
    void Init();
    bool JudgeInputFrame(int input_frame);
    void HandleMsgFromSettings(int msg_info);

public slots:
    void OnReceiveMsgFromSettings(int input_frame, int msg_info);
    void OnReceiveMsgFromEngine(int msg_info, int input_num);
    void MsgToSettings(int msg);
signals:
    void CheckUpdate();
    void UpdateWordlib(int login_or_logout);
    void StatisticInputInfo(int input_num);
};

#endif // DBUSMSGMANAGER_H
