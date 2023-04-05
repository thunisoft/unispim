#include "fileviewerdialog.h"
#include "ui_fileviewerdialog.h"

#include <QFile>
#include <QTextStream>
#include <QTextCodec>

FileViewerDialog::FileViewerDialog(const QString& title, const QString& file_path, bool is_editable, QWidget *parent) :
    m_title(title),
    m_file_path(file_path),
    m_is_editable(is_editable),
    CustomizeQWidget(parent),
    ui(new Ui::FileViewerDialog)
{
    Init();

    connect(ui->cancel_btn, SIGNAL(clicked(bool)), this, SLOT(OnCloseButtonClicked()));
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
    ui->textEdit->setContextMenuPolicy(Qt::NoContextMenu);
    ui->ok_push_button->setProperty("type","normal");
    ui->textEdit->setProperty("type","normal");
    ui->cancel_btn->setProperty("type","normal");
    ui->ok_push_button->setCursor(Qt::PointingHandCursor);
    ui->cancel_btn->setCursor(Qt::PointingHandCursor);

    ui->title_label->setProperty("type","h1");
    this->setProperty("type","borderwidget");

}

void FileViewerDialog::SetUpCustomizeUI()
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_TranslucentBackground,true);
    ui->title_label->setText(m_title);
    ui->textEdit->setReadOnly(!m_is_editable);

    SetStyleSheet();

}

void FileViewerDialog::FillDataToTextEdit()
{
    QFile file(m_file_path);
    file.open(QFile::ReadOnly);
    ui->textEdit->setText(QTextCodec::codecForName( "UTF-16LE" )->toUnicode(file.readAll()));
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

}

void FileViewerDialog::OnCloseButtonClicked()
{
    this->close();
}
