#ifndef INFORMMSGBOX_H
#define INFORMMSGBOX_H

#include "customize_ui/customize_qwidget.h"

namespace Ui {
class InformMsgbox;
}

class InformMsgbox : public CustomizeQWidget
{
    Q_OBJECT

public:
    explicit InformMsgbox(QWidget *parent = nullptr);
    ~InformMsgbox();

public:
    void SetContent(QString inputContent, bool is_abbre = true);
    void SetMsgBoxType(int type = 0);
public slots:
    void OnConfrimBtnClicked();
    void OnCancelBtnClicked();

private:
    void InitWidget();

private:
    Ui::InformMsgbox *ui;
};

#endif // INFORMMSGBOX_H
