#include "customizemessageboxwithreturnvaluedialog.h"
#include "ui_customizemessageboxwithreturnvaluedialog.h"

CustomizeMessageBoxWithReturnValueDialog::CustomizeMessageBoxWithReturnValueDialog(const QString& content, QWidget *parent) :
    m_content(content),
    CustomizeQWidget(parent),
    ui(new Ui::CustomizeMessageBoxWithReturnValueDialog)
{
    Init();
}

CustomizeMessageBoxWithReturnValueDialog::~CustomizeMessageBoxWithReturnValueDialog()
{
    delete ui;
}

void CustomizeMessageBoxWithReturnValueDialog::Init()
{
    ui->setupUi(this);
    QFont title_font = ui->title_label->font();
    title_font.setPixelSize(18);
    ui->title_label->setFont(title_font);

    ui->content_label->setText(m_content);
    QFont font = ui->content_label->font();
    font.setPixelSize(16);;
    ui->content_label->adjustSize();

    this->setStyleSheet(""
                        " QDialog { "
                        " border : 1px solid rgb(51, 153, 214);"
                        " background-color : rgb(255, 255, 255);"
                        " } "
                        "");

    ui->ok_push_button->setProperty("type","normalBtn");
    ui->cancel_push_button->setProperty("type","normalBtn");
    ui->close_push_button->setProperty("type","closeBtn");
    connect(ui->close_push_button, SIGNAL(clicked(bool)), this, SLOT(OnCloseButtonClicked()));
    connect(ui->cancel_push_button, SIGNAL(clicked(bool)), this, SLOT(OnCancelButtonClicked()));
    connect(ui->ok_push_button, SIGNAL(clicked(bool)), this, SLOT(OnOkButtonClicked()));
}

void CustomizeMessageBoxWithReturnValueDialog::OnOkButtonClicked()
{
    this->accept();
}

void CustomizeMessageBoxWithReturnValueDialog::OnCloseButtonClicked()
{
    this->reject();
}

void CustomizeMessageBoxWithReturnValueDialog::OnCancelButtonClicked()
{
    this->reject();
}
