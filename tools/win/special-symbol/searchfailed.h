#ifndef SEARCHFAILED_H
#define SEARCHFAILED_H

#include <QWidget>

namespace Ui {
class SearchFailed;
}

class SearchFailed : public QWidget
{
    Q_OBJECT

public:
    explicit SearchFailed(QWidget *parent = nullptr);
    ~SearchFailed();

signals:
    void returnMainWindow();


private slots:
    void ReturnBtnClickSlot();
private:
    void InitWidget();
private:
    Ui::SearchFailed *ui;
};

#endif // SEARCHFAILED_H
