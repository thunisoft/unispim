#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QMap>
#include <QVariant>
#include <QVector>

using namespace std;

struct ConfigItemStruct
{
    QString itemName;
    QString itemGroupName;
    QString itemType;
    QString itemDefaultStrValue;
    QString itemCurrentStrValue;
    int itemDefaultIntValue;
    int itemCurrentIntValue;
    ConfigItemStruct()
    {
        itemName = "";
        itemGroupName = "user";
        itemType = "int";
        itemDefaultStrValue = "";
        itemCurrentStrValue = "";
        itemCurrentIntValue = 1;
        itemDefaultIntValue = 1;
    }
};

class Config
{

public:

    static Config* Instance();    
    QString configJsonFilePath();
    static bool SetUserNameToReg(bool flag);
    static bool Is64BitSystem();
    static void WriteVersionNumToIniFile(QString current_version);
    static bool SendMsgToEngine();

public:
    Config();
    ~Config();
public:
    //加载和保存配置选项
    void LoadConfig();

private:
    void LoadSystemConfig();
    bool LoadMapInfoFromJson(QString filePath, QMap<QString,ConfigItemStruct>& infoMap);
    void MergeConfigInfo();

public:
    //获取对应的配置项
    bool GetConfigItemByJson(QString key,ConfigItemStruct& keyValue);
    QString GetHostMacAddress();
    QString GetOSVersion();
    QString GetCPUType();
    QString GetIMEVersion();


private:
    static Config* m_instance;

    QMap<QString,ConfigItemStruct> m_currentConfigMap;

    //系统的配置信息(这个是不变的)
    QMap<QString,ConfigItemStruct> m_systemConfigMap;

};

#endif // CONFIG_H
