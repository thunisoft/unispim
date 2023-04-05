#ifndef TOOLBARMENU_H
#define TOOLBARMENU_H
#include <QWidget>
#include <QShowEvent>

class ToolbarMenuItem;
class SimpleTraditionMenu;
class HelpChildMenu;
class ToolBarMainMenu : public QWidget
{
    Q_OBJECT
public:
    ToolBarMainMenu(QWidget* parent = 0);
    ~ToolBarMainMenu();
private:
    void InitWidget();
    void InitMenuItem();
public:
    void LoadConfig();
    void HideAllMenu();

protected:
    bool event(QEvent * e);
    //悬浮进入事件
    void hoverEnter(QHoverEvent * event);
    //悬浮退出事件
    void hoverLeave(QHoverEvent * event);
    //悬浮移动事件
    void hoverMove(QHoverEvent * event);
    void showEvent(QShowEvent* event);

signals:
    void simple_tra_state_changed(int state);

private slots:
    void slot_on_item_hover_state_changed(int state);
    void slot_on_item_click();
    void slot_simple_tra_changed(int state);
private:
    QWidget* m_inner_widget;
    ToolbarMenuItem* m_config_tool_item = nullptr;
    ToolbarMenuItem* m_symbol_item = nullptr;
    ToolbarMenuItem* m_hide_toobar_item = nullptr;
    ToolbarMenuItem* m_simple_traditon_item = nullptr;
    ToolbarMenuItem* m_help_child_item = nullptr;
    SimpleTraditionMenu* m_simple_tra_menu = nullptr;
    HelpChildMenu* m_help_child_menu = nullptr;
};

class SimpleTraditionMenu : public QWidget
{
    Q_OBJECT
public:
    SimpleTraditionMenu(QWidget* parent = 0);
    ~SimpleTraditionMenu();

public:
    void LoadConfig();
protected:
    bool event(QEvent * e);
    //悬浮进入事件
    void hoverEnter(QHoverEvent * event);
    //悬浮退出事件
    void hoverLeave(QHoverEvent * event);
    //悬浮移动事件
    void hoverMove(QHoverEvent * event);
    void showEvent(QShowEvent* event);
private slots:
    void slot_on_item_click();
signals:
    void simple_tra_changed(bool is_tra);
private:
    QWidget* m_inner_widget;
    ToolbarMenuItem* m_simple_item = nullptr;
    ToolbarMenuItem* m_traditon_item = nullptr;    
};

class HelpChildMenu : public QWidget
{
    Q_OBJECT
public:
    HelpChildMenu(QWidget* parent = 0);
    ~HelpChildMenu();


protected:
    bool event(QEvent * e);
    //悬浮进入事件
    void hoverEnter(QHoverEvent * event);
    //悬浮退出事件
    void hoverLeave(QHoverEvent * event);
    //悬浮移动事件
    void hoverMove(QHoverEvent * event);

private slots:
    void slot_on_item_click();
    void slot_open_url(QString target_url);

private:
    QWidget* m_inner_widget;
    ToolbarMenuItem* m_official_web_item = nullptr;
    ToolbarMenuItem* m_official_bbs_item = nullptr;
    ToolbarMenuItem* m_check_update_item = nullptr;
};

#endif // TOOLBARMENU_H
