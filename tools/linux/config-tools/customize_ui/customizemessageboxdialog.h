#ifndef CUSTOMIZEMESSAGEBOXDIALOG_H
#define CUSTOMIZEMESSAGEBOXDIALOG_H

#include "customize_qwidget.h"

namespace Ui {
class CustomizeMessageBoxDialog;
}

class CustomizeMessageBoxDialog : public CustomizeQWidget
{
    Q_OBJECT

public:
    explicit CustomizeMessageBoxDialog(const QString& content, QWidget *parent = 0);
    ~CustomizeMessageBoxDialog();

public:
    void SetTitle(QString titleContent);
    void EnableWithReturnValue(bool flag);

private slots:
    void OnOkButtonClicked();
    void OnCloseButtonClicked();
    void OnCancelBtnClicked();
private:
    void Init();

    QString m_content;
    Ui::CustomizeMessageBoxDialog *ui;
};

#endif // CUSTOMIZEMESSAGEBOXDIALOG_H
