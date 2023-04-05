#ifndef TOVIEWPHRASEDIALOG_H
#define TOVIEWPHRASEDIALOG_H

#include <QDialog>
#include <QMap>
#include <QCloseEvent>
#include "customize_ui/customize_qwidget.h"

#include "../public/utils.h"

namespace Ui {
class ToViewPhraseDialog;
}
class PhraseModel;

class ToViewPhraseDialog : public CustomizeQWidget
{
    Q_OBJECT

public:
    explicit ToViewPhraseDialog(QWidget *parent = 0);
    ~ToViewPhraseDialog();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void OnOptionPhraseClicked(int button_type, int phrase_index);
    void OnAddPhraseButtonClicked();
    void OnPhraseAddedOrChanged(PHRASE phrase, int phrase_index = -1);
    void OnEditePhraseFileButtonClicked();
    void OnViewSysPhraseButtonClicked();
    void OnApplyButtonClicked();
    void OnCloseButtonClicked();
    void OnCancelButtonClicked();

private:
    void Init();
    void SetUpCustomizeUI();
    void RegisterSlots();
    void FillDataIntoTable();
    void SetStyleSheet();
    void SetDefaultPhrasePosition();
    void SaveDefaultPhrasePoistion();

    void SetPhraseIsModified(bool toggle);
    bool GetPhraseIsModified();

    Ui::ToViewPhraseDialog *ui;

    bool m_phrase_modified;
};

#endif // TOVIEWPHRASEDIALOG_H
