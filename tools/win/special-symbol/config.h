#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QVector>

class Config
{
private:
    Config();

public:
    static Config* Instance();

public:
    QVector<QString> GetHistorySymbol();
    void SetHistorySymbol(QVector<QString>& symbolVector);

private:

     //获取loginid
     QString GetLoginID_MD5();
     QString GetSymbolHistoryFilePth();
     QString GetConfigFilePath();

public:
     static Config* m_instance;

};

#endif // CONFIG_H
