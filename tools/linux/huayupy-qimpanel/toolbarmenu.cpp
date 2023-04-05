#include "toolbarmenu.h"
#include "toolbarmenuitem.h"
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QApplication>
#include <QDesktopWidget>
#include <QtDBus/QDBusConnection>
#include <string>
#include <QtDBus/QDBusError>
#include <QDBusMessage>
#include <QDesktopServices>
#include "../public/utils.h"
#include "../public/config.h"
#include "maincontroller.h"
#include "toolbarmodel.h"
#include "../public/configmanager.h"


ToolBarMainMenu::ToolBarMainMenu(QWidget* parent):QWidget(parent)
{
    InitWidget();
    LoadConfig();
}

ToolBarMainMenu::~ToolBarMainMenu()
{

}


void ToolBarMainMenu::InitWidget()
{
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
                         Qt::WindowDoesNotAcceptFocus | Qt::Tool | Qt::X11BypassWindowManagerHint);
    this->setAttribute(Qt::WA_Hover,true);
    this->setAttribute(Qt::WA_TranslucentBackground,true);
    m_inner_widget = new QWidget(this);
    m_inner_widget->setProperty("type","inner_widget");
    m_inner_widget->setFixedWidth(254);
    m_inner_widget->setFixedHeight(178);
    this->setFixedHeight(178);

    m_config_tool_item = new ToolbarMenuItem(":/image/toolbarMenu/menu_config.png","属性设置","",m_inner_widget);
    m_symbol_item = new ToolbarMenuItem(":/image/toolbarMenu/menu_special_symbol.png","符号大全","",m_inner_widget);
    m_hide_toobar_item = new ToolbarMenuItem(":/image/toolbarMenu/menu_toolbar_visible.png","隐藏工具栏","",m_inner_widget);
    m_simple_traditon_item = new ToolbarMenuItem(":/image/toolbarMenu/menu_simpleTradition.png","繁简体切换",":/image/toolbarMenu/childMenuArrow.png",m_inner_widget);
    m_help_child_item = new ToolbarMenuItem(":/image/toolbarMenu/menu_help.png","帮助",":/image/toolbarMenu/childMenuArrow.png",this);
    QList<ToolbarMenuItem*> item_list;
    item_list.append(m_config_tool_item);
    item_list.append(m_symbol_item);
    item_list.append(m_hide_toobar_item);
    item_list.append(m_simple_traditon_item);
    item_list.append(m_help_child_item);

    for(ToolbarMenuItem* index: item_list)
    {
         connect(index,&ToolbarMenuItem::item_hovered_state,this,&ToolBarMainMenu::slot_on_item_hover_state_changed);
         connect(index,&ToolbarMenuItem::item_clicked,this,&ToolBarMainMenu::slot_on_item_click);
    }

    QVBoxLayout* inner_main_layout = new QVBoxLayout(m_inner_widget);
    inner_main_layout->addWidget(m_config_tool_item);
    inner_main_layout->addWidget(m_symbol_item);
    inner_main_layout->addWidget(m_hide_toobar_item);
    inner_main_layout->addWidget(m_simple_traditon_item);
    inner_main_layout->addWidget(m_help_child_item);
    inner_main_layout->setMargin(3);
    m_inner_widget->setLayout(inner_main_layout);


    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->addWidget(m_inner_widget);
    main_layout->setMargin(0);
    this->setLayout(main_layout);

    m_simple_tra_menu = new SimpleTraditionMenu(this);
    connect(m_simple_tra_menu,&SimpleTraditionMenu::simple_tra_changed,this,&ToolBarMainMenu::slot_simple_tra_changed);
    m_help_child_menu = new HelpChildMenu(this);
}

void ToolBarMainMenu::InitMenuItem()
{

}

void ToolBarMainMenu::LoadConfig()
{
    ConfigManager::Instance()->ReloadUserConfig();
    int hide_flag;
    ConfigManager::Instance()->GetIntConfigItem("hide_tray",hide_flag);
    if(hide_flag)
    {
        m_hide_toobar_item->set_text("显示工具栏");
        m_hide_toobar_item->set_icon(":/image/toolbarMenu/menu_toolbar_show.png");
    }
    else
    {
        m_hide_toobar_item->set_text("隐藏工具栏");
        m_hide_toobar_item->set_icon(":/image/toolbarMenu/menu_toolbar_visible.png");
    }
}

void ToolBarMainMenu::HideAllMenu()
{
    if(this->isVisible())
    {
        this->hide();
    }
    if(m_help_child_menu->isVisible())
    {
        m_help_child_menu->hide();
    }
    if(m_simple_tra_menu->isVisible())
    {
        m_simple_tra_menu->hide();
    }
}

bool ToolBarMainMenu::event(QEvent *e)
{
    switch(e->type())
    {
    case QEvent::HoverEnter:
        hoverEnter(static_cast<QHoverEvent*>(e));
        return true;
    case QEvent::HoverLeave:
        hoverLeave(static_cast<QHoverEvent*>(e));
        return true;
    case QEvent::HoverMove:
        hoverMove(static_cast<QHoverEvent*>(e));
        return true;
    default:
        break;
    }
    return QWidget::event(e);
}

void ToolBarMainMenu::hoverEnter(QHoverEvent *event)
{
    QWidget::event(event);
}

void ToolBarMainMenu::hoverLeave(QHoverEvent *event)
{
    QPoint current_pos = QCursor::pos();
    if(!m_help_child_menu->geometry().contains(current_pos) && !m_simple_tra_menu->geometry().contains(current_pos))
    {
        m_help_child_menu->hide();
        m_simple_tra_menu->hide();
    }
    QWidget::event(event);
}

void ToolBarMainMenu::hoverMove(QHoverEvent *event)
{
    QWidget::event(event);
}

void ToolBarMainMenu::showEvent(QShowEvent *event)
{
    LoadConfig();
    event->accept();
}

void ToolBarMainMenu::slot_on_item_hover_state_changed(int state)
{
    if(sender() == m_simple_traditon_item && state)
    {
        QRect desk_rect = QApplication::desktop()->availableGeometry();
        QPoint item_pos = m_simple_traditon_item->mapToGlobal(m_simple_traditon_item->rect().topRight());
        QPoint end_pos = item_pos + QPoint(m_simple_tra_menu->width(),0);

        if(end_pos.x() > desk_rect.width())
        {
            item_pos = m_simple_traditon_item->mapToGlobal(m_simple_traditon_item->rect().topLeft());
            item_pos = item_pos - QPoint(m_simple_tra_menu->width(),0);
        }

        m_simple_tra_menu->move(item_pos);

        m_help_child_menu->setVisible(false);
        m_simple_tra_menu->setVisible(true);
    }
    else if(sender() == m_help_child_item && state)
    {

        QRect desk_rect = QApplication::desktop()->availableGeometry();
        QPoint item_pos = m_help_child_item->mapToGlobal(m_help_child_item->rect().topRight());
        QPoint end_pos = item_pos + QPoint(m_help_child_menu->width(),0);

        if(end_pos.x() > desk_rect.width())
        {
            item_pos = m_help_child_item->mapToGlobal(m_help_child_item->rect().topLeft());
            item_pos = item_pos - QPoint(m_help_child_menu->width(),0);
        }

        m_help_child_menu->move(item_pos);
        m_help_child_menu->setVisible(true);
        m_simple_tra_menu->setVisible(false);
    }
    else if(state)
    {
        m_help_child_menu->setVisible(false);
        m_simple_tra_menu->setVisible(false);
    }
}

void ToolBarMainMenu::slot_on_item_click()
{
    ToolbarMenuItem* index = (ToolbarMenuItem*)sender();
    if(index == m_config_tool_item)
    {
        //调用配置工具
        if(!Utils::IsProcessOn(QString("huayupy-config-tools-fcitx")))
        {
            FILE* ptr2 = NULL;
            std::string command = Utils::GetBinFilePath().append("huayupy-config-tools-fcitx 82717623-mhe4-0293-aduh-ku87wh6328ne &").toStdString();
            if((ptr2 = popen(command.c_str(), "r")) != NULL)
            {
                pclose(ptr2);
            }
        }
        else
        {
            QDBusMessage msg = QDBusMessage::createSignal("/com/thunisoft/huayupy", "com.thunisoft.huayupy", QString("to_show"));
            QDBusConnection::sessionBus().send(msg);
        }
        this->hide();
    }
    else if(index == m_symbol_item)
    {
        //调用符号大全
        if(!Utils::IsProcessOn(QString("special-symbols-fcitx")))
        {
            FILE* ptr2 = NULL;
            std::string command = Utils::GetBinFilePath().append("special-symbols-fcitx 82717623-mhe4-0293-aduh-ku87wh6328ne &").toStdString();
            if((ptr2 = popen(command.c_str(), "r")) != NULL)
            {
                pclose(ptr2);
            }
        }
        else
        {
            QDBusMessage msg = QDBusMessage::createSignal("/com/thunisoft/huayupy", "com.thunisoft.huayupy", QString("to_show"));
            QDBusConnection::sessionBus().send(msg);
        }
        this->hide();
    }
    else if(index == m_hide_toobar_item)
    {

        ConfigManager::Instance()->ReloadUserConfig();
        int hide_flag;
        ConfigManager::Instance()->GetIntConfigItem("hide_tray",hide_flag);
        ConfigManager::Instance()->SetIntConfigItem("hide_tray",!hide_flag);

        LoadConfig();
        MainController::Instance()->ReloadHideTray();
        //this->parentWidget()->setVisible(hide_flag);
        this->hide();
    }
}

void ToolBarMainMenu::slot_simple_tra_changed(int state)
{
    emit simple_tra_state_changed(state);
}

SimpleTraditionMenu::SimpleTraditionMenu(QWidget* parent):QWidget(parent)
{

    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
                         Qt::WindowDoesNotAcceptFocus | Qt::Tool | Qt::X11BypassWindowManagerHint);

    m_inner_widget = new QWidget(this);
    m_inner_widget->setProperty("type","inner_widget");
    m_inner_widget->setFixedWidth(220);
    m_inner_widget->setFixedHeight(66);
    this->setFixedHeight(66);

    m_traditon_item = new ToolbarMenuItem("","繁体","",m_inner_widget);
    m_simple_item = new ToolbarMenuItem("","简体","",m_inner_widget);

    QVBoxLayout* inner_main_layout = new QVBoxLayout(m_inner_widget);
    inner_main_layout->addWidget(m_simple_item);
    inner_main_layout->addWidget(m_traditon_item);
    inner_main_layout->setMargin(3);

    m_inner_widget->setLayout(inner_main_layout);
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->addWidget(m_inner_widget);
    main_layout->setMargin(0);
    this->setLayout(main_layout);

    this->setAttribute(Qt::WA_TranslucentBackground,true);
    this->setAttribute(Qt::WA_Hover,true);

    QList<ToolbarMenuItem*> item_list;
    item_list.append(m_traditon_item);
    item_list.append(m_simple_item);

    for(ToolbarMenuItem* index: item_list)
    {
         connect(index,&ToolbarMenuItem::item_clicked,this,&SimpleTraditionMenu::slot_on_item_click);
    }
    LoadConfig();
}

SimpleTraditionMenu::~SimpleTraditionMenu()
{

}

void SimpleTraditionMenu::LoadConfig()
{
   ToolBarModel* model = MainController::Instance()->GetToolbarModel();
    if(model->trad())
    {
        m_simple_item->set_expand_icon("");
        m_traditon_item->set_expand_icon(":/image/toolbarConfig/left_btn_check.png");
        m_traditon_item->set_text_style("font-size:14px;color:#4D9CF8");
        m_simple_item->set_text_style("font-size:14px;color:#000000");

    }
    else
    {
        m_simple_item->set_expand_icon(":/image/toolbarConfig/left_btn_check.png");
        m_traditon_item->set_expand_icon("");
        m_simple_item->set_text_style("font-size:14px;color:#4D9CF8");
        m_traditon_item->set_text_style("font-size:14px;color:#000000");
    }
}


bool SimpleTraditionMenu::event(QEvent *e)
{
    switch(e->type())
    {
    case QEvent::HoverEnter:
        hoverEnter(static_cast<QHoverEvent*>(e));
        return true;
    case QEvent::HoverLeave:
        hoverLeave(static_cast<QHoverEvent*>(e));
        return true;
    case QEvent::HoverMove:
        hoverMove(static_cast<QHoverEvent*>(e));
        return true;
    default:
        break;
    }
    return QWidget::event(e);
}

void SimpleTraditionMenu::hoverEnter(QHoverEvent *event)
{
    QWidget::event(event);
}

void SimpleTraditionMenu::hoverLeave(QHoverEvent *event)
{
    this->hide();
    QWidget::event(event);
}

void SimpleTraditionMenu::hoverMove(QHoverEvent *event)
{
    QWidget::event(event);
}

void SimpleTraditionMenu::showEvent(QShowEvent *event)
{
    LoadConfig();
    event->accept();
}

void SimpleTraditionMenu::slot_on_item_click()
{
    ToolbarMenuItem* index = (ToolbarMenuItem*)sender();
    int hz_option;
    if(ConfigManager::Instance()->GetIntConfigItem("hz_option",hz_option))
    {
        if(index == m_simple_item)
        {
            hz_option |= HZ_OUTPUT_SIMPLIFIED;
            hz_option &= ~HZ_OUTPUT_TRADITIONAL;
            ConfigManager::Instance()->SetIntConfigItem("hz_option",hz_option);
            emit simple_tra_changed(0);
            this->hide();
            this->parentWidget()->hide();
        }
        else if(index == m_traditon_item)
        {
            hz_option |= HZ_OUTPUT_TRADITIONAL;
            hz_option &= ~HZ_OUTPUT_SIMPLIFIED;
            ConfigManager::Instance()->SetIntConfigItem("hz_option",hz_option);

            this->hide();
            emit simple_tra_changed(1);
            this->parentWidget()->hide();
        }
    }
    LoadConfig();
}


HelpChildMenu::HelpChildMenu(QWidget* parent):QWidget(parent)
{
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
                         Qt::WindowDoesNotAcceptFocus | Qt::Tool | Qt::X11BypassWindowManagerHint);
    this->setAttribute(Qt::WA_Hover,true);
    m_inner_widget = new QWidget(this);
    m_inner_widget->setProperty("type","inner_widget");
    m_inner_widget->setFixedWidth(220);
    m_inner_widget->setFixedHeight(96);
    this->setFixedHeight(96);

    m_official_web_item = new ToolbarMenuItem("","官方网站","",m_inner_widget);
    m_official_bbs_item = new ToolbarMenuItem("","官方论坛","",m_inner_widget);
    m_check_update_item = new ToolbarMenuItem("","检查更新","",m_inner_widget);

    QVBoxLayout* inner_main_layout = new QVBoxLayout(m_inner_widget);
    inner_main_layout->addWidget(m_official_web_item);
    inner_main_layout->addWidget(m_official_bbs_item);
    inner_main_layout->addWidget(m_check_update_item);
    inner_main_layout->setMargin(3);

    m_inner_widget->setLayout(inner_main_layout);
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->addWidget(m_inner_widget);
    main_layout->setMargin(0);
    this->setLayout(main_layout);

    this->setAttribute(Qt::WA_TranslucentBackground,true);

    QList<ToolbarMenuItem*> item_list;
    item_list.append(m_official_web_item);
    item_list.append(m_official_bbs_item);
    item_list.append(m_check_update_item);

    for(ToolbarMenuItem* index: item_list)
    {
         connect(index,&ToolbarMenuItem::item_clicked,this,&HelpChildMenu::slot_on_item_click);
    }
}

HelpChildMenu::~HelpChildMenu()
{

}

bool HelpChildMenu::event(QEvent *e)
{
    switch(e->type())
    {
    case QEvent::HoverEnter:
        hoverEnter(static_cast<QHoverEvent*>(e));
        return true;
    case QEvent::HoverLeave:
        hoverLeave(static_cast<QHoverEvent*>(e));
        return true;
    case QEvent::HoverMove:
        hoverMove(static_cast<QHoverEvent*>(e));
        return true;
    default:
        break;
    }
    return QWidget::event(e);
}

void HelpChildMenu::hoverEnter(QHoverEvent *event)
{
    QWidget::event(event);
}

void HelpChildMenu::hoverLeave(QHoverEvent *event)
{
    this->hide();
    QWidget::event(event);
}

void HelpChildMenu::hoverMove(QHoverEvent *event)
{
    QWidget::event(event);
}


void HelpChildMenu::slot_on_item_click()
{
    ToolbarMenuItem* index = (ToolbarMenuItem*)sender();
    if(index == m_official_web_item)
    {
        ConfigManager::Instance()->ReloadUserConfig();
        QString domain = ConfigManager::Instance()->GetServerUrl();
        QString index_page = QString("%1%2").arg(domain).arg("/index.html#/sy");
        slot_open_url(index_page);
        this->hide();
        this->parentWidget()->hide();
    }
    else if(index == m_official_bbs_item)
    {
        slot_open_url("http://bbs.pinyin.thunisoft.com/forum.php");
        this->hide();
        this->parentWidget()->hide();
    }
    else if(index == m_check_update_item)
    {
        //调用配置工具
        if(!Utils::IsProcessOn(QString("huayupy-config-tools-fcitx")))
        {
            FILE* ptr2 = NULL;
            std::string command = Utils::GetBinFilePath().append("huayupy-config-tools-fcitx 82717623-mhe4-0293-aduh-ku87wh6328ne update_page &").toStdString();
            if((ptr2 = popen(command.c_str(), "r")) != NULL)
            {
                pclose(ptr2);
            }
        }
        else
        {
            QDBusMessage msg = QDBusMessage::createSignal("/com/thunisoft/huayupy", "com.thunisoft.huayupy", QString("to_show"));
            QDBusConnection::sessionBus().send(msg);
        }
        this->hide();
        this->parentWidget()->hide();
    }
}

void HelpChildMenu::slot_open_url(QString target_url)
{
    QDesktopServices::openUrl(QUrl(target_url));
}

