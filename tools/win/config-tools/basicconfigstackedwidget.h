#ifndef BASICCONFIGSTACKEDWIDGET_H
#define BASICCONFIGSTACKEDWIDGET_H

#include <QWidget>
#include "customize_ui/basestackedwidget.h"

namespace Ui {
class BasicConfigStackedWidget;
}

class FuzzyDialog;
class SuggestsetWidget;
class ToViewPhraseDialog;
class QButtonGroup;
class SPSchemeViewDlg;
class BasicConfigStackedWidget : public BaseStackedWidget
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
    void SlotShowSPSetDlg();
    void SlotShowFuzzyDlg();
    void SlotShowWordSuggestDlg();
    void SlotShowAddSingleItem();
    void SlotShowAddLotsItem();

    void SlotEnORChMode(int id, bool state);
    void SlotSimpleORTraditional(int id, bool state);
    void SlotHalfORFullMode(int id, bool state);
    void OnInputModeGroupToggled(int id, bool state);

    void SlotEnableSuggestWord(int flag);
    void SlotEnableSuperSimple(int flag);
    void SlotEnableFuzzyMode(int flag);
    void SlotEnableSpwMode(int flag);

    void SlotFastReplySet();
    void SlotFuzzyEnabled(int state);
    void SlotSPSchemeChanged(int index);

    void SlotChangeMinLetters(int index);

    void SlotEditSPScheme();

private:
   FuzzyDialog* m_fuzzyDlg;
   SuggestsetWidget* m_suggestSetDlg;

   ToViewPhraseDialog *m_toviewPhraseDlg;

   QButtonGroup* m_halfOrFullGroup;
   QButtonGroup* m_inputModeGroup;
   QButtonGroup* m_simpleOrTraditionalGroup;
   QButtonGroup* m_chorenGroup;
   SPSchemeViewDlg* m_spview_dlg;


};

#endif // BASICCONFIGSTACKEDWIDGET_H
