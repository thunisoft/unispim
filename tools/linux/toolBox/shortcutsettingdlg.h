#ifndef SHORTCUTSETTINGDLG_H
#define SHORTCUTSETTINGDLG_H

#include <QDialog>
#include <QKeyEvent>
#include <QCloseEvent>

namespace Ui {
class ShortcutSettingDlg;
}

class ShortcutSettingDlg : public QDialog
{
    Q_OBJECT

public:
    explicit ShortcutSettingDlg(QWidget *parent = 0);
    ~ShortcutSettingDlg();

private:
    void set_window_shadow();
    void check_hotkey_conflict();

protected:
    void keyPressEvent(QKeyEvent* event);

public slots:
    void slot_key_sequence_changed(const QKeySequence &keySequence);
    QKeySequence get_key_sequence();

private:
    Ui::ShortcutSettingDlg *ui;
};

#endif // SHORTCUTSETTINGDLG_H
