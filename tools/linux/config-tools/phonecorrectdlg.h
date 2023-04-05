#ifndef PHONECORRECTDLG_H
#define PHONECORRECTDLG_H

#include <QDialog>
#include <QVector>
#include <QCheckBox>
#include "customize_ui/customize_qwidget.h"

namespace Ui {
class PhoneCorrectDlg;
}

class PhoneCorrectDlg : public CustomizeQWidget
{
    Q_OBJECT

public:
    explicit PhoneCorrectDlg(QWidget *parent = 0);
    ~PhoneCorrectDlg();
    int GetCongigResult();
    void SetInitValue(const int inputValue);
private:
    void InitCheckBox();
    void SaveConfig();
    void InitCheckState();
    void SetStyleSheet();

private slots:
    void onCancelBtn();
    void onOKBtn();
    void onResetBtn();
    void onCloseBtn();

signals:
    void settingChanged();

private:
    Ui::PhoneCorrectDlg *ui;
    QVector<QCheckBox*> m_checkBoxVector;
    int m_configResult;
    QStringList m_errorList;

};

#endif // PHONECORRECTDLG_H
