/**@file  spkeyboard.h
* @brief       自定义双拼按键的键盘widget
* @details    详细描述

* @date        2020-9-21
* @version     V1.0
* @copyright    Copyright (c) 2018-2020  华宇信息
**********************************************************************************
* @attention
* 注意事项1:
* @par 修改日志:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* </table>
*
**********************************************************************************
*/
#ifndef SPKEYBOARD_H
#define SPKEYBOARD_H

#include <QWidget>
#include <QGridLayout>
#include <QVector>
#include <QMap>
#include <QSpacerItem>
#include "config.h"
#include "spschememodel.h"

class SingleKey;
class SPKeyBoard : public QWidget
{
    Q_OBJECT
public:
    explicit SPKeyBoard(QWidget *parent = nullptr);


public:
    void RefreshTheWidget();
    void ResetTheWidget();

    void SetSpSchemeModel(SpSchemeModel scheme_model);

private:
    /**@brief 初始化UI显示
   */
    void InitWidget();


private:
    SpSchemeModel m_sp_scheme_model;
    QVector<SingleKey*> m_key_vector;

};

#endif // SPKEYBOARD_H
