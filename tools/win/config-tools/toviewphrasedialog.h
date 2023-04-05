#ifndef TOVIEWPHRASEDIALOG_H
#define TOVIEWPHRASEDIALOG_H

#include <QDialog>
#include <QMap>
#include <QCloseEvent>
#include <QMouseEvent>
#include <QModelIndex>
#include <QTableWidgetItem>

#include "utils.h"

using namespace  OLD_VERSION;

namespace Ui {
class ToViewPhraseDialog;
}
class PhraseModel;
class AddOrEditPhraseDialog;
class EditPhraseFileDialog;
class FileViewerDialog;
class Customizephrasebtn;
class ToViewPhraseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ToViewPhraseDialog(QWidget *parent = nullptr);
    ~ToViewPhraseDialog();

public slots:
    void ScaledToHighDpi();
    void UpdateComboBoxStyleSheet();


protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void OnAddPhraseButtonClicked();
    void OnPhraseAddedOrChanged(PHRASE phrase, int phrase_index = -1);
    void OnEditPhraseFileButtonClicked();
    void OnViewSysPhraseButtonClicked();
    void OnApplyButtonClicked();
    void OnCloseButtonClicked();
    void OnCancelButtonClicked();
private slots:
    void SlotEditPrase();
    void SlotDelPrase();
    void SlotRowIndexChanged(QPoint index);
    void SlotTableWidgetItemPressed(QTableWidgetItem* clickedItem);
public slots:
     void SlotTableWidgetLoseFocus();

protected:
    virtual void focusOutEvent(QFocusEvent* e);

private:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    void Init();
    void SetUpCustomizeUI();
    void RegisterSlots();
    void FillDataIntoTable(QMap<int,PHRASE> phraseMap);
    void SetStyleSheet();
    void SetDefaultPhrasePosition();
    void SaveDefaultPhrasePoistion();

    void SetPhraseIsModified(bool toggle);
    bool GetPhraseIsModified();
    bool CompareTempPhraseAndRealPhrase();

    bool m_phrase_modified;
    Ui::ToViewPhraseDialog *ui;
    AddOrEditPhraseDialog* m_addorEditDlg;
    EditPhraseFileDialog* m_editPhraseFileDlg;
    FileViewerDialog* m_fileViewDlg;
    QMap<int, PHRASE> m_tempPhrase;
    Customizephrasebtn* m_btnGroupEdit;
    Customizephrasebtn* m_btnGroupDel;

private:
    QPoint m_last_mouse_position;
    bool m_move_widget_flag;
};

#endif // TOVIEWPHRASEDIALOG_H
