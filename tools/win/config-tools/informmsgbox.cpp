#include "informmsgbox.h"
#include "ui_informmsgbox.h"
#include <QGraphicsDropShadowEffect>
#include "utils.h"

InformMsgbox::InformMsgbox(QWidget *parent) :
    CustomizeQWidget(parent),
    ui(new Ui::InformMsgbox)
{
    ui->setupUi(this);
    this->setWindowFlag(Qt::WindowStaysOnTopHint);
    InitWidget();
}

InformMsgbox::~InformMsgbox()
{
    delete ui;
}

void InformMsgbox::InitWidget()
{
    SetMsgBoxType();
    ui->confirm_btn->setProperty("type","normal");
    ui->cancel_btn->setProperty("type","normal");
    ui->content_label->setProperty("type","greenLabel");

    QString informWidgetQss = "InformMsgbox{"
                              "border:1px solid #CCCCCC"
                              "}";
    this->setAttribute(Qt::WA_TranslucentBackground,true);
    this->setStyleSheet(informWidgetQss);
    this->setWindowTitle("提醒");


    connect(ui->confirm_btn,SIGNAL(clicked()),this, SLOT(OnConfrimBtnClicked()));
    connect(ui->cancel_btn, SIGNAL(clicked()), this, SLOT(OnCancelBtnClicked()));

}

void InformMsgbox::SetContent(QString inputContent, bool is_abbre)
{
    QFont font("Microsoft YaHei");
    font.setPixelSize(24);
    if(is_abbre)
    {
        if(inputContent.contains("\n")){
            QStringList part_str_list = inputContent.split("\n");
            if(part_str_list.size() == 2){

                QString eliedStr_part1 = Utils::GetElidedText(font,part_str_list.at(0),350);
                QString eliedStr_part2 = Utils::GetElidedText(font,part_str_list.at(1),350);
                QString total_str = eliedStr_part1 + "\n" + eliedStr_part2;
                ui->content_label->setText(total_str);
            }
            else
            {
                QString eliedStr = Utils::GetElidedText(font,inputContent,400);
                ui->content_label->setText(eliedStr);
            }

        }
        else
        {
            QString eliedStr = Utils::GetElidedText(font,inputContent,400);
            ui->content_label->setText(eliedStr);
        }

    }
    else
    {
        ui->content_label->setText(inputContent);
    }
    ui->content_label->setToolTip(inputContent);
}

void InformMsgbox::OnConfrimBtnClicked()
{
    QDialog::accept();
}

void InformMsgbox::OnCancelBtnClicked()
{
    QDialog::reject();
}

void InformMsgbox::SetMsgBoxType(int type)
{
    if(type == 0)
    {
        ui->cancel_btn->setVisible(false);
        ui->betweenBtnSpacer->changeSize(0,0);

    }
    else
    {
        ui->cancel_btn->setVisible(true);
        ui->betweenBtnSpacer->changeSize(40,20);
    }

}
