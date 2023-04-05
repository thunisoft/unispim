#include "customizemessageboxdialog.h"
#include "ui_customizemessageboxdialog.h"

CustomizeMessageBoxDialog::CustomizeMessageBoxDialog(const QString& content, QWidget *parent) :
    m_content(content),
    CustomizeQWidget(parent),
    ui(new Ui::CustomizeMessageBoxDialog)
{
    Init();
}

CustomizeMessageBoxDialog::~CustomizeMessageBoxDialog()
{
    delete ui;
}

void CustomizeMessageBoxDialog::SetTitle(QString titleContent)
{
    ui->title_label->setText(titleContent);
}

void CustomizeMessageBoxDialog::EnableWithReturnValue(bool flag)
{
    if(flag)
    {
        ui->cancel_push_btn->setVisible(true);
    }
    else
    {
        ui->cancel_push_btn->setVisible(false);
    }
}

void CustomizeMessageBoxDialog::Init()
{
    ui->setupUi(this);    
    QFont title_font = ui->title_label->font();
    title_font.setPixelSize(18);
    ui->title_label->setFont(title_font);

    QFont font = ui->content_label->font();
    font.setPixelSize(16);
    ui->content_label->setText(m_content);
    ui->content_label->adjustSize();
    ui->content_label->setScaledContents(true);

    //set the style sheet
    QFile file(":/feedback/basicstyle.qss");
    if(file.open(QIODevice::ReadOnly))
    {
        this->setStyleSheet(file.readAll());
        file.close();
    }
    setWindowIcon(QIcon(":/image/logo.png"));
    setWindowTitle("提示窗口");

    ui->ok_push_button->setProperty("type","normalBtn");
    ui->cancel_push_btn->setProperty("type","normalBtn");
    ui->close_push_button->setProperty("type","closeBtn");
    connect(ui->close_push_button, SIGNAL(clicked(bool)), this, SLOT(OnCloseButtonClicked()));
    connect(ui->ok_push_button, SIGNAL(clicked(bool)), this, SLOT(OnOkButtonClicked()));
    connect(ui->cancel_push_btn, SIGNAL(clicked(bool)),this,SLOT(OnCancelBtnClicked()));
}

void CustomizeMessageBoxDialog::OnOkButtonClicked()
{
    this->accept();
}

void CustomizeMessageBoxDialog::OnCloseButtonClicked()
{
    this->reject();
}

void CustomizeMessageBoxDialog::OnCancelBtnClicked()
{
    this->reject();
}
