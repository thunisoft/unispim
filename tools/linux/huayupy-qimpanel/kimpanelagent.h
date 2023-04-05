#ifndef KIMPANELAGENT_H
#define KIMPANELAGENT_H

#include <QObject>
#include <QtDBus>
#include "agenttype.h"

class ImpanelAdaptor;
class Impanel2Adaptor;
class OrgKdeImpanelInterface;
class OrgKdeImpanel2Interface;

class KimpanelAgent
        : public QObject
        , public QDBusContext
{
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", "org.kde.impanel2")
public:
    explicit KimpanelAgent(QObject *parent = 0);
    virtual ~KimpanelAgent();

    void OpenDBus();
    void CloseDBus();
    void Created();
    void MovePreeditCaret(int pos);
    void SelectCandidate(int index);
    void LookupTablePageUp();
    void LookupTablePageDown();
    void TriggerProperty(const QString& key);
    void Exit();
    void ReloadConfig();
    void Restart();
    void Configure();
    void ChangeInputMode(int modeFlag);
    void TogglePinyinOrWbMode(int modeFlag);
    void ToggleQuanOrShuangMode(int inputMode);

    void ExitUI();        
    void QueryEngineState();
    void SetCurrentOutputMode(const int is_traditional);
    void QueryHuayuInUse();
    void QueryPersonNameCandidatesIndex();
    void RemoveUserCiByindex(const int ci_index);

signals:
    void RegisterPropertiesSignal(const QList<KimpanelProperty>& props);
    void ExecMenuSignal(const QList<KimpanelProperty>& props);
    void UpdatePropertySignal(const KimpanelProperty& prop);
    void UpdateLookupTableSignal(const KimpanelLookupTable& table);
    void UpdatePreeditTextSignal(const QString& text, const QList<TextAttribute>& attr);
    void UpdateLookupTableFullSignal(const KimpanelLookupTable& lookup_table, int cursor, int layout);
    void UpdateSpotLocationSignal(int x, int y);
    void UpdateSpotRectSignal(int x, int y, int width, int height);
    void ShowPreeditSignal(bool toShow);
    void ShowAuxSignal(bool toShow);
    void UpdateAuxSignal(const QString& text, const QList<TextAttribute>& attrs);
    void ShowLookupTableSignal(bool toShow);
    void UpdateLookupTableCursorSignal(int pos);
    void UpdatePreeditCaretSignal(int pos);
    void ExecDialogSignal(const KimpanelProperty& prop);
    void OnRecieveSignalHiddenQimPanel();
    void OnRecieveSignalIsShowToolBar(const int is_show_tool_bar);
    void OnRecieveSingalToWubiMode(const int is_wubimode_enable);
    void OnRecieveSignalToSetTraditional(const int is_traditional);
    void OnUmodeChanged(const int is_umode_enabel);
    void OnReciveGetInputMode(const int input_mode);
    void GotResponseInputMode(const int input_mode);
    void OnReciveSetPinyinMode(const int pinyin_mode);
    void GotResponseEngineState(const int state);
    void GotResponseHuayuInUse(const int use);
    void SetHypyEnable(const bool enable);
    void OnReciveSetExpandMode(const int expand_flag);
    void OnHiddenToolbar();

// kimpanel发来的消息
public slots:
    void ServiceUnregistered(const QString& service);

    void ExecDialog(const QString& prop);
    void ExecMenu(const QStringList& entries);
    void RegisterProperties(const QStringList& props);
    void UpdateProperty(const QString& prop);
    void RemoveProperty(const QString& prop);
    void ShowAux(bool toShow);
    void ShowPreedit(bool toShow);
    void ShowLookupTable(bool toShow);
    void UpdateLookupTable(const QStringList &labels,
                           const QStringList &candis,
                           const QStringList &attrlists,
                           bool has_prev, bool has_next);
    void UpdateLookupTableCursor(int pos);
    void UpdatePreeditCaret(int pos);
    void UpdatePreeditText(const QString& text, const QString& attr);
    void UpdateAux(const QString& text, const QString& attr);
    void UpdateSpotLocation(int x, int y);
    void UpdateScreen(int id);
    void Enable(bool enable);

    void SetSpotRect(int x, int y, int width, int height);
    void SetLookupTable(const QStringList &labels,
                        const QStringList &candis,
                        const QStringList &attrlists,       //layout参数：用来覆盖是否要强制竖排/横排
                        bool hasPrev, bool hasNext, int cursor, int layout);
    void HiddenQimPanel();
    void IsShowToolBar(const int is_show_tool_bar);
    void IsWubiModeEnabled(const int is_wubi_enable);
    void IsUmodeEnabled(const int is_umode_enable);
    void GetInputMode(const int input_mode);
    void GetIsTraditional(const int is_traditional);
    void SetPinyinMode(const int pinyin_mode);
    void ResponseInputMode(const int input_mode);
    void ResponseEngineState(const int state);
    void ResponseHuayuInUse(const int use);

    void ChangeToNextIm();
    void SetCurrentCandidatesExpand(const int expand_flag);
    void HiddenToolbar();
protected:
    void SendSignal(const QString& name);
    void SendSignal(const QString& name, const QVariant& arg1);
    void SendSignal(const QString& name, const QList<QVariant>& args);

private:
    ImpanelAdaptor* mAdaptor;
    Impanel2Adaptor* mAdaptor2;
    QDBusServiceWatcher* mWatcher;
    OrgKdeImpanelInterface* mInterface;
    OrgKdeImpanel2Interface* mInterface2;
    QString mCurrentService;
    QStringList mCachedProps;
};

#endif // KIMPANELAGENT_H
