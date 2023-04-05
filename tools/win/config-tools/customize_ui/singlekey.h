/**@file  SingleKey.h
* @brief       按键键盘中单一按键对应的widget
* @details    详细描述
* @date        2020-9-21
* @version     V1.0
* @copyright    Copyright (c) 2018-2020  华宇信息
**********************************************************************************
* @attention
* 注意事项1:  同一按键上的韵母可以配置两个以上，但是只显示2个
* @par 修改日志:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* </table>
*
**********************************************************************************
*/
#ifndef SINGLEKEY_H
#define SINGLEKEY_H

#include <QWidget>

namespace Ui {
class SingleKey;
}

class SingleKey : public QWidget
{
    Q_OBJECT

public:
    explicit SingleKey(QWidget *parent = nullptr);
    ~SingleKey();
    /**@brief 设置每一个按键UI里面的声母
   * @param[in]  con_str 声母对应的字符串
   */
    void SetConStr(QString con_str);
    /**@brief 设置每个按键的韵母显示
   * @param[in]  vow_str_set 韵母对应的集合
   */
    void SetVowStr(QSet<QString> vow_str_set);

    void SetKeyName(QString key_name);

private:
    void InitWidget();
private:
    Ui::SingleKey *ui;
};

#endif // SINGLEKEY_H
