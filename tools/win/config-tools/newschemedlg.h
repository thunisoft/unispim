/**@file  NewSchemeDlg.h
* @brief      创建双拼方案的界面
* @details    详细描述
* @author      作者及联系方式
* @date        2020年9月22日
* @version     V1.0
* @copyright    Copyright (c) 2018-2020  华宇信息
**********************************************************************************
* @par 修改日志:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* </table>
*
**********************************************************************************
*/
#ifndef NEWSCHEMEDLG_H
#define NEWSCHEMEDLG_H

#include <QWidget>
#include "customize_ui/customize_qwidget.h"
#include "spschememodel.h"

namespace Ui {
class NewSchemeDlg;
}
class SpSchemeModel;
class SPKeyBoard;
class NewSchemeDlg : public CustomizeQWidget
{
    Q_OBJECT

public:
    explicit NewSchemeDlg(QWidget *parent = nullptr);
    ~NewSchemeDlg();

public:
    void ResetTheWidget();

    inline QString GetCurrentSchemeName()
    {
        return m_current_scheme_name;
    }

protected:
    bool eventFilter(QObject *obj, QEvent *event);
private:
    void InitWidget();
    void ConnectSignalToSlot();

private slots:
    void SlotChangeBasicScheme(int index);
    void SlotGetAvailableKey(int index);
    void SlotCurrentKeyChanged(int index);
    void SlotConfirmScheme();

private:
    SpSchemeModel* m_currentSPModel = nullptr;
    Ui::NewSchemeDlg *ui;
    SPKeyBoard* current_key_board;
    QString m_current_scheme_name;
};

#endif // NEWSCHEMEDLG_H
