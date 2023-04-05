#include "phraseoptionwidget.h"
#include "ui_phraseoptionwidget.h"
#include "../public/utils.h"
#include "msgboxutils.h"

PhraseOptionWidget::PhraseOptionWidget(int phrase_idnex, QWidget *parent) :
    m_phrase_index(phrase_idnex),
    QWidget(parent),
    ui(new Ui::PhraseOptionWidget)
{
    Init();

    ui->delete_push_button->setProperty("type","noborderbtn");
    ui->edit_push_button->setProperty("type","noborderbtn");

    connect(ui->edit_push_button, SIGNAL(clicked(bool)), this, SLOT(OnEditButtonClicked()));
    connect(ui->delete_push_button, SIGNAL(clicked(bool)), this, SLOT(OnDeleteButtonClicked()));
}

PhraseOptionWidget::~PhraseOptionWidget()
{
    delete ui;
}


void PhraseOptionWidget::Init()
{
    ui->setupUi(this);

    QString style_sheet = ""
                          " QWidget { "
                          " border : 0px solid;"
                          " } "
                          "";
    this->setStyleSheet(style_sheet);
}


void PhraseOptionWidget::OnEditButtonClicked()
{
    emit OptionPhraseButtonClicked(EDIT_PHRASE, m_phrase_index);
}

void PhraseOptionWidget::OnDeleteButtonClicked()
{
    if(!MsgBoxUtils::NoticeMsgBox(QString::fromLocal8Bit("确定要删除吗？"),"系统提示",1, this))
        return;
    emit OptionPhraseButtonClicked(DELETE_PHRASE, m_phrase_index);
}
