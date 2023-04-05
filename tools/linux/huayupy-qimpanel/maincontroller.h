#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include <QSystemTrayIcon>

#include "agenttype.h"
#include "mainmodel.h"
#include "toolbarmodel.h"
#include "candidatewindow.h"
#include "toolbarwindow.h"
#include <atomic>
#include <QDateTime>
#include "../public/addon_config.h"



class KimpanelAgent;
class SystemTrayMenu;
class SkinBase;
class TrayController;

class MainController : public QObject
{
    Q_OBJECT
    // 定义Interface名称为"com.huayupy_qimpanel.hotel"
    Q_CLASSINFO("D-Bus Interface", "com.thunisoft.qimpanel")

public:
    static MainController* Instance();
    virtual ~MainController();

    void Init();
    void LoadConfig();
    void ReloadHideTray();
    QString GetSkinName();
    void SetSkinName(const QString& skinName);
    void SetSkinBase(SkinBase* skinBase, int skinType);
    SkinBase* GetSkinBase();
    ToolBarModel* GetToolbarModel();

    bool HasUpdate();

private:
    explicit MainController(QObject *parent = 0);
    void CreateDBusService();
    void OpenDBus();
    void CloseDBus();

signals:

public slots:
    void UpdateProperty(const KimpanelProperty& prop);
    void UpdatePreeditText(const QString inputString, QList<TextAttribute> attributes);
    void UpdateLookupTable(const KimpanelLookupTable& table);
    void UpdateLookupTableFull(const KimpanelLookupTable& table, int cursor, int layout);
    void UpdateSpotLocation(int x, int y);
    void UpdateSpotRect(int x, int y, int width, int height);
    void ShowPreedit(bool toShow);
    void ShowAux(bool toShow);
    void UpdateAux(const QString& text, const QList<TextAttribute>& attrs);
    void ShowLookupTable(bool toShow);
    void UpdateLookupTableCursor(int pos);
    void RegisterProperties(const QList<KimpanelProperty>& props);
    void UpdatePreeditCaret(int pos);
    void ShowUI(bool toShow);
    void OnTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void ChangeQimPanelSize(int size);
    void ChangeInputMode(int modeFlag);
    void GotInputModeFromEngine(int input_mode);
    void ChangeWubiMode(int modeFlag);
    void ChangedUmode(int modeFlag);
    void ChangeTraditional(int is_traditional);
    void SetPinyinMode(int pinyin_mode);
    void ChangePinyinMode(int pinyin_mode);
    void ShowMenu(int x, int y);
    void SetQimpanel(const QString& key, int value);
    void SetQimpanel(const QString& key, const QString& value);
    void OnUsingEnglishPunctuationInChineseMode(int enabled);
    void OnHzOptionChanged(int option);
    void GotPunchFromEngine(const bool is_cn);
    void GotStateFromEngine(const int state);
    void GotInUseFromEngine(const int use);
    void SetHasUpdate(const bool flag);
    void ChangeToEnglishMode();
    void OnSetHypyEnable(const bool enable);
    void RefreshTheWindowUI();
    void ReloadLoginState();
    int GetPinyinMode();
    void ChangeCandiatesExpandFlag(const int flag);
    void OnTabKeyclicked();
    void OnHiddenToolbar();
    void execuate_ocr_addon();
    void execuate_voice_addon();
    void update_person_name_candidate_index(int index);

public:
    Q_INVOKABLE void getPrevPage();
    Q_INVOKABLE void getNextPage();
    Q_INVOKABLE void selectCandidate(int index);
    Q_INVOKABLE void changeCnMode(bool isCn);
    Q_INVOKABLE void toggleWbOrPinyinMode(bool isWubi);
    Q_INVOKABLE void changeTrad(bool isTrad);
    Q_INVOKABLE void changeFullWidth(bool isFull);
    Q_INVOKABLE void changePunc(bool isCnMark);
    Q_INVOKABLE void showSoftKbd(bool show);
    Q_INVOKABLE void showConfigDialog(bool show);
    Q_INVOKABLE void showSymbolsDialog(bool show);
    Q_INVOKABLE void showToolTip(QString toolTip);
    Q_INVOKABLE void hideToolTip();
	Q_INVOKABLE void showToolBox(QPoint point);


private:
    void CreateToolBarView();
    void CreateInputView(KimpanelAgent* agent);
    void UpdateImProperty(const KimpanelProperty& prop);
    void UpdateChttransProperty(const KimpanelProperty& prop);
    void UpdateFullWidthProperty(const KimpanelProperty& prop);
    void UpdatePuncProperty(const KimpanelProperty& prop);
    void UpdateSoftKbdProperty(const KimpanelProperty& prop);
    void ChangeImToEnglish(bool falg = false);
    /*五笔拼音切换*/
    void ChangeImToWB(bool flag);
    void MonitorFcitx();
    bool IsFcitxOn();
    void InitPanelSize();
    /*保存五笔设置*/
    void SaveWubiConfig(bool is_wubi);

    /*保存繁简设置*/
    void SaveIsTraditional(bool is_trad);
    QString GetOSType();

private slots:
    void Wait();
    void HiddenQimPanel();
    void JudgeShouldHide();
    void HandlerHideTray(const int hide_tray);
    void GetHideTrayFromSet(const int hide_tray);
public slots:
    void DoShowUI(bool toShow);

    void slot_fix_target_addon_on_toolbar(QString addon_name);
    void SetToolBarConfig(int configValue);
private:
    static MainController* mInstance;
#ifdef USE_FCITX
    TrayController* mSystemTray;        // 我们写的
#else
    QSystemTrayIcon* mSystemTray;       // QT 的
#endif
    KimpanelAgent* mAgent;
    SystemTrayMenu* mTrayMenu;
    SkinBase* mSkinBase;

    MainModel* mModel;
    ToolBarModel* mToolBarModel;    
    QIcon* mIcon;

    bool mIsHorizontal;
    QString mSkinName;
    bool mServiceOpen;
    QTimer *mFcitxMonitorTimer;
    bool mIsWb;
    bool mIsUmode;
    int mPinyinMode;
    int mToolbarConfig;
    bool mIsFisrtUpdateImProperty;
    bool mHideTray;
    bool mHasUpdate;
    atomic_bool mCanShow;
    atomic_bool mEngineNotResponse;

    CandidateWindow* mCandidateWindow;
    ToolbarWindow* mToolbarWindow;

};

#endif // MAINCONTROLLER_H
