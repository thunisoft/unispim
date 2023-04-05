#include "toolbtn.h"
#include <QVBoxLayout>
#include <QPixmap>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QMenu>
#include <QStandardPaths>
#include <QFileInfo>
#include <QToolTip>
#include <QProcess>
#include <QIODevice>
#include <QDir>
#include <QSettings>
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>

#include <QTimer>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include "networkhandler.h"
#include "customkeysequenceedit.h"
#include "shortcutsettingdlg.h"
#include "custominfombox.h"
#include "config.h"
#include "utils.h"
#include "hotkeymanager.h"
#include "toolupdatetask.h"
#include "customporgressbar.h"
#include "mainwindow.h"
#ifdef _WIN32
#include <Windows.h>
#include <winnls.h>
#include <shobjidl.h>
#include <objbase.h>
#include <objidl.h>
#include <shlguid.h>
#include <QHotkey>
#else
#include <QDBusMessage>
#include <QDBusConnection>
#endif


ToolBtn::ToolBtn(QWidget *parent) : QPushButton(parent)
{
    setFixedHeight(68);
    setFixedWidth(68);
    QVBoxLayout* content_layout = new QVBoxLayout(this);
    content_layout->setContentsMargins(4,16,4,4);
    m_icon_label = new QLabel(this);
    m_name_label = new QLabel(this);
    m_custom_progressbar = new QProgressBar(this);
    m_name_label->setAlignment(Qt::AlignCenter);
    m_icon_label->setAlignment(Qt::AlignCenter);
    m_custom_progressbar->setAlignment(Qt::AlignCenter);
    m_custom_progressbar->setTextVisible(false);
    m_name_label->setProperty("type","button-name");
    m_custom_progressbar->setFixedSize(60,6);
    m_icon_label->setMinimumHeight(24);
    content_layout->addWidget(m_icon_label);
    content_layout->addWidget(m_name_label);
    content_layout->addWidget(m_custom_progressbar);

    m_custom_progressbar->setVisible(false);
    this->setLayout(content_layout);
    m_right_menu = new QMenu(this);
    m_set_checkupdate_action = new QAction("插件更新",m_right_menu);
    m_send_to_desktop_action = new QAction("发送到桌面",m_right_menu);
    m_fix_toolbar_action = new QAction("固定到工具栏",m_right_menu);
    m_jump_to_config_tools_action = new QAction("设置快捷键",m_right_menu);
    m_del_addon_action = new QAction("删除插件",m_right_menu);
    m_fix_toolbar_action->setVisible(false);
    m_jump_to_config_tools_action->setVisible(false);

    m_right_menu->addAction(m_set_checkupdate_action);
    m_right_menu->addAction(m_fix_toolbar_action);
    m_right_menu->addAction(m_send_to_desktop_action);
    m_right_menu->addAction(m_jump_to_config_tools_action);
    m_right_menu->addAction(m_del_addon_action);
    //m_right_menu->setFixedWidth(90);

    if(m_custom_tooltip == nullptr)
    {
        m_custom_tooltip = new CustomTooltip(this);
    }

    if(m_tool_manuupdate_task == nullptr)
    {
        m_tool_manuupdate_task = new ToolUpdateTask();
        m_tool_manuupdate_task->set_update_type(ManuUpdate);
        connect(m_tool_manuupdate_task,SIGNAL(update_tool_info()),this,SIGNAL(tool_has_update()));
        connect(m_tool_manuupdate_task,&ToolUpdateTask::download_progress,this,&ToolBtn::slot_show_download_progress);
        connect(m_tool_manuupdate_task,&ToolUpdateTask::check_failed,this,&ToolBtn::slot_on_check_failed);
        connect(m_tool_manuupdate_task,&ToolUpdateTask::has_no_update,this,&ToolBtn::slot_on_has_no_update);
    }

    setCursor(Qt::PointingHandCursor);
    connect(m_send_to_desktop_action,SIGNAL(triggered(bool)),this,SLOT(slot_create_desktop_ink()));
    connect(m_set_checkupdate_action, SIGNAL(triggered(bool)),this,SLOT(slot_check_tool_update()));
    connect(m_fix_toolbar_action, SIGNAL(triggered(bool)),this,SLOT(slot_fix_to_toolbar()));
    connect(m_jump_to_config_tools_action,SIGNAL(triggered(bool)),this,SLOT(slot_junmp_to_cofig_tools()));
    connect(m_del_addon_action,SIGNAL(triggered(bool)),this,SLOT(slot_delete_addon()));

#ifdef _WIN32
    if(m_current_hot_key == nullptr)
    {
        m_current_hot_key = new QHotkey(this);
    }
    HotkeyManager::Instance()->RegisterNewHotkey(m_current_hot_key);
     m_set_shortcut_action = new QAction("设置快捷键",m_right_menu);
     m_set_shortcut_action->setVisible(false);
     m_right_menu->addAction(m_set_shortcut_action);
     connect(m_current_hot_key, &QHotkey::activated,this, &ToolBtn::start_the_tool);
     connect(m_set_shortcut_action,SIGNAL(triggered(bool)),this,SLOT(slot_set_shortcut()));
#endif


}

ToolBtn::~ToolBtn()
{
    if(m_tool_manuupdate_task != nullptr)
    {
        delete m_tool_manuupdate_task;
        m_tool_manuupdate_task = nullptr;
    }
}

void ToolBtn::set_tool_info(const TOOL_ADDON_INFO input_info)
{
    m_addon_info = input_info;
#ifdef _WIN32
    QString tip_info = m_addon_info.addon_describe;
    if(!input_info.short_cut.isEmpty())
    {

       QKeySequence keySquence = QKeySequence::fromString(m_addon_info.short_cut);
       m_current_hot_key->setShortcut(keySquence);
       m_current_hot_key->setRegistered(true);
       tip_info += QString("\n%1").arg(m_addon_info.short_cut);
    }
#endif

    if(m_addon_info.addon_name == "special-symbol" || m_addon_info.addon_name == "config-tools")
    {
        m_del_addon_action->setVisible(false);
    }

#ifdef _WIN32
    if(m_addon_info.addon_name == "OCR" || m_addon_info.addon_name == "AsrInput")
    {
        m_fix_toolbar_action->setVisible(true);
        m_jump_to_config_tools_action->setVisible(true);
    }
#else
    if(m_addon_info.addon_name == "OCR" || m_addon_info.addon_name == "AsrInput")
    {
        m_fix_toolbar_action->setVisible(true);
        m_jump_to_config_tools_action->setVisible(true);
    }
#endif

    m_name_label->setText(m_addon_info.addon_displayname);
    QPixmap pixmap(input_info.icon_path);
    m_icon_label->setPixmap(pixmap.scaled(24,24,Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
    if(m_addon_info.addon_status == "hasupdate")
    {
        SetUpdateFlag(true);
    }
}

void ToolBtn::slot_create_desktop_ink()
{
    QString exe_path = QDir::toNativeSeparators(m_addon_info.exe_path);
    QString addon_display_name = m_addon_info.addon_displayname;
    QString argument = m_addon_info.call_paramer;
    QString version = m_addon_info.addon_version;
#ifdef _WIN32
    QString icon_path = "";
    if(exe_path.isEmpty() || addon_display_name.isEmpty())
    {
        return;
    }

    QString desktop_path = QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    QString target_lnk = QDir::toNativeSeparators(desktop_path + (QString("\\%1.lnk").arg(addon_display_name)));

    QFileInfo file_info(exe_path);
    QString working_dir = QDir::toNativeSeparators(file_info.absoluteDir().absolutePath());

    CoInitialize(NULL);
    create_desktop_shortcut(exe_path.toStdWString().c_str(),target_lnk.toStdWString().c_str(),working_dir.toStdWString().c_str(),
                            argument.toStdWString().c_str(),0,0,L"",icon_path.toStdWString().c_str());
    CoUninitialize();
#else
    QString desktop_path = QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    QString file_name = QString("%1.desktop").arg(addon_display_name);
    QString file_path = QDir::toNativeSeparators(QString("%1/%2").arg(desktop_path).arg(file_name));
    QFile file(file_path);
    if(file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        QTextStream txt_output_stream(&file);
        txt_output_stream << "[Desktop Entry]" << endl;
        txt_output_stream << "Type=Application" << endl;
        txt_output_stream << QString("Version=%1").arg(version)<< endl;
        txt_output_stream << QString("Version=%1").arg(version)<< endl;
        txt_output_stream << QString("Name=%1").arg(addon_display_name)<< endl;
        txt_output_stream << QString("Comment=Run %1").arg(m_addon_info.addon_name)<< endl;
        txt_output_stream << QString("Icon=%1").arg(m_addon_info.icon_path)<< endl;
        txt_output_stream << QString("Exec=%1 %2").arg(exe_path).arg(argument)<< endl;
        txt_output_stream << "Terminal=false" << endl;
        file.close();
        Utils::excute_command_line(QString("chmod 777 %1").arg(file_path));
    }

#endif
}

void ToolBtn::slot_set_shortcut()
{
#ifdef _WIN32
   m_current_hot_key->setRegistered(false);
   ShortcutSettingDlg* shortcut_dlg = new ShortcutSettingDlg(this);
   int ret = shortcut_dlg->exec();
   if(ret == QDialog::Accepted)
   {
        QKeySequence current_key_sequence = shortcut_dlg->get_key_sequence();
        if(current_key_sequence.count() == 1)
        {
            m_current_hot_key->resetShortcut();
            m_current_hot_key->setShortcut(current_key_sequence);
            m_addon_info.short_cut = current_key_sequence.toString();

            Config::Instance()->ChangeHotkey(m_addon_info.addon_name,m_addon_info.short_cut);
            Config::Instance()->SaveHotkeyInfo();
        }

   }
   m_current_hot_key->setRegistered(true);
#endif
}

void ToolBtn::slot_check_tool_update()
{
   if(!m_tool_manuupdate_task->isRunning())
   {
       m_tool_process.kill();
       m_tool_manuupdate_task->set_tool_addon_info(m_addon_info);
       m_tool_manuupdate_task->start();
   }
}

void ToolBtn::slot_show_download_progress(qint64 receiveBytes, qint64 totalBytes)
{
    this->layout()->setSizeConstraint(QLayout::SetFixedSize);

    if(!m_custom_progressbar->isVisible())
    {
        m_custom_progressbar->setVisible(true);
    }
    m_name_label->setFixedHeight(0);
    m_custom_progressbar->setMaximum(totalBytes);
    m_custom_progressbar->setMinimum(0);
    m_custom_progressbar->setValue(receiveBytes);
}

void ToolBtn::slot_on_has_no_update()
{
    Utils::show_messge_box("已经是最新版本",this);
}

void ToolBtn::slot_on_check_failed()
{
    Utils::show_messge_box("检查更新失败",this);
}

void ToolBtn::slot_fix_to_toolbar()
{
#ifdef _WIN32
    QString config_tools_path = Config::Instance()->get_install_bin_path() + "config-tools.exe";
    QProcess* process = new QProcess(this);
    QStringList arugmentlist;
    arugmentlist << "fix_addon" << m_addon_info.addon_name;
    process->startDetached(config_tools_path,arugmentlist);
#else
    QDBusMessage dbusMsg = QDBusMessage::createSignal("/", "com.thunisoft.qimpanel", "show_addon_str");
    dbusMsg << m_addon_info.addon_name;
    QDBusConnection::sessionBus().send(dbusMsg);
#endif
}

void ToolBtn::slot_junmp_to_cofig_tools()
{
#ifdef _WIN32
    QString config_tools_path = Config::Instance()->get_install_bin_path() + "config-tools.exe";
    QProcess* process = new QProcess(this);
    QStringList arugmentlist;
    arugmentlist << "{82717623-mhe4-0293-aduh-ku87wh6328ne}" << "edit_hotkey";
    process->startDetached(config_tools_path,arugmentlist);
#else
    QString config_tools_path = Config::Instance()->get_install_bin_path() + "huayupy-config-tools-fcitx";
    QProcess* process = new QProcess(this);
    QStringList arugmentlist;
    arugmentlist << "82717623-mhe4-0293-aduh-ku87wh6328ne" << "edit_hotkey";
    process->startDetached(config_tools_path,arugmentlist);
#endif

}

void ToolBtn::slot_delete_addon()
{
    QString exe_path = m_addon_info.exe_path;
    if(!QFile::remove(exe_path))
    {
       Utils::show_messge_box("插件使用中无法删除",this);
       return;
    }

    Config::Instance()->RemoveUserAddon(m_addon_info.addon_name);
    Utils::DeleteAddonByName(m_addon_info.addon_name,m_addon_info.addon_version);
    emit addon_removed(m_addon_info.addon_name);
}

void ToolBtn::mousePressEvent(QMouseEvent *e)
{
    QPushButton::mousePressEvent(e);
}

void ToolBtn::mouseReleaseEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton)
    {
        start_the_tool();
    }

    if(e->button() == Qt::RightButton)
    {
        m_custom_tooltip->hide();
        QPoint toolTipPos(width()-10,height()-10);
        QPoint global_pos = mapToGlobal(toolTipPos);

        int rightBorder = global_pos.x() + m_right_menu->width();
        int bottom_border = global_pos.y() + m_right_menu->height();

        QRect desk_rect = QApplication::desktop()->availableGeometry();
        int screen_height = QApplication::primaryScreen()->availableGeometry().height();

        if (rightBorder > desk_rect.width())
        {
            global_pos.setX(desk_rect.width() - m_right_menu->width());
        }

        if (bottom_border > screen_height)
        {
            global_pos.setY(screen_height - m_right_menu->height());
        }
        m_right_menu->move(global_pos);
        m_right_menu->show();
    }
    QPushButton::mouseReleaseEvent(e);
}


bool ToolBtn::event(QEvent *e)
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

void ToolBtn::hoverEnter(QHoverEvent *event)
{
    QPoint toolTipPos(width()-10,height()-10);
    m_custom_tooltip->show_text(tool_tip_info(),mapToGlobal(toolTipPos));
    QPushButton::event(event);
}

void ToolBtn::hoverLeave(QHoverEvent *event)
{
    m_custom_tooltip->hide();
    QPushButton::event(event);
}

void ToolBtn::hoverMove(QHoverEvent *event)
{
    QPushButton::event(event);
}

QString ToolBtn::tool_tip_info()
{
    QString text;
    if(m_addon_info.short_cut.isEmpty())
    {
        text = m_addon_info.addon_describe;
    }
    else
    {
        text = QString("%1\n%2").arg(m_addon_info.addon_describe).arg(m_addon_info.short_cut);
    }
    return text;
}

bool ToolBtn::create_desktop_shortcut(const wchar_t *target_exe_path, const wchar_t *lnk_path,
                                      const wchar_t *working_path, const wchar_t *argument,int shortcut,
                                      bool isShowCmd,const wchar_t* describe,const wchar_t* icon_path)
{
#ifdef _WIN32
    Q_UNUSED(icon_path);
    if((target_exe_path == NULL) || (lnk_path == NULL) || (working_path == NULL) || (argument == NULL))
    {
        return false;
    }

    HRESULT hr;
    IShellLink     *pLink;
    IPersistFile   *ppf;

    hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&pLink);
    if (FAILED(hr))
        return false;

    hr = pLink->QueryInterface(IID_IPersistFile, (void**)&ppf);
    if (FAILED(hr))
    {
        pLink->Release();
        return false;
    }

    //设置目标文件的地址
    pLink->SetPath(target_exe_path);
    //设置工作目录
    pLink->SetWorkingDirectory(working_path);
    //pLink->SetIconLocation(target_exe_path,0);
    pLink->SetArguments(argument);

    if (shortcut != 0)
        pLink->SetHotkey(shortcut);

    //设置描述信息
    pLink->SetDescription(describe);

    //是否显示终端
    pLink->SetShowCmd(isShowCmd);

    hr = ppf->Save(lnk_path, TRUE);

    ppf->Release();
    pLink->Release();
    return SUCCEEDED(hr);
#else
    return true;
#endif

}

void ToolBtn::start_the_tool()
{
   QString exe_path = m_addon_info.exe_path;
   QString argument = m_addon_info.call_paramer;
   QFileInfo fileInfo(exe_path);
   if(!fileInfo.isFile() || !QFile::exists(exe_path))
   {
      //m_custom_tooltip->hide_for_seconds(1200);
      Utils::show_messge_box("插件不可用",this);
      return;
   }

   QStringList argument_list;
   argument_list << argument;
   if(m_addon_info.addon_name == QString("OCR"))
   {
       m_custom_tooltip->setVisible(false);
   }
   if(m_tool_process.state() == QProcess::Running)
   {
        m_tool_process.startDetached(exe_path,argument_list);
   }
   else
   {
        m_tool_process.start(exe_path,argument_list);
   }
}

void ToolBtn::SetUpdateFlag(bool has_update)
{
    QString style_sheet =
            "QLabel"
            "{"
            "padding-left:0px;"
            "padding-right:10px;"
            "padding-top:0px;"
            "padding-bottom:10px;"
            "}";
    if(!m_notice_label)
    {
        m_notice_label = new QLabel(this);
        m_notice_label->setStyleSheet(style_sheet);
        m_notice_label->setAlignment(Qt::AlignCenter | Qt::AlignTop);
        QPixmap pix(5,5);
        pix.fill(Qt::transparent);
        QPainter painter(&pix);
        painter.setPen(Qt::transparent);
        painter.setBrush(Qt::red);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.drawEllipse(pix.rect());
        m_notice_label->setPixmap(pix);
    }

    if(has_update)
    {
        m_notice_label->setVisible(true);
    }
    else
    {
        m_notice_label->setVisible(false);
    }
    update();
}

void ToolBtn::set_shortcut_enable(bool switch_flag)
{
#ifdef _WIN32
    m_current_hot_key->setRegistered(switch_flag);
#endif
}

