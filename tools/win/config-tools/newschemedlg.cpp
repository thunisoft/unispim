#include "newschemedlg.h"
#include "ui_newschemedlg.h"
#include "config.h"
#include "utils.h"
#include <QListView>
#include <QStandardItemModel>
#include "customize_ui/spkeyboard.h"

NewSchemeDlg::NewSchemeDlg(QWidget *parent) :
    CustomizeQWidget(parent),
    ui(new Ui::NewSchemeDlg)
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
    ui->new_scheme_name->setText("新建双拼方案");
    ui->key_combo->setCurrentIndex(-1);
    ui->syllable_combo->setCurrentIndex(-1);

    if(m_currentSPModel != nullptr)
    {
        delete m_currentSPModel;
        m_currentSPModel = nullptr;
    }

    m_currentSPModel = SpSchemeModel::CreateSpSchemeMoel(false);
    current_key_board->SetSpSchemeModel(*m_currentSPModel);

    ui->basic_scheme_combo->blockSignals(true);
    ui->basic_scheme_combo->clear();
    QList<QString> planNameList = Config::Instance()->GetSPSchemeList();
    QStringList planNameStringList;
    for(int index=0; index<planNameList.size(); ++index)
    {
        planNameStringList << planNameList.at(index);
    }
    ui->basic_scheme_combo->addItems(planNameStringList);
    ui->basic_scheme_combo->setCurrentIndex(0);
    ui->basic_scheme_combo->blockSignals(false);
    SlotChangeBasicScheme(0);
}

void NewSchemeDlg::InitWidget()
{
    this->setAttribute(Qt::WA_TranslucentBackground,true);
    QList<QPushButton*> btn_list = this->findChildren<QPushButton*>();
    for(int index=0; index<btn_list.size(); ++index)
    {
        btn_list.at(index)->setCursor(Qt::PointingHandCursor);
        if(btn_list.at(index)->objectName() == "close_btn")
        {
            continue;
        }
        else if(btn_list.at(index)->objectName() == "save_btn")
        {
            btn_list.at(index)->setProperty("type","normalChecked");
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

    QList<QString> planNameList = Config::Instance()->GetSPSchemeList();

    QStandardItemModel* model = new QStandardItemModel();
    for(int index=0; index<planNameList.size(); ++index)
    {
        QStandardItem* item = new QStandardItem(planNameList.at(index));
        item->setToolTip(planNameList.at(index));
        model->appendRow(item);
    }

    ui->basic_scheme_combo->setModel(model);
    ui->basic_scheme_combo->installEventFilter(this);
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
    ui->syllable_combo->installEventFilter(this);

    QStringList key_str_list;
    for(int index=0; index<m_currentSPModel->m_all_key_vector.size(); ++index)
    {
        key_str_list << m_currentSPModel->m_all_key_vector.at(index);
    }
    ui->key_combo->addItems(key_str_list);
    ui->key_combo->setView(new QListView());
    ui->key_combo->installEventFilter(this);

    ui->new_scheme_name->setProperty("type","NormalLineEdit");
    ui->new_scheme_name->setText("新建双拼方案");

    current_key_board = new SPKeyBoard(this);
    ui->key_board_layout->addWidget(current_key_board);
    ResetTheWidget();

}

void NewSchemeDlg::ConnectSignalToSlot()
{
    connect(ui->close_btn, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->cancel_btn, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->save_btn, SIGNAL(clicked()), this, SLOT(SlotConfirmScheme()));
    connect(ui->basic_scheme_combo,SIGNAL(currentIndexChanged(int)), this, SLOT(SlotChangeBasicScheme(int)));
    connect(ui->syllable_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(SlotGetAvailableKey(int)));
    connect(ui->key_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(SlotCurrentKeyChanged(int)));
}

void NewSchemeDlg::SlotChangeBasicScheme(int index)
{
    //先查找系统默认的双拼方案不存在的话再去找自定义的方案
    ui->syllable_combo->blockSignals(true);
    ui->key_combo->blockSignals(true);
    ui->syllable_combo->setCurrentIndex(-1);
    ui->key_combo->setCurrentIndex(-1);
    ui->syllable_combo->blockSignals(false);
    ui->key_combo->blockSignals(false);

    QString schemName = ui->basic_scheme_combo->itemText(index);
    QString filepath = Config::GetProgramdataDir() + "ini\\" + schemName + ".ini";
    if(!QFile::exists(filepath))
    {
        filepath = Config::Instance()->GetUserSPIniFileDir() + schemName + ".ini";
    }

    if(!QFile::exists(filepath))
    {
        //Utils::NoticeMsgBox("双拼方案不存在",this);
        Utils::WriteLogToFile(QString("%1 方案不存在").arg(filepath));
    }
    else {

        if(m_currentSPModel)
        {
            delete  m_currentSPModel;
            m_currentSPModel = NULL;
        }
        m_currentSPModel = SpSchemeModel::CreateSpSchemeMoel(true,filepath);
        current_key_board->SetSpSchemeModel(*m_currentSPModel);
    }


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
    std::sort(itemStrList.begin(),itemStrList.end());

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
    if(ui->new_scheme_name->text().isEmpty())
    {
        Utils::NoticeMsgBox("方案名称不能为空",this);
        return;
    }
    QString fileName = Config::Instance()->GetUserSPIniFileDir() + ui->new_scheme_name->text() + ".ini";
    if(QFile::exists(fileName))
    {
        Utils::NoticeMsgBox("方案已经存在",this);
        return;
    }
    int returnCode = m_currentSPModel->SaveSPConfigToFile(fileName);
    if(returnCode == 0)
    {
        //Config::Instance()->AddOrDeleteTheSpScheme(ui->new_scheme_name->text(),true);
        Utils::NoticeMsgBox("方案保存成功",this);
        m_current_scheme_name = ui->new_scheme_name->text();
        QDialog::accept();
    }
    else if(returnCode == 2)
    {
        QVector<QString> unconfiged_syllable_vector = m_currentSPModel->unconfiged_syllable_vector();
        Utils::NoticeMsgBox(QString("音节%1未配置").arg(unconfiged_syllable_vector.at(0)),this);
        return;
    }
    else
    {
        Utils::NoticeMsgBox("方案保存失败",this);
        return;
    }
}

bool NewSchemeDlg::eventFilter(QObject *obj, QEvent *event)
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
