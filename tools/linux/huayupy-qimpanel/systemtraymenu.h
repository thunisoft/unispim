#ifndef SYSTEMTRAYMENU_H
#define SYSTEMTRAYMENU_H

#include <QObject>
#include <QMenu>
#include <QTimer>

#include "agenttype.h"

class KimpanelAgent;

class SystemTrayMenu : public QMenu
{
    Q_OBJECT
public:
    SystemTrayMenu(KimpanelAgent* agent, const bool hide_tray);
    virtual ~SystemTrayMenu();

    void Init();

    void TriggerUpdateVKListMenu();
    void TriggerUpdateIMListMenu();
    void TriggerUpdateMozcHiraganaMenu();
    void TriggerUpdateMozcToolMenu();
    void SetToolStatus(bool is_show);

public slots:
    void ClearMenu();
    void DoAboutToShow();
    void UpdateMainMenu();
    void MenuItemOnClick(QAction* action);

    void RegisterProperties(const QList<KimpanelProperty>& props);
    void ExecMenu(const QList<KimpanelProperty>& props);
    void UpdateProperty(const KimpanelProperty& prop);
    void OnDelayShowTimeOut();

    bool eventFilter(QObject *, QEvent *);
protected:
    void leaveEvent(QEvent* event);

private:
    bool IsIMList(const QString& key);
    bool IsVKList(const QString& key);
    bool IsMozcHiraganaList(const QString& key);
    bool IsMozcToolList(const QString& key);
    bool IsUnity();
    QString DoUpdateIMListMenu(const QList<KimpanelProperty> props);
    void DoUpdateVKListMenu(const QList<KimpanelProperty> props);
    void DoUpdateMozcHiraganaListMenu(const QList<KimpanelProperty> props);
    void DoUpdateMozcToolListMenu(const QList<KimpanelProperty> props);
    //void DoUpdateSkinListMenu();
    void DoUpdateStatusMenu(const QList<KimpanelProperty> props);

    bool IsKbdIcon(const QString& icon);
    void SkinMenuItemOnClick(QAction* action);
    void OpenConfigTool();
    void ShowORHideToolBar();
    void RestartFcitx();
    void ExitTheFcitx();
signals:
    void hideTray(int hide);

private:
    KimpanelAgent* mAgent;
    QList<KimpanelProperty> mStatusMenuList;
    QList<KimpanelProperty> mIMList;
    QList<KimpanelProperty> mVKList;
    QList<KimpanelProperty> mMozcHiraganaList;
    QList<KimpanelProperty> mMozcToolList;

    QMenu* mVKListMenu;
    QMenu* mMozcToolMenu;
    QMenu* mMozcHiraganaMenu;
    QMenu* mSkinMenu;
    QTimer* mTimerToDelayShow;

    QString mCurrentIMLable;
    QString mCurrentVkName;
    bool mHideTray;
    bool mIsShowMenu;
};

#endif // SYSTEMTRAYMENU_H
