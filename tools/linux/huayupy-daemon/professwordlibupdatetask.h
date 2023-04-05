#ifndef PROFESSWORDLIBUPDATETASK_H
#define PROFESSWORDLIBUPDATETASK_H

#include <QThread>
#include <QVector>
#include "networkhandler.h"
#include "configbus.h"
#include "../public/config.h"
#include "../public/utils.h"


class ProfessWordlibUpdateTask : public QThread
{
    Q_OBJECT
public:
    ProfessWordlibUpdateTask() = default;
    ~ProfessWordlibUpdateTask() = default;
protected:
    virtual void run() override;


public:
    bool download_wordlib_from_server();
    void WriteLog(const QString inputStr);
    void FilterRepeatWordlibNameItem();

private:
    void task_sleep();

private:
    QVector<WordlibListInfo> m_server_wordlibinfo_vector;
    QVector<WordlibListInfo> m_hasupdate_wordlib_vector;
    bool m_task_flag = true;
};

#endif // PROFESSWORDLIBUPDATETASK_H
