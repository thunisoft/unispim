#include "addoreditphrasedialog.h"
#include "phrasevalidator.h"
#include "phrasemodel.h"
#include "customize_ui/customizefont.h"
#include "customize_ui/generalstyledefiner.h"
#include "msgboxutils.h"

#include <QListView>

AddOrEditPhraseDialog::AddOrEditPhraseDialog(PHRASE_TITLE phrase_title, int phrase_index, PHRASE* phrase, QWidget* parent):
                                             ui(new Ui::AddPhrase)
                                            ,m_phrase_title(phrase_title)
                                            ,m_phrase_index(phrase_index)
                                            ,m_phrase(phrase)
                                            ,CustomizeQWidget(parent)


{
    Init();
}

void AddOrEditPhraseDialog::Init()
{
    SetUpCustomizeUI();

    SetDialogTitle();
    if(m_phrase_title == EDIT_PHRASE_DIALOG)
        FillPhraseDataToEveryCell();



    connect(ui->close_push_button, SIGNAL(clicked()), this, SLOT(OnCloseButtonClicked()));
    connect(ui->push_button_cancel, SIGNAL(clicked()), this, SLOT(OnCloseButtonClicked()));
    connect(ui->push_button_ok, SIGNAL(clicked(bool)), this, SLOT(OnOkButtonClicked()));
}

void AddOrEditPhraseDialog::SetUpCustomizeUI()
{
    ui->setupUi(this);

    ui->close_push_button->setProperty("type","closeBtn");


    QString line_style_sheet = ""
                               " QLabel { "
                               " background-color : rgb(51, 153, 255); "
                               " border : 0px solid;"
                               " } "
                               "";
    ui->line_label_1->setStyleSheet(line_style_sheet);
    ui->line_label_2->setStyleSheet(line_style_sheet);

    ui->push_button_cancel->setProperty("type","normalBtn");
    ui->push_button_ok->setProperty("type","normalBtn");

    ui->display_position_comboBox->setFixedWidth(120);
    ui->display_position_comboBox->setFixedHeight(26);
    ui->display_position_comboBox->setView(new QListView());
    ui->display_position_comboBox->setAutoFillBackground(true);
    ui->display_position_comboBox->setMaxVisibleItems(10);


    QStringList items;
    items << "默认" << "1" << "2" << "3" << "4"
          << "5" << "6" << "7" << "8" << "9";
    ui->display_position_comboBox->addItems(items);
    ui->display_position_comboBox->setCurrentIndex(0);


    SetStyleSheet();
}

void AddOrEditPhraseDialog::SetDialogTitle()
{
    if(m_phrase_title == EDIT_PHRASE_DIALOG)
    {
        ui->title_label->setText(QString::fromLocal8Bit("编辑短语"));
    }
    else
    {
        ui->title_label->setText(QString::fromLocal8Bit("新增短语"));
    }
}

void AddOrEditPhraseDialog::FillPhraseDataToEveryCell()
{
    if(!m_phrase)
        return;
    ui->line_edit_phrase_name->setText(m_phrase->abbr);
    ui->display_position_comboBox->setCurrentIndex(m_phrase->position);
    ui->textEdit->setText(m_phrase->content);
}

bool AddOrEditPhraseDialog::ValidPhrase(QString& error_info)
{
    if(ui->line_edit_phrase_name->text().isEmpty())
    {
        error_info = QString::fromLocal8Bit("短语名不能为空！");
        return false;
    }

    if(!PhraseValidator::IsValidAbbr(ui->line_edit_phrase_name->text(), error_info))
    {
        return false;
    }

    if(ui->textEdit->document()->toPlainText().isEmpty())
    {
        error_info = QString::fromLocal8Bit("短语内容不能为空！");
        return false;
    }
    if(ui->textEdit->document()->toPlainText().contains("="))
    {
        error_info = QString::fromLocal8Bit("短语内容中不能包含符号\"=\"");
        return false;
    }
    if(ui->textEdit->document()->toPlainText().length() > 100)
    {
        error_info = QString::fromLocal8Bit("短语长度不能超过100个字符！");
        return false;
    }
    QString abbr = ui->line_edit_phrase_name->text();
    abbr.remove(QRegExp("\\s"));
    QString content = ui->textEdit->document()->toPlainText();
    if(PhraseModel::Instance()->IsRepeat(abbr, content, m_phrase_index))
    {
        error_info = QString::fromLocal8Bit("该短语已存在！");
        return false;
    }

    return true;
}

void AddOrEditPhraseDialog::OnOkButtonClicked()
{
    QString error_info;
    if(!ValidPhrase(error_info))
    {
        MsgBoxUtils::NoticeMsgBox(error_info,"系统提示",0, this);
        return ;
    }

    PHRASE phrase;
    WrapPhrase(phrase);
    emit SignalToSavePhrase(phrase, m_phrase_index);
    this->close();
}

void AddOrEditPhraseDialog::WrapPhrase(PHRASE& phrase)
{
    phrase.abbr = ui->line_edit_phrase_name->text().remove(QRegExp("\\s"));
    phrase.content = ui->textEdit->document()->toPlainText();
    phrase.position = ui->display_position_comboBox->currentIndex();
    if(m_phrase_title == EDIT_PHRASE_DIALOG)
    {
        phrase.state = MODIFIED;
    }
    else
    {
        phrase.state = ADDED;
    }
}

void AddOrEditPhraseDialog::SetStyleSheet()
{
    this->setStyleSheet(QString(""
                        "#AddPhrase  { "
                        " %1 "
                        " } "
                        "").arg(GeneralStyleDefiner::BorderStyle()));
    //this->ui->title_label->setFont(CustomizeFont::GetH1Font());
    this->ui->title_label->setProperty("type", "h1");
}

void AddOrEditPhraseDialog::OnCloseButtonClicked()
{
    close();
}
