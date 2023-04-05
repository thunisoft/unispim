/**@file  spschememodel.h
* @brief       主要是双拼方案的数据模型
* @details    主要记录的双拼按键的匹配和音节的匹配
* @date        2020-9-18
* @version     V1.0
* @copyright    Copyright (c) 2018-2020  华宇信息
**********************************************************************************
* @attention
* 注意事项1: 有的音节能进行配置有的音节不能进行配置需要进行设定
* 注意事项2：不同双拼方法的配置策略不同，需要单独设定规则
* @par 修改日志:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* </table>
*
**********************************************************************************
*/
#ifndef SPSCHEMEMODEL_H
#define SPSCHEMEMODEL_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QSet>
#include <QTextStream>


struct Key_Syllable_Table_Stru
{
    QString initial_syllable;       ///< 按键映射的声母
    QSet<QString> final_syllable_Set; ///< 按键映射的韵母
};

class SpSchemeModel : public QObject
{
    Q_OBJECT

public:

    /**@brief 双拼方案模型的工厂方法
   * @param[in]  hasbasic 是否从一个现有的方案进行创建
   * @param[in]  basic_scheme_path 现有方案的文件地址
   * @return  函数方案的实例(路径无效返回空实例)
   */
    static SpSchemeModel* CreateSpSchemeMoel(bool hasbasic, QString basic_scheme_path = "");

public:
    explicit SpSchemeModel();
    SpSchemeModel(QString basic_scheme_path);
    SpSchemeModel(SpSchemeModel& model);
    SpSchemeModel& operator=(const SpSchemeModel& model);
public:
    ~SpSchemeModel();

public:
    /**@brief 获取按键和音节的映射表
   * @return  按键和音节的映射Map
   */
    inline QMap<QString, Key_Syllable_Table_Stru> GetKeySyllableMap()
    {
        return m_key_use_map;
    }


    /**@brief 获取声母配置的容器
   * @return  返回声母与对应的按键对应的配置
   */
    inline QMap<QString,QString> GetConfigInitial()
    {
        return m_configurable_initial_map;
    }


    /**@brief 设置声母对应的按键映射表
   * @param[in]  config_initial_map 设置声母配置对应的按键映射表
   */
    void SetConfigInitial(QMap<QString,QString>& config_initial_map);
    /**@brief 获取韵母对应的配置表
   * @return  返回韵母与对应按键的映射表
   */
    inline QMap<QString,QString> GetConfigFinal()
    {
        return  m_configurable_final_map;
    }

    /**@brief 甚至韵母与对应按键的映射表
   * @param[in]  config_final_map 设置韵母按键对应的映射表
   */
    void SetConfigFinal(QMap<QString,QString>& config_final_map);

    /**@brief 获取音节可配置的按键组合
   * @param[in]  syllable 当前输入的音节
   * @return 按键组合对应的容器
   */
    QVector<QString> GetSyllableAvailableKeys(QString& syllable);

    /**@brief 设置音节和对应按键的映射
   * @param[in]  syllable 对应的音节
   * @param[in]  key 音节设置的按键
   */
    void SetSyllableAndKeypair(QString syllable, QString key);


    /**@brief 将方案保存到对应的目录文件下
   * @param[in]  filePath 文件的目录地址
   * @return 保存文件是否成功
   * - 0 保存成功
   * - 1 保存失败(文件已经存在)
   * - 2 其它错误
   */
    int  SaveSPConfigToFile(QString filePath);

    /**@brief 检查双拼方案的正确性
   * @return 文件是否合理
   * - 0 合理
   * - 1 不合理
   */
    int CheckSPPlan();
private:
    /**@brief 初始化各种固定的音节键位
   */
    void InitDataModel();

    /**@brief 初始化按键对应的映射表
   */
    void InitKeyMapModel();

    /**@brief 解析对应的双拼方案的ini文件
   * @param[in]  scheme_path 双拼方案所在的目录
   */
    void ParserSPScheme(QString scheme_path);

    /**@brief 解析UTF-16LE的函数接口
   * @param[in]  fileName 文件的地址
   * @param[in]  groupname 对应的组名
   * @param[in]  result 查询的映射结果
   */
    void ParserUTF16INIFile(QString fileName, QString groupname, QMap<QString,QString>& result);

    /**@brief 将内容输出到文件
   * @param[in]  data_map 数据映射
   * @param[in]  outputStream 对应的组名
   */
    void WriteMapToDataStream(QMap<QString,QString>& data_map, QTextStream& outputStream);
public:
    QVector<QString> m_all_key_vector;///< 所有按键的容器
    QVector<QString> m_all_initial_key_vector; ///< 所有声母按键(zh、ch、sh)
    QVector<QString> m_all_syllable_vector; ///< 所有可配置的音节的组合
    QVector<QString> m_special_syllbale; ///< 配置过程中按键比较特殊的音节，用于设置规则

    QVector<QString> unconfiged_syllable_vector() const;

private:
    QMap<QString,QString> m_fixed_initial_map; ///< 固定键位的声母
    QMap<QString,QString> m_fixed_final_map; ///< 固定键位的韵母

    QMap<QString,QString> m_configurable_initial_map; ///< 可配置键位的声母 zh ch sh
    QMap<QString,QString> m_configurable_final_map;  ///< 可配置键位的韵母
    QMap<QString, QString> m_zero_final_map;   ///< 零声母的键位配置(从现有的方案中继承过来的)

    QMap<QString, Key_Syllable_Table_Stru> m_key_use_map;  ///< 按键占用的映射表

    bool m_ismodel_valid;

    QVector<QString> m_unconfiged_syllable_vector;
};

#endif // SPSCHEMEMODEL_H
