#include "passworddialog.h"
#include "ui_password_dialog.h"

PasswordDialog::PasswordDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PasswordDialog),
    m_retry(false)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/image/logo.png"));
    setWindowTitle("密码输入");
}

PasswordDialog::~PasswordDialog()
{
    delete ui;
}

QString PasswordDialog::GetPassword()
{
    return m_pwd;
}

void PasswordDialog::SetRetry(bool retry)
{
    m_retry = retry;
    ui->label->setText("密码错误，请重新输入:");
}

void PasswordDialog::on_pushButton_clicked()
{
    m_pwd = ui->lineEdit->text();
    this->accept();
}

void PasswordDialog::on_toolButton_2_clicked()
{
    this->reject();
}
