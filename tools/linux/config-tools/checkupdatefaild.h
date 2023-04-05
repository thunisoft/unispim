#ifndef CHECKUPDATEFAILD_H
#define CHECKUPDATEFAILD_H
#include <QWidget>
#include <QDialog>
#include "ui_openofficewebsite.h"
class CheckUpdateFaild :public QDialog
{
    Q_OBJECT
public:
    explicit CheckUpdateFaild(QWidget *parent = 0);
public slots:
        void OnOpenOfficeWebsiteClicked();
        void OnOkButtonClicked();
        void closeEvent(QCloseEvent *event);
private:
   Ui::openofficewebsite *ui;
signals:
   void SignalCloseCheckFaildDialog();
};

#endif // CHECKUPDATEFAILD_H
