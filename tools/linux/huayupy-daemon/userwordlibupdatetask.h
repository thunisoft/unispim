#ifndef USERWORDLIBUPDATETASK_H
#define USERWORDLIBUPDATETASK_H

#include <QThread>


class UserWordlibUpdateTask : public QThread
{
public:
    UserWordlibUpdateTask() = default;
    ~UserWordlibUpdateTask() = default;

protected:
    virtual void run() override;

private:
    int update_user_wordlib();
    void task_sleep();
    void WriteLogToFile(const QString &content);
    int RestoreUserWordlib();
    QString GetUserWordlibPath();
    bool IsWordlibValid(const QString &wordlib_path);
    int LoginedUpdateUserWordlib();
    int DownloadAndCombineUserWordlib();
    bool combine_wordlib(QString destwordlib_path,QString sourcewordlib_path);
};

#endif // USERWORDLIBUPDATETASK_H
