#include "editphrasefiledialog.h"
#include "ui_editphrasefiledialog.h"
#include <QFile>
#include <QTextCursor>
#include <QTextStream>
#include <QTextCodec>

#include "customize_ui/customizeqtextedit.h"
#include "customize_ui/generalstyledefiner.h"
#include "customize_ui/customizefont.h"
#include "../public/utils.h"

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
    ui->edit_push_button->setChecked(true);
    InsertWidgetIntoStackedWidget();
    LoadDataIntoPhraseWidget();
    LoadDataIntoInstructionWidget();
    ui->stackedWidget->setCurrentIndex(2);
    RegisterSlots();
}

void EditPhraseFileDialog::InsertWidgetIntoStackedWidget()
{
    m_phrase_file_text_edit = new CustomizeQTextEdit(this);
    m_instruction_text_edit = new CustomizeQTextEdit(this);
    m_instruction_text_edit->setReadOnly(true);
    m_phrase_file_text_edit->setGeometry(0, 0, 351, 380);
    m_instruction_text_edit->setGeometry(0, 0, 351, 380);

    ui->stackedWidget->insertWidget(2, m_phrase_file_text_edit);
    ui->stackedWidget->insertWidget(3, m_instruction_text_edit);

    ui->ok_push_button->setProperty("type","normalBtn");
    ui->edit_push_button->setProperty("type","normalBtn");
    ui->instruction_push_button->setProperty("type","normalBtn");
    ui->close_push_button->setProperty("type","closeBtn");
}

void EditPhraseFileDialog::LoadDataIntoPhraseWidget()
{
    QFile phrase_file(Utils::GetRealPhraseFilePath());
    phrase_file.open(QFile::ReadOnly);
    QTextStream inputStream(&phrase_file);

    if(Utils::GetFileCodeType(Utils::GetRealPhraseFilePath().toStdString().c_str()) == UTF16LE)
    {
        QTextCodec *codec = QTextCodec::codecForName("UTF-16LE");
        inputStream.setCodec(codec);
    }

    QString phrase_data = inputStream.readAll();
    m_phrase_file_text_edit->insertPlainText(phrase_data);
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
    connect(ui->edit_push_button, SIGNAL(clicked(bool)), this, SLOT(OnEditButtonClicked()));
    connect(ui->instruction_push_button, SIGNAL(clicked(bool)), this, SLOT(OnInstructionButtonClicked()));
    connect(ui->close_push_button, SIGNAL(clicked(bool)), this, SLOT(OnCloseButtonClicked()));
    connect(ui->ok_push_button, SIGNAL(clicked(bool)), this, SLOT(OnOkButtonClicked()));
}

void EditPhraseFileDialog::OnEditButtonClicked()
{
    ChangeCurrentStackedIndex(2);
    ui->edit_push_button->setChecked(true);
    ui->ok_push_button->setVisible(true);
}

void EditPhraseFileDialog::OnInstructionButtonClicked()
{
    ChangeCurrentStackedIndex(3);
    ui->instruction_push_button->setChecked(true);
    ui->ok_push_button->setVisible(false);
}

void EditPhraseFileDialog::ChangeCurrentStackedIndex(const int index)
{
    ui->stackedWidget->setCurrentIndex(index);
}

void EditPhraseFileDialog::OnOkButtonClicked()
{
    QFile tmp_file(Utils::GetRealPhraseFilePath().append(".tmp"));
    tmp_file.open(QFile::WriteOnly);
    QTextStream out_stream(&tmp_file);
    out_stream.setGenerateByteOrderMark(true);    //这句是重点

    QTextCodec *codec = QTextCodec::codecForName("UTF-16LE");
    out_stream.setCodec(codec);

    out_stream << m_phrase_file_text_edit->toPlainText();
    out_stream.flush();
    tmp_file.close();

    QFile bak_file(Utils::GetRealPhraseFilePath().append(".bak"));
    if(bak_file.exists())
    {
        bak_file.remove();
    }
    QFile file(Utils::GetRealPhraseFilePath());
    file.rename(Utils::GetRealPhraseFilePath().append(".bak"));

    tmp_file.rename(Utils::GetRealPhraseFilePath());

    this->close();
}

void EditPhraseFileDialog::SetStyleSheet()
{
    this->setStyleSheet(QString(""
                        "#EditPhraseFileDialog  { "
                        " %1 "
                        " } "
                        "").arg(GeneralStyleDefiner::BorderStyle()));
}

void EditPhraseFileDialog::OnCloseButtonClicked()
{
    this->close();
}

void EditPhraseFileDialog::SetCustomizeUI()
{
    ui->setupUi(this);
    SetStyleSheet();
    ui->edit_push_button->setCheckable(true);
    ui->instruction_push_button->setCheckable(true);
    m_button_group = new QButtonGroup(this);
    m_button_group->setExclusive(true);
    m_button_group->addButton(ui->edit_push_button);
    m_button_group->addButton(ui->instruction_push_button);
}
