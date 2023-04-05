#ifndef TOOLBTN_H
#define TOOLBTN_H
#include <QPushButton>
#include <QLabel>
#include <QPaintEvent>
#include <QMouseEvent>
#include "commondef.h"
#include "toolupdatetask.h"

#include <QLabel>
#include <QProgressBar>
#include <QEvent>
#include <QHoverEvent>
#include <QProcess>
#include "customtooltip.h"

#ifdef _WIN32
#include <QHotkey>
#endif

#pragma execution_character_set("utf-8")

class ShortcutSettingDlg;
class ToolBtn : public QPushButton
{
    Q_OBJECT
public:
    ToolBtn(QWidget* parent=0);
    ~ToolBtn();

public:
    void set_tool_info(const TOOL_ADDON_INFO input_info);
    void SetUpdateFlag(bool has_update);
    void set_shortcut_enable(bool switch_flag);

public slots:
    void slot_create_desktop_ink();
    void slot_set_shortcut();
    void slot_check_tool_update();
    void slot_show_download_progress(qint64 receiveBytes, qint64 totalBytes);
    void slot_on_has_no_update();
    void slot_on_check_failed();
    void slot_fix_to_toolbar();
    void slot_junmp_to_cofig_tools();
    void slot_delete_addon();

protected:
   void mousePressEvent(QMouseEvent *e);
   void mouseReleaseEvent(QMouseEvent *e);

protected:
   bool event(QEvent * e);
   void hoverEnter(QHoverEvent * event);
   void hoverLeave(QHoverEvent * event);
   void hoverMove(QHoverEvent * event);

private:
   QString tool_tip_info();
   bool create_desktop_shortcut(const wchar_t *target_exe_path, const wchar_t *lnk_path,
                                const wchar_t *working_path, const wchar_t *argument,int shortcut,
                                bool isShowCmd,const wchar_t* describe,const wchar_t* icon_path);
public slots:
   void start_the_tool();
signals:
   void tool_has_update();
   void addon_removed(QString addon_name);

private:
    QLabel* m_icon_label = nullptr;
    QLabel* m_name_label = nullptr;
    TOOL_ADDON_INFO m_addon_info;
    QMenu* m_right_menu = nullptr;
    QAction* m_set_shortcut_action = nullptr;
    QAction* m_send_to_desktop_action = nullptr;
    QAction* m_set_checkupdate_action = nullptr;
    QAction* m_fix_toolbar_action = nullptr;
    QAction* m_jump_to_config_tools_action = nullptr;
    QAction* m_del_addon_action = nullptr;
    QProgressBar* m_custom_progressbar = nullptr;
#ifdef _WIN32
    QHotkey* m_current_hot_key = nullptr;
#endif
    QLabel* m_notice_label = nullptr;
    ToolUpdateTask* m_tool_manuupdate_task = nullptr;
    CustomTooltip* m_custom_tooltip = nullptr;
    QProcess m_tool_process;
};

#endif // TOOLBTN_H
