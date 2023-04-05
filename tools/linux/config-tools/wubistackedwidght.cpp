#include "wubistackedwidght.h"
#include "ui_wubistackedwidght.h"
#include "customize_ui/customizefont.h"
#include "../public/configmanager.h"
#include <QFile>

wubistackedwidght::wubistackedwidght(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::wubistackedwidght)
{
    ui->setupUi(this);
    InitWidght();
    LoadConfigInfo();
}

wubistackedwidght::~wubistackedwidght()
{
    delete ui;
}

void wubistackedwidght::OnFourCharacterInputCheckedSlot(int checkstate)
{
   ConfigManager::Instance()->SetIntConfigItem("wubi_four_unique",(checkstate == Qt::Checked)? 1:0);
}

void wubistackedwidght::OnFiveCharacterInputCheckedSlot(int checkstate)
{
    ConfigManager::Instance()->SetIntConfigItem("wubi_five_unique",(checkstate == Qt::Checked)? 1:0);
}

void wubistackedwidght::OnVersionRadioButtonToggled(int index, bool state)
{
    if(state == true)
    {
        ConfigManager::Instance()->SetIntConfigItem("wubi_version",index);
    }
}

void wubistackedwidght::OnNoCandidateCancelInputCheckedSlot(int checkstate)
{
    ConfigManager::Instance()->SetIntConfigItem("wubi_no_candidate_cancel",(checkstate == Qt::Checked)? 1:0);
}

void wubistackedwidght::OnEnterCancelInputCheckedSlot(int checkstate)
{
    ConfigManager::Instance()->SetIntConfigItem("wubi_enter_cancel",(checkstate == Qt::Checked)? 1:0);
}

void wubistackedwidght::OnHintCheckedSlot(int checkstate)
{
    ConfigManager::Instance()->SetIntConfigItem("wubi_hint",(checkstate == Qt::Checked)? 1:0);
}

void wubistackedwidght::OnDynamicWubiRateChekedSlot(int checkstate)
{
    ConfigManager::Instance()->SetIntConfigItem("dynamic_wubi_rate",(checkstate == Qt::Checked)? 1:0);
}

void wubistackedwidght::InitWidght()
{

    QFile file(":/feedback/basicstyle.qss");
    if(file.open(QIODevice::ReadOnly))
    {
        this->setStyleSheet(file.readAll());
        file.close();
    }


    QList<QRadioButton*> childRaidoList = this->findChildren<QRadioButton*>();
    for(QRadioButton* index : childRaidoList)
    {
       index->setFocusPolicy(Qt::NoFocus);
    }

    m_wubi_version_button_group = new QButtonGroup(this);
    m_wubi_version_button_group->addButton(ui->version86_radio_button, 0);
    m_wubi_version_button_group->addButton(ui->version98_radio_button, 1);

    setProperty("type", "h1");
    ui->label_input_style->setProperty("type", "h1");
    ui->label_special_behavior->setProperty("type", "h1");
    ui->label_advanced_option->setProperty("type", "h1");
    ui->wubi_option_image_label->setPixmap(QPixmap(":/image/config/smart_option.png"));
    ui->wubi_option_image_label_2->setPixmap(QPixmap(":/image/config/smart_option.png"));
    ui->wubi_option_image_label_3->setPixmap(QPixmap(":/image/config/smart_option.png"));

    connect(ui->four_part_input_checkbox, SIGNAL(stateChanged(int)), this, SLOT(OnFourCharacterInputCheckedSlot(int)));
    connect(ui->five_input_choose_first_checkbox, SIGNAL(stateChanged(int)), this, SLOT(OnFiveCharacterInputCheckedSlot(int)));
    connect(m_wubi_version_button_group, SIGNAL(buttonToggled(int, bool)), this, SLOT(OnVersionRadioButtonToggled(int, bool)));
    connect(ui->wubi_hint_checkbox, SIGNAL(stateChanged(int)), this, SLOT(OnHintCheckedSlot(int)));
    connect(ui->enter_cancel_checkbox, SIGNAL(stateChanged(int)), this, SLOT(OnEnterCancelInputCheckedSlot(int)));
    connect(ui->no_candidate_cancel_checkbox, SIGNAL(stateChanged(int)), this, SLOT(OnNoCandidateCancelInputCheckedSlot(int)));
    connect(ui->wubi_dynamic_rate, SIGNAL(stateChanged(int)), this, SLOT(OnDynamicWubiRateChekedSlot(int)));
}


void wubistackedwidght::ResetConfigInfo()
{

    QStringList configitemkeylist;
    configitemkeylist << "wubi_four_unique" << "wubi_five_unique" << "wubi_enter_cancel"
                      << "wubi_no_candidate_cancel" << "dynamic_wubi_rate" << "wubi_version" << "wubi_hint";

    for(QString index : configitemkeylist)
    {
        ConfigManager::Instance()->ClearConfigItem(index);
    }
    LoadConfigInfo();
}

void wubistackedwidght::LoadConfigInfo()
{
    ui->four_part_input_checkbox->disconnect();
    ui->five_input_choose_first_checkbox->disconnect();
    m_wubi_version_button_group->disconnect();
    ui->wubi_hint_checkbox->disconnect();
    ui->enter_cancel_checkbox->disconnect();
    ui->no_candidate_cancel_checkbox->disconnect();
    ui->wubi_dynamic_rate->disconnect();

    int configItemValue;
    if(ConfigManager::Instance()->GetIntConfigItem("wubi_four_unique",configItemValue))
    {
        ui->four_part_input_checkbox->setChecked(configItemValue);
    }
    if(ConfigManager::Instance()->GetIntConfigItem("wubi_five_unique",configItemValue))
    {
        ui->five_input_choose_first_checkbox->setChecked(configItemValue);
    }
    if(ConfigManager::Instance()->GetIntConfigItem("wubi_enter_cancel",configItemValue))
    {
        ui->enter_cancel_checkbox->setChecked(configItemValue);
    }
    if(ConfigManager::Instance()->GetIntConfigItem("wubi_no_candidate_cancel",configItemValue))
    {
        ui->no_candidate_cancel_checkbox->setChecked(configItemValue);
    }
    if(ConfigManager::Instance()->GetIntConfigItem("dynamic_wubi_rate",configItemValue))
    {
        ui->wubi_dynamic_rate->setChecked(configItemValue);
    }

    if(ConfigManager::Instance()->GetIntConfigItem("wubi_version",configItemValue))
    {
        if(configItemValue == 0)
        {
            ui->version86_radio_button->setChecked(true);
        }
        else
        {
            ui->version98_radio_button->setChecked(true);
        }
    }
    if(ConfigManager::Instance()->GetIntConfigItem("wubi_hint",configItemValue))
    {
        ui->wubi_hint_checkbox->setChecked(configItemValue);
    }

    connect(ui->four_part_input_checkbox, SIGNAL(stateChanged(int)), this, SLOT(OnFourCharacterInputCheckedSlot(int)));
    connect(ui->five_input_choose_first_checkbox, SIGNAL(stateChanged(int)), this, SLOT(OnFiveCharacterInputCheckedSlot(int)));
    connect(m_wubi_version_button_group, SIGNAL(buttonToggled(int, bool)), this, SLOT(OnVersionRadioButtonToggled(int, bool)));
    connect(ui->wubi_hint_checkbox, SIGNAL(stateChanged(int)), this, SLOT(OnHintCheckedSlot(int)));
    connect(ui->enter_cancel_checkbox, SIGNAL(stateChanged(int)), this, SLOT(OnEnterCancelInputCheckedSlot(int)));
    connect(ui->no_candidate_cancel_checkbox, SIGNAL(stateChanged(int)), this, SLOT(OnNoCandidateCancelInputCheckedSlot(int)));
    connect(ui->wubi_dynamic_rate, SIGNAL(stateChanged(int)), this, SLOT(OnDynamicWubiRateChekedSlot(int)));
}
