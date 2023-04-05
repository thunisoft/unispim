#ifndef BASICCONFIGSTACKEDWIDGET_H
#define BASICCONFIGSTACKEDWIDGET_H

#include <QWidget>
#include "ui_basicconfigstackedwidget.h"

namespace Ui {
class BasicConfigStackedWidget;
}

class FuzzyDialog;
class SuggestsetWidget;
class ToViewPhraseDialog;
class QButtonGroup;
class SPSchemeViewDlg;
class BasicConfigStackedWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BasicConfigStackedWidget(QWidget *parent = nullptr);
    ~BasicConfigStackedWidget();

private:
    Ui::BasicConfigStackedWidget *ui;
private:
    void  InitWidget();
    void  ConnectSignalToSlot();
public:
    void  LoadConfigInfo();

public slots:
    void ResetConfigInfo();

    void SlotHideToolbarStateChanged(int checkStatus);
protected:
    bool eventFilter(QObject *obj, QEvent *event);
private slots:
    void SlotShowFuzzyDlg();
    void SlotShowWordSuggestDlg();

    void SlotEnORChMode(int id, bool state);
    void SlotSimpleORTraditional(int id, bool state);
    void OnInputModeGroupToggled(int id, bool state);
    void SlotHalfORFullMode(int id, bool state);

    void SlotEnableSuggestWord(int flag);
    void SlotEnableSuperSimple(int flag);
    void SlotEnableFuzzyMode(int flag);
    void SlotEnableSpwMode(int flag);

    void SlotFastReplySet();
    void SlotFuzzyEnabled(int state);

    void SlotChangeMinLetters(int index);

    void SlotEditSPScheme();

    void on_button_group_default_chinese_input_mode_button_toggled(int, bool);
private:
   FuzzyDialog* m_fuzzyDlg;
   SuggestsetWidget* m_suggestSetDlg;

   ToViewPhraseDialog *m_toviewPhraseDlg;

   QButtonGroup* m_halfOrFullGroup;
   QButtonGroup* m_inputModeGroup;
   QButtonGroup* m_simpleOrTraditionalGroup;
   QButtonGroup* m_chorenGroup;
   QButtonGroup* m_button_group_default_chinese_input_mode;
   SPSchemeViewDlg* m_spview_dlg;


};

#endif // BASICCONFIGSTACKEDWIDGET_H
