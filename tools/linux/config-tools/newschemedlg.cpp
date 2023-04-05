#include "newschemedlg.h"
#include "ui_newschemedlg.h"
#include "../public/configmanager.h"
#include "wordlibpage/myutils.h"
#include <QListView>
#include <QStandardItemModel>
#include <QScrollBar>
#include "customize_ui/spkeyboard.h"
#include "customize_ui/generalstyledefiner.h"
#include "msgboxutils.h"
#include "editspfiledlg.h"

NewSchemeDlg::NewSchemeDlg(QWidget *parent) :
    CustomizeQWidget(parent),
    ui(new Ui::NewSchemeDlg),
    m_edit_sp_file(nullptr)
{
    ui->setupUi(this);
    InitWidget();
    ConnectSignalToSlot();
}

NewSchemeDlg::~NewSchemeDlg()
{
    delete ui;
}

void NewSchemeDlg::ResetTheWidget()
{
    QFile file(":/feedback/basicstyle.qss");
    if(file.open(QIODevice::ReadOnly))
    {
        this->setStyleSheet(file.readAll());
        file.close();
    }

    this->setStyleSheet(QString(""
                        "#NewSchemeDlg{ "
                        " %1 "
                        " } "
                        "").arg(GeneralStyleDefiner::BorderStyle()));

    ui->new_scheme_name->setText("新建双拼方案");
    ui->key_combo->setCurrentIndex(-1);
    ui->syllable_combo->setCurrentIndex(-1);
    current_key_board->SetSpSchemeModel(*SpSchemeModel::CreateSpSchemeMoel(false));

    ui->basic_scheme_combo->clear();
    QList<QString> planNameList = ConfigManager::Instance()->GetAllSPSchemeList();
    QStringList planNameStringList;
    for(int index=0; index<planNameList.size(); ++index)
    {
        planNameStringList << planNameList.at(index);
    }
    ui->basic_scheme_combo->addItems(planNameStringList);
    ui->basic_scheme_combo->setCurrentIndex(-1);
}

void NewSchemeDlg::InitWidget()
{
    this->setAttribute(Qt::WA_TranslucentBackground,true);
    QList<QPushButton*> btn_list = this->findChildren<QPushButton*>();
    for(int index=0; index<btn_list.size(); ++index)
    {
        btn_list.at(index)->setFocusPolicy(Qt::NoFocus);
        if(btn_list.at(index)->objectName() == "close_btn")
        {
            btn_list.at(index)->setProperty("type","closeBtn");
            continue;
        }
        btn_list.at(index)->setProperty("type","normal");
    }

    QList<QLabel*> label_list = this->findChildren<QLabel*>();
    for(int index=0; index<label_list.size(); ++index)
    {
        if(label_list.at(index)->objectName() == "logo_title_label")
        {
            continue;
        }
        label_list.at(index)->setProperty("type","h2");
    }

    this->setProperty("type","borderwidget");
    ui->new_scheme_title->setProperty("type","window_title_widget");

    QList<QString> planNameList = ConfigManager::Instance()->GetDefaultSPSchemeList();

    QStandardItemModel* model = new QStandardItemModel();
    for(int index=0; index<planNameList.size(); ++index)
    {
        QStandardItem* item = new QStandardItem(planNameList.at(index));
        item->setToolTip(planNameList.at(index));
        model->appendRow(item);
    }

    ui->basic_scheme_combo->setModel(model);
    ui->basic_scheme_combo->setView(new QListView);
    m_currentSPModel = SpSchemeModel::CreateSpSchemeMoel(false);


    QStandardItemModel* syllable_model = new QStandardItemModel();
    for(int index=0; index<m_currentSPModel->m_all_syllable_vector.size(); ++index)
    {
        QStandardItem* item = new QStandardItem(m_currentSPModel->m_all_syllable_vector.at(index));
        syllable_model->appendRow(item);
    }
    ui->syllable_combo->setModel(syllable_model);
    ui->syllable_combo->setView(new QListView());

    QStringList key_str_list;
    for(int index=0; index<m_currentSPModel->m_all_key_vector.size(); ++index)
    {
        key_str_list << m_currentSPModel->m_all_key_vector.at(index);
    }
    ui->key_combo->addItems(key_str_list);
    ui->key_combo->setView(new QListView());

    ui->new_scheme_name->setProperty("type","NormalLineEdit");
    ui->new_scheme_name->setText("新建双拼方案");

    current_key_board = new SPKeyBoard(this);
    ui->key_board_layout->addWidget(current_key_board);
    ResetTheWidget();

    //当前逻辑改变，不再新建时能够编辑双拼文件，先隐藏按钮
    ui->edit_file_button->setVisible(false);
}

void NewSchemeDlg::ConnectSignalToSlot()
{
    connect(ui->close_btn, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->cancel_btn, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->save_btn, SIGNAL(clicked()), this, SLOT(SlotConfirmScheme()));
    connect(ui->basic_scheme_combo,SIGNAL(currentIndexChanged(int)), this, SLOT(SlotChangeBasicScheme(int)));
    connect(ui->syllable_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(SlotGetAvailableKey(int)));
    connect(ui->key_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(SlotCurrentKeyChanged(int)));
    connect(ui->edit_file_button, SIGNAL(clicked(bool)), this, SLOT(SlotEditFile()));
}

void NewSchemeDlg::SlotChangeBasicScheme(int index)
{
    QString schemeName = ui->basic_scheme_combo->itemText(index);
    QStringList defaultSpList = ConfigManager::Instance()->GetDefaultSPSchemeList();

    QString filepath;
    if(defaultSpList.contains(schemeName))
    {
          filepath = TOOLS::PathUtils::GetSPFileDir() + schemeName + ".ini";
    }
    else
    {
        filepath = ConfigManager::Instance()->GetUserSPFileDir() + schemeName + ".ini";
    }

    if(m_currentSPModel)
    {
        delete  m_currentSPModel;
        m_currentSPModel = NULL;
    }
    m_currentSPModel = SpSchemeModel::CreateSpSchemeMoel(true,filepath);
    current_key_board->SetSpSchemeModel(*m_currentSPModel);

    SlotGetAvailableKey(ui->syllable_combo->currentIndex());
}

void NewSchemeDlg::SlotGetAvailableKey(int index)
{
    QString syllableStr = ui->syllable_combo->itemText(index);

    QMap<QString, Key_Syllable_Table_Stru> current_map = m_currentSPModel->GetKeySyllableMap();
    QList<QString> keyList = current_map.keys();
    QString selectedKey;
    for(QString keyIndex : keyList)
    {
       Key_Syllable_Table_Stru currentValue = current_map[keyIndex];
       if((currentValue.initial_syllable == syllableStr) || (currentValue.final_syllable_Set.contains(syllableStr)))
       {

           selectedKey = keyIndex;

       }
    }

    QVector<QString> available_key_vector = m_currentSPModel->GetSyllableAvailableKeys(syllableStr);
    QStringList itemStrList;
    for(QString name:available_key_vector)
    {
        itemStrList << name;
    }
    std::sort(itemStrList.begin(), itemStrList.end());
    ui->key_combo->clear();
    ui->key_combo->addItems(itemStrList);


    if(itemStrList.contains(selectedKey))
    {
        ui->key_combo->setCurrentText(selectedKey);
    }
}

void NewSchemeDlg::SlotCurrentKeyChanged(int index)
{
    Q_UNUSED(index)
    //设置对应音节和按键的映射表
   QString currentSyllable = ui->syllable_combo->currentText();
   QString currentKey = ui->key_combo->currentText();
   m_currentSPModel->SetSyllableAndKeypair(currentSyllable,currentKey);
   current_key_board->SetSpSchemeModel(*m_currentSPModel);
}

void NewSchemeDlg::SlotConfirmScheme()
{
    if(!IsSpNameLegal())
        return;
    QString fileName = ConfigManager::Instance()->GetUserSPFileDir()+ ui->new_scheme_name->text() + ".ini";
    int returnCode = m_currentSPModel->SaveSPConfigToFile(fileName);
    if(returnCode == 0)
    {
        ConfigManager::Instance()->AddOrDeleteTheSpScheme(ui->new_scheme_name->text(),true);
        MsgBoxUtils::NoticeMsgBox("方案保存成功","系统提示",0,this);
        m_current_scheme_name = ui->new_scheme_name->text();
        QDialog::accept();
    }
    else if(returnCode == 2)
    {
        MsgBoxUtils::NoticeMsgBox("还有未配置的音节","系统提示",0,this);
        return;
    }
    else
    {
        MsgBoxUtils::NoticeMsgBox("方案保存失败","系统提示",0,this);
        return;
    }
}

void NewSchemeDlg::SlotEditFile()
{
    if(!IsSpNameLegal())
        return;
    if(!m_edit_sp_file)
    {
        m_edit_sp_file = new EditSPFileDlg(this);
        connect(m_edit_sp_file, SIGNAL(Saved()), this, SLOT(SlotSavedFile()));
    }

    m_edit_sp_file->SetNameAndContent(ui->new_scheme_name->text(), m_currentSPModel);
    m_edit_sp_file->exec();
}

bool NewSchemeDlg::IsSpNameLegal()
{
    if(ui->new_scheme_name->text().isEmpty())
    {
        MsgBoxUtils::NoticeMsgBox("方案名称不能为空","系统提示",0,this);
        return false;
    }
    QString fileName = ConfigManager::Instance()->GetUserSPFileDir()+ ui->new_scheme_name->text() + ".ini";
    QStringList fileNameList = ConfigManager::Instance()->GetDefaultSPSchemeList();

    if(fileNameList.contains(ui->new_scheme_name->text()))
    {
        MsgBoxUtils::NoticeMsgBox("和自带方案名称冲突","系统提示",0,this);
        return false;
    }
    if(QFile::exists(fileName))
    {
         MsgBoxUtils::NoticeMsgBox("方案已经存在","系统提示",0,this);
        return false;
    }
    return true;
}

void NewSchemeDlg::SlotSavedFile()
{
    ConfigManager::Instance()->AddOrDeleteTheSpScheme(ui->new_scheme_name->text(),true);
    m_current_scheme_name = ui->new_scheme_name->text();
    QDialog::accept();
}
