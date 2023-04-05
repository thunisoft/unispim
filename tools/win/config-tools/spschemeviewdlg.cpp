#include "spschemeviewdlg.h"
#include "ui_spschemeviewdlg.h"
#include "newschemedlg.h"
#include "config.h"
#include "customize_ui/spkeyboard.h"
#include "config.h"
#include <QStandardItemModel>
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
     ConfigItemStruct configInfo;
     if(Config::Instance()->GetConfigItemByJson("sp_file_name",configInfo))
     {
         int index = ui->scheme_combo->findText(configInfo.itemCurrentStrValue);
         ui->scheme_combo->setCurrentIndex(index);

         QString filepath = Config::GetProgramdataDir() + "ini\\" + configInfo.itemCurrentStrValue + ".ini";
         if(!QFile::exists(filepath))
         {
            filepath = Config::Instance()->GetUserSPIniFileDir() + configInfo.itemCurrentStrValue + ".ini";
         }

         ChangeTheKeyBoardView(filepath);
         ChangeTheDeleteBtnState(configInfo.itemCurrentStrValue);
     }
}

void SPSchemeViewDlg::InitWidget()
{
    this->setAttribute(Qt::WA_TranslucentBackground,true);
    ui->current_scheme_label->setProperty("type","h2");
    QList<QPushButton*> btn_list = this->findChildren<QPushButton*>();
    for(int index=0; index<btn_list.size(); ++index)
    {
        btn_list.at(index)->setCursor(Qt::PointingHandCursor);
        if(btn_list.at(index)->objectName() == "close_btn")
        {
            continue;
        }
        else if(btn_list.at(index)->objectName() == "reset_scheme_btn")
        {
            btn_list.at(index)->setProperty("type","puretextbtn");
            continue;
        }
        else if(btn_list.at(index)->objectName() == "confirm_btn")
        {
            btn_list.at(index)->setProperty("type","normalChecked");
            continue;
        }

        btn_list.at(index)->setProperty("type","normal");
    }
    this->setProperty("type","borderwidget");
    ui->scheme_title_widget->setProperty("type","window_title_widget");
    m_new_scheme_Dlg = new NewSchemeDlg(this);

    QList<QString> planNameList = Config::Instance()->GetSPSchemeList();

    QStandardItemModel* model = new QStandardItemModel();
    for(int index=0; index<planNameList.size(); ++index)
    {
        QStandardItem* item = new QStandardItem(planNameList.at(index));
        item->setToolTip(planNameList.at(index));
        model->appendRow(item);
    }

    ui->scheme_combo->setModel(model);
    ui->scheme_combo->setView(new QListView);
    ui->scheme_combo->installEventFilter(this);
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

void SPSchemeViewDlg::ChangeTheDeleteBtnState(QString sp_file_name)
{
    QList<QString> system_scheme_list = Config::Instance()->GetDefaultSpSchemeList();
    ui->delete_scheme_btn->setEnabled(!system_scheme_list.contains(sp_file_name));

}

void SPSchemeViewDlg::RefreshTheSchemeList()
{
    QList<QString> planNameList = Config::Instance()->GetSPSchemeList();
    ui->scheme_combo->clear();
    QStringList filenameList;
    for(int index=0; index<planNameList.size(); ++index)
    {
        filenameList << planNameList.at(index);
    }
    ui->scheme_combo->addItems(filenameList);
}

void SPSchemeViewDlg::SlotSubmitTheDlg()
{
    ConfigItemStruct configInfo;
    configInfo.itemType = "string";
    configInfo.itemName = "sp_file_name";
    configInfo.itemCurrentStrValue = ui->scheme_combo->currentText();
    Config::Instance()->SetConfigItemByJson("sp_file_name",configInfo);
    this->close();
}

void SPSchemeViewDlg::SlotDeleteSPScheme()
{
    QString current_name = ui->scheme_combo->currentText();
    //Config::Instance()->AddOrDeleteTheSpScheme(current_name,false);
    QString filepath = Config::Instance()->GetUserSPIniFileDir()+ current_name + ".ini";
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

    QString schemName = schemeCombo->currentText();
    QString filepath = Config::GetProgramdataDir() + "ini\\" + schemName + ".ini";
    if(!QFile::exists(filepath))
    {
        filepath = Config::Instance()->GetUserSPIniFileDir() + schemName + ".ini";
    }
    ChangeTheKeyBoardView(filepath);
    ChangeTheDeleteBtnState(schemName);
}

void SPSchemeViewDlg::SlotResetThescheme()
{
    int index = ui->scheme_combo->findText("华宇双拼");
    ui->scheme_combo->setCurrentIndex(index);
    ui->delete_scheme_btn->setEnabled(false);
}

bool SPSchemeViewDlg::eventFilter(QObject *obj, QEvent *event)
{
    if(obj->metaObject()->className() == QStringLiteral("QComboBox"))
    {
        if (event->type() == QEvent::Wheel)//鼠标滚轮事件
        {
            return true;//禁用下拉框的滚轮改变项的功能
        }
    }
    return false;
}
