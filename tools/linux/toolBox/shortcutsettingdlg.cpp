#include "shortcutsettingdlg.h"
#include "ui_shortcutsettingdlg.h"
#include <QDebug>
#include <QTimer>
#include <QGraphicsDropShadowEffect>
#include "custominfombox.h"
#include "config.h"

ShortcutSettingDlg::ShortcutSettingDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShortcutSettingDlg)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint|Qt::Dialog | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground,true);
    set_window_shadow();
    connect(ui->keySequenceEdit, &QKeySequenceEdit::keySequenceChanged,
            this, &ShortcutSettingDlg::slot_key_sequence_changed);
    ui->keySequenceEdit->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    ui->tip_label->setProperty("type","tip_info");
}

ShortcutSettingDlg::~ShortcutSettingDlg()
{
    delete ui;
}

void ShortcutSettingDlg::slot_key_sequence_changed(const QKeySequence &keySequence)
{
    qDebug() << keySequence.toString();
    qDebug() << keySequence.count();
    if(keySequence.count() >= 2)
    {
        ui->keySequenceEdit->clear();
    }
    check_hotkey_conflict();
}

QKeySequence ShortcutSettingDlg::get_key_sequence()
{
    return ui->keySequenceEdit->keySequence();
}

void ShortcutSettingDlg::set_window_shadow()
{
    QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
    shadow_effect->setOffset(0, 0);
    shadow_effect->setColor(QColor(150,150,150));
    shadow_effect->setBlurRadius(6);
    ui->shortcut_widget->setGraphicsEffect(shadow_effect);
}

void ShortcutSettingDlg::check_hotkey_conflict()
{
    QMap<QString,QString> hotkey_map = Config::Instance()->GetHotkeyMap();
    QString shortcut_str = ui->keySequenceEdit->keySequence().toString();
    if(hotkey_map.values().contains(shortcut_str))
    {
        CustomInfomBox* inform_msgbox = new CustomInfomBox(this);
        inform_msgbox->set_inform_content("快捷键冲突");
        inform_msgbox->show();
        QTimer *m_time = new QTimer();
        m_time->setSingleShot(true);
        m_time->start(1200);
        connect(m_time, &QTimer::timeout, [=](){
            inform_msgbox->hide();
        });
    }
}

void ShortcutSettingDlg::keyPressEvent(QKeyEvent *event)
{
    event->accept();
    if(event->key() == Qt::Key_Escape)
    {
       reject();
    }
    else if(event->key() == Qt::Key_Return)
    {
        accept();
    }
}

