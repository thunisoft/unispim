#ifndef EDITPHRASEFILEDIALOG_H
#define EDITPHRASEFILEDIALOG_H

#include <QDialog>
#include <QTextEdit>
#include <QButtonGroup>
#include "customize_ui/customize_qwidget.h"

namespace Ui {
class EditPhraseFileDialog;
}

class EditPhraseFileDialog : public CustomizeQWidget
{
    Q_OBJECT

public:
    explicit EditPhraseFileDialog(QWidget *parent = 0);
    ~EditPhraseFileDialog();

private slots:
    void OnEditButtonClicked();
    void OnInstructionButtonClicked();
    void OnOkButtonClicked();
    void OnCloseButtonClicked();
    void OnCancelButtonClicked();

private:
    void Init();
    void InsertWidgetIntoStackedWidget();

    void LoadDataIntoInstructionWidget();
    void RegisterSlots();
    void ChangeCurrentStackedIndex(const int index);
    void SetStyleSheet();
    void SetCustomizeUI();


private:
    int CheckThePhraseContent(QString inputContent);

public:
    void LoadDataIntoPhraseWidget();

    Ui::EditPhraseFileDialog *ui;
    QTextEdit *m_phrase_file_text_edit;
    QTextEdit *m_instruction_text_edit;
    QButtonGroup *m_button_group;
    QString m_rawString;
};

#endif // EDITPHRASEFILEDIALOG_H
