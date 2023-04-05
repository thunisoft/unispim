#include "mysqlite.h"
#include "config.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlRecord>
#include "utils.h"


MySQLite* MySQLite::m_instance = NULL;

MySQLite* MySQLite::instance()
{
    if(!m_instance)
    {
        m_instance = new MySQLite();
        return m_instance;
    }
    return m_instance;
}

MySQLite::MySQLite(QObject* parent):QObject(parent)
{
    InitTheDatabase();
}

MySQLite::~MySQLite()
{

}

void MySQLite::InitTheDatabase()
{

    m_sqldatabase = QSqlDatabase::addDatabase("QSQLITE");
    m_sqldatabase.setDatabaseName(Config::databasePath());

    if(!m_sqldatabase.open())
    {
        Utils::WriteLogToFile(QString("数据库连接失败:%1").arg(m_sqldatabase.lastError().text()));
    }
    else
    {
        Utils::WriteLogToFile("数据库连接成功");
    }
    //创建用户信息表
    QSqlQuery sql_query;

    sql_query.exec("select count(*) from sqlite_master where type='table' and name='myaccountinfo'");

    if(sql_query.next())
    {
        int tableCount = sql_query.value(0).toUInt();
        if(tableCount == 0)
        {
            if(!sql_query.exec("create table myaccountinfo(id int primary key, account text, password text)"))
            {
                qDebug() << "Error:Fail to create table" << sql_query.lastError();
            }
            else
            {
                qDebug() << "Succeed to Create the table";
            }
        }

    }

}

 bool MySQLite::InsertUserInfo(QString userName, QString password)
 {
    QSqlQuery sql_query(m_sqldatabase);
    sql_query.prepare("INSERT INTO myaccountinfo VALUES(NULL,?,?)");
    sql_query.bindValue(0,userName);
    sql_query.bindValue(1,password);
    if(!sql_query.exec())
    {
        qDebug() << sql_query.lastError();
        return  false;
    }
    else
    {
        return true;
    }
 }

 bool MySQLite::DeleteUserInfo(QString userName)
 {
     QSqlQuery sql_query(m_sqldatabase);
     sql_query.prepare("delete from myaccountinfo where account=?");
     sql_query.bindValue(0,userName);

     if(!sql_query.exec())
     {
         return  false;
     }
     else
     {
         return true;
     }
 }


 bool MySQLite::UpdateUserInfo(QString userName, QString password)
 {
     QSqlQuery sql_query(m_sqldatabase);
     sql_query.prepare("update myaccountinfo set password=? where account=?");
     sql_query.bindValue(0,password);
     sql_query.bindValue(1,userName);

     if(!sql_query.exec())
     {
         return  false;
     }
     else
     {
         return true;
     }
 }

 QString MySQLite::SelecteUserPassword(QString userName)
 {
     QSqlQuery sql_query(m_sqldatabase);
     sql_query.prepare("select * from myaccountinfo where account=?");
     sql_query.bindValue(0,userName);

     if(!sql_query.exec())
     {
        return "";
     }
     else
     {
         while(sql_query.next())
         {
             QString password = sql_query.value(2).toString();
             return password;
         }
     }
     return "";
 }

 bool MySQLite::IsUserExist(QString userName)
 {
     QSqlQuery sql_query(m_sqldatabase);
     sql_query.prepare("select * from myaccountinfo where account=?");
     sql_query.bindValue(0,userName);

     if(!sql_query.exec())
     {
        return false;
     }
     else
     {
         while(sql_query.next())
         {
             return true;
         }
     }
     return false;
 }



