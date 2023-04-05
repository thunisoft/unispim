#include "addoreditphrasedialog.h"
#include "phrasevalidator.h"
#include "phrasemodel.h"
#include <QRegExp>
#include <QValidator>
#include <QRegExpValidator>
#include <QListView>
#include "config.h"

AddOrEditPhraseDialog::AddOrEditPhraseDialog(PHRASE_TITLE phrase_title, int phrase_index, PHRASE* phrase, QWidget* parent):
                                             CustomizeQWidget(parent)
                                            ,m_phrase_title(phrase_title)
                                            ,m_phrase_index(phrase_index)
                                            ,m_phrase(phrase)
                                            ,ui(new Ui::AddPhrase)
{
    Init();
}

void AddOrEditPhraseDialog::Init()
{
    SetUpCustomizeUI();

    SetDialogTitle();
    connect(ui->textEdit,&QTextEdit::textChanged,this, &AddOrEditPhraseDialog::OnContentChanged);
    if(m_phrase_title == EDIT_PHRASE_DIALOG)
        FillPhraseDataToEveryCell();

    //设置输入栏的限制
    QRegExp reg("[a-zA-Z;]{1,15}");
    ui->line_edit_phrase_name->setValidator(new QRegExpValidator(reg));
    connect(ui->push_button_cancel, SIGNAL(clicked()), this, SLOT(OnCloseButtonClicked()));
    connect(ui->push_button_ok, SIGNAL(clicked(bool)), this, SLOT(OnOkButtonClicked()));
}

void AddOrEditPhraseDialog::SetUpCustomizeUI()
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_TranslucentBackground,true);
    QStringList items;
    items << "默认" << "1" << "2" << "3" << "4"
          << "5" << "6" << "7" << "8" << "9";
    ui->display_position_comboBox->addItems(items);
    ui->display_position_comboBox->setCurrentIndex(0);

    ui->content_label->setProperty("type","h2");
    ui->display_position_label->setProperty("type","h2");
    ui->label_tip->setProperty("type","h2");
    ui->label_tip_2->setProperty("type","h2");

    ui->line_label_1->setProperty("type","h2");

    ui->phrase_name_tip_label->setProperty("type","h2");
    ui->push_button_cancel->setProperty("type","normal");
    ui->push_button_ok->setProperty("type","normal");
    ui->push_button_cancel->setCursor(Qt::PointingHandCursor);
    ui->push_button_ok->setCursor(Qt::PointingHandCursor);

    this->setProperty("type","borderwidget");
    ui->widget->setProperty("type","window_title_widget");
    ui->add_title_label->setProperty("type","h2");

    ui->display_position_comboBox->setView(new QListView());
    ui->display_position_comboBox->setAutoFillBackground(true);
    ui->display_position_comboBox->setMaxVisibleItems(6);

    SetStyleSheet();
}

void AddOrEditPhraseDialog::SetDialogTitle()
{
    if(m_phrase_title == EDIT_PHRASE_DIALOG)
    {
        ui->add_title_label->setText(QString::fromUtf8("编辑短语"));
    }
    else
    {
        ui->add_title_label->setText(QString::fromLocal8Bit("新增短语"));
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
    QString phrase_name = ui->line_edit_phrase_name->text();
    if(phrase_name.isEmpty())
    {
        error_info = QString::fromLocal8Bit("短语名不能为空!");
        return false;
    }
    if(phrase_name.startsWith(";"))
    {
        error_info = ";不能在开头";
        return false;
    }

    if(!PhraseValidator::IsValidAbbr(ui->line_edit_phrase_name->text(), error_info))
    {
        return false;
    }

    if(ui->textEdit->document()->toPlainText().isEmpty())
    {
        error_info = QString::fromLocal8Bit("内容不能为空!");
        return false;
    }
    if(ui->textEdit->document()->toPlainText().contains("="))
    {
        error_info = QString::fromLocal8Bit("内容中不能包含符号\"=\"");
        return false;
    }
    int all_character_length = ui->textEdit->document()->toPlainText().length();
    if(all_character_length > 2048)
    {
        error_info = QString::fromLocal8Bit("内容超过2048个字符!");
        return false;
    }
    QString abbr = ui->line_edit_phrase_name->text();
    abbr.remove(QRegExp("\\s"));
    QString content = ui->textEdit->document()->toPlainText();
    if(PhraseModel::Instance()->IsRepeat(abbr, content, m_phrase_index))
    {
        error_info = QString::fromLocal8Bit("该短语已存在!");
        return false;
    }

    return true;
}

void AddOrEditPhraseDialog::OnOkButtonClicked()
{
    QString error_info;
    if(!ValidPhrase(error_info))
    {
        Utils::NoticeMsgBox(error_info, this);
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
}

void AddOrEditPhraseDialog::OnCloseButtonClicked()
{
    close();
}

void AddOrEditPhraseDialog::OnContentChanged()
{
    QString content = ui->textEdit->document()->toPlainText();
    int all_character_length = content.length();
    ui->label_tip_2->setText(QString("说明：最多2048个字符(%1/2048)").arg(all_character_length));
}

void AddOrEditPhraseDialog::ResetContent()
{
    ui->display_position_comboBox->setCurrentIndex(0);
    ui->line_edit_phrase_name->clear();
    ui->textEdit->clear();
}
