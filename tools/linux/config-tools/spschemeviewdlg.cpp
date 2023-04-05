#include "spschemeviewdlg.h"
#include "ui_spschemeviewdlg.h"
#include "../public/configmanager.h"
#include "customize_ui/spkeyboard.h"
#include "customize_ui/generalstyledefiner.h"
#include <QStandardItemModel>
#include "ui_spschemeviewdlg.h"
#include "newschemedlg.h"
#include "msgboxutils.h"
#include "editspfiledlg.h"
#include "wordlibpage/myutils.h"
#include <QListView>
#include <QFileInfo>

SPSchemeViewDlg::SPSchemeViewDlg(QWidget *parent) :
    CustomizeQWidget(parent),
    ui(new Ui::SPSchemeViewDlg)
{
    ui->setupUi(this);
    InitWidget();
    ConnectSignalToSlot();
    LoadTheConfig();
}

SPSchemeViewDlg::~SPSchemeViewDlg()
{
    delete ui;
}

void SPSchemeViewDlg::LoadTheConfig()
{
    //加载双拼的配置和应用
    QString sp_scheme_name;
    ConfigManager::Instance()->GetStrConfigItem("sp_file_name",sp_scheme_name);
    QString sp_scheme_name_raw = QByteArray::fromBase64(sp_scheme_name.toUtf8());

    int index = ui->scheme_combo->findText(sp_scheme_name_raw);
    ui->scheme_combo->setCurrentIndex(index);

     QString filepath = ConfigManager::Instance()->GetSpSchemeDir() + sp_scheme_name_raw + ".ini";
     ChangeTheKeyBoardView(filepath);
     ChangeTheDeleteBtnState(sp_scheme_name_raw);
}

void SPSchemeViewDlg::InitWidget()
{
    this->setProperty("type","spViewDlg");
    QFile file(":/feedback/basicstyle.qss");
    if(file.open(QIODevice::ReadOnly))
    {
        this->setStyleSheet(file.readAll());
        file.close();
    }
    this->setStyleSheet(QString(""
                        "#SPSchemeViewDlg  { "
                        " %1 "
                        " } "
                        "").arg(GeneralStyleDefiner::BorderStyle()));

    //初始化对应的Combobx
    ui->scheme_combo->setFixedWidth(120);
    ui->scheme_combo->setFixedHeight(26);
    ui->scheme_combo->setView(new QListView());
    ui->scheme_combo->setAutoFillBackground(true);
    ui->scheme_combo->setMaxVisibleItems(10);

    ui->current_scheme_label->setProperty("type","h2");
    QList<QPushButton*> btn_list = this->findChildren<QPushButton*>();
    for(int index=0; index<btn_list.size(); ++index)
    {
        btn_list.at(index)->setFocusPolicy(Qt::NoFocus);
        if(btn_list.at(index)->objectName() == "close_btn")
        {
            btn_list.at(index)->setProperty("type","closeBtn");
            continue;
        }
        else if(btn_list.at(index)->objectName() == "reset_scheme_btn")
        {
            btn_list.at(index)->setProperty("type","puretextbtn");
            continue;
        }
        else if(btn_list.at(index)->objectName() == "confirm_btn")
        {
            btn_list.at(index)->setProperty("type","normal");
            continue;
        }

        btn_list.at(index)->setProperty("type","normal");
    }
    this->setProperty("type","borderwidget");
    ui->scheme_title_widget->setProperty("type","window_title_widget");
    m_new_scheme_Dlg = new NewSchemeDlg(this);

    QList<QString> planNameList = ConfigManager::Instance()->GetAllSPSchemeList();

    QStandardItemModel* model = new QStandardItemModel();
    for(int index=0; index<planNameList.size(); ++index)
    {
        QStandardItem* item = new QStandardItem(planNameList.at(index));
        item->setToolTip(planNameList.at(index));
        model->appendRow(item);
    }

    ui->scheme_combo->setModel(model);
    ui->scheme_combo->setView(new QListView);
    ui->scheme_combo->setProperty("type","expandingcombo");
    current_key_board = new SPKeyBoard(this);
    ui->key_board_layout->addWidget(current_key_board);
}

void SPSchemeViewDlg::ConnectSignalToSlot()
{
    connect(ui->cancal_btn, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->close_btn, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->confirm_btn, SIGNAL(clicked()), this, SLOT(SlotSubmitTheDlg()));
    connect(ui->delete_scheme_btn, SIGNAL(clicked()), this, SLOT(SlotDeleteSPScheme()));
    connect(ui->new_scheme_btn, SIGNAL(clicked()), this, SLOT(SlotCreateNewScheme()));
    connect(ui->scheme_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(SlotSchemeChanged()));
    connect(ui->reset_scheme_btn, SIGNAL(clicked()), this, SLOT(SlotResetThescheme()));
    connect(ui->edit_btn, SIGNAL(clicked()), this, SLOT(SlotEditSchema()));
}

void SPSchemeViewDlg::ChangeTheKeyBoardView(QString filePath)
{
    QString suffix_name = QFileInfo(filePath).suffix();
    if(QFile::exists(filePath) && (suffix_name == "ini"))
    {
        SpSchemeModel* currentModel = SpSchemeModel::CreateSpSchemeMoel(true,filePath);
        if(currentModel)
        {
            current_key_board->SetSpSchemeModel(*currentModel);
            delete currentModel;
            currentModel = NULL;
        }
    }
}

void SPSchemeViewDlg::CustomProjectChanged()
{
    QString schemeName = ui->scheme_combo->currentText();
    QString path = ConfigManager::Instance()->GetSpSchemeDir()+ schemeName + ".ini";
    ChangeTheKeyBoardView(path);
    ConfigManager::Instance()->SetStrConfigItem("sp_file_name",schemeName.toUtf8().toBase64());
}

void SPSchemeViewDlg::ChangeTheDeleteBtnState(QString sp_file_name)
{
    QList<QString> system_scheme_list = ConfigManager::Instance()->GetDefaultSPSchemeList();
    ui->delete_scheme_btn->setEnabled(!system_scheme_list.contains(sp_file_name));
    ui->edit_btn->setEnabled(!system_scheme_list.contains(sp_file_name));
}

void SPSchemeViewDlg::RefreshTheSchemeList()
{
    QList<QString> planNameList = ConfigManager::Instance()->GetAllSPSchemeList();
    ui->scheme_combo->clear();
    QStringList filenameList;
    for(int index=0; index<planNameList.size(); ++index)
    {
        filenameList << planNameList.at(index);
    }
    ui->scheme_combo->addItems(filenameList);

    QString current_scheme_name;
    ConfigManager::Instance()->GetStrConfigItem("sp_file_name",current_scheme_name);

    QString current_scheme_name_raw = QByteArray::fromBase64(current_scheme_name.toUtf8());
    int index = ui->scheme_combo->findText(current_scheme_name_raw);
    if(index != -1)
    {
        ui->scheme_combo->setCurrentIndex(index);
    }
}

void SPSchemeViewDlg::SlotSubmitTheDlg()
{
    QString sp_file_name = ui->scheme_combo->currentText();
    ConfigManager::Instance()->SetStrConfigItem("sp_file_name",sp_file_name.toUtf8().toBase64());
    this->close();
}

void SPSchemeViewDlg::SlotDeleteSPScheme()
{
    QString current_name = ui->scheme_combo->currentText();
    QString current_scheme_name;
    ConfigManager::Instance()->GetStrConfigItem("sp_file_name",current_scheme_name);
    current_scheme_name = QByteArray::fromBase64(current_scheme_name.toUtf8());

    if(current_scheme_name == current_name)
    {
        int returnValue = MsgBoxUtils::NoticeMsgBox("删除当前双拼方案\n删除后将使用华宇双拼?","系统提示",1,this);
        if(returnValue == 1)
        {
            ConfigManager::Instance()->AddOrDeleteTheSpScheme(current_name,false);
            QString filepath = ConfigManager::Instance()->GetSpSchemeDir() + current_name + ".ini";
            QFile::remove(filepath);
            ConfigManager::Instance()->SetStrConfigItem("sp_file_name",QByteArray("华宇双拼").toBase64());
            RefreshTheSchemeList();

        }
        return;
    }

    ConfigManager::Instance()->AddOrDeleteTheSpScheme(current_name,false);
    QString filepath = ConfigManager::Instance()->GetSpSchemeDir() + current_name + ".ini";
    QFile::remove(filepath);
    RefreshTheSchemeList();

}

void SPSchemeViewDlg::SlotCreateNewScheme()
{
    m_new_scheme_Dlg->ResetTheWidget();
    if(m_new_scheme_Dlg->exec() == QDialog::Accepted)
    {
        RefreshTheSchemeList();
        int index = ui->scheme_combo->findText(m_new_scheme_Dlg->GetCurrentSchemeName());
        if(index != -1)
        {
            ui->scheme_combo->setCurrentIndex(index);
        }
    }
}

void SPSchemeViewDlg::SlotSchemeChanged()
{
    QComboBox* schemeCombo = qobject_cast<QComboBox*>(sender());
    QString schemeName = schemeCombo->currentText();

    QStringList defaultSpList = ConfigManager::Instance()->GetDefaultSPSchemeList();
    QString filepath;
    if(defaultSpList.contains(schemeName))
    {
          filepath = TOOLS::PathUtils::GetSPFileDir() + schemeName + ".ini";
    }
    else
    {
         filepath = ConfigManager::Instance()->GetSpSchemeDir()+ schemeName + ".ini";
    }

    ChangeTheKeyBoardView(filepath);
    ChangeTheDeleteBtnState(schemeName);
}

void SPSchemeViewDlg::SlotResetThescheme()
{
    int index = ui->scheme_combo->findText("华宇双拼");
    ui->scheme_combo->setCurrentIndex(index);
    ui->delete_scheme_btn->setEnabled(false);
    ui->edit_btn->setEnabled(false);
}

void SPSchemeViewDlg::SlotEditSchema()
{
    QString schemeName = ui->scheme_combo->currentText();
    EditSPFileDlg * sp_editor = new EditSPFileDlg(schemeName, this);
    sp_editor->setAttribute(Qt::WA_DeleteOnClose);
    connect(sp_editor, SIGNAL(Saved()), this, SLOT(CustomProjectChanged()));
    sp_editor->exec();
}
