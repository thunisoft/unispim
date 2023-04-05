#ifndef WUBISTACKEDWIDGHT_H
#define WUBISTACKEDWIDGHT_H

#include <QWidget>
#include <QButtonGroup>

namespace Ui {
class wubistackedwidght;
}

class wubistackedwidght : public QWidget
{
    Q_OBJECT

public:
    explicit wubistackedwidght(QWidget *parent = 0);
    ~wubistackedwidght();
    void ResetConfigInfo();    
    void LoadConfigInfo();
private slots:
    void OnFourCharacterInputCheckedSlot(int checkstate);
    void OnFiveCharacterInputCheckedSlot(int checkstate);
    void OnNoCandidateCancelInputCheckedSlot(int checkstate);
    void OnEnterCancelInputCheckedSlot(int checkstate);
    void OnVersionRadioButtonToggled(int index, bool state);
    void OnHintCheckedSlot(int checkoutstate);
    void OnDynamicWubiRateChekedSlot(int checkstate);

private:
    void InitWidght();
private:
    Ui::wubistackedwidght *ui;
    QButtonGroup *m_wubi_version_button_group;

};

#endif // WUBISTACKEDWIDGHT_H
