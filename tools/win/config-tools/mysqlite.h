#ifndef SQLITEINTERFACE_H
#define SQLITEINTERFACE_H
#include <QObject>
#include <QSqlDatabase>


class MySQLite : public  QObject
{
    Q_OBJECT

public:
    static MySQLite* instance();
    explicit MySQLite(QObject* parent = 0);
    ~MySQLite();

private:
    void InitTheDatabase();

public:
    //向数据库中增加用户信息
    bool InsertUserInfo(QString userName, QString password);

    //删除对应的用户信息
    bool DeleteUserInfo(QString userName);

    //修改用户的密码信息
    bool UpdateUserInfo(QString userName, QString password);

    //查询对应用户的密码信息
    QString SelecteUserPassword(QString userName);

    bool IsUserExist(QString userName);

private:
    static MySQLite* m_instance;
    QSqlDatabase m_sqldatabase;
};

#endif // SQLITEINTERFACE_H
