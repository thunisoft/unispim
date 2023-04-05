#include "editphrasefiledialog.h"
#include "ui_editphrasefiledialog.h"
#include <QFile>
#include <QTextCursor>
#include <QTextStream>
#include <QTextCodec>
#include "config.h"
#include "utils.h"

EditPhraseFileDialog::EditPhraseFileDialog(QWidget *parent) :
    CustomizeQWidget(parent),
    ui(new Ui::EditPhraseFileDialog)
{
    Init();
}

EditPhraseFileDialog::~EditPhraseFileDialog()
{
    delete ui;
}

void EditPhraseFileDialog::Init()
{

    SetCustomizeUI();
    //ui->edit_push_button->setChecked(true);
    InsertWidgetIntoStackedWidget();
    LoadDataIntoPhraseWidget();
    LoadDataIntoInstructionWidget();
    ui->stackedWidget->setCurrentIndex(2);
    RegisterSlots();
}

void EditPhraseFileDialog::InsertWidgetIntoStackedWidget()
{
    m_phrase_file_text_edit = new QTextEdit(this);
    m_instruction_text_edit = new QTextEdit(this);
    m_instruction_text_edit->setReadOnly(true);

    m_phrase_file_text_edit->setContextMenuPolicy(Qt::NoContextMenu);
    m_instruction_text_edit->setContextMenuPolicy(Qt::NoContextMenu);

    m_phrase_file_text_edit->setPlaceholderText("如hh=哈哈哈哈");


    ui->stackedWidget->insertWidget(2, m_phrase_file_text_edit);
    ui->stackedWidget->insertWidget(3, m_instruction_text_edit);
}

void EditPhraseFileDialog::LoadDataIntoPhraseWidget()
{
    m_phrase_file_text_edit->clear();
    QFile phrase_file(Config::Instance()->CustomPhraseFilePath());
    phrase_file.open(QFile::ReadOnly);
    QTextCodec *codec = QTextCodec::codecForName("UTF-16LE");
    QTextStream inputStream(&phrase_file);
    inputStream.setCodec(codec);
    m_rawString = inputStream.readAll();
    m_phrase_file_text_edit->insertPlainText(m_rawString);
    phrase_file.close();
}

void EditPhraseFileDialog::LoadDataIntoInstructionWidget()
{
    QFile instruction_file(Utils::GetHuayuPYInstallDirPath().append("phrase/instruction.txt"));
    instruction_file.open(QFile::ReadOnly);
    QString instruction_data = instruction_file.readAll();
    m_instruction_text_edit->insertPlainText(instruction_data);
    QTextCursor text_cursor = m_instruction_text_edit->textCursor();
    text_cursor.movePosition(QTextCursor::Start);
    m_instruction_text_edit->setTextCursor(text_cursor);
}

void EditPhraseFileDialog::RegisterSlots()
{
    connect(ui->ok_push_button, SIGNAL(clicked(bool)), this, SLOT(OnOkButtonClicked()));
    connect(ui->cancel_btn,SIGNAL(clicked()), this, SLOT(OnCancelButtonClicked()));
}

void EditPhraseFileDialog::OnEditButtonClicked()
{
    ChangeCurrentStackedIndex(2);
    //ui->edit_push_button->setChecked(true);
    ui->ok_push_button->setVisible(true);
}

void EditPhraseFileDialog::OnInstructionButtonClicked()
{
    ChangeCurrentStackedIndex(3);
    //ui->instruction_push_button->setChecked(true);
    ui->ok_push_button->setVisible(false);
}

void EditPhraseFileDialog::ChangeCurrentStackedIndex(const int index)
{
    ui->stackedWidget->setCurrentIndex(index);
}

void EditPhraseFileDialog::OnOkButtonClicked()
{
    QString newString = m_phrase_file_text_edit->toPlainText().trimmed();
    if(newString != m_rawString)
    {
        if(CheckThePhraseContent(newString) == 1)
        {
            Utils::NoticeMsgBox("自定义短语的键值应以字母开头",this);
            return;
        }
        if(Utils::NoticeMsgBox("已添加自定义短语\n是否保存到文件?",this,1) == QDialog::Rejected)
        {
            return;
        }
    }

    QFile tmp_file(Config::Instance()->CustomPhraseFilePath());
    tmp_file.open(QFile::WriteOnly);
    QTextCodec *codec = QTextCodec::codecForName("UTF-16LE");
    QTextStream out_stream(&tmp_file);
    out_stream.setCodec(codec);
    out_stream << m_phrase_file_text_edit->toPlainText().trimmed();
    out_stream.flush();
    tmp_file.close();

    QDialog::accept();
}

void EditPhraseFileDialog::SetStyleSheet()
{
    //ui->edit_push_button->setProperty("type","normal");
    //ui->instruction_push_button->setProperty("type","normal");
    ui->ok_push_button->setProperty("type","normal");
    this->setProperty("type","borderwidget");
    ui->add_title_widget->setProperty("type","window_title_widget");
    ui->cancel_btn->setProperty("type","normal");
    ui->label->setProperty("type","h2");
    ui->ok_push_button->setCursor(Qt::PointingHandCursor);
    ui->cancel_btn->setCursor(Qt::PointingHandCursor);
}

void EditPhraseFileDialog::OnCloseButtonClicked()
{
    this->close();
}

void EditPhraseFileDialog::SetCustomizeUI()
{
    ui->setupUi(this);
    SetStyleSheet();
    this->setAttribute(Qt::WA_TranslucentBackground,true);
    //ui->edit_push_button->setCheckable(true);
    //ui->instruction_push_button->setCheckable(true);
    m_button_group = new QButtonGroup(this);
    m_button_group->setExclusive(true);
    //m_button_group->addButton(ui->edit_push_button);
    //m_button_group->addButton(ui->instruction_push_button);
}

int EditPhraseFileDialog::CheckThePhraseContent(QString inputContent)
{
    QStringList strList = inputContent.split("\n");
    QRegExp formatExp("^[a-zA-Z].*");
    for(int index=0; index<strList.length(); ++index)
    {
        if(strList.at(index).isEmpty())
        {
            continue;
        }
        if(formatExp.exactMatch(strList.at(index)))
        {
            continue;
        }
        else
        {
            return 1;
        }
    }
    return 0;
}

void EditPhraseFileDialog::OnCancelButtonClicked()
{
    QDialog::reject();
}


