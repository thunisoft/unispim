#ifndef _TOOLBOX_CONFIG_H
#define _TOOLBOX_CONFIG_H

#include <QString>
#include <QMap>
#include <QVariant>
#include <QVector>
#include <mutex>
#include <QString>
#include <QUrl>
#include "commondef.h"

using namespace std;
#define DEBUG

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
    static QString GetHostMacAddress();
    static Config* Instance();
    static QString get_user_config_dir_path();
    static QString get_state_config_dir_path();
    static QString get_user_addon_dir_path();
    static QString get_install_bin_path();
public:
    Config();
    ~Config();
public:
    //加载和保存配置选项
    void LoadConfig();
    void SaveConfig();
    void SaveUserConfig();
    void ClearUserConfig();
    void SaveStateConfig();
    void ReloadConfig();
    void SaveHotkeyInfo();
    void ChangeHotkey(QString addon_name, QString shortcut);
    void UpdateToolStatus(QString addon_name,QString status);
    void AddNewAddonInfo(TOOL_ADDON_INDEX_INFO addon_index);
    void ReloadUserAddon();
    void RemoveUserAddon(QString addonName);

private:
    bool SaveConfigInfoToJson(QString filePath,QMap<QString,ConfigItemStruct> infoMap);
    void LoadSystemConfig();
    void LoadUserConfig();
    void SaveUserAddonInfo();

    void LoadStateConfig();
    bool LoadMapInfoFromJson(QString filePath, QMap<QString,ConfigItemStruct>& infoMap);
    bool LoadMapAddonInfoFromJson(QString system_addon_dir);
    void LoadSystemAddon();
    void LoadUserAddon();
    void LoadHotkeyInfo();

private:
    void MergeConfigInfo();
    bool IsAddonExist(QString guid);
    bool IsAddonExistByName(QString addon_name);
    TOOL_ADDON_INFO GetToolAddonInfo(QString file_path);
    bool LoadHotkeyinfoFromFile(QString file_path);
    bool LoadAddonInfoFromFile(QString file_path,QVector<TOOL_ADDON_INFO>& output_info_vector);

public:
    bool GetConfigItemByJson(QString key,ConfigItemStruct& keyValue);
    int SetConfigItemByJson(QString key,ConfigItemStruct keyValue);

    QString configJsonFilePath();
    QString GetHttpsCertificatePath();
    QVector<TOOL_ADDON_INFO> GetSystemAddonVector();
    QVector<TOOL_ADDON_INFO> GetUsermAddonVector();
    QVector<TOOL_ADDON_INFO> GetAllAddonVector();
    QVector<TOOL_ADDON_INFO> GetAvailableAddonVector();
    QMap<QString,QString> GetHotkeyMap();
    QString GetLinuxServerDomain();    
    bool GetToolAddonInfoByName(QString addon_name, TOOL_ADDON_INFO &index_addon_info);
private:
    static Config* m_instance;
    static std::mutex m_Mutex;
    QMap<QString, ConfigItemStruct> m_userChangedMap;  ///<用户修改的配置信息
    QMap<QString,ConfigItemStruct> m_currentConfigMap; ///<整合在一起的配置信息

    //系统的配置信息(这个是不变的)
    QMap<QString,ConfigItemStruct> m_systemConfigMap;  ///<系统配置配置信息
    //用户的配置信息
    QMap<QString,ConfigItemStruct> m_userConfigMap;    ///< 用户的配置信息
    QMap<QString,ConfigItemStruct> m_stateConfigMap;   ///< 状态配置信息
    QVector<TOOL_ADDON_INFO> m_system_addon_info_vector;
    QVector<TOOL_ADDON_INFO> m_user_addon_info_vector;
    QMap<QString,QString> m_addon_shortcut_map;
};

#endif // CONFIG_H
