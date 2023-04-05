#ifndef FILEVIEWERDIALOG_H
#define FILEVIEWERDIALOG_H

#include "customize_ui/customize_qwidget.h"

namespace Ui {
class FileViewerDialog;
}

class FileViewerDialog : public CustomizeQWidget
{
    Q_OBJECT

public:
    explicit FileViewerDialog(const QString& title, const QString& file_path, bool is_editable, QWidget *parent = 0);
    ~FileViewerDialog();

private slots:
    void OnOkButtonClicked();
    void OnCloseButtonClicked();

private:
    void Init();
    void SetUpCustomizeUI();
    void FillDataToTextEdit();
    void SetStyleSheet();

    QString m_title;
    QString m_file_path;
    bool m_is_editable;
    Ui::FileViewerDialog *ui;
};

#endif // FILEVIEWERDIALOG_H
