#ifndef PASSWORDDIALOG_H
#define PASSWORDDIALOG_H

#include <QDialog>

namespace Ui {
class PasswordDialog;
}

class PasswordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PasswordDialog(QWidget *parent = 0);
    ~PasswordDialog();

    QString GetPassword();
    void SetRetry(bool retry);

private slots:
    void on_pushButton_clicked();

    void on_toolButton_2_clicked();

private:
    Ui::PasswordDialog *ui;
    QString m_pwd;
    bool m_retry;
};

#endif // PASSWORDDIALOG_H
