#ifndef CUSTOMIZEBTNGROUP_H
#define CUSTOMIZEBTNGROUP_H

#include <QWidget>
#include <QDockWidget>
#include "customize_qwidget.h"

namespace Ui {
class CustomizeBtnGroup;
}

class CustomizeBtnGroup : public QWidget
{
    Q_OBJECT

public:
    explicit CustomizeBtnGroup(QWidget *parent = nullptr);
    ~CustomizeBtnGroup();


signals:
    void deleteclicked();

private slots:
    void SlotDeletebtnClicked();

public:
    inline int GetRowNum()
    {
        return m_rowNum;
    }
    void SetRowNum(int rowNum)
    {
        m_rowNum = rowNum;
    }

private:
    void InitWidget();

private:
    int m_rowNum;
private:
    Ui::CustomizeBtnGroup *ui;
};

#endif // CUSTOMIZEBTNGROUP_H
