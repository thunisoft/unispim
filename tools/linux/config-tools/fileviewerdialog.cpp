#include "fileviewerdialog.h"
#include "ui_fileviewerdialog.h"

#include <QFile>
#include <QTextStream>
#include "customize_ui/generalstyledefiner.h"
#include "customize_ui/customizefont.h"

FileViewerDialog::FileViewerDialog(const QString& title, const QString& file_path, bool is_editable, QWidget *parent) :
    m_title(title),
    m_file_path(file_path),
    m_is_editable(is_editable),
    CustomizeQWidget(parent),
    ui(new Ui::FileViewerDialog)
{
    Init();

    ui->ok_push_button->setProperty("type","normalBtn");
    ui->close_push_button->setProperty("type","closeBtn");
    connect(ui->close_push_button, SIGNAL(clicked(bool)), this, SLOT(OnCloseButtonClicked()));
    connect(ui->ok_push_button, SIGNAL(clicked(bool)), this, SLOT(OnOkButtonClicked()));
}

FileViewerDialog::~FileViewerDialog()
{
    delete ui;
}

void FileViewerDialog::Init()
{
    SetUpCustomizeUI();
    FillDataToTextEdit();
}

void FileViewerDialog::SetUpCustomizeUI()
{
    ui->setupUi(this);
    ui->title_label->setText(m_title);
    ui->textEdit->setReadOnly(!m_is_editable);

    SetStyleSheet();

}

void FileViewerDialog::FillDataToTextEdit()
{
    QFile file(m_file_path);
    file.open(QFile::ReadOnly);
    ui->textEdit->setText(file.readAll());
    file.close();
}

void FileViewerDialog::OnOkButtonClicked()
{
    if(m_is_editable)
    {
        QFile file(m_file_path);
        file.open(QFile::WriteOnly);
        QTextStream out_stream(&file);
        out_stream << ui->textEdit->document()->toPlainText();
        out_stream.flush();
        file.close();
    }
    this->close();
}

void FileViewerDialog::SetStyleSheet()
{
    this->setStyleSheet(QString(""
                        "#FileViewerDialog  { "
                        " %1 "
                        " } "
                        "").arg(GeneralStyleDefiner::BorderStyle()));

    //ui->title_label->setFont(CustomizeFont::GetH1Font());
    ui->title_label->setProperty("type", "h1");
}

void FileViewerDialog::OnCloseButtonClicked()
{
    this->close();
}
