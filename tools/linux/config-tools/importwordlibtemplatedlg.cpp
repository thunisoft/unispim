#include "importwordlibtemplatedlg.h"
#include "ui_importwordlibtemplatedlg.h"

ImportWordlibTemplateDlg::ImportWordlibTemplateDlg(QWidget *parent) :
    CustomizeQWidget(parent),
    ui(new Ui::ImportWordlibTemplateDlg)
{
    ui->setupUi(this);
    this->setWindowTitle("词库文本导入模板");

    QString style_sheet = ""
                          " QPushButton { "
                          " image : url(:/image/close.svg); "
                          " border : 0px solid;"
                          " width: 20px;"
                          " height:18px;"
                          " } "
                          " QPushButton:hover { "
                          " image : url(:/image/close_hover.svg); "
                          " } "
                          " QPushButton:pressed { "
                          " image : url(:/image/close.svg); "
                          " border : 0px solid;"
                          " } ";
    ui->close_btn->setFlat(true);
    ui->close_btn->setStyleSheet(style_sheet);
    ui->close_btn->setFocusPolicy(Qt::NoFocus);
    ui->close_btn->setToolTip("关闭");
    this->setStyleSheet(QString("QWidget#ImportWordlibTemplateDlg{border:1px solid rgb(51, 153, 214);}"));
    connect(ui->close_btn, &QPushButton::clicked, this, [&](){this->close();});
    ui->textEdit->setStyleSheet("border:none;");

}

ImportWordlibTemplateDlg::~ImportWordlibTemplateDlg()
{
    delete ui;
}

void ImportWordlibTemplateDlg::OnConfirmBtnClicked()
{
    this->close();
}

void ImportWordlibTemplateDlg::SetTemplateText(QString inputContent)
{
    ui->textEdit->clear();
    ui->textEdit->setText(inputContent);
}
