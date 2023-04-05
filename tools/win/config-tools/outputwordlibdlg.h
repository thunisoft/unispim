#ifndef OUTPUTWORDLIBDLG_H
#define OUTPUTWORDLIBDLG_H

#include <QWidget>
#include "customize_ui/customize_qwidget.h"
#include <QTableWidgetItem>

namespace Ui {
class OutputWordlibDlg;
}

class OutputWordlibDlg : public CustomizeQWidget
{
    Q_OBJECT

public:
    explicit OutputWordlibDlg(QWidget *parent = nullptr);
    ~OutputWordlibDlg();
    inline QVector<QString> GetWordlibNameVector()
    {
        return m_itemNameVector;
    }
    QString GetSaveDirPath();
    QString GetOutputFileType();

private slots:
    void SlotsGetSaveFileDir();
    void OnOkButtonClicked();
    void OnCancelButtonClicked();
    void UpdateShow();


public slots:
    void SetWordlibVector(QVector<QString> wordlibVector);
    void RefreshWordlibVector(QString inputItem);


private:
    void InitWidget();

private:
    Ui::OutputWordlibDlg *ui;
    QVector<QString> m_itemNameVector;
};

#endif // OUTPUTWORDLIBDLG_H
