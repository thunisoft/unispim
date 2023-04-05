/**@file  spschemeviewdlg.h
* @brief      双拼防范预览的界面
* @date        2020年9月22日
* @version     V1.0
* @copyright    Copyright (c) 2018-2020  华宇信息
**********************************************************************************
* @attention
* 注意事项1:  事项描述
* @par 修改日志:
* <table>
* <tr><th>Date        <th>Version  <th>Author    <th>Description
* </table>
*
**********************************************************************************
*/
#ifndef SPSCHEMEVIEWDLG_H
#define SPSCHEMEVIEWDLG_H


#include <QWidget>
#include "customize_ui/customize_qwidget.h"

namespace Ui {
class SPSchemeViewDlg;
}

class NewSchemeDlg;
class SPKeyBoard;
class SPSchemeViewDlg : public CustomizeQWidget
{
    Q_OBJECT
public:
    explicit SPSchemeViewDlg(QWidget *parent = nullptr);
    ~SPSchemeViewDlg();


public:
    void LoadTheConfig();
    void RefreshTheSchemeList();
protected:
    bool eventFilter(QObject *obj, QEvent *event);
private:
    void InitWidget();
    void ConnectSignalToSlot();
    void ChangeTheKeyBoardView(QString filePath);
    void ChangeTheDeleteBtnState(QString sp_file_name);


private slots:
    /**@brief 确定按钮对应的槽函数
   */
    void SlotSubmitTheDlg();

    /**@brief 删除双拼方案对应的槽函数
   */
    void SlotDeleteSPScheme();

    /**@brief 创建双拼方案对应的槽函数
   */
    void SlotCreateNewScheme();

    /**@brief 修改了对应的方案
   */
    void SlotSchemeChanged();


    /**@brief 修改了对应的方案
   */
    void SlotResetThescheme();
private:
    Ui::SPSchemeViewDlg *ui;
    NewSchemeDlg* m_new_scheme_Dlg;
    SPKeyBoard* current_key_board;
};

#endif // SPSCHEMEVIEWDLG_H
