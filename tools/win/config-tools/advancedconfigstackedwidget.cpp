#include "advancedconfigstackedwidget.h"
#include "ui_advancedconfigstackedwidget.h"
#include "config.h"
#include "networkhandler.h"
#include "utils.h"
#include "../../../toolBox/addon_config.h"

#include <QButtonGroup>
#include <QListView>
#include <QScreen>
#include <QVector>
#include <QDesktopServices>


AdvancedConfigStackedWidget::AdvancedConfigStackedWidget(QWidget *parent) :
    BaseStackedWidget(parent),
    ui(new Ui::AdvancedConfigStackedWidget)
{
    ui->setupUi(this);
    InitWidget();
    LoadConfigInfo();
}

AdvancedConfigStackedWidget::~AdvancedConfigStackedWidget()
{
    delete ui;
}

void AdvancedConfigStackedWidget::InitWidget()
{
    ui->key_set_head_label->setProperty("type","h1");
    ui->ChEnSwitch_label->setProperty("type","h2");
    ui->CandiatesPageKeyLabel->setProperty("type","h2");
    ui->second_third_candidates_label->setProperty("type","h2");
    ui->CiToCharacterLabel->setProperty("type","h2");
    ui->shortcut_label->setProperty("type","h2");
    ui->other_set_head_label->setProperty("type","h1");
    ui->candidates_2nd_3nd_tip->setProperty("type","tip");
    ui->ci_to_zi_tip->setProperty("type","tip");
    ui->pushButton_3->setProperty("type","tip");


    QList<QLabel*> label_list;
    label_list << ui->zi_rate_freq_label << ui->ci_rate_freq_label;
    for(QLabel* label : label_list)
    {
        label->setProperty("type","h2");
    }

    QStringList frequenceList;
    frequenceList << "固定" << "快速" << "慢速";
    ui->character_frequence_combo->addItems(frequenceList);
    ui->ci_frequence_combo->addItems(frequenceList);

    ui->experience_program_label->setProperty("type", "h1");
    ui->label_domain_option->setProperty("type", "h1");
    ui->domain_tip_label->setStyleSheet("color:#FF0000");

    QString requesetHeader;
    QString user_plan_url;
    if(NetworkHandler::Instance()->GetRequesetHeader(requesetHeader))
    {
         user_plan_url =  requesetHeader + QString("/privacy.html");

    }

    ui->read_plan_btn->setProperty("type","puretextbtn");
    ui->read_plan_btn->setCursor(Qt::PointingHandCursor);

    QString fileContent = "通过`键可以切换当前候选字的音调从而获取当前音调下的候选";
    ui->pushButton_3->setToolTip(fileContent);
    this->setProperty("type","stackedwidget");
    ui->scrollArea->setBackgroundRole(QPalette::Light);

    QVector<int> labelLayout;
    bool isOk = Config::Instance()->GetLayoutVector("labellayout",labelLayout);
    if(isOk)
    {
        for(int index=0; index<labelLayout.size(); ++index)
        {
            ui->label_layout->setStretch(index,labelLayout.at(index));
            ui->label_layout_2->setStretch(index,labelLayout.at(index));
            ui->layout_join_the_plan_option_label->setStretch(index,labelLayout.at(index));
            ui->layout_domin_option_label->setStretch(index, labelLayout.at(index));
        }
    }
    isOk = Config::Instance()->GetLayoutVector("part3_layout",labelLayout);

    m_comboBoxVector.clear();
    m_comboBoxVector.push_back(ui->cnen_switch_combo);
    m_comboBoxVector.push_back(ui->simple_traditional_switch_combo);
    m_comboBoxVector.push_back(ui->quan_shuang_combo);
    m_comboBoxVector.push_back(ui->character_frequence_combo);
    m_comboBoxVector.push_back(ui->ci_frequence_combo);
    m_comboBoxVector.push_back(ui->name_mode_shortcut_combo);
    m_comboBoxVector.push_back(ui->ocr_switch_combo);
    m_comboBoxVector.push_back(ui->voice_switch_combo);
    for(int index=0; index<m_comboBoxVector.size(); ++index)
    {
        m_comboBoxVector.at(index)->setMaxVisibleItems(6);
        m_comboBoxVector.at(index)->setAutoFillBackground(true);
        m_comboBoxVector.at(index)->setView(new QListView);
        m_comboBoxVector.at(index)->installEventFilter(this);
    }
    m_modeChangeGroup = new QButtonGroup(this);
    m_modeChangeGroup->addButton(ui->CnEn_Shift_RadioBtn,0);
    m_modeChangeGroup->addButton(ui->CnEn_Ctrl_RadioBtn,1);
    m_modeChangeGroup->addButton(ui->CnEn_Unused,2);


    //翻页的界面操作
    m_pairKeyGroup = new QButtonGroup(this);
    m_pairKeyGroup->setExclusive(false);
    m_pairKeyGroup->addButton(ui->PageTurn_dot_checkbox,0);
    m_pairKeyGroup->addButton(ui->PageTurn_bracket_checkbox,1);
    m_pairKeyGroup->addButton(ui->PageTurn_minusandplus_checkbox,2);
    m_pairKeyGroup->addButton(ui->PageTurn_upanddown_checkbox,3);


    //二三候选的界面操作
    m_candidates2nd3ndGroup = new QButtonGroup(this);
    m_candidates2nd3ndGroup->addButton(ui->Candidates_None,0);
    m_candidates2nd3ndGroup->addButton(ui->Candidates_ctrl_raidoBtn,1);
    m_candidates2nd3ndGroup->addButton(ui->Candidates_Shift_radiobtn,2);


    //以词定字
    m_citoziGroup = new QButtonGroup(this);
    m_citoziGroup->addButton(ui->citozi_bracket_radioBtn,0);
    m_citoziGroup->addButton(ui->citozi_unused_radiobtn,1);

    m_scopeGroup = new QButtonGroup(this);
    m_scopeGroup->addButton(ui->gbk_set_radiobtn,1);
    m_scopeGroup->addButton(ui->unicode_set_radiobtn,2);



    m_shortcutList << "A" << "B" << "C" << "D" << "E" << "F" << "G" << "H" << "I" <<
               "J" << "K" << "L" << "M" << "N" << "O" << "P" << "Q" << "R" <<
               "S" << "T" << "U" << "V" << "W" << "X" << "Y" << "Z";

    m_shortcutGroup = new QButtonGroup(this);
    m_shortcutGroup->setExclusive(false);
    m_shortcutGroup->addButton(ui->soft_panel_switch_checkbox,0);
    m_shortcutGroup->addButton(ui->cnEn_switch_checkbox,1);
    m_shortcutGroup->addButton(ui->quan_shuang_checkbox, 2);
    m_shortcutGroup->addButton(ui->simple_traditoal_switch_checkbox,3);
    m_shortcutGroup->addButton(ui->statusbar_switch_checkbox,4);
    m_shortcutGroup->addButton(ui->name_mode_shortcut_checkbox,5);
    m_shortcutGroup->addButton(ui->ocr_switch_checkbox,6);
    m_shortcutGroup->addButton(ui->voice_switch_checkbox,7);

    ui->soft_panel_switch_combo->setVisible(false);
    ui->soft_panel_switch_checkbox->setVisible(false);
    ui->statusbar_switch_combo->setVisible(false);
    ui->statusbar_switch_checkbox->setVisible(false);
    ui->label_4->setVisible(false);
    ui->label_5->setVisible(false);


    QString candidates_2nd_3nd_tip = "常规选择第二、三候选，用数字键2和3，此处定义额外扩充键";
    QString ci_to_zi_tip = "一组键对，用于输入一个词条的首字和末字\r\n这是一种有效的降低重码的方法，输入非常用字时，尤显体贴;";
    ui->candidates_2nd_3nd_tip->setToolTip(candidates_2nd_3nd_tip);
    ui->ci_to_zi_tip->setToolTip(ci_to_zi_tip);

    ui->PageTurn_minusandplus_checkbox->setEnabled(false);

    ui->line_edit_domain->setTextMargins(8,0,0,0);
    ConfigItemStruct default_domain;
    if(Config::Instance()->GetDefualtConfigItem("server_addr", default_domain))
        ui->line_edit_domain->setPlaceholderText(default_domain.itemCurrentStrValue);

    ConfigItemStruct show_join_the_plan_option;
    Config::Instance()->GetConfigItemByJson("show_join_the_plan_option", show_join_the_plan_option);
    if (show_join_the_plan_option.itemCurrentIntValue == 0)
    {
        ui->experience_program_label->setVisible(false);
        ui->join_the_plan_checkbox->setVisible(false);
        ui->read_plan_btn->setVisible(false);
        ui->exp_logo_label->setVisible(false);
    }
    else
    {
        ui->experience_program_label->setVisible(true);
        ui->join_the_plan_checkbox->setVisible(true);
        ui->read_plan_btn->setVisible(true);
        ui->exp_logo_label->setVisible(true);
    }
}

void AdvancedConfigStackedWidget::LoadConfigInfo()
{
    ui->UseHZToneCheckBox->disconnect();
    ui->symboleChangeAfterNumCheckBox->disconnect();
    ui->notePadToGetCandiatesCheckBox->disconnect();
    ui->join_the_plan_checkbox->disconnect();
    m_pairKeyGroup->disconnect();
    m_shortcutGroup->disconnect();
    m_modeChangeGroup->disconnect();
    m_candidates2nd3ndGroup->disconnect();
    m_citoziGroup->disconnect();
    m_scopeGroup->disconnect();
    ui->cnen_switch_combo->disconnect();
    ui->quan_shuang_combo->disconnect();
    ui->name_mode_shortcut_combo->disconnect();
    ui->simple_traditional_switch_combo->disconnect();
    ui->post_after_switch->disconnect();
    ui->checkBoxUsingEnglishPunctuationInChineseMode->disconnect();
    ui->read_plan_btn->disconnect();
    ui->checkBoxUseEnglishInput->disconnect();
    ui->checkBoxUsingHistoryShortcut->disconnect();
    ui->line_edit_domain->disconnect();
    ui->character_frequence_combo->disconnect();
    ui->ci_frequence_combo->disconnect();
    ui->name_mode_enable_checkbox->disconnect();
    ui->ocr_switch_combo->disconnect();
    ui->voice_switch_combo->disconnect();

    //中英文切换
    ConfigItemStruct configInfo;
    if(Config::Instance()->GetConfigItemByJson("key_change_mode",configInfo))
    {
        int keyMode = configInfo.itemCurrentIntValue;
        if(keyMode == KEY_SWITCH_SHIFT)
        {
            ui->CnEn_Shift_RadioBtn->setChecked(true);
        }
        else if(keyMode == KEY_SWITCH_CONTROL)
        {
            ui->CnEn_Ctrl_RadioBtn->setChecked(true);
        }
        else if(keyMode == KEY_SWITCH_NONE)
        {
            ui->CnEn_Unused->setChecked(true);
        }
    }
    else
    {
        ui->CnEn_Shift_RadioBtn->setChecked(true);
    }


    //第二第三候选词
    if(Config::Instance()->GetConfigItemByJson("key_candidate_2nd_3rd",configInfo))
    {
        int Key_2nd_3nd = configInfo.itemCurrentIntValue;
        if(Key_2nd_3nd == KEY_2ND_3RD_SHIFT)
        {
            ui->Candidates_Shift_radiobtn->setChecked(true);
        }
        else if(Key_2nd_3nd == KEY_2ND_3RD_CONTROL)
        {
            ui->Candidates_ctrl_raidoBtn->setChecked(true);
        }
        else if(Key_2nd_3nd == KEY_2ND_3RD_NONE)
        {
            ui->Candidates_None->setChecked(true);
        }
    }
    else
    {
        ui->Candidates_ctrl_raidoBtn->setChecked(true);
    }


    //以词定字
    if(Config::Instance()->GetConfigItemByJson("key_pair_zi_from_word",configInfo))
    {
         int ci_to_zi_config = configInfo.itemCurrentIntValue;
        if(ci_to_zi_config & KEY_PAIR_2)
        {
            ui->citozi_bracket_radioBtn->setChecked(true);
        }
        else if(ci_to_zi_config == 0)
        {
            ui->citozi_unused_radiobtn->setChecked(true);
        }
    }
    else
    {
        ui->citozi_unused_radiobtn->setChecked(true);
    }



    if(Config::Instance()->GetConfigItemByJson("use_hz_tone",configInfo))
    {
        ui->UseHZToneCheckBox->setChecked(configInfo.itemCurrentIntValue);
    }

    if(Config::Instance()->GetConfigItemByJson("join_the_plan",configInfo))
    {
        ui->join_the_plan_checkbox->setChecked(configInfo.itemCurrentIntValue);
    }

    if(Config::Instance()->GetConfigItemByJson("english_dot_follow_number",configInfo))
    {
       ui->symboleChangeAfterNumCheckBox->setChecked(configInfo.itemCurrentIntValue);
    }

    if(Config::Instance()->GetConfigItemByJson("numpad_select_enabled",configInfo))
    {
        ui->notePadToGetCandiatesCheckBox->setChecked(configInfo.itemCurrentIntValue);
    }

    if(Config::Instance()->GetConfigItemByJson("scope_gbk",configInfo))
    {
        int scopeValue = configInfo.itemCurrentIntValue;
        if(scopeValue == HZ_SCOPE_GBK)
        {
            ui->gbk_set_radiobtn->setChecked(true);
        }
        else if(scopeValue == HZ_SCOPE_UNICODE)
        {
            ui->unicode_set_radiobtn->setChecked(true);
        }
    }

    //中文模式下shift切换上屏
    if(Config::Instance()->GetConfigItemByJson("post_after_switch",configInfo))
    {
        ui->post_after_switch->setChecked(configInfo.itemCurrentIntValue);
    }

    if (Config::Instance()->GetConfigItemByJson("using_english_punctuation_in_chinese_mode", configInfo))
    {
        ui->checkBoxUsingEnglishPunctuationInChineseMode->setChecked(configInfo.itemCurrentIntValue);
    }

    if (Config::Instance()->GetConfigItemByJson("using_history_shortcut", configInfo))
    {
        ui->checkBoxUsingHistoryShortcut->setChecked(configInfo.itemCurrentIntValue);
    }

    //是否启动人名模式
    if (Config::Instance()->GetConfigItemByJson("enable_name_mode", configInfo))
    {
        ui->name_mode_enable_checkbox->setChecked(configInfo.itemCurrentIntValue);
    }

    //是否使用在候选中出现英文
    if(Config::Instance()->GetConfigItemByJson("use_english_input", configInfo))
    {
        ui->checkBoxUseEnglishInput->setChecked(configInfo.itemCurrentIntValue);
    }

    if(Config::Instance()->GetConfigItemByJson("zirate",configInfo))
    {
        ui->character_frequence_combo->setCurrentIndex(configInfo.itemCurrentIntValue);
    }
    else
    {
        ui->character_frequence_combo->setCurrentIndex(1);
    }

    if(Config::Instance()->GetConfigItemByJson("cirate",configInfo))
    {
        ui->ci_frequence_combo->setCurrentIndex(configInfo.itemCurrentIntValue);
    }
    else
    {
        ui->ci_frequence_combo->setCurrentIndex(1);
    }

    bool ocr_exist = AddonConfig::Instance()->IsAddonExist("OCR");
    bool voice_exist = AddonConfig::Instance()->IsAddonExist("AsrInput");
    ui->ocr_switch_checkbox->setVisible(ocr_exist);
    ui->ocr_switch_label->setVisible(ocr_exist);
    ui->ocr_switch_combo->setVisible(ocr_exist);

    ui->voice_switch_checkbox->setVisible(voice_exist);
    ui->voice_switch_label->setVisible(voice_exist);
    ui->voice_switch_combo->setVisible(voice_exist);

    InitKeyPairCandidates();
    InitShortcutWidget();
    ShortcutContentExclusive();
    ConnectSignalToSlot();
    OnServerConfigOptionChanged();

}

void AdvancedConfigStackedWidget::ConnectSignalToSlot()
{   
    connect(ui->UseHZToneCheckBox,SIGNAL(stateChanged(int)),this, SLOT(SlotUseHZToneToAssist(int)));
    connect(ui->symboleChangeAfterNumCheckBox, SIGNAL(stateChanged(int)), this, SLOT(SlotChangeSymbolAfterNum(int)));
    connect(ui->notePadToGetCandiatesCheckBox, SIGNAL(stateChanged(int)), this, SLOT(SlotSelectCandidatesByNumpad(int)));
    connect(ui->join_the_plan_checkbox, SIGNAL(stateChanged(int)), this, SLOT(SlotChangetheJoinPlanState(int)));

    connect(ui->name_mode_enable_checkbox,SIGNAL(stateChanged(int)),this,SLOT(SlotEnableNameModeCheck(int)));

    connect(m_pairKeyGroup, SIGNAL(buttonClicked(int)),this, SLOT(SlotKeyPairSet(int)));
    connect(m_shortcutGroup,SIGNAL(buttonClicked(int)),this,SLOT(SlotShortCutSet(int)));
    connect(m_modeChangeGroup,SIGNAL(buttonToggled(int, bool)),this, SLOT(SlotEnCnSwitch(int,bool)));
    connect(m_candidates2nd3ndGroup,SIGNAL(buttonToggled(int, bool)), this, SLOT(Slot2nd3ndCandidates(int,bool)));
    connect(m_citoziGroup, SIGNAL(buttonToggled(int, bool)), this, SLOT(SlotCiToziSet(int,bool)));
    connect(m_scopeGroup, SIGNAL(buttonToggled(int,bool)),this,SLOT(SlotScopeChanged(int,bool)));

    connect(ui->cnen_switch_combo, SIGNAL(currentTextChanged(const QString &)),this,SLOT(SlotCnEnComboIndexChanged(const QString &)));
    connect(ui->name_mode_shortcut_combo, SIGNAL(currentTextChanged(const QString &)),this,SLOT(OnNameModeComboIndexChanged(const QString&)));
    connect(ui->ocr_switch_combo, SIGNAL(currentTextChanged(const QString &)),this,SLOT(SlotOnOCRKeyChanged(QString)));
    connect(ui->voice_switch_combo, SIGNAL(currentTextChanged(const QString &)),this,SLOT(SlotOnVoiceKeyChanged(QString)));

    connect(ui->quan_shuang_combo, SIGNAL(currentTextChanged(const QString &)),this,SLOT(SlotQuanShuangComboIndexChanged(const QString &)));
    connect(ui->simple_traditional_switch_combo, SIGNAL(currentTextChanged(const QString &)), this, SLOT(SlotSimpleTraditionalComboIndexChanged(const QString &)));
    connect(ui->post_after_switch, SIGNAL(stateChanged(int)), this ,SLOT(SlotPostAfterSwitch(int)));
    connect(ui->read_plan_btn, SIGNAL(clicked(bool)),this,SLOT(SlotOpenPalnContent()));

    connect(ui->checkBoxUsingEnglishPunctuationInChineseMode, SIGNAL(stateChanged(int)), this, SLOT(SlotCheckBoxUsingEnglishPunctuationInChineseMode(int)));
    connect(ui->checkBoxUseEnglishInput, SIGNAL(stateChanged(int)), this, SLOT(SlotCheckBoxUseEnglishInput(int)));

    connect(ui->checkBoxUsingHistoryShortcut, SIGNAL(stateChanged(int)), this, SLOT(SlotCheckBoxUsingHistoryShortcut(int)));

    connect(ui->line_edit_domain, SIGNAL(textChanged(QString)), this, SLOT(onLineEditDomainTextChanged(QString)));

    connect(ui->character_frequence_combo, SIGNAL(currentIndexChanged(int)),this, SLOT(OnCharacterFrequenceChanged(int)));
    connect(ui->ci_frequence_combo, SIGNAL(currentIndexChanged(int)),this, SLOT(OnCiFrequenceChanged(int)));
}

void AdvancedConfigStackedWidget::SlotEnCnSwitch(int id, bool checkStatus)
{
    ConfigItemStruct configInfo;
    if(checkStatus == true)
    {
        configInfo.itemName = "key_change_mode";
        configInfo.itemCurrentIntValue = id;

        ConfigItemStruct old_configinfo;
        Config::Instance()->GetConfigItemByJson("key_change_mode",old_configinfo);

        int set_code =Config::Instance()->SetConfigItemByJson("key_change_mode",configInfo);
        if((set_code == 1))
        {
            Config::Instance()->SetConfigItemByJson("key_change_mode",old_configinfo);
            LoadConfigInfo();
        }
    }
}

void AdvancedConfigStackedWidget::SlotKeyPairSet(int id)
{
    ConfigItemStruct configInfo;
    QCheckBox* currentCheck = (QCheckBox*)m_pairKeyGroup->button(id);
    int m_pairconfig_value = 0;
    if(Config::Instance()->GetConfigItemByJson("key_pair_candidate_page",configInfo))
    {
        m_pairconfig_value = configInfo.itemCurrentIntValue;
    }

    int checkState = currentCheck->checkState();
    switch(id){
    case 0:
        if(checkState == Qt::Checked)
        {
            m_pairconfig_value |= KEY_PAIR_1;
        }
        else
        {
            m_pairconfig_value &= ~KEY_PAIR_1;
        }
        break;
    case 1:
        if(checkState == Qt::Checked)
        {
            m_pairconfig_value |= KEY_PAIR_2;
        }
        else
        {
            m_pairconfig_value &= ~KEY_PAIR_2;
        }
        break;
    case 2:
        if(checkState == Qt::Checked)
        {
            m_pairconfig_value |= KEY_PAIR_3;
        }
        else
        {
            m_pairconfig_value &= ~KEY_PAIR_3;
        }
        break;
    case 3:
        if(checkState == Qt::Checked)
        {
            m_pairconfig_value |= KEY_PAIR_7;
        }
        else
        {
            m_pairconfig_value &= ~KEY_PAIR_7;
        }
        break;
    }
    configInfo.itemName = "key_pair_candidate_page";
    configInfo.itemCurrentIntValue = m_pairconfig_value;

    ConfigItemStruct old_configinfo;
    Config::Instance()->GetConfigItemByJson("key_pair_candidate_page",old_configinfo);

    int set_code =Config::Instance()->SetConfigItemByJson("key_pair_candidate_page",configInfo);
    if((set_code == 2) && ((checkState == Qt::Checked)))
    {
        Config::Instance()->SetConfigItemByJson("key_pair_candidate_page",old_configinfo);
        LoadConfigInfo();
    }

}

void AdvancedConfigStackedWidget::InitKeyPairCandidates()
{
    ConfigItemStruct configInfo;
    int keyPair = 0;
    if(Config::Instance()->GetConfigItemByJson("key_pair_candidate_page",configInfo))
    {
        keyPair = configInfo.itemCurrentIntValue;
    }
    (keyPair & KEY_PAIR_1)?ui->PageTurn_dot_checkbox->setChecked(true):ui->PageTurn_dot_checkbox->setChecked(false);

    (keyPair & KEY_PAIR_3)?ui->PageTurn_minusandplus_checkbox->setChecked(true): ui->PageTurn_minusandplus_checkbox->setChecked(false);
    (keyPair & KEY_PAIR_2)?ui->PageTurn_bracket_checkbox->setChecked(true): ui->PageTurn_bracket_checkbox->setChecked(false);


    (keyPair & KEY_PAIR_7)? ui->PageTurn_upanddown_checkbox->setChecked(true): ui->PageTurn_upanddown_checkbox->setChecked(false);
}

void AdvancedConfigStackedWidget::Slot2nd3ndCandidates(int id, bool checkStatus)
{
    if(checkStatus == true)
    {
        ConfigItemStruct configInfo;
        configInfo.itemName = "key_candidate_2nd_3rd";
        configInfo.itemCurrentIntValue = id;

        ConfigItemStruct old_configinfo;
        Config::Instance()->GetConfigItemByJson("key_candidate_2nd_3rd",old_configinfo);

        int set_code =Config::Instance()->SetConfigItemByJson("key_candidate_2nd_3rd",configInfo);
        if((set_code == 1))
        {
            Config::Instance()->SetConfigItemByJson("key_candidate_2nd_3rd",old_configinfo);
            LoadConfigInfo();
        }
    }
}

void AdvancedConfigStackedWidget::SlotCiToziSet(int id, bool checkStatus)
{
    QRadioButton* currentBtn = (QRadioButton*)m_citoziGroup->button(id);

    ConfigItemStruct configInfo;
    configInfo.itemName = "key_pair_zi_from_word";
    if(checkStatus == true)
    {
        switch(id)
        {
        case 0:
            configInfo.itemCurrentIntValue = KEY_PAIR_2;
            break;
        case 1:
            configInfo.itemCurrentIntValue = 0;
            break;
        }
        ConfigItemStruct old_configInfo;
        Config::Instance()->GetConfigItemByJson("key_pair_zi_from_word",old_configInfo);

        int set_code =Config::Instance()->SetConfigItemByJson("key_pair_zi_from_word",configInfo);
        if((set_code == 2) && (currentBtn->isChecked()))
        {
            Config::Instance()->SetConfigItemByJson("key_pair_zi_from_word",old_configInfo);
            LoadConfigInfo();
        }

    }
}

void AdvancedConfigStackedWidget::SlotShortCutSet(int id)
{
    QCheckBox* currentCheckBox = (QCheckBox*)m_shortcutGroup->button(id);
    int status = currentCheckBox->checkState();
    ConfigItemStruct configInfo;
    switch(id)
    {
    case 0:
        (status == Qt::Checked)? ui->soft_panel_switch_combo->setEnabled(true):ui->soft_panel_switch_combo->setEnabled(false);
        break;
    case 1:
        (status == Qt::Checked)? ui->cnen_switch_combo->setEnabled(true):ui->cnen_switch_combo->setEnabled(false);
        configInfo.itemName = "use_key_english_input";
        configInfo.itemCurrentIntValue = (status == Qt::Checked);
        Config::Instance()->SetConfigItemByJson("use_key_english_input",configInfo);
        break;
    case 2:
        (status == Qt::Checked)? ui->quan_shuang_combo->setEnabled(true):ui->quan_shuang_combo->setEnabled(false);
        configInfo.itemName = "use_key_quan_shuang_pin";
        configInfo.itemCurrentIntValue = (status == Qt::Checked);
        Config::Instance()->SetConfigItemByJson("use_key_quan_shuang_pin",configInfo);
        break;
    case 3:
        (status == Qt::Checked)? ui->simple_traditional_switch_combo->setEnabled(true):ui->simple_traditional_switch_combo->setEnabled(false);
        configInfo.itemName = "use_key_jian_fan";
        configInfo.itemCurrentIntValue = (status == Qt::Checked);
        Config::Instance()->SetConfigItemByJson("use_key_jian_fan",configInfo);
        break;
    case 4:
        (status == Qt::Checked)? ui->statusbar_switch_combo->setEnabled(true):ui->statusbar_switch_combo->setEnabled(false);
        break;

    case 5:
        (status == Qt::Checked)? ui->name_mode_shortcut_combo->setEnabled(true):ui->name_mode_shortcut_combo->setEnabled(false);
        configInfo.itemName = "use_key_name_mode";
        configInfo.itemCurrentIntValue = (status == Qt::Checked);
        Config::Instance()->SetConfigItemByJson("use_key_name_mode",configInfo);
        break;
    case 6:
        (status == Qt::Checked)? ui->ocr_switch_combo->setEnabled(true):ui->ocr_switch_combo->setEnabled(false);
        configInfo.itemName = "use_key_shortcut_ocr";
        configInfo.itemCurrentIntValue = (status == Qt::Checked);
        Config::Instance()->SetConfigItemByJson("use_key_shortcut_ocr",configInfo);
        break;
    case 7:
        (status == Qt::Checked)? ui->voice_switch_combo->setEnabled(true):ui->voice_switch_combo->setEnabled(false);
        configInfo.itemName = "use_key_shortcut_voice";
        configInfo.itemCurrentIntValue = (status == Qt::Checked);
        Config::Instance()->SetConfigItemByJson("use_key_shortcut_voice",configInfo);
        break;
    }
}

void AdvancedConfigStackedWidget::SlotScopeChanged(int id, bool checkStatus)
{
    if(checkStatus == true)
    {
        ConfigItemStruct configInfo;
        configInfo.itemName = "scope_gbk";
        configInfo.itemCurrentIntValue = id;
        Config::Instance()->SetConfigItemByJson("scope_gbk",configInfo);
    }
}


void AdvancedConfigStackedWidget::InitShortcutWidget()
{
    ui->cnen_switch_combo->clear();
    ui->quan_shuang_combo->clear();
    ui->simple_traditional_switch_combo->clear();
    ui->name_mode_shortcut_combo->clear();
    ui->ocr_switch_combo->clear();
    ui->voice_switch_combo->clear();

    ConfigItemStruct configInfo;
    if(Config::Instance()->GetConfigItemByJson("use_key_soft_kbd",configInfo))
    {
        int use_soft_kbd = configInfo.itemCurrentIntValue;
        use_soft_kbd? ui->soft_panel_switch_checkbox->setChecked(true):ui->soft_panel_switch_checkbox->setChecked(false);
        use_soft_kbd? ui->soft_panel_switch_combo->setEnabled(true):ui->soft_panel_switch_combo->setEnabled(false);
    }


    if(Config::Instance()->GetConfigItemByJson("use_key_english_input",configInfo))
    {
        int use_english_input = configInfo.itemCurrentIntValue;
        use_english_input?ui->cnEn_switch_checkbox->setChecked(true):ui->cnEn_switch_checkbox->setChecked(false);
        use_english_input?ui->cnen_switch_combo->setEnabled(true):ui->cnen_switch_combo->setEnabled(false);
    }
    if(Config::Instance()->GetConfigItemByJson("key_english_input",configInfo))
    {
        QString tempStr = QString(QChar::fromLatin1((char)configInfo.itemCurrentIntValue));
        m_shortcutMap[ui->cnen_switch_combo] = tempStr;
        ui->cnen_switch_combo->addItem(tempStr,configInfo.itemCurrentIntValue);
        ui->cnen_switch_combo->setCurrentText(tempStr);
    }

    if(Config::Instance()->GetConfigItemByJson("use_key_quan_shuang_pin",configInfo))
    {
        int use_quan_shuang_pin = configInfo.itemCurrentIntValue;
        use_quan_shuang_pin?ui->quan_shuang_checkbox->setChecked(true):ui->quan_shuang_checkbox->setChecked(false);
        use_quan_shuang_pin?ui->quan_shuang_combo->setEnabled(true):ui->quan_shuang_combo->setEnabled(false);
    }
    if(Config::Instance()->GetConfigItemByJson("key_jian_fan",configInfo))
    {
        QString tempStr = QString(QChar::fromLatin1((char)configInfo.itemCurrentIntValue));
        m_shortcutMap[ui->simple_traditional_switch_combo] = tempStr;
        ui->simple_traditional_switch_combo->addItem(tempStr,configInfo.itemCurrentIntValue);
        ui->simple_traditional_switch_combo->setCurrentText(tempStr);
    }


   if(Config::Instance()->GetConfigItemByJson("use_key_jian_fan",configInfo))
   {
       int use_fan_jian = configInfo.itemCurrentIntValue;
       use_fan_jian?ui->simple_traditoal_switch_checkbox->setChecked(true):ui->simple_traditoal_switch_checkbox->setChecked(false);
       use_fan_jian?ui->simple_traditional_switch_combo->setEnabled(true):ui->simple_traditional_switch_combo->setEnabled(false);
   }
   if(Config::Instance()->GetConfigItemByJson("key_quan_shuang_pin",configInfo))
   {
       QString tempStr = QString(QChar::fromLatin1((char)configInfo.itemCurrentIntValue));
       m_shortcutMap[ui->quan_shuang_combo] = tempStr;
       ui->quan_shuang_combo->addItem(tempStr,configInfo.itemCurrentIntValue);
       ui->quan_shuang_combo->setCurrentText(tempStr);
   }

   if(Config::Instance()->GetConfigItemByJson("use_key_name_mode",configInfo))
   {
       int use_name_mode = configInfo.itemCurrentIntValue;
       use_name_mode? ui->name_mode_shortcut_checkbox->setChecked(true):ui->name_mode_shortcut_checkbox->setChecked(false);
       use_name_mode? ui->name_mode_shortcut_combo->setEnabled(true):ui->name_mode_shortcut_combo->setEnabled(false);
   }

   if(Config::Instance()->GetConfigItemByJson("key_name_mode",configInfo))
   {
       QString tempStr = QString(QChar::fromLatin1((char)configInfo.itemCurrentIntValue));
       m_shortcutMap[ui->name_mode_shortcut_combo] = tempStr;
       ui->name_mode_shortcut_combo->addItem(tempStr,configInfo.itemCurrentIntValue);
       ui->name_mode_shortcut_combo->setCurrentText(tempStr);
   }
   if(Config::Instance()->GetConfigItemByJson("key_shortcut_ocr",configInfo))
   {
       QString tempStr = QString(QChar::fromLatin1((char)configInfo.itemCurrentIntValue));
       m_shortcutMap[ui->ocr_switch_combo] = tempStr;
       ui->ocr_switch_combo->addItem(tempStr,configInfo.itemCurrentIntValue);
       ui->ocr_switch_combo->setCurrentText(tempStr);
   }

   if(Config::Instance()->GetConfigItemByJson("use_key_shortcut_ocr",configInfo))
   {
       int use_ocr = configInfo.itemCurrentIntValue;
       use_ocr? ui->ocr_switch_checkbox->setChecked(true):ui->ocr_switch_checkbox->setChecked(false);
       use_ocr? ui->ocr_switch_combo->setEnabled(true):ui->ocr_switch_combo->setEnabled(false);
   }

   if(Config::Instance()->GetConfigItemByJson("key_shortcut_voice",configInfo))
   {
       QString tempStr = QString(QChar::fromLatin1((char)configInfo.itemCurrentIntValue));
       m_shortcutMap[ui->voice_switch_combo] = tempStr;
       ui->voice_switch_combo->addItem(tempStr,configInfo.itemCurrentIntValue);
       ui->voice_switch_combo->setCurrentText(tempStr);
   }

   if(Config::Instance()->GetConfigItemByJson("use_key_shortcut_voice",configInfo))
   {
       int use_voice = configInfo.itemCurrentIntValue;
       use_voice? ui->voice_switch_checkbox->setChecked(true):ui->voice_switch_checkbox->setChecked(false);
       use_voice? ui->voice_switch_combo->setEnabled(true):ui->voice_switch_combo->setEnabled(false);
   }
}


void AdvancedConfigStackedWidget::SlotCnEnComboIndexChanged(const QString &text)
{
    ConfigItemStruct configInfo;
    configInfo.itemName = "key_english_input";
    configInfo.itemCurrentIntValue = text.at(0).unicode();
    QString oldShortcut = m_shortcutMap[ui->cnen_switch_combo];
    QString newShortcut = text;
    m_shortcutMap[ui->cnen_switch_combo] = text;
    RefreshShortcutList(ui->cnen_switch_combo,oldShortcut,newShortcut);

    Config::Instance()->SetConfigItemByJson("key_english_input",configInfo);
}

void AdvancedConfigStackedWidget::SlotQuanShuangComboIndexChanged(const QString &text)
{

    ConfigItemStruct configInfo;
    configInfo.itemName = "key_quan_shuang_pin";
    configInfo.itemCurrentIntValue = text.at(0).unicode();
    QString oldShortcut = m_shortcutMap[ui->quan_shuang_combo];
    QString newShortcut = text;
    m_shortcutMap[ui->quan_shuang_combo] = text;
    RefreshShortcutList(ui->quan_shuang_combo,oldShortcut,newShortcut);

    Config::Instance()->SetConfigItemByJson("key_quan_shuang_pin",configInfo);
}
void AdvancedConfigStackedWidget::SlotSimpleTraditionalComboIndexChanged(const QString &text)
{
    ConfigItemStruct configInfo;
    configInfo.itemName = "key_jian_fan";
    configInfo.itemCurrentIntValue = text.at(0).unicode();
    QString oldShortcut = m_shortcutMap[ui->simple_traditional_switch_combo];
    QString newShortcut = text;
    m_shortcutMap[ui->simple_traditional_switch_combo] = text;
    RefreshShortcutList(ui->simple_traditional_switch_combo,oldShortcut,newShortcut);


    Config::Instance()->SetConfigItemByJson("key_jian_fan",configInfo);
}


void AdvancedConfigStackedWidget::SlotUseHZToneToAssist(int checkStatus)
{
    int flag = (checkStatus == Qt::Checked)? 1:0;
    ConfigItemStruct configInfo;
    configInfo.itemName = "use_hz_tone";
    configInfo.itemCurrentIntValue = flag;
    Config::Instance()->SetConfigItemByJson("use_hz_tone",configInfo);
}

void AdvancedConfigStackedWidget::SlotEnableNameModeCheck(int checkStatus)
{
    int flag = (checkStatus == Qt::Checked)? 1:0;
    ConfigItemStruct configInfo;
    configInfo.itemName = "enable_name_mode";
    configInfo.itemCurrentIntValue = flag;
    Config::Instance()->SetConfigItemByJson("enable_name_mode",configInfo);
}

void AdvancedConfigStackedWidget::SlotChangeSymbolAfterNum(int checkStatus)
{
    ConfigItemStruct configInfo;
    int flag = (checkStatus == Qt::Checked)? 1:0;
    configInfo.itemName = "english_dot_follow_number";
    configInfo.itemCurrentIntValue = flag;
    Config::Instance()->SetConfigItemByJson("english_dot_follow_number",configInfo);
}

void AdvancedConfigStackedWidget::SlotSelectCandidatesByNumpad(int checkStatus)
{
    int flag = (checkStatus == Qt::Checked)? 1:0;
    ConfigItemStruct configInfo;
    configInfo.itemName = "numpad_select_enabled";
    configInfo.itemCurrentIntValue = flag;
    Config::Instance()->SetConfigItemByJson("numpad_select_enabled",configInfo);
}

void AdvancedConfigStackedWidget::SlotChangetheJoinPlanState(int checkStatus)
{
    int flag = (checkStatus == Qt::Checked)? 1:0;
    ConfigItemStruct configInfo;
    configInfo.itemName = "join_the_plan";
    configInfo.itemCurrentIntValue = flag;
    configInfo.itemGroupName = "state";
    Config::Instance()->SetConfigItemByJson("join_the_plan",configInfo);
}

void AdvancedConfigStackedWidget::SlotPostAfterSwitch(int checkStatus)
{
    int flag = (checkStatus == Qt::Checked)? 1:0;
    ConfigItemStruct configInfo;
    configInfo.itemName = "post_after_switch";
    configInfo.itemCurrentIntValue = flag;
    Config::Instance()->SetConfigItemByJson("post_after_switch",configInfo);
}

void AdvancedConfigStackedWidget::SlotOpenPalnContent()
{
    ui->read_plan_btn->setEnabled(false);
    QString requesetHeader;
    QString user_plan_url;
    if(NetworkHandler::Instance()->GetRequesetHeader(requesetHeader))
    {
         user_plan_url =  requesetHeader + QString("/privacy.html");

    }
    wchar_t wchar_array[512];
    std::memset(wchar_array,0x00,sizeof(wchar_t)*512);
    user_plan_url.toWCharArray(wchar_array);
    ShellExecute(NULL, L"open",wchar_array, NULL, NULL, SW_SHOWNORMAL);
    Utils::WriteLogToFile("visite the user plan website");
    ui->read_plan_btn->setEnabled(true);
    //防止重复点击事件
    Sleep(1000);
}

void AdvancedConfigStackedWidget::SlotCheckBoxUsingEnglishPunctuationInChineseMode(int checkStatus)
{
    int flag = (checkStatus == Qt::Checked) ? 1 : 0;
    ConfigItemStruct configInfo;
    configInfo.itemName = "using_english_punctuation_in_chinese_mode";
    configInfo.itemCurrentIntValue = flag;
    Config::Instance()->SetConfigItemByJson("using_english_punctuation_in_chinese_mode", configInfo);
}

void AdvancedConfigStackedWidget::SlotCheckBoxUseEnglishInput(int checkStatus)
{
    int flag = (checkStatus == Qt::Checked) ? 1 : 0;
    ConfigItemStruct configInfo;
    configInfo.itemName = "use_english_input";
    configInfo.itemCurrentIntValue = flag;
    Config::Instance()->SetConfigItemByJson("use_english_input", configInfo);
}

void AdvancedConfigStackedWidget::SlotCheckBoxUsingHistoryShortcut(int checkStatus)
{
    int flag = (checkStatus == Qt::Checked) ? 1 : 0;
    ConfigItemStruct configInfo;
    configInfo.itemName = "using_history_shortcut";
    configInfo.itemCurrentIntValue = flag;
    Config::Instance()->SetConfigItemByJson("using_history_shortcut", configInfo);
}

void AdvancedConfigStackedWidget::ResetConfigInfo()
{
    QStringList config_item_name_list;
    config_item_name_list << "key_change_mode"
                          << "key_candidate_2nd_3rd"
                          << "key_pair_zi_from_word"
                          << "use_key_english_input"
                          << "use_key_quan_shuang_pin"
                          << "use_key_jian_fan"
                          << "key_english_input"
                          << "key_quan_shuang_pin"
                          << "key_jian_fan"
                          << "english_dot_follow_number"
                          << "numpad_select_enabled"
                          << "use_hz_tone"
                          << "key_pair_candidate_page"
                          << "scope_gbk"
                          << "post_after_switch"
                          << "using_english_punctuation_in_chinese_mode"
                          << "use_english_input"
                          << "using_history_shortcut"
                          << "join_the_plan"
                          << "zirate"
                          << "cirate"
                          << "use_key_name_mode"
                          << "key_name_mode"
                          << "enable_name_mode"
                          << "use_key_shortcut_ocr"
                          << "key_shortcut_ocr"
                          << "use_key_shortcut_voice"
                          << "key_shortcut_voice"   ;


    for(int index=0; index<config_item_name_list.size(); ++index)
    {
        QString currentItem = config_item_name_list.at(index);
        ConfigItemStruct configInfo;
        if(Config::Instance()->GetDefualtConfigItem(currentItem,configInfo))
        {
            Config::Instance()->SetConfigItemByJsonWithoutCheck(currentItem,configInfo);
        }

    }
}

void AdvancedConfigStackedWidget::ShortcutContentExclusive()
{
    QList<QComboBox*> keyList = m_shortcutMap.keys();
    for(int index=0; index<keyList.size(); ++index)
    {
        QStringList tempList = m_shortcutList;
        QComboBox* currentCombo = keyList.at(index);

        QString currentStr = currentCombo->currentText();
        for(int sideIndex=0; sideIndex<keyList.size(); ++sideIndex)
        {
            if(sideIndex == index)
            {
                continue;
            }
            QComboBox* nextCombo = keyList.at(sideIndex);
            QString nextStr = nextCombo->currentText();
            tempList.removeOne(nextStr);
        }
        currentCombo->clear();
        currentCombo->addItems(tempList);
        int currentShortcutIndex = currentCombo->findText(currentStr);
        currentCombo->setCurrentIndex(currentShortcutIndex);
    }
}

void AdvancedConfigStackedWidget::RefreshShortcutList(QComboBox* currentCombo,QString oldStr,QString newStr)
{
    QList<QComboBox*> keyList = m_shortcutMap.keys();
    for(int index=0; index<keyList.size(); ++index)
    {
        if(keyList.at(index) == currentCombo)
        {
            continue;
        }

        int newStrIndex = keyList.at(index)->findText(newStr);
        if(newStrIndex >= 0)
        {
            keyList.at(index)->blockSignals(true);
            keyList.at(index)->removeItem(newStrIndex);
            keyList.at(index)->blockSignals(false);
        }
        int oldStrIndex = keyList.at(index)->findText(oldStr);
        if(oldStrIndex < 0)
        {
            QList<int> itemList;

            for(int countIndex=0; countIndex<keyList.at(index)->count(); ++countIndex)
            {
                int countIndexValue = keyList.at(index)->itemText(countIndex).at(0).unicode();
                itemList.push_back(countIndexValue);
            }
            itemList.push_back(oldStr.at(0).unicode());
            qSort(itemList.begin(),itemList.end());
            int insertIndex = itemList.indexOf(oldStr.at(0).unicode());
            keyList.at(index)->blockSignals(true);
            keyList.at(index)->insertItem(insertIndex,oldStr);
            keyList.at(index)->blockSignals(false);
        }
    }
}

bool AdvancedConfigStackedWidget::eventFilter(QObject *obj, QEvent *event)
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

void AdvancedConfigStackedWidget::onLineEditDomainTextChanged(QString text)
{
    ui->domain_tip_label->clear();
    ConfigItemStruct configInfoHost;
    configInfoHost.itemName = "server_addr";
    configInfoHost.itemCurrentStrValue = text;
    configInfoHost.itemType = "string";
    configInfoHost.itemGroupName = "state";

    if(!text.isEmpty())
    {
        QRegExp ip_reg("^((25[0-5]|2[0-4]\\d|[1]{1}\\d{1}\\d{1}|[1-9]{1}\\d{1}|\\d{1})($|(?!\\.$)\\.)){4}$");
        QRegExp domain_reg("^(?=^.{3,255}$)[a-zA-Z][-a-zA-Z]{0,62}(\\.[a-zA-Z][-a-zA-Z]{0,62})+$");

        if(ip_reg.exactMatch(text))
        {
            Config::Instance()->SetConfigItemByJson("server_addr", configInfoHost);

        }
        else if(domain_reg.exactMatch(text))
        {
             Config::Instance()->SetConfigItemByJson("server_addr", configInfoHost);
        }
        else
        {
            QString  message = QString::fromLocal8Bit("无效域名");
            ui->domain_tip_label->setText(message);
        }
    }
    else
    {
        QString message = "域名为空";
        ui->domain_tip_label->setText(message);
    }

}

void AdvancedConfigStackedWidget::OnCharacterFrequenceChanged(int index)
{
    ConfigItemStruct configInfo;
    configInfo.itemName = "zirate";
    configInfo.itemCurrentIntValue = index;
    Config::Instance()->SetConfigItemByJson("zirate", configInfo);
}

void AdvancedConfigStackedWidget::OnNameModeComboIndexChanged(const QString &text)
{
    ConfigItemStruct configInfo;
    configInfo.itemName = "key_name_mode";
    configInfo.itemCurrentIntValue = text.at(0).unicode();
    QString oldShortcut = m_shortcutMap[ui->name_mode_shortcut_combo];
    QString newShortcut = text;
    m_shortcutMap[ui->name_mode_shortcut_combo] = text;
    RefreshShortcutList(ui->name_mode_shortcut_combo,oldShortcut,newShortcut);
    Config::Instance()->SetConfigItemByJson("key_name_mode",configInfo);
}

void AdvancedConfigStackedWidget::OnServerConfigOptionChanged()
{
    ConfigItemStruct server_type_struct;
    bool isExist = Config::Instance()->GetConfigItemByJson("server_type", server_type_struct);
    if (isExist && (server_type_struct.itemCurrentStrValue == "private"))
    {

        ui->label_domain_option->setVisible(true);
        ui->label_domain_address->setVisible(true);
        ui->line_edit_domain->setVisible(true);
        ui->label_domain_option_icon->setVisible(true);
        ui->label_domain_address_hint->setVisible(true);

        ConfigItemStruct server_scheme;
        Config::Instance()->GetConfigItemByJson("server_scheme", server_scheme);
        ConfigItemStruct server_addr;
        Config::Instance()->GetConfigItemByJson("server_addr", server_addr);
        ConfigItemStruct server_port;
        Config::Instance()->GetConfigItemByJson("server_port", server_port);
        QUrl url;
        url.setScheme(server_scheme.itemCurrentStrValue);
        url.setHost(server_addr.itemCurrentStrValue);
        url.setPort(server_port.itemCurrentIntValue);
        ui->line_edit_domain->setText(url.host());
    }
    else
    {
        ui->label_domain_option->setVisible(false);
        ui->label_domain_address->setVisible(false);
        ui->line_edit_domain->setVisible(false);
        ui->label_domain_option_icon->setVisible(false);
        ui->label_domain_address_hint->setVisible(false);
    }

    ui->label_domain_option_icon->setPixmap(QPixmap(":/image/config/shortcut.png"));
}

void AdvancedConfigStackedWidget::OnCiFrequenceChanged(int index)
{
    ConfigItemStruct configInfo;
    configInfo.itemName = "cirate";
    configInfo.itemCurrentIntValue = index;
    Config::Instance()->SetConfigItemByJson("cirate", configInfo);
}

void AdvancedConfigStackedWidget::SlotOnOCRKeyChanged(const QString &text)
{

    ConfigItemStruct configInfo;
    configInfo.itemName = "key_shortcut_ocr";
    configInfo.itemCurrentIntValue = text.at(0).unicode();
    QString oldShortcut = m_shortcutMap[ui->ocr_switch_combo];
    QString newShortcut = text;
    m_shortcutMap[ui->ocr_switch_combo] = text;
    RefreshShortcutList(ui->ocr_switch_combo,oldShortcut,newShortcut);

    Config::Instance()->SetConfigItemByJson("key_shortcut_ocr",configInfo);
}

void AdvancedConfigStackedWidget::SlotOnVoiceKeyChanged(const QString &text)
{

    ConfigItemStruct configInfo;
    configInfo.itemName = "key_shortcut_voice";
    configInfo.itemCurrentIntValue = text.at(0).unicode();
    QString oldShortcut = m_shortcutMap[ui->voice_switch_combo];
    QString newShortcut = text;
    m_shortcutMap[ui->voice_switch_combo] = text;
    RefreshShortcutList(ui->voice_switch_combo,oldShortcut,newShortcut);

    Config::Instance()->SetConfigItemByJson("key_shortcut_voice",configInfo);
}


