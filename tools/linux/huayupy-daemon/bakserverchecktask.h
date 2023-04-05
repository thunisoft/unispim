#ifndef BAKSERVERCHECKTASK_H
#define BAKSERVERCHECKTASK_H

#include <QThread>
#include "networkhandler.h"
#include "../public/config.h"
#include "../public/utils.h"

struct ADDR_INFO
{
    QString name;
    QString scheme;
    int port;
    QString addr;
public:
    ADDR_INFO() = default;
    ADDR_INFO(QString input_name,QString input_scheme,int input_port,QString input_addr)
    {
        name = input_name;
        scheme = input_scheme;
        port = input_port;
        addr = input_addr;
    }
};


class BakServerCheckTask : public QThread
{
public:
    BakServerCheckTask() = default;
    ~BakServerCheckTask() = default;
    void check_to_get_valid_addr();


protected:
    virtual void run() override;
private:
    void task_sleep();
    void WriteLog(const QString inputStr);
    void RecordCheckTime();
    bool is_able_to_check();
    QVector<ADDR_INFO> parser_addr_list(QString file_path);
};

#endif // BAKSERVERCHECKTASK_H
