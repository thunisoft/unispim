#ifndef CUSTOMIZEMESSAGEBOXWITHRETURNVALUEDIALOG_H
#define CUSTOMIZEMESSAGEBOXWITHRETURNVALUEDIALOG_H

#include "customize_qwidget.h"

namespace Ui {
class CustomizeMessageBoxWithReturnValueDialog;
}

class CustomizeMessageBoxWithReturnValueDialog : public CustomizeQWidget
{
    Q_OBJECT

public:
    explicit CustomizeMessageBoxWithReturnValueDialog(const QString& content, QWidget *parent = 0);
    ~CustomizeMessageBoxWithReturnValueDialog();

private slots:
    void OnOkButtonClicked();
    void OnCloseButtonClicked();
    void OnCancelButtonClicked();
private:
    void Init();

    QString m_content;
    Ui::CustomizeMessageBoxWithReturnValueDialog *ui;
};

#endif // CUSTOMIZEMESSAGEBOXWITHRETURNVALUEDIALOG_H
