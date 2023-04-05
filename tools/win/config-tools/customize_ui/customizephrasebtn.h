#ifndef CUSTOMIZEPHRASEBTN_H
#define CUSTOMIZEPHRASEBTN_H

#include <QWidget>

namespace Ui {
class Customizephrasebtn;
}

class Customizephrasebtn : public QWidget
{
    Q_OBJECT

public:
    explicit Customizephrasebtn(QString tip, QWidget *parent = nullptr);
    ~Customizephrasebtn();


signals:
    void btnclicked();

private slots:
    void SlotbtnClicked();

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
    void InitWidget(QString tip);

private:
    int m_rowNum;
private:
    Ui::Customizephrasebtn *ui;
};

#endif // CUSTOMIZEPHRASEBTN_H
