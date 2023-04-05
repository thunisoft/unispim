#ifndef CONFIG_H
#define CONFIG_H
#define DEFAULT_SERVER_IP   "172.16.160.74"
#define DEFAULT_SERVER_PORT "443"

#include <QString>
#include <QMap>
#include <QVariant>
#include <QVector>
#include <QUrl>
#include <Windows.h>
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

struct SystemWordlibInfoStru
{
    QString date_str;
    QString name_str;
    int valid;
    int block_num;
    QString id_str;

};

/**@struct WORDLIB_BLOCK_INFO
* @brief  词块的信息\n
* 词块的信息
*/
struct WORDLIB_BLOCK_INFO
{
    QString block_md5; ///< 词库添加之后的md5值
    QString  block_filename; ///< 词块文件的名称
    QString block_url;///< 词块地址对应的url
    int block_index; ///< block序号
};

class Config
{

public:
    static QString GetHostMacAddress();
    static Config* Instance();
    static QString wordLibDirPath();
    static QString GetIMEVersion();
    static bool RemoveTaskBarRightClickName();
    static QString GetHttpsCertificatePath();
    static QString SystemWordlibPath();
    static QString GetInstallDir();
    static QString GetProgramdataDir();

    QString configJsonFilePath();
    static QString databasePath();
    QString CustomPhraseFilePath();
    QString UserWordlibFilePath();
    QString GetLoginID();
    bool ChangeLoginStateWhenExit();

    /**@brief 返回用于检查版本更新的文件地址
   * @return  版本检查文件ini对应的路径
   */
    QString GetVersionCheckIniPath();

    /**@brief 判断是否有新版本
   * @return  函数执行结果
   * - true 有新版本
   * - false 没有新版本
   */
    bool HasNewVersion();

    /**@brief 判断是否有新版本
   * @return 函数安装包的名称
   */
    QString DownloadPackageName();

    bool CheckValidation(QString &message);
    //QString AddPublishWordlibToUser(QString )

public:
    Config();
    ~Config();
public:
    //加载和保存配置选项
    void LoadConfig();
    void SaveConfig();
    void SaveSystemConfig();
    void SaveUserConfig();
    void ClearUserConfig();
    void SaveStateConfig();

    void ShowNewFeature();

    void ReloadConfig();
    bool LoadPublishWordlib();

private:
    bool SaveConfigInfoToJson(QString filePath,QMap<QString,ConfigItemStruct> infoMap);

    void LoadSystemConfig();
    void LoadUserConfig();
    /**@brief 获取状态配置信息:\n
     * 用户是否登录，登录的用户名\n
     * 是否记住密码，是否自动版本更新
   */
    void LoadStateConfig();
    bool LoadMapInfoFromJson(QString filePath, QMap<QString,ConfigItemStruct>& infoMap);

    bool check_bracket_item_conflict();
    bool check_shift_ctrl_conflict();

private:
    void MergeConfigInfo();
public:
    inline QString GetSPIniFileDir()
    {
        return Config::GetProgramdataDir() + "ini\\";
    }

    QString GetUserSPIniFileDir();

    inline bool GetLayoutVector(QString key, QVector<int>& valueVector)
    {
        if(!m_layoutMap.contains(key))
        {
            return false;
        }
        valueVector = m_layoutMap[key];
        return true;

    }
    inline double GetGeoRate()
    {
        return m_geoScaledRate;
    }
    inline void SetGeoRate(double inputRate)
    {
        m_geoScaledRate = inputRate;
    }
    inline double GetFontScaledRate()
    {
        return m_fontScaledRate;
    }
    inline void SetFontScaledRate(double inputRate)
    {
        m_fontScaledRate = inputRate;
    }

    //获取对应的配置项
    bool GetConfigItemByJson(QString key,ConfigItemStruct& keyValue);

    //return 0 success
    //return 1 ctrl shift conflict
    //return 2 [] conflict
    int SetConfigItemByJson(QString key,ConfigItemStruct keyValue);
    int SetStateConfigItemByJson(QString key, ConfigItemStruct keyValue);
    int SetConfigItemByJsonWithoutCheck(QString key,ConfigItemStruct keyValue);
    QVector<QString> GetSelectedWordlibContainer();
    void SetSelectedWordlibContainer(const QVector<QString> newWordlibVector);

    bool GetDefualtConfigItem(QString key, ConfigItemStruct& keyValue);

    bool IsToolBarChanged();

    /**@brief 将临时配置保存到内存中，并不会讲修改添加到文本文件中
   * @param[in]  key 配置项对应的键
   * @param[in]  keyValue 配置项对应的值
   */
    void SetTmpConfigItemByJson(QString key,ConfigItemStruct keyValue);

    /**@brief 获得双拼方案的列表
   * @return  双拼方案的列表
   */
    QList<QString> GetSPSchemeList();

    QList<QString> GetDefaultSpSchemeList();

    /**@brief 添加用户自定义的双拼方案
   * @param[in]  fileName 双拼方案的名称
   * @param[in]  flag 操作是删除还是添加
   */
    void AddOrDeleteTheSpScheme(QString fileName, bool flag);

    void setTempDomain(const QString tempDomain);

public:
    void make_config_work();
    void make_state_config_work();
    
    QString GetOSVersion();
    QString GetCPUType();
private:
    static Config* m_instance;
    QMap<QString, ConfigItemStruct> m_userChangedMap;  ///<用户修改的配置信息

    double m_geoScaledRate; ///<依据分辨率尺寸变化系数
    double m_fontScaledRate; ///<依据分辨率字体变化系数
    QMap<QString, QVector<int> > m_layoutMap;

    QMap<QString,ConfigItemStruct> m_currentConfigMap; ///<整合在一起的配置信息

    //系统的配置信息(这个是不变的)
    QMap<QString,ConfigItemStruct> m_systemConfigMap; ///<系统配置配置信息
    //用户的配置信息
    QMap<QString,ConfigItemStruct> m_userConfigMap;  ///< 用户的配置信息

    QMap<QString,ConfigItemStruct> m_stateConfigMap;  ///< 状态配置信息

    QString m_tempDomain;

    bool has_domainChanged;
    HANDLE m_configCountSem = NULL;
};

#endif // CONFIG_H
