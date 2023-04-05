#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QHeaderView>
#include <QMenu>
#include <QDir>
#include <QTimer>
#include <QDesktopWidget>
#include <QGraphicsDropShadowEffect>
#include "toolbtn.h"
#include "shortcutsettingdlg.h"
#include "config.h"
#include "hotkeymanager.h"
#include "custominfombox.h"
#include "utils.h"
#ifdef _WIN32
#include <Windows.h>
#endif

MainWindow *MainWindow::instance()
{
    if(m_instance == nullptr)
    {
        m_instance = new MainWindow;
    }
    return m_instance;
}

MainWindow* MainWindow::m_instance = nullptr;

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Tool | Qt::FramelessWindowHint);
    //设置背景透明

#ifdef USE_UOS
    this->layout()->setMargin(0);
    setAttribute(Qt::WA_TranslucentBackground,true);
    this->setStyleSheet("QWidget#inner_widget{border:1px solid #CCCCCC;border-radius:0px;background-color:rgb(255,255,255);}");
    ui->inner_widget->setFixedWidth(305);
#else
    this->layout()->setMargin(6);
    setAttribute(Qt::WA_TranslucentBackground,true);
    set_window_shadow();
#endif

#ifdef Q_OS_WIN
    //不使用焦点
    LONG exs = GetWindowLong((HWND)this->winId(),GWL_EXSTYLE);
    exs |= 0x08000000;
    SetWindowLong((HWND)this->winId(),GWL_EXSTYLE,exs);
#endif

    init_tool_box();
    load_toobox_info();
    ui->close_btn->setProperty("type","closebtn");
    m_tool_menu = new QMenu(this);

    m_tool_menu->setFixedWidth(140);


    if(m_share_memory_check_thread == nullptr)
    {
        m_share_memory_check_thread = new SharedMemoryCheckThread();
        connect(m_share_memory_check_thread,&SharedMemoryCheckThread::ShowNormal,this,&MainWindow::slot_show_mainwindow);
        connect(m_share_memory_check_thread,&SharedMemoryCheckThread::enable_addon,this,&MainWindow::slot_use_addon_by_name);
        m_share_memory_check_thread->start();
    }

    if(m_tool_autoupdate_task == nullptr)
    {
        m_tool_autoupdate_task = new ToolUpdateTask();
        connect(m_tool_autoupdate_task,SIGNAL(update_tool_info()),this,SLOT(load_toobox_info()));
        m_tool_autoupdate_task->start();
    }

    connect(ui->close_btn, &QPushButton::clicked,this,&MainWindow::OnCloseBtnClicked);

#ifdef _WIN32
    m_set_tool_shortcut_action = new QAction("设置工具箱的快捷键");
    m_tool_menu->addAction(m_set_tool_shortcut_action);
    connect(m_set_tool_shortcut_action,&QAction::triggered,this,&MainWindow::slot_show_shortcut_dlg);

    m_toolbox_shortcut = new QHotkey(this);
    HotkeyManager::Instance()->RegisterNewHotkey(m_toolbox_shortcut);
    connect(m_toolbox_shortcut,&QHotkey::activated,this,&MainWindow::slot_show_toolbox_mainwindow);

    if(m_pipe_msg_thread == nullptr)
    {
        m_pipe_msg_thread = new PipeMsgThread();
        connect(m_pipe_msg_thread, &PipeMsgThread::EnableHotkey,this,&MainWindow::slot_enable_all_hotkey);
        m_pipe_msg_thread->start();
    }
#endif
}

MainWindow::~MainWindow()
{
    delete m_share_memory_check_thread;
    delete m_tool_autoupdate_task;
#ifdef _WIN32
    delete m_pipe_msg_thread;
#endif
    delete ui;
}

void MainWindow::slot_show_shortcut_dlg()
{
#ifdef _WIN32
    m_toolbox_shortcut->setRegistered(false);
    ShortcutSettingDlg* shortcut_dlg = new ShortcutSettingDlg(this);
    int ret = shortcut_dlg->exec();
    if(ret == QDialog::Accepted)
    {
         QKeySequence current_key_sequence = shortcut_dlg->get_key_sequence();
         if(current_key_sequence.count() == 1)
         {
             m_toolbox_shortcut->resetShortcut();
             m_toolbox_shortcut->setShortcut(current_key_sequence);
         }

    }
    m_toolbox_shortcut->setRegistered(true);
#endif
}

void MainWindow::slot_show_mainwindow(int x, int y)
{
    int topleft_x = x - width() + 4;
    int topleft_y = y - height() + 4;
    this->move(topleft_x,topleft_y);
    if(!isVisible())
    {
        this->setVisible(true);
#ifdef _WIN32
    ::SetWindowPos(HWND(this->winId()),HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
    ::SetWindowPos(HWND(this->winId()),HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
#endif
    }
    else
    {
        this->setVisible(false);
    }

}

void MainWindow::slot_enable_all_hotkey(bool flag)
{
 #ifdef _WIN32
    if(flag)
    {
        HotkeyManager::Instance()->EnableAllHotkey();
    }
    else
    {
        HotkeyManager::Instance()->DisableAllHotkey();
    }
#endif
}

void MainWindow::slot_use_addon_by_name(QString addon_name)
{
    TOOL_ADDON_INFO addon_info;
    if(Config::Instance()->GetToolAddonInfoByName(addon_name,addon_info))
    {
        QString exe_path = addon_info.exe_path;
        QString argument = addon_info.call_paramer;
        QStringList argument_list;
        argument_list << argument;
        QProcess* process = new QProcess(this);
        process->startDetached(exe_path,argument_list);
    }
}


void MainWindow::OnCloseBtnClicked()
{
    this->setVisible(false);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_last_mouse_position = event->globalPos();
        m_move_widget_flag = true;
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_move_widget_flag = false;
    }
    if(event->button() == Qt::RightButton)
    {
        QPoint global_pos = event->globalPos();
        m_tool_menu->move(global_pos);
        m_tool_menu->show();
    }

}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (!event->buttons().testFlag(Qt::LeftButton))
            return;
    if(!m_move_widget_flag)
            return;
    if (m_move_widget_flag)
    {
        QPoint movePos(this->x() - m_last_mouse_position.x() + event->globalX(),
                       this->y() - m_last_mouse_position.y() + event->globalY());
        if (movePos.x() >= 0
                && movePos.x() + this->width() <= qApp->desktop()->width()
                && movePos.y() >= 0
                && movePos.y() + this->height() <= qApp->desktop()->height())
        {
            this->move(movePos);
        }
         m_last_mouse_position = event->globalPos();
    }
}

void MainWindow::set_window_shadow()
{
    QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
    shadow_effect->setOffset(0, 0);
    shadow_effect->setColor(QColor(150,150,150));
    shadow_effect->setBlurRadius(6);
    ui->inner_widget->setGraphicsEffect(shadow_effect);
}

void MainWindow::init_tool_box()
{
    m_toolbox_model = tool_box_model(new MyTableModel());
    m_toolbox_view = tool_box_view(new MyTableView());
    m_toolbox_delegate = tool_box_delegate(new Delegate(m_toolbox_view.data()));

    m_toolbox_view->setItemDelegate(m_toolbox_delegate.data());
    m_toolbox_view->setModel(m_toolbox_model.data());
    m_toolbox_view->verticalHeader()->setDefaultSectionSize(68);
    m_toolbox_view->horizontalHeader()->setDefaultSectionSize(68);
    m_toolbox_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //智能选中单个单元格
    m_toolbox_view->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_toolbox_view->setSelectionMode(QAbstractItemView::ExtendedSelection);

    QHBoxLayout* wordlib_tab_layout = new QHBoxLayout(ui->tool_content_frame);
    wordlib_tab_layout->setMargin(0);
    wordlib_tab_layout->addWidget(m_toolbox_view.data());
    m_toolbox_view->setProperty("type","toolbox_view");
    connect(m_toolbox_delegate.data(), SIGNAL(toolbox_hasupdate()),this,SLOT(slot_update_toolbox_view()));
    connect(m_toolbox_delegate.data(), SIGNAL(addon_removed(QString)),this,SLOT(remove_addon_info(QString)));
}

void MainWindow::slot_update_toolbox_view()
{
    Utils::write_log_tofile("load toolbox info");
    QVector<TOOL_ADDON_INFO> addon_vector = Config::Instance()->GetAllAddonVector();
    m_tool_addon_vector.clear();
    for(TOOL_ADDON_INFO index_info : addon_vector)
    {
         tool_addon_info point_index_info = QSharedPointer<TOOL_ADDON_INFO>(new TOOL_ADDON_INFO(index_info));
         m_tool_addon_vector.push_back(point_index_info);
    }
    m_toolbox_model->set_data(m_tool_addon_vector);
    m_toolbox_view->openAllEditor();
}

void MainWindow::load_toobox_info()
{
    Utils::write_log_tofile("load toolbox info");
    QVector<TOOL_ADDON_INFO> addon_vector = Config::Instance()->GetAvailableAddonVector();
    m_tool_addon_vector.clear();
    for(TOOL_ADDON_INFO index_info : addon_vector)
    {
         tool_addon_info point_index_info = QSharedPointer<TOOL_ADDON_INFO>(new TOOL_ADDON_INFO(index_info));
         m_tool_addon_vector.push_back(point_index_info);
    }

    m_toolbox_model->set_data(m_tool_addon_vector);
    m_toolbox_view->reset();
    m_toolbox_view->update();

    m_toolbox_view->openAllEditor();

}

void MainWindow::remove_addon_info(QString addon_name)
{
    Utils::write_log_tofile("load toolbox info");
    for(tool_addon_info index : m_tool_addon_vector)
    {
        if(index->addon_name == addon_name)
        {
            m_tool_addon_vector.removeOne(index);
        }
    }
    m_toolbox_model->refresh_data(m_tool_addon_vector);
}

void MainWindow::slot_show_toolbox_mainwindow()
{
    if(!this->isVisible())
    {
#ifdef _WIN32
    ::SetWindowPos(HWND(this->winId()),HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
    ::SetWindowPos(HWND(this->winId()),HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
#endif
        this->show();
    }
    else
    {
        this->hide();
    }
}

void MainWindow::show_messge_box(QString content)
{
    CustomInfomBox* inform_msgbox = new CustomInfomBox(this);
    inform_msgbox->set_inform_content(content);

    QRect rect = this->geometry();
    //计算显示原点
    int pos_x = rect.x() + rect.width()/2 - inform_msgbox->width() /2;
    int pos_y = rect.y() + rect.height()/2 - inform_msgbox->height()/2;
    inform_msgbox->move(pos_x, pos_y);

    inform_msgbox->show();
    QTimer *m_time = new QTimer();
    m_time->setSingleShot(true);
    m_time->start(1200);
    QObject::connect(m_time, &QTimer::timeout, [=](){
        inform_msgbox->hide();
    });
}


