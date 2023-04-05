#include "wordlibcellwidget.h"
#include "ui_wordlibcellwidget.h"
#include "msgboxutils.h"
#include <QHBoxLayout>
#include <QCheckBox>
#include <QFile>
#include "../public/config.h"
#include "configbus.h"
#include "../public/utils.h"
#include "customize_ui/customizefont.h"
#include "wordlibshowwidget.h"
#include <QDebug>

WordlibCellWidget::WordlibCellWidget(const WORDLIB_INFO& wordlib_info, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WordlibCellWidget),
    m_wordlib_info(wordlib_info),
    m_checkBox(nullptr)
{
    Init();
}

WordlibCellWidget::~WordlibCellWidget()
{
    delete ui;
}

void WordlibCellWidget::Init()
{
    ui->setupUi(this);
    QLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setContentsMargins(7, 7, 7, 7);
    ui->delete_push_button->setFocusPolicy(Qt::NoFocus);
    ui->delete_push_button->setCursor(Qt::PointingHandCursor);

    ui->widget->setLayout(layout);

    ui->label->setProperty("type", "h2");

    ui->label->setText(m_wordlib_info.wordlib_name);


    QFontMetrics elidfont(ui->label->font());
    ui->label->setText(elidfont.elidedText(m_wordlib_info.wordlib_name,Qt::ElideRight,90));
    ui->label->setToolTip(m_wordlib_info.wordlib_name);
    this->setStyleSheet("background-color:rgb(255,255,255);border: none;");
}

void WordlibCellWidget::SetUpElements()
{
    if(!m_wordlib_info.is_cloud_wordlib || (!m_wordlib_info.is_local_file && m_wordlib_info.is_cloud_wordlib))
    {
         ui->delete_push_button->setVisible(false);
    }
    else
    {
        connect(ui->delete_push_button, SIGNAL(clicked(bool)), this, SLOT(OnDeleteButtonClicked()));
    }

    if(!m_wordlib_info.is_local_file)
    {
        AddDownloadButtonToWidget();
    }
    else
    {
        
        QCheckBox *check_box = new QCheckBox();

        ui->widget->layout()->addWidget(check_box);
        if(m_wordlib_info.is_selected)
        {
            check_box->setChecked(true);
        }
        connect(check_box, SIGNAL(stateChanged(int)), this, SLOT(OnCheckBoxStateChaged(int)));

        m_checkBox = check_box;
    }
}


void WordlibCellWidget::OnCheckBoxStateChaged(int state)
{
    if(Qt::Checked == state)
    {
        //MoveFileToSelectedDir();
        SelectWordlib(m_wordlib_info.file_name);
    }
    else
    {
        UnSelectWordlib(m_wordlib_info.file_name);
        //RemoveFileFromSelectedDir();
    }
}

void WordlibCellWidget::OnDownloadButtonClicked()
{
    emit DownloadWordlib(m_wordlib_info.file_name);
}

void WordlibCellWidget::OnDeleteButtonClicked()
{
    if(!MsgBoxUtils::NoticeMsgBox("确定要删除吗？","系统提示",1, this))
        return;
    ui->delete_push_button->setVisible(false);
    DeleteFile();
    UnSelectWordlib(m_wordlib_info.file_name);
    ConfigBus::instance()->valueChanged("loadWordlib", ""); //通知引擎加载词库
    /*将checkbox去掉
      然后将“云型”下载放上去*/
    QLayoutItem *layout_item = ui->widget->layout()->takeAt(0);
    ui->widget->layout()->removeItem(layout_item);

    AddDownloadButtonToWidget();
}

void WordlibCellWidget::DeleteFile()
{    
    QFile local_cloud_dir(Config::wordLibDirPath(WLDPL_CONFIG).append("cloud/").append(m_wordlib_info.file_name));
    local_cloud_dir.remove();
}

void WordlibCellWidget::OnDownloadDone(const QString& file_name)
{
    if(file_name != m_wordlib_info.file_name)
    {
        Utils::WriteLogToFile(QString("%1 not equal %2").arg(file_name).arg(m_wordlib_info.file_name));
        return;
    }
    MoveFileToCloudDir();    
    SelectWordlib(m_wordlib_info.file_name);
    ui->delete_push_button->setVisible(true);
    /*去除“云型”下载按钮
      然后将checkbox放上去*/
    QLayoutItem *layout_item = ui->widget->layout()->takeAt(0);
    ui->widget->layout()->removeItem(layout_item);

    disconnect(ui->delete_push_button, SIGNAL(clicked(bool)), 0, 0);
    connect(ui->delete_push_button, SIGNAL(clicked(bool)), this, SLOT(OnDeleteButtonClicked()));

    QCheckBox *check_box = new QCheckBox(this);
    ui->widget->layout()->addWidget(check_box);
    check_box->setChecked(true);
    connect(check_box, SIGNAL(stateChanged(int)), this, SLOT(OnCheckBoxStateChaged(int)));
}

void WordlibCellWidget::AddDownloadButtonToWidget()
{
    QPushButton *push_button = new QPushButton(this);
    push_button->setCursor(Qt::PointingHandCursor);
    push_button->setToolTip("下载专业词库");
    push_button->setFocusPolicy(Qt::NoFocus);
    push_button->setStyleSheet(" QPushButton {"
                               " border : 0px solid;"
                               " image : url(:/image/cloud.png);"
                               " } ");
    ui->widget->layout()->addWidget(push_button);
    connect(push_button, SIGNAL(clicked(bool)), this, SLOT(OnDownloadButtonClicked()));
}

void WordlibCellWidget::MoveFileToCloudDir()
{

    QFile file(Utils::GetTmpDir().append(m_wordlib_info.file_name));
    Utils::MakeDir(Utils::GetConfigDirPath().append("wordlib/cloud/"));
    file.copy(Utils::GetConfigDirPath().append("wordlib/cloud/").append(m_wordlib_info.file_name));
    Utils::WriteLogToFile("copy wordlib to cloud dir");
}

void WordlibCellWidget::SelectWordlib(const QString& name)
{
    QString selected_wordlibs = Config::Instance()->SelectedWordlibs();
    QStringList wordlibs = selected_wordlibs.split(",");
    //check if has repeat item
    QSet<QString> wordlib_set = wordlibs.toSet();
    int wordlib_set_size = wordlib_set.size();
    int string_size = wordlibs.size();
    if(wordlib_set_size != string_size)
    {
        QStringList newList = QStringList::fromSet(wordlib_set);
        wordlibs = newList;
    }

    if(!wordlibs.contains(name))
    {
        wordlibs.push_back(name);
        Config::Instance()->SetSelectedWordlibs(wordlibs.join(","));
    }
}

void WordlibCellWidget::UnSelectWordlib(const QString& name)
{
    QString selected_wordlibs = Config::Instance()->SelectedWordlibs();
    QStringList wordlibs = selected_wordlibs.split(",");
    if(wordlibs.contains(name))
    {
        wordlibs.removeOne(name);
        Config::Instance()->SetSelectedWordlibs(wordlibs.join(","));
    }
}

void WordlibCellWidget::check()
{
    if (checkable())
    {
        m_checkBox->setChecked(true);
    }
}

void WordlibCellWidget::uncheck()
{
    if (checkable())
    {
        m_checkBox->setChecked(false);
    }
}
