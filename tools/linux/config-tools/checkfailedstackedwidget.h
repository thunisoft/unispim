#ifndef CHECKFAILEDSTACKEDWIDGET_H
#define CHECKFAILEDSTACKEDWIDGET_H

#include <QWidget>

namespace Ui {
class CheckFailedStackedWidget;
}

class CheckFailedStackedWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CheckFailedStackedWidget(QWidget *parent = 0);
    ~CheckFailedStackedWidget();



private:
    void InitWidget();
    void ConnectSignalToSlot();

private slots:
    void ReUpdateSlot();


signals:
    void reupdate();

private:
    Ui::CheckFailedStackedWidget *ui;
};

#endif // CHECKFAILEDSTACKEDWIDGET_H
