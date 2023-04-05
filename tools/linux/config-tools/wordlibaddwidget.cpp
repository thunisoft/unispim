#include "wordlibaddwidget.h"
#include "ui_wordlibaddwidget.h"
#include <QTextStream>
#include <QFileDialog>
#include "../public/utils.h"
#include "../public/config.h"
#include "msgboxutils.h"
#include "configbus.h"
#include <QFileInfo>

WordlibAddWidget::WordlibAddWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WordlibAddWidget),
    m_is_proceess_running(false)
{
    Init();
}

WordlibAddWidget::~WordlibAddWidget()
{
    delete ui;
    if(m_process)
        delete m_process;
}

void WordlibAddWidget::OnReturnButtonClicked()
{
    ui->textEdit->clear();
    emit ReturnButtonClicked();
}

void WordlibAddWidget::Init()
{
    ui->setupUi(this);
    m_process = new QProcess(this);
    m_is_proceess_running = false;
    m_wait_write_to_uwl_timer.setSingleShot(true);
    SetOkButtonDisable();

    ui->import_from_file_push_button->setProperty("type","normalBtn");
    ui->import_push_button->setProperty("type","normalBtn");
    ui->return_push_button->setProperty("type","normalBtn");
    ui->template_btn->setProperty("type","puretextbtn");
    ui->template_btn->setCursor(Qt::PointingHandCursor);

    connect(ui->return_push_button, SIGNAL(clicked()), this, SLOT(OnReturnButtonClicked()));
    connect(ui->import_push_button, SIGNAL(clicked()), this, SLOT(OnImportButtonClicked()));
    connect(ui->import_from_file_push_button, SIGNAL(clicked()), this, SLOT(OnImportFromFileClicked()));
    connect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(OnTextChanged()));
    connect(ui->template_btn, SIGNAL(clicked()),this, SLOT(OnTemplateBtnClicked()));
}

void WordlibAddWidget::CallWordlibTool(const QStringList& param)
{
    connect(m_process, SIGNAL(finished(int)), this, SLOT(OnImportDone(int)));
    QString process_name(Utils::GetWordlibToolPath());

    Utils::AppendFrameSuffix(process_name);
    QString user_uwl_file_path = Config::Instance()->GetUserUwlPath();
    QFile user_uwl_file(user_uwl_file_path);

    if(!user_uwl_file.exists())
    {
        ConfigBus::instance()->valueChanged("loadWordlib", "");
    }

    if (!QFileInfo(process_name).exists())
    {
        MsgBoxUtils::NoticeMsgBox(QString::fromLocal8Bit("找不到导库工具, 导入失败！") + process_name,"系统提示",0,this);
        return;
    }

    m_process->start(process_name, param);
    if (m_process->error() == QProcess::FailedToStart)
    {
        MsgBoxUtils::NoticeMsgBox(QString::fromLocal8Bit("找不到导库工具, 导入失败！"),"系统提示",0,this);
        return;
    }
    m_is_proceess_running = true;
}

void WordlibAddWidget::OnImportDone(int exit_code)
{
    m_is_proceess_running = false;
    disconnect(m_process, SIGNAL(finished(int)), 0, 0);
    if(exit_code != 0)
    {
        QString filePath = QString(Utils::GetTmpDir()).append("wordlib_file_error_info.txt");
        if (!QFileInfo(filePath).exists())
        {
            MsgBoxUtils::NoticeMsgBox(QString::fromLocal8Bit("无法获取导入结果, 导入失败！[%1]").arg(filePath),"系统提示",0,this);
            return;
        }
        QFile file(filePath);
        file.open(QFile::ReadOnly);
        ui->textEdit->setText(file.readAll());
    }
    else
    {
        MsgBoxUtils::NoticeMsgBox(QString::fromLocal8Bit("导入成功！"),"系统提示",0,this);
        ui->textEdit->clear();
    }
    ConfigBus::instance()->valueChanged("loadWordlib", "");
    ConfigBus::instance()->valueChanged("userWordlibCanSave", 1);
}

void WordlibAddWidget::OnImportButtonClicked()
{
    if(m_is_proceess_running)
        return;
    QString edit_area_context = ui->textEdit->toPlainText();
    if(edit_area_context.isEmpty())
        return;
    if(!WriteTextEditContentToTmpFile(edit_area_context))
        return;
    ConfigBus::instance()->valueChanged("saveUserWordlib", "");
    ConfigBus::instance()->valueChanged("userWordlibCanSave", 0);
    //引擎将用户词库写入文件需要时间，所以停1秒
    connect(&m_wait_write_to_uwl_timer, SIGNAL(timeout()), this, SLOT(OnWriteToUwlDone()));
    m_wait_write_to_uwl_timer.start(1000);
}

void WordlibAddWidget::OnImportFromFileClicked()
{
    if(m_is_proceess_running)
        return;
    QFileDialog* file_dialog = new QFileDialog(this);
    file_dialog->setWindowTitle(QString::fromLocal8Bit("选择词库文本"));
    file_dialog->setDirectory(QDir::homePath());
    file_dialog->setNameFilter(tr("File(*.txt)"));
    file_dialog->setFileMode(QFileDialog::ExistingFiles);
    //设置视图模式
    file_dialog->setViewMode(QFileDialog::Detail);
    QStringList file_names;
    if(file_dialog->exec() == QDialog::Accepted)
    {
        file_names = file_dialog->selectedFiles();
    }
    else
    {
        return;
    }
    m_txt_file_path = file_names.at(0);

    ConfigBus::instance()->valueChanged("saveUserWordlib", "");
    ConfigBus::instance()->valueChanged("userWordlibCanSave", 0);
    //引擎将用户词库写入文件需要时间，所以停1秒
    connect(&m_wait_write_to_uwl_timer, SIGNAL(timeout()), this, SLOT(ImportWordlibFromTxtFile()));
    m_wait_write_to_uwl_timer.start(1000);

}

void WordlibAddWidget::OnWriteToUwlDone()
{
    disconnect(&m_wait_write_to_uwl_timer, SIGNAL(timeout()), 0, 0);
    QStringList param;
    param << "-append";
    param << QString(Utils::GetTmpDir()).append("user_wordlib.tmp");
    param << Config::Instance()->GetUserUwlPath();
    CallWordlibTool(param);
}

void WordlibAddWidget::ImportWordlibFromTxtFile()
{
    disconnect(&m_wait_write_to_uwl_timer, SIGNAL(timeout()), 0, 0);
    QStringList param;
    param << "-combine-uwl-text";
    param << m_txt_file_path;
    param << Config::Instance()->GetUserUwlPath();

    CallWordlibTool(param);
}

void WordlibAddWidget::OnTemplateBtnClicked()
{
    ImportWordlibTemplateDlg text_template_dlg(this);
    text_template_dlg.SetTemplateText("名称=自定义词库\n作者=华宇拼音输入法\n编辑=1\n\n你好 ni'hao 3000");
    text_template_dlg.exec();
}

bool WordlibAddWidget::WriteTextEditContentToTmpFile(const QString& content)
{
    if(!Utils::MakeDir(Utils::GetTmpDir()))
        return false;
    QFile tmp_file(QString(Utils::GetTmpDir()).append("user_wordlib.tmp"));
    if(!tmp_file.open(QFile::WriteOnly))
        return false;
    QTextStream input_stream(&tmp_file);
    input_stream << content ;
    input_stream.flush();
    tmp_file.close();
    return true;
}

void WordlibAddWidget::SetOkButtonDisable()
{
    ui->import_push_button->setEnabled(false);
    ui->import_push_button->setStyleSheet(""
                                          "QPushButton { "
                                          " background-color : rgb(230, 230, 230);"
                                          " border : 1px solid;"
                                          " border-color : rgb(212, 212, 212);"
                                          " } ");
}

void WordlibAddWidget::SetOkButtonEnable()
{
    ui->import_push_button->setEnabled(true);
    ui->import_push_button->setStyleSheet(" QPushButton { "
                                          " border:1px solid rgb(77, 156, 248); "
                                          " color:rgb(77, 156, 248); "
                                          " background-color : rgb(255, 255, 255);"
                                          " border-radius : 1px; "
                                          " } "
                                          " QPushButton:hover { "
                                          " background-color : rgb(77, 156, 248); "
                                          " border : 0px solid rgb(77, 156, 248); "
                                          " color : rgb(255, 255, 255);"
                                          " border-radius : 1px; "
                                          " } "
                                          " QPushButton:pressed { "
                                          " background-color : rgb(77, 156, 248); "
                                          " border : 0px solid rgb(77, 156, 248); "
                                          " color : rgb(255, 255, 255);"
                                          " bord);"
                                          );
}

void WordlibAddWidget::OnTextChanged()
{
    QString content;
    content = ui->textEdit->document()->toPlainText();
    content.remove("\n");
    content.remove(QRegExp("\\s"));
    if(!content.isEmpty())
    {
        SetOkButtonEnable();
    }
    else
    {
        SetOkButtonDisable();
    }
}
