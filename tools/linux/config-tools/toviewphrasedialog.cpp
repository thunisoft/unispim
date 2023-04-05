#include "toviewphrasedialog.h"
#include "ui_toviewphrasedialog.h"
#include "phraseoptionwidget.h"
#include "addoreditphrasedialog.h"
#include "../public/utils.h"
#include <QDir>
#include <QTextStream>
#include "fileviewerdialog.h"
#include "../public/configmanager.h"
#include "configbus.h"
#include "phrasemodel.h"
#include "editphrasefiledialog.h"
#include "customize_ui/generalstyledefiner.h"
#include "customize_ui/customizefont.h"

#include <QListView>
#include <QScrollBar>
#include <QTextCodec>

ToViewPhraseDialog::ToViewPhraseDialog(QWidget *parent) :
    m_phrase_modified(false),
    CustomizeQWidget(parent),
    ui(new Ui::ToViewPhraseDialog)
{
    Init();
}

ToViewPhraseDialog::~ToViewPhraseDialog()
{
    delete ui;
}

void ToViewPhraseDialog::Init()
{
    SetUpCustomizeUI();
    PhraseModel::Instance()->LoadPhraseFromFile();
    FillDataIntoTable();
    RegisterSlots();
}

void ToViewPhraseDialog::SetUpCustomizeUI()
{
    ui->setupUi(this);

    ui->add_push_button->setProperty("type","normalBtn");
    ui->edit_phrase_push_button->setProperty("type","normalBtn");
    ui->apply_push_button->setProperty("type","normalBtn");
    ui->cancel_push_button->setProperty("type","normalBtn");
    ui->close_push_button->setProperty("type","closeBtn");


    ui->comboBox->setFixedWidth(120);
    ui->comboBox->setFixedHeight(26);
    ui->comboBox->setView(new QListView());
    ui->comboBox->setAutoFillBackground(true);
    ui->comboBox->setMaxVisibleItems(10);

    ui->to_view_sys_phrase_button->setProperty("type","noborderbtn");
    ui->phrase_show_eara->setStyleSheet(" QWidget#phrase_show_eara{ border : 1px solid rgb(188, 188, 188) };");

    ui->table_widget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
    ui->table_widget->horizontalHeader()->setVisible(false);
    ui->table_widget->verticalHeader()->setHidden(true);
    ui->table_widget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->table_widget->setSelectionMode(QAbstractItemView::NoSelection);

    ui->table_widget->setAlternatingRowColors(false);
    ui->table_widget->setPalette(QColor(255, 255, 255));
    ui->table_widget->setFocusPolicy(Qt::NoFocus);
    ui->table_widget->setShowGrid(false);


    ui->table_widget->clear();
    ui->table_widget->setColumnCount(4); //缩写 内容 位置 操作
    ui->table_widget->setColumnWidth(0, 131);
    ui->table_widget->setColumnWidth(1, 200);
    ui->table_widget->setColumnWidth(2, 45);
    ui->table_widget->setColumnWidth(3, 152);
    ui->table_widget->setStyleSheet("QTableView{"
                                         "border: none;"
                                         "}"
                                         "QTableView::item"
                                         "{"
                                         "border-left: 0px solid ;"
                                         "border-right: 1px solid rgb(188, 188, 188);"
                                         "border-top: 0px solid rgb(188, 188, 188);"
                                         "border-bottom: 1px solid rgb(188, 188, 188);"
                                         "background-color:rgb(255,255,255);"
                                         "}"
                                         " QScrollArea {"
                                         " border: 1px solid rgb(188, 188, 188);"
                                         " background-color: transparent;"
                                         "}");

    ui->table_widget->setHorizontalHeaderLabels(QStringList() << "1" << "2" << "3" << "4");
    ui->table_widget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->table_widget->verticalScrollBar()->setStyleSheet("QScrollArea:vertical{"
                                                         //"border: 1px solid rgb(188, 188, 188);"
                                                         "background-color: transparent;"
                                                         "}"
                                                         "QScrollArea QScrollBar:vertical {"
                                                         "background : rgba(0, 0, 0, 0%);"
                                                         "width : 5px;"
                                                         "margin : 0px, 0px, 0px, 0px;"
                                                         "}"
                                                         "QScrollArea QScrollBar::handle:vertical {"
                                                         "width : 5px;"
                                                         "background:rgb(226, 226, 226);"
                                                         "border-radius : 2px;"
                                                         "}"
                                                         "QScrollArea QScrollBar::handle:hover {"
                                                         "width : 5px;"
                                                         "background:rgb(128, 128, 128);"
                                                         "border-radius : 2px;"
                                                         "}"
                                                         "QScrollArea QScrollBar::add-line:vertical {"
                                                         "border:none;"
                                                         "}"
                                                         "QScrollArea QScrollBar::sub-line:vertical {"
                                                         "border:none;"
                                                         "}");
    ui->table_widget->viewport()->setStyleSheet(" background-color:transparent;");

    int key_value;
    if(ConfigManager::Instance()->GetIntConfigItem("use_sys_phrase",key_value))
    {
        ui->open_phrase_check_box->setChecked(key_value);
    }

    SetStyleSheet();
    SetDefaultPhrasePosition();

}

void ToViewPhraseDialog::RegisterSlots()
{
    connect(ui->close_push_button, SIGNAL(clicked(bool)), this, SLOT(OnCloseButtonClicked()));
    connect(ui->add_push_button, SIGNAL(clicked()), this, SLOT(OnAddPhraseButtonClicked()));
    connect(ui->edit_phrase_push_button, SIGNAL(clicked(bool)), this, SLOT(OnEditePhraseFileButtonClicked()));
    connect(ui->to_view_sys_phrase_button, SIGNAL(clicked(bool)), this, SLOT(OnViewSysPhraseButtonClicked()));
    connect(ui->apply_push_button, SIGNAL(clicked(bool)), this, SLOT(OnApplyButtonClicked()));
    connect(ui->cancel_push_button, SIGNAL(clicked(bool)), this, SLOT(OnCancelButtonClicked()));
}

void ToViewPhraseDialog::FillDataIntoTable()
{
    ui->table_widget->clearContents();
    ui->table_widget->setRowCount(0);
    int row_index = 0;
    QMap<int, PHRASE>* phrases = PhraseModel::Instance()->GetParsedPhrase();
    QMap<int, PHRASE>::iterator it = phrases->begin();
    for(; it!= phrases->end(); it++)
    {
        if(it.value().state == REMOVED)
            continue;
        QTableWidgetItem *item = new QTableWidgetItem(((PHRASE)it.value()).abbr);
        item->setTextAlignment(Qt::AlignCenter);
        Qt::ItemFlags flags = item->flags();
        flags &= ~Qt::ItemIsEditable;
        item->setFlags(flags);
        ui->table_widget->insertRow(row_index);
        ui->table_widget->setItem(row_index, 0, item);

        QString content = ((PHRASE)it.value()).content;
        if(content.length() > 9)
        {
            content = content.left(9);
            content.append("...");
        }
        item = new QTableWidgetItem(content);
        item->setToolTip(((PHRASE)it.value()).content);
        item->setTextAlignment(Qt::AlignCenter);
        flags = item->flags();
        flags &= ~Qt::ItemIsEditable;
        item->setFlags(flags);
        ui->table_widget->setItem(row_index, 1, item);

        item = new QTableWidgetItem(((PHRASE)it.value()).position > 0 ? QString::number(((PHRASE)it.value()).position) : "-");
        item->setTextAlignment(Qt::AlignCenter);
        flags = item->flags();
        flags &= ~Qt::ItemIsEditable;
        item->setFlags(flags);
        ui->table_widget->setItem(row_index, 2, item);

        PhraseOptionWidget *option_widget = new PhraseOptionWidget(it.key(), this);
        ui->table_widget->setCellWidget(row_index, 3, option_widget);

        connect(option_widget, SIGNAL(OptionPhraseButtonClicked(int, int)), this, SLOT(OnOptionPhraseClicked(int, int)));
        row_index++;
    }

}

void ToViewPhraseDialog::OnOptionPhraseClicked(int button_type, int phrase_index)
{
    if(button_type == EDIT_PHRASE)
    {
        PHRASE phrase = PhraseModel::Instance()->GetParsedPhrase()->find(phrase_index).value();
        AddOrEditPhraseDialog *dialog = new AddOrEditPhraseDialog(EDIT_PHRASE_DIALOG,phrase_index, &phrase, this);
        connect(dialog, SIGNAL(SignalToSavePhrase(PHRASE, int)), this, SLOT(OnPhraseAddedOrChanged(PHRASE, int)));
        dialog->exec();
    }
    else //button_type == DELETE_PHRASE
    {
        /*将操作记录暂存，待点击应用后执行操作记录到文件*/

        (*PhraseModel::Instance()->GetParsedPhrase())[phrase_index].state = REMOVED;
         PhraseModel::Instance()->SavePhraseToRealFile();
         PhraseModel::Instance()->LoadPhraseFromFile();
        //刷新界面
        SetPhraseIsModified(true);
        FillDataIntoTable();
    }
}

void ToViewPhraseDialog::OnAddPhraseButtonClicked()
{
    AddOrEditPhraseDialog *dialog = new AddOrEditPhraseDialog(ADD_PHRASE_DIALOG, -1, NULL, this);
    connect(dialog, SIGNAL(SignalToSavePhrase(PHRASE, int)), this, SLOT(OnPhraseAddedOrChanged(PHRASE, int)));
    dialog->exec();
}

void ToViewPhraseDialog::OnPhraseAddedOrChanged(PHRASE phrase, int phrase_index)
{
    if(phrase_index == -1)
    {
        int index = PhraseModel::Instance()->GetParsedPhrase()->isEmpty() ? 0 : PhraseModel::Instance()->GetParsedPhrase()->lastKey() + 1;
        PhraseModel::Instance()->GetParsedPhrase()->insert(index, phrase);
    }
    else
    {
        (*PhraseModel::Instance()->GetParsedPhrase())[phrase_index] = phrase;
    }
    FillDataIntoTable();
    SetPhraseIsModified(true);
}

void ToViewPhraseDialog::OnEditePhraseFileButtonClicked()
{
    //先保存一下当前页面
    PhraseModel::Instance()->SavePhraseToRealFile();
    PhraseModel::Instance()->SavePhraseToFile();
    EditPhraseFileDialog *dialog = new EditPhraseFileDialog(this);
    dialog->exec();
    //刷新当前页面
    PhraseModel::Instance()->LoadPhraseFromFile();
    FillDataIntoTable();
    //将过滤出来的合法的短语保存到RealFile
    PhraseModel::Instance()->SavePhraseToRealFile();
}

void ToViewPhraseDialog::OnViewSysPhraseButtonClicked()
{
    FileViewerDialog* dialog = new FileViewerDialog(QString::fromLocal8Bit("系统短语"), Utils::GetHuayuPYInstallDirPath().append("phrase/系统短语库.ini"), false, this);
    dialog->exec();
}

void ToViewPhraseDialog::OnApplyButtonClicked()
{
    int toggle = 0;
    if(Qt::Checked == ui->open_phrase_check_box->checkState())
        toggle = 1;
    ConfigManager::Instance()->SetIntConfigItem("use_sys_phrase",toggle);
    ConfigBus::instance()->valueChanged("useSysPhrase", toggle);
    SaveDefaultPhrasePoistion();
    this->close();
}

void ToViewPhraseDialog::SetPhraseIsModified(bool toggle)
{
    m_phrase_modified = toggle;
}

bool ToViewPhraseDialog::GetPhraseIsModified()
{
    return m_phrase_modified;
}

void ToViewPhraseDialog::closeEvent(QCloseEvent *event)
{
    if(GetPhraseIsModified())
    {
        PhraseModel::Instance()->SavePhraseToRealFile();
        //PhraseModel::Instance()->SavePhraseToFile();
    }
    ConfigBus::instance()->valueChanged("loadSpw", "");
    event->accept();
}

void ToViewPhraseDialog::SetStyleSheet()
{
    this->setStyleSheet(QString(""
                        "#ToViewPhraseDialog  { "
                        " %1 "
                        " } "
                        "").arg(GeneralStyleDefiner::BorderStyle()));

    //ui->title_label->setFont(CustomizeFont::GetH1Font());
    ui->title_label->setProperty("type", "h1");
}

void ToViewPhraseDialog::SetDefaultPhrasePosition()
{
    QStringList positions;
    positions << "  1" << "  2" << "  3"
              << "  4" << "  5" << "  6"
              << "  7" << "  8" << "  9";
    for(int index=0; index<positions.size(); ++index)
    {
        ui->comboBox->addItem(positions.at(index),(index+1));
    }

    int key_value;
    if(ConfigManager::Instance()->GetIntConfigItem("customphrasedisplaypos",key_value))
    {
        ui->comboBox->setCurrentIndex(key_value - 1);
    }
}

void ToViewPhraseDialog::SaveDefaultPhrasePoistion()
{
    int current_value = ui->comboBox->currentData().toInt();
    ConfigBus::instance()->valueChanged("customPhraseDisplayPos", current_value);
    ConfigManager::Instance()->SetIntConfigItem("customphrasedisplaypos",current_value);

}

void ToViewPhraseDialog::OnCloseButtonClicked()
{
    close();
}

void ToViewPhraseDialog::OnCancelButtonClicked()
{
    close();
}
