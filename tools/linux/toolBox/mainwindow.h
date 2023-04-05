#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QSharedMemory>
#include <QMainWindow>
#include "mytableview.h"
#include "sharedmemorycheckthread.h"
#include "pipemsgthread.h"
#include "toolupdatetask.h"
#ifdef _WIN32
#include <QHotkey>
#endif
#pragma execution_character_set("utf-8")


namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT
    using tool_box_model = QSharedPointer<MyTableModel>;
    using tool_box_view = QSharedPointer<MyTableView>;
    using tool_box_delegate = QSharedPointer<Delegate>;
    using tool_addon_info = QSharedPointer<TOOL_ADDON_INFO>;

public:
    static MainWindow* instance();

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void show_messge_box(QString content);

public slots:
    void slot_show_shortcut_dlg();
    void slot_show_mainwindow(int x, int y);
    void slot_enable_all_hotkey(bool flag);
    void slot_use_addon_by_name(QString addon_name);

public slots:
    void OnCloseBtnClicked();
private:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    void set_window_shadow();
    void init_tool_box();

public slots:
    void slot_update_toolbox_view();
    void load_toobox_info();
private slots:
    void remove_addon_info(QString addon_name);
private:
    void slot_show_toolbox_mainwindow();
private:
    QPoint m_last_mouse_position;
    bool m_move_widget_flag;
    Ui::MainWindow *ui;
    tool_box_view m_toolbox_view;
    tool_box_model m_toolbox_model;
    tool_box_delegate m_toolbox_delegate;
    QVector<tool_addon_info> m_tool_addon_vector;
    QMenu* m_tool_menu = nullptr;
    QAction* m_set_tool_shortcut_action = nullptr;
#ifdef _WIN32
    QHotkey* m_toolbox_shortcut = nullptr;
#endif
    SharedMemoryCheckThread* m_share_memory_check_thread = nullptr;
    PipeMsgThread* m_pipe_msg_thread = nullptr;
    ToolUpdateTask* m_tool_autoupdate_task = nullptr;
    static MainWindow* m_instance;
};

#endif // MAINWINDOW_H
