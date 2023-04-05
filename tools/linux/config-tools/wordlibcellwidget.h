#ifndef WORDLIBCELLWIDGET_H
#define WORDLIBCELLWIDGET_H

#include <QWidget>
#include "../public/utils.h"
#include "../public/defines.h"
#include <QCheckBox>

namespace Ui {
class WordlibCellWidget;
}

class WordlibCellWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WordlibCellWidget(const WORDLIB_INFO& wordlib_info, QWidget *parent = 0);
    ~WordlibCellWidget();
    void SetUpElements();
    bool checkable() const { return m_checkBox != nullptr; }

signals:
    void WordlibStateChanged(int state, const QString& wordlib_file_name);
    void DownloadWordlib(const QString& wordlib_name);

public slots:
    void OnDownloadDone(const QString& file_name);
    void check();
    void uncheck();

private slots:
    void OnCheckBoxStateChaged(int state);
    void OnDownloadButtonClicked();
    void OnDeleteButtonClicked();

private:
    void Init();
    void DeleteFile();
    void AddDownloadButtonToWidget();
    void MoveFileToCloudDir();
    void SelectWordlib(const QString& name);
    void UnSelectWordlib(const QString& name);

    Ui::WordlibCellWidget *ui;
    WORDLIB_INFO m_wordlib_info;

    QCheckBox *m_checkBox;
};

#endif // WORDLIBCELLWIDGET_H
