#ifndef ADVANCEDCONFIGSTACKEDWIDGET_H
#define ADVANCEDCONFIGSTACKEDWIDGET_H

#include <QWidget>
#include <QButtonGroup>
#include <QComboBox>

namespace Ui {
class AdvancedConfigStackedWidget;
}

class AdvancedConfigStackedWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AdvancedConfigStackedWidget(QWidget *parent = nullptr);
    ~AdvancedConfigStackedWidget();

private:
    void InitWidget();
    void SetItemObjectName();
    void ConnectSignalToSlot();
    void InitKeyPairCandidates();
    void InitShortcutWidget();
    void ShortcutContentExclusive();
    void fillComboBoxA2Z(QComboBox *comboBox);

public:
    void LoadConfigInfo();

    void slot_scroll_to_hotkey();
public slots:
    void ResetConfigInfo();
    void RefreshShortcutList(QComboBox* currentCombo,QString oldStr,QString newStr);
    void SlotShortCutSet(int id);

protected:
    bool eventFilter(QObject *obj, QEvent *event);
private slots:
    void SlotEnCnSwitch(int id, bool checkStatus);
    void SlotKeyPairSet(int id, bool checked);
    void OnCheckStateChanged(int checkStatus);


    //以词定字
    void OnButtonGroupPickWord(int id);

    void OnButtonGroup2nd3rdCandidateClicked(int id);
    void OnButtonGroupOutputScope(int id);

    void SlotOpenPalnContent();
    void ConfigFcitxSlot();

    void onLineEditDomainTextChanged(QString text);
    void slot_on_combo_text_changed(QString comboText);


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
