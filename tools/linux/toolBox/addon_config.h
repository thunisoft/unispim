#ifndef _TOOLBOX_CONFIG_H
#define _TOOLBOX_CONFIG_H

#include <QString>
#include <QMap>
#include <QVariant>
#include <QVector>
#include <mutex>
#include <QString>
#include <QUrl>
#include "../../../toolBox/commondef.h"

using namespace std;
#define DEBUG

class AddonConfig
{

public:
    static AddonConfig* Instance();
    static QString get_user_addon_dir_path();
public:
    AddonConfig();
    ~AddonConfig();
public:
    //加载和保存配置选项
    void UpdateToolStatus(QString addon_name,QString status);
    void AddNewAddonInfo(TOOL_ADDON_INDEX_INFO addon_index);
    bool IsAddonExist(QString addon_name);
    bool GetToolAddonInfoByName(QString addon_name, TOOL_ADDON_INFO &index_addon_info);
    void LoadUserAddon();

private:
    void SaveUserAddonInfo();
    void LoadSystemAddon();

private:

    TOOL_ADDON_INFO GetToolAddonInfo(QString file_path);
    bool LoadAddonInfoFromFile(QString file_path,QVector<TOOL_ADDON_INFO>& output_info_vector);

public:
    QVector<TOOL_ADDON_INFO> GetSystemAddonVector();
    QVector<TOOL_ADDON_INFO> GetUsermAddonVector();
    QVector<TOOL_ADDON_INFO> GetAllAddonVector();
    QVector<TOOL_ADDON_INFO> GetAvailableAddonVector();

private:
    static AddonConfig* m_instance;
    static std::mutex m_Mutex;

    QVector<TOOL_ADDON_INFO> m_system_addon_info_vector;
    QVector<TOOL_ADDON_INFO> m_user_addon_info_vector;
    QVector<TOOL_ADDON_INDEX_INFO> m_user_addon_index_info;
};

#endif // CONFIG_H
