#ifndef SPSETPLANDLG_H
#define SPSETPLANDLG_H

#include <QDialog>
#include <QTableWidgetItem>
#include "customize_ui/customize_qwidget.h"

namespace Ui {
class SPSetPlanDlg;
}

class SPSetPlanDlg : public CustomizeQWidget
{
    Q_OBJECT

public:
    explicit SPSetPlanDlg(QWidget *parent = nullptr);
    ~SPSetPlanDlg();

private:
    void InitWidget();
    void IniParser(const QString iniFilePath);
    void ConnectSignalToSlot();
    void LoadConfig();

private:
    void SaveConfigInfo();


private slots:
    void SlotShengmuSelected(QTableWidgetItem* item);
    void SlogYunmuSelected(QTableWidgetItem* item);
    void SlotZeroShengmuSelected(QTableWidgetItem* item);

    void SlotConfirmed();
    void SlotCanceled();
    void SlotResetSPPlan();
    void SlotLineEditChanged(const QString str);

    void SlotSPPlanChanged(int index);

private:
    Ui::SPSetPlanDlg *ui;
    QTableWidgetItem* m_selectedItem;
};

#endif // SPSETPLANDLG_H
