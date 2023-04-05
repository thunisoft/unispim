#ifndef WORDLIBADDWIDGET_H
#define WORDLIBADDWIDGET_H

#include <QWidget>
#include <QProcess>
#include <QTimer>
#include "importwordlibtemplatedlg.h"

namespace Ui {
class WordlibAddWidget;
}

class WordlibAddWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WordlibAddWidget(QWidget *parent = 0);
    ~WordlibAddWidget();

signals:
    void ReturnButtonClicked();

private slots:
    void OnReturnButtonClicked();
    void OnImportDone(int exit_code);
    void OnImportButtonClicked();
    void OnWriteToUwlDone();
    void OnTextChanged();
    void OnImportFromFileClicked();
    void ImportWordlibFromTxtFile();
    void OnTemplateBtnClicked();

private:
    void Init();
    void CallWordlibTool(const QStringList& param);
    bool WriteTextEditContentToTmpFile(const QString& content);
    void SetOkButtonDisable();
    void SetOkButtonEnable();

    QProcess* m_process;
    Ui::WordlibAddWidget *ui;
    bool m_is_proceess_running;    
    QTimer m_wait_write_to_uwl_timer;
    QString m_txt_file_path;
};

#endif // WORDLIBADDWIDGET_H
