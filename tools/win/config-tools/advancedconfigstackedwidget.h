#ifndef ADVANCEDCONFIGSTACKEDWIDGET_H
#define ADVANCEDCONFIGSTACKEDWIDGET_H

#include <QWidget>
#include <QButtonGroup>
#include <QComboBox>
#include "customize_ui/basestackedwidget.h"

namespace Ui {
class AdvancedConfigStackedWidget;
}

class AdvancedConfigStackedWidget : public BaseStackedWidget
{
    Q_OBJECT

public:
    explicit AdvancedConfigStackedWidget(QWidget *parent = nullptr);
    ~AdvancedConfigStackedWidget();

private:
    void InitWidget();

    void ConnectSignalToSlot();
    void InitKeyPairCandidates();
    void InitShortcutWidget();
    void ShortcutContentExclusive();
public:
    void LoadConfigInfo();

public slots:
    void ResetConfigInfo();
    void RefreshShortcutList(QComboBox* currentCombo,QString oldStr,QString newStr);

    void SlotOnOCRKeyChanged(const QString &text);
    void SlotOnVoiceKeyChanged(const QString &text);
protected:
    bool eventFilter(QObject *obj, QEvent *event);
private slots:
    void SlotEnCnSwitch(int id, bool checkStatus);
    void SlotKeyPairSet(int id);
    void Slot2nd3ndCandidates(int id, bool checkStatus);
    //以词定字
    void SlotCiToziSet(int id, bool checkStatus);
    void SlotShortCutSet(int id);
    void SlotScopeChanged(int id, bool checkStatus);

    //软键盘切换的快捷键
    //void SlotSoftPanelComboIndexChanged(int index);

    //英文输入法的切换
    void SlotCnEnComboIndexChanged(const QString &text);

    //全双拼切换
    void SlotQuanShuangComboIndexChanged(const QString &text);

    //繁体和简体的切换
    void SlotSimpleTraditionalComboIndexChanged(const QString &text);

    //使用汉字声调辅助
    void SlotUseHZToneToAssist(int checkStatus);
    
    void SlotEnableNameModeCheck(int checkStatus);

    void OnNameModeComboIndexChanged(const QString &text);

    void SlotChangeSymbolAfterNum(int checkStatus);
    void SlotSelectCandidatesByNumpad(int checkStatus);

    void SlotChangetheJoinPlanState(int checkStatus);

    void SlotPostAfterSwitch(int checkStatus);

    void SlotOpenPalnContent();

    void SlotCheckBoxUsingEnglishPunctuationInChineseMode(int checkStatus);

    void SlotCheckBoxUseEnglishInput(int checkStatus);

    void SlotCheckBoxUsingHistoryShortcut(int checkStatus);

    void onLineEditDomainTextChanged(QString text);

    void OnCiFrequenceChanged(int index);
    void OnCharacterFrequenceChanged(int index);

private:
    void OnServerConfigOptionChanged();

private:
    Ui::AdvancedConfigStackedWidget *ui;
    QButtonGroup* m_pairKeyGroup;
    QButtonGroup* m_shortcutGroup;
    QButtonGroup* m_modeChangeGroup;
    QButtonGroup* m_candidates2nd3ndGroup;
    QButtonGroup* m_citoziGroup;
    QButtonGroup* m_scopeGroup;
    QVector<QComboBox*> m_comboBoxVector;
    QStringList m_shortcutList;
    QMap<QComboBox*,QString> m_shortcutMap;
};

#endif // ADVANCEDCONFIGSTACKEDWIDGET_H
