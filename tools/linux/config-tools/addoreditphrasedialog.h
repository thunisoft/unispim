#ifndef ADDOREDITPHRASEDIALOG_H
#define ADDOREDITPHRASEDIALOG_H

#include "customize_ui/customize_qwidget.h"
#include "../public/utils.h"
#include "ui_addoreditphrasedialog.h"
#include <QWidget>

enum PHRASE_TITLE
{
    EDIT_PHRASE_DIALOG,
    ADD_PHRASE_DIALOG
};

class AddOrEditPhraseDialog : public CustomizeQWidget
{
    Q_OBJECT
public:
    explicit AddOrEditPhraseDialog(PHRASE_TITLE phrase_title, int phrase_index, PHRASE* phrase = NULL, QWidget* parent = 0);

signals:
    void SignalToSavePhrase(PHRASE phrase, int phrase_index);

private slots:
    void OnOkButtonClicked();
    void OnCloseButtonClicked();

private:
    void Init();
    void SetUpCustomizeUI();
    void SetDialogTitle();
    void FillPhraseDataToEveryCell();
    bool ValidPhrase(QString& error_info);
    void WrapPhrase(PHRASE& phrase);
    void SetStyleSheet();

    PHRASE_TITLE m_phrase_title;
    int m_phrase_index;
    PHRASE* m_phrase;

    Ui::AddPhrase *ui;
};

#endif // ADDOREDITPHRASEDIALOG_H
