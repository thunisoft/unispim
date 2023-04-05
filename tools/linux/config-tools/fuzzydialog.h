#ifndef FUZZYDIALOG_H
#define FUZZYDIALOG_H

#include <QWidget>
#include "customize_ui/customize_qwidget.h"
#include <QCheckBox>
#include <QVector>

namespace Ui {
class FuzzyDialog;
}

class FuzzyDialog : public CustomizeQWidget
{
    Q_OBJECT

public:
    explicit FuzzyDialog(QWidget *parent = 0);
    ~FuzzyDialog();

public:
    void SetInitValue(const int inputValue);


private:
    void InitWidget();
    void ConnectSignalToSlot();
    void InitCheckBox();
    void SetStyleSheet();

private slots:
    void ResetSlot();
    void ConfirmSlot();
    void CancelSlot();

private:
    Ui::FuzzyDialog *ui;
    QVector<QCheckBox*> m_checkVector;
    QStringList m_objectNameList;
    int m_initValue;
};

#endif // FUZZYDIALOG_H
