#include "advancedconfigstackedwidget.h"
#include "ui_advancedconfigstackedwidget.h"

#include <QButtonGroup>
#include <QListView>
#include <QScreen>
#include <QVector>
#include <QDesktopServices>
#include "../public/configmanager.h"
#include "../public/addon_config.h"
#include "configbus.h"
#include <QMessageBox>
#include <QProcess>
#include <QDebug>
#include <QScrollBar>

AdvancedConfigStackedWidget::AdvancedConfigStackedWidget(QWidget *parent) :
    QWidget(parent),
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
    ui->set_fcitx_btn->setProperty("type","normal-set-btn");
    ui->charactersetLabel_3->setProperty("type","h2");
    ui->charactersetLabel_4->setProperty("type","h2");
    ui->charactersetLabel_5->setProperty("type","h2");

    ui->experience_program_label->setProperty("type", "h1");
    ui->label_domain_option->setProperty("type", "h1");


    ui->read_plan_btn->setProperty("type","puretextbtn");
    SetItemObjectName();

    QString fileContent = "通过`键可以切换当前候选字的音调从而获取当前音调下的候选";
    ui->pushButton_3->setToolTip(fileContent);
    this->setProperty("type","stackedwidget");
    ui->scrollArea->setBackgroundRole(QPalette::Light);


    QVector<int> labelLayout= {21,16,1,77,454};

    for(int index=0; index<labelLayout.size(); ++index)
    {
        ui->label_layout->setStretch(index,labelLayout.at(index));
        ui->label_layout_2->setStretch(index,labelLayout.at(index));
        ui->layout_join_the_plan_option_label->setStretch(index,labelLayout.at(index));
        ui->layout_domin_option_label->setStretch(index,labelLayout.at(index));
    }



    m_comboBoxVector.clear();
    m_comboBoxVector.push_back(ui->cnen_switch_combo);
    m_comboBoxVector.push_back(ui->simple_traditional_switch_combo);
    m_comboBoxVector.push_back(ui->quan_shuang_combo);
    m_comboBoxVector.push_back(ui->toolbar_hide_switch_combo);
    m_comboBoxVector.push_back(ui->ocr_combo);
    m_comboBoxVector.push_back(ui->voice_switch_combo);
    m_comboBoxVector.push_back(ui->phrase_frequence_combo);
    m_comboBoxVector.push_back(ui->character_frequence_combo);
    m_comboBoxVector.push_back(ui->shortcut_combo);
    m_comboBoxVector.push_back(ui->pinyin_wubi_combo);
    m_comboBoxVector.push_back(ui->name_mode_shortcut_combo);

    for(int index=0; index<m_comboBoxVector.size(); ++index)
    {
        m_comboBoxVector.at(index)->setMaxVisibleItems(6);
        m_comboBoxVector.at(index)->setAutoFillBackground(true);
        m_comboBoxVector.at(index)->setView(new QListView);
        m_comboBoxVector.at(index)->installEventFilter(this);
    }
    m_modeChangeGroup = new QButtonGroup(this);
    m_modeChangeGroup->addButton(ui->CnEn_Shift_RadioBtn, KEY_SWITCH_SHIFT);
    m_modeChangeGroup->addButton(ui->CnEn_Ctrl_RadioBtn, KEY_SWITCH_CONTROL);
    m_modeChangeGroup->addButton(ui->CnEn_Unused, KEY_SWITCH_NONE);

    //翻页的界面操作
    m_pairKeyGroup = new QButtonGroup(this);
    m_pairKeyGroup->setExclusive(false);
    m_pairKeyGroup->addButton(ui->PageTurn_dot_checkbox, KEY_PAIR_1);
    m_pairKeyGroup->addButton(ui->PageTurn_bracket_left_and_right_checkbox, KEY_PAIR_2);
    m_pairKeyGroup->addButton(ui->PageTurn_minusandplus_checkbox, KEY_PAIR_3);
    m_pairKeyGroup->addButton(ui->PageTurn_upanddown_checkbox, KEY_PAIR_6);


    m_candidates2nd3ndGroup = new QButtonGroup(this);
    m_candidates2nd3ndGroup->addButton(ui->Candidates_None,0);
    m_candidates2nd3ndGroup->addButton(ui->Candidates_ctrl_raidoBtn,1);
    m_candidates2nd3ndGroup->addButton(ui->Candidates_Shift_radiobtn,2);

    //二三候选的界面操作
    m_candidates2nd3ndGroup = new QButtonGroup(this);
    m_candidates2nd3ndGroup->addButton(ui->Candidates_ctrl_raidoBtn, KEY_2ND_3RD_CONTROL);
    m_candidates2nd3ndGroup->addButton(ui->Candidates_Shift_radiobtn, KEY_2ND_3RD_SHIFT);
    m_candidates2nd3ndGroup->addButton(ui->Candidates_None, KEY_2ND_3RD_NONE);
    m_candidates2nd3ndGroup->setExclusive(true);
    ui->Candidates_None->setChecked(true);

    //以词定字
    m_citoziGroup = new QButtonGroup(this);
    m_citoziGroup->addButton(ui->citozi_bracket_radioBtn, KEY_PAIR_2);
    m_citoziGroup->addButton(ui->citozi_unused_radiobtn, KEY_ZI_FROM_CI_NONE);
    m_citoziGroup->setExclusive(true);
    ui->citozi_bracket_radioBtn->setVisible(false);
    ui->citozi_unused_radiobtn->setVisible(false);
    ui->ci_to_zi_tip->setVisible(false);
    ui->CiToCharacterLabel->setVisible(false);

    m_scopeGroup = new QButtonGroup(this);
    m_scopeGroup->addButton(ui->gbk_set_radiobtn, HZ_SCOPE_GBK);
    m_scopeGroup->addButton(ui->unicode_set_radiobtn, HZ_SCOPE_UNICODE);
    m_scopeGroup->setExclusive(true);

    m_shortcutList << "A" << "B" << "C" << "D" << "E" << "F" << "G" << "H" << "I" <<
               "J" << "K" << "L" << "M" << "N" << "O" << "P" << "Q" << "R" <<
               "S" << "T" << "U" << "V" << "W" << "X" << "Y" << "Z";

    m_shortcutGroup = new QButtonGroup(this);
    m_shortcutGroup->setExclusive(false);
    m_shortcutGroup->addButton(ui->soft_panel_switch_checkbox,0);
    m_shortcutGroup->addButton(ui->cnEn_switch_checkbox,1);
    m_shortcutGroup->addButton(ui->quan_shuang_checkbox, 2);
    m_shortcutGroup->addButton(ui->simple_traditoal_switch_checkbox,3);
    m_shortcutGroup->addButton(ui->full_half_switch_checkbox,4);
    m_shortcutGroup->addButton(ui->toolbar_hide_switch_checkbox,5);
    m_shortcutGroup->addButton(ui->ocr_checkbox,6);
    m_shortcutGroup->addButton(ui->voice_switch_checkbox,7);
    m_shortcutGroup->addButton(ui->pinyin_wubi_checkbox,8);
    m_shortcutGroup->addButton(ui->name_mode_shortcut_checkbox,9);

    ui->soft_panel_switch_combo->setVisible(false);
    ui->soft_panel_switch_checkbox->setVisible(false);
    ui->full_half_switch_combo->setVisible(false);
    ui->label_5->setVisible(false);
    ui->cnEn_switch_checkbox->setVisible(false);
    ui->label->setVisible(false);
    ui->cnen_switch_combo->setVisible(false);

    QString candidates_2nd_3nd_tip = "常规选择第二、三候选，用数字键2和3，此处定义额外扩充键";
    QString ci_to_zi_tip = "一组键对，用于输入一个词条的首字和末字\r\n这是一种有效的降低重码的方法，输入非常用字时，尤显体贴;";
    ui->candidates_2nd_3nd_tip->setToolTip(candidates_2nd_3nd_tip);
    ui->ci_to_zi_tip->setToolTip(ci_to_zi_tip);

    QStringList frequenceList;
    frequenceList << "固定" << "快速" << "慢速";
    ui->character_frequence_combo->addItems(frequenceList);
    ui->phrase_frequence_combo->addItems(frequenceList);

    ui->UseHZToneCheckBox->setFixedWidth(1);
    ui->UseHZToneCheckBox->setVisible(false);
    ui->checkBoxUsingHistoryShortcut->setVisible(false);
    ui->pushButton_3->setVisible(false);
    ui->pushButton_3->setFixedWidth(0);



    ui->experience_program_label->setVisible(true);
    ui->join_the_plan_checkbox->setVisible(true);
    ui->read_plan_btn->setVisible(true);
    ui->exp_logo_label->setVisible(true);



    ui->label_domain_option->setVisible(false);
    ui->label_domain_address->setVisible(false);
    ui->line_edit_domain->setVisible(false);
    ui->label_domain_option_icon->setVisible(false);
    ui->label_domain_address_hint->setVisible(false);



    ui->notePadToGetCandiatesCheckBox->setVisible(false);

}

void AdvancedConfigStackedWidget::SetItemObjectName()
{
    ui->UseHZToneCheckBox->setObjectName("use_hz_tone");
    ui->symboleChangeAfterNumCheckBox->setObjectName("english_dot_follow_number");
    ui->notePadToGetCandiatesCheckBox->setObjectName("numpad_select_enabled");
    ui->checkBoxUsingEnglishPunctuationInChineseMode->setObjectName("using_english_punctuation_in_chinese_mode");
    ui->post_after_switch->setObjectName("post_after_switch");
    ui->checkBoxUsingHistoryShortcut->setObjectName("using_history_shortcut");
    ui->checkBoxUseEnglishInput->setObjectName("use_english_input");
    ui->Auto_compeleted_punctuation_checkbox->setObjectName("punctuation_auto_complete");
    ui->join_the_plan_checkbox->setObjectName("join_the_plan");


    ui->cnen_switch_combo->setObjectName("key_english_input");
    ui->simple_traditional_switch_combo->setObjectName("key_jian_fan");
    ui->quan_shuang_combo->setObjectName("key_quan_shuang_pin");
    ui->toolbar_hide_switch_combo->setObjectName("key_toolbar_hide");
    ui->ocr_combo->setObjectName("key_shortcut_ocr");
    ui->voice_switch_combo->setObjectName("key_shortcut_voice");
    ui->phrase_frequence_combo->setObjectName("cirate");
    ui->character_frequence_combo->setObjectName("zirate");
    ui->shortcut_combo->setObjectName("shortcutindex");
    ui->pinyin_wubi_combo->setObjectName("key_wubi_pinyin");


    ui->cnEn_switch_checkbox->setObjectName("use_key_english_input");
    ui->quan_shuang_checkbox->setObjectName("use_key_quan_shuang_pin");
    ui->simple_traditoal_switch_checkbox->setObjectName("use_key_jian_fan");
    ui->full_half_switch_checkbox->setObjectName("use_shift_space_switch_full_half");
    ui->toolbar_hide_switch_checkbox->setObjectName("use_toolbar_hide_shortcut");
    ui->ocr_checkbox->setObjectName("use_key_shortcut_ocr");
    ui->voice_switch_checkbox->setObjectName("use_key_shortcut_voice");
    ui->pinyin_wubi_checkbox->setObjectName("use_wubi_pinyin");

    ui->name_mode_checkbox->setObjectName("enable_name_mode");
    ui->name_mode_shortcut_combo->setObjectName("key_name_mode");
    ui->name_mode_shortcut_checkbox->setObjectName("use_key_name_mode");

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
    ui->toolbar_hide_switch_combo->disconnect();
    ui->simple_traditional_switch_combo->disconnect();
    ui->post_after_switch->disconnect();
    ui->checkBoxUsingEnglishPunctuationInChineseMode->disconnect();
    ui->shortcut_combo->disconnect();
    ui->read_plan_btn->disconnect();
    ui->ocr_checkbox->disconnect();
    ui->pinyin_wubi_checkbox->disconnect();
    ui->ocr_combo->disconnect();
    ui->pinyin_wubi_combo->disconnect();
    ui->toolbar_hide_switch_checkbox->disconnect();
    ui->ocr_combo->disconnect();
    ui->voice_switch_combo->disconnect();
    ui->name_mode_shortcut_checkbox->disconnect();
    ui->name_mode_shortcut_combo->disconnect();


    int configItemValue;
    if(ConfigManager::Instance()->GetIntConfigItem("key_change_mode",configItemValue))
    {
        if (configItemValue == KEY_SWITCH_SHIFT)
        {
            ui->CnEn_Shift_RadioBtn->setChecked(true);
        }
        else if (configItemValue == KEY_SWITCH_CONTROL)
        {
            ui->CnEn_Ctrl_RadioBtn->setChecked(true);
        }
        else if (configItemValue == KEY_SWITCH_NONE)
        {
            ui->CnEn_Unused->setChecked(true);
        }
    }
    if(ConfigManager::Instance()->GetIntConfigItem("key_candidate_2nd_3rd",configItemValue))
    {
        if (configItemValue == KEY_2ND_3RD_SHIFT)
        {
            ui->Candidates_Shift_radiobtn->setChecked(true);
        }
        else if (configItemValue == KEY_2ND_3RD_CONTROL)
        {
            ui->Candidates_ctrl_raidoBtn->setChecked(true);
        }
        else if (configItemValue == KEY_2ND_3RD_NONE)
        {
            ui->Candidates_None->setChecked(true);
        }
    }
    if(ConfigManager::Instance()->GetIntConfigItem("key_pair_zi_from_word",configItemValue))
    {
        if (configItemValue & KEY_PAIR_2)
        {
            ui->citozi_bracket_radioBtn->setChecked(true);
        }
        else if (configItemValue == KEY_ZI_FROM_CI_NONE)
        {
            ui->citozi_unused_radiobtn->setChecked(true);
        }
    }

    if(ConfigManager::Instance()->GetIntConfigItem("scope_gbk",configItemValue))
    {
        if (configItemValue == HZ_SCOPE_GBK)
        {
            ui->gbk_set_radiobtn->setChecked(true);
        }
        else if (configItemValue == HZ_SCOPE_UNICODE)
        {
            ui->unicode_set_radiobtn->setChecked(true);
        }
    }
    if(ConfigManager::Instance()->GetIntConfigItem("use_hz_tone",configItemValue))
    {
        ui->UseHZToneCheckBox->setChecked(configItemValue);
    }
    if(ConfigManager::Instance()->GetIntConfigItem("english_dot_follow_number",configItemValue))
    {
         ui->symboleChangeAfterNumCheckBox->setChecked(configItemValue);
    }
    if(ConfigManager::Instance()->GetIntConfigItem("numpad_select_enabled",configItemValue))
    {
         ui->notePadToGetCandiatesCheckBox->setChecked(configItemValue);
    }
    if(ConfigManager::Instance()->GetIntConfigItem("post_after_switch",configItemValue))
    {
         ui->post_after_switch->setChecked(configItemValue);
    }
    if(ConfigManager::Instance()->GetIntConfigItem("using_english_punctuation_in_chinese_mode",configItemValue))
    {
         ui->checkBoxUsingEnglishPunctuationInChineseMode->setChecked(configItemValue);
    }
    if(ConfigManager::Instance()->GetIntConfigItem("using_history_shortcut",configItemValue))
    {
         ui->checkBoxUsingHistoryShortcut->setChecked(configItemValue);
    }
    if(ConfigManager::Instance()->GetIntConfigItem("use_english_input",configItemValue))
    {
         ui->checkBoxUseEnglishInput->setChecked(configItemValue);
    }
    if(ConfigManager::Instance()->GetIntConfigItem("punctuation_auto_complete",configItemValue))
    {
         ui->Auto_compeleted_punctuation_checkbox->setChecked(configItemValue);
    }
    if(ConfigManager::Instance()->GetIntConfigItem("join_the_plan",configItemValue))
    {
         ui->join_the_plan_checkbox->setChecked(configItemValue);
    }

    if(ConfigManager::Instance()->GetIntConfigItem("enable_name_mode",configItemValue))
    {
         ui->name_mode_checkbox->setChecked(configItemValue);
    }

    QStringList shortcutList;
    shortcutList << "无" << "Ctrl + ，" << "Ctrl + \'" << "Ctrl + /";
    ui->shortcut_combo->clear();
    ui->shortcut_combo->addItems(shortcutList);
    if(ConfigManager::Instance()->GetIntConfigItem("shortcutindex",configItemValue))
    {
         ui->shortcut_combo->setCurrentIndex(configItemValue);
    }
    if(ConfigManager::Instance()->GetIntConfigItem("cirate",configItemValue))
    {
         ui->phrase_frequence_combo->setCurrentIndex(configItemValue);
    }
    if(ConfigManager::Instance()->GetIntConfigItem("zirate",configItemValue))
    {
         ui->character_frequence_combo->setCurrentIndex(configItemValue);
    }

    InitKeyPairCandidates();
    InitShortcutWidget();
    ShortcutContentExclusive();
    ConnectSignalToSlot();
}

void AdvancedConfigStackedWidget::ConnectSignalToSlot()
{   
    QList<QCheckBox*> checkboxList;
    checkboxList << ui->UseHZToneCheckBox << ui->symboleChangeAfterNumCheckBox << ui->notePadToGetCandiatesCheckBox
                 << ui->checkBoxUsingEnglishPunctuationInChineseMode << ui->post_after_switch << ui->checkBoxUsingHistoryShortcut
                 << ui->checkBoxUseEnglishInput << ui->Auto_compeleted_punctuation_checkbox << ui->join_the_plan_checkbox
                 << ui->name_mode_checkbox;

    for(QCheckBox* index : checkboxList)
    {
        connect(index,SIGNAL(stateChanged(int)),this, SLOT(OnCheckStateChanged(int)));
    }

    connect(m_pairKeyGroup, SIGNAL(buttonToggled(int, bool)),this, SLOT(SlotKeyPairSet(int, bool)));
    connect(m_modeChangeGroup,SIGNAL(buttonToggled(int, bool)),this, SLOT(SlotEnCnSwitch(int,bool)));
    connect(m_candidates2nd3ndGroup, SIGNAL(buttonClicked(int)), this, SLOT(OnButtonGroup2nd3rdCandidateClicked(int)));
    connect(m_citoziGroup, SIGNAL(buttonClicked(int)), this, SLOT(OnButtonGroupPickWord(int)));
    connect(m_scopeGroup, SIGNAL(buttonClicked(int)), this, SLOT(OnButtonGroupOutputScope(int)));

    connect(ui->read_plan_btn, SIGNAL(clicked(bool)),this,SLOT(SlotOpenPalnContent()));
    connect(ui->set_fcitx_btn, SIGNAL(clicked(bool)), this, SLOT(ConfigFcitxSlot()));

    connect(ui->line_edit_domain, SIGNAL(textChanged(QString)), this, SLOT(onLineEditDomainTextChanged(QString)));
    for(QComboBox* indexBox : m_comboBoxVector)
    {
        connect(indexBox,SIGNAL(currentTextChanged(QString)),this,SLOT(slot_on_combo_text_changed(QString)));
    }
    connect(m_shortcutGroup,SIGNAL(buttonClicked(int)),this,SLOT(SlotShortCutSet(int)));
}

void AdvancedConfigStackedWidget::SlotEnCnSwitch(int id, bool checkStatus)
{
    if (checkStatus)
    {
        ConfigManager::Instance()->SetIntConfigItem("key_change_mode",id);
    }
}

void AdvancedConfigStackedWidget::SlotKeyPairSet(int id, bool checked)
{
    int key_pair_value;
    if(ConfigManager::Instance()->GetIntConfigItem("key_pair_candidate_page",key_pair_value))
    {
        if (checked)
        {
            key_pair_value |= id;
        }
        else
        {
            key_pair_value &= ~id;
        }
        ConfigManager::Instance()->SetIntConfigItem("key_pair_candidate_page",key_pair_value);
    }
}

void AdvancedConfigStackedWidget::OnCheckStateChanged(int checkStatus)
{
    int configValue = 0;
    if(checkStatus == Qt::Checked)
    {
        configValue = 1;
    }
    else if(checkStatus == Qt::Unchecked)
    {
        configValue = 0;
    }
    QString key = sender()->objectName();
    ConfigManager::Instance()->SetIntConfigItem(key,configValue);
}

void AdvancedConfigStackedWidget::InitKeyPairCandidates()
{

    int keyPair;
    if(ConfigManager::Instance()->GetIntConfigItem("key_pair_candidate_page",keyPair))
    {
        (keyPair & KEY_PAIR_1)
                ? ui->PageTurn_dot_checkbox->setChecked(true)
                : ui->PageTurn_dot_checkbox->setChecked(false);

        ui->PageTurn_minusandplus_checkbox->setChecked(true);
        ui->PageTurn_minusandplus_checkbox->setEnabled(false);

        ui->PageTurn_upanddown_checkbox->setChecked(true);
        ui->PageTurn_upanddown_checkbox->setEnabled(false);

        (keyPair & KEY_PAIR_2)
                ? ui->PageTurn_bracket_left_and_right_checkbox->setChecked(true)
                : ui->PageTurn_bracket_left_and_right_checkbox->setChecked(false);
    }

}

void AdvancedConfigStackedWidget::OnButtonGroup2nd3rdCandidateClicked(int id)
{
    ConfigManager::Instance()->SetIntConfigItem("key_candidate_2nd_3rd",id);
}

void AdvancedConfigStackedWidget::OnButtonGroupPickWord(int id)
{
    ConfigManager::Instance()->SetIntConfigItem("key_pair_zi_from_ci",id);
}

void AdvancedConfigStackedWidget::OnButtonGroupOutputScope(int id)
{
    ConfigManager::Instance()->SetIntConfigItem("scope_gbk",id);
}

void AdvancedConfigStackedWidget::fillComboBoxA2Z(QComboBox *comboBox)
{
    if (!comboBox)
    {
        return;
    }
    comboBox->clear();

    for (char i = 'A'; i <= 'Z'; ++i)
    {
        comboBox->addItem(QString(i));
    }
}

void AdvancedConfigStackedWidget::InitShortcutWidget()
{
    ui->cnen_switch_combo->clear();
    ui->quan_shuang_combo->clear();
    ui->simple_traditional_switch_combo->clear();
    ui->pinyin_wubi_combo->clear();
    ui->toolbar_hide_switch_combo->clear();
    ui->ocr_combo->clear();
    ui->voice_switch_combo->clear();

    fillComboBoxA2Z(ui->cnen_switch_combo);
    fillComboBoxA2Z(ui->quan_shuang_combo);
    fillComboBoxA2Z(ui->simple_traditional_switch_combo);
    fillComboBoxA2Z(ui->pinyin_wubi_combo);
    fillComboBoxA2Z(ui->ocr_combo);
    fillComboBoxA2Z(ui->voice_switch_combo);
    fillComboBoxA2Z(ui->toolbar_hide_switch_combo);
    fillComboBoxA2Z(ui->name_mode_shortcut_combo);


    int configItemValue;
    if(ConfigManager::Instance()->GetIntConfigItem("use_key_english_input",configItemValue))
    {
        configItemValue ? ui->cnEn_switch_checkbox->setChecked(true) : ui->cnEn_switch_checkbox->setChecked(false);
        configItemValue ? ui->cnen_switch_combo->setEnabled(true) : ui->cnen_switch_combo->setEnabled(false);
    }
    if(ConfigManager::Instance()->GetIntConfigItem("key_english_input",configItemValue))
    {
        int index = ui->cnen_switch_combo->findText(QString::fromLatin1((char*)&configItemValue, 1));
        if (index == -1)
        {
            index = 0;
        }
        ui->cnen_switch_combo->setCurrentIndex(index);
    }

    if(ConfigManager::Instance()->GetIntConfigItem("use_key_quan_shuang_pin",configItemValue))
    {
        configItemValue?ui->quan_shuang_checkbox->setChecked(true):ui->quan_shuang_checkbox->setChecked(false);
        configItemValue?ui->quan_shuang_combo->setEnabled(true):ui->quan_shuang_combo->setEnabled(false);
    }
    if(ConfigManager::Instance()->GetIntConfigItem("key_quan_shuang_pin",configItemValue))
    {
        int index = ui->quan_shuang_combo->findText(QString::fromLatin1((char*)&configItemValue, 1));
        if (index == -1)
        {
            index = 0;
        }
        ui->quan_shuang_combo->setCurrentIndex(index);
    }

    if(ConfigManager::Instance()->GetIntConfigItem("use_key_jian_fan",configItemValue))
    {
        configItemValue?ui->simple_traditoal_switch_checkbox->setChecked(true):ui->simple_traditoal_switch_checkbox->setChecked(false);
        configItemValue?ui->simple_traditional_switch_combo->setEnabled(true):ui->simple_traditional_switch_combo->setEnabled(false);
    }
    if(ConfigManager::Instance()->GetIntConfigItem("key_jian_fan",configItemValue))
    {
        int index = ui->simple_traditional_switch_combo->findText(QString::fromLatin1((char*)&configItemValue, 1));
        if (index == -1)
        {
            index = 0;
        }
        ui->simple_traditional_switch_combo->setCurrentIndex(index);
    }

    if(ConfigManager::Instance()->GetIntConfigItem("use_shift_space_switch_full_half",configItemValue))
    {
        configItemValue? ui->full_half_switch_checkbox->setChecked(true):ui->full_half_switch_checkbox->setChecked(false);
    }

    if(ConfigManager::Instance()->GetIntConfigItem("use_toolbar_hide_shortcut",configItemValue))
    {
        if(configItemValue)
        {
            ui->toolbar_hide_switch_checkbox->setChecked(true);
            ui->toolbar_hide_switch_combo->setEnabled(true);
        }
        else
        {
            ui->full_half_switch_checkbox->setChecked(false);
            ui->toolbar_hide_switch_combo->setEnabled(false);
        }
    }
    if(ConfigManager::Instance()->GetIntConfigItem("key_toolbar_hide",configItemValue))
    {
        int index = ui->toolbar_hide_switch_combo->findText(QString::fromLatin1((char*)&configItemValue, 1));
        if (index == -1)
        {
            index = 0;
        }
        ui->toolbar_hide_switch_combo->setCurrentIndex(index);
    }

    if(ConfigManager::Instance()->GetIntConfigItem("use_wubi_pinyin",configItemValue))
    {
        if(configItemValue)
        {
            ui->pinyin_wubi_checkbox->setChecked(true);
            ui->pinyin_wubi_combo->setEnabled(true);
        }
        else
        {
            ui->pinyin_wubi_checkbox->setChecked(false);
            ui->pinyin_wubi_combo->setEnabled(false);
        }
    }
    if(ConfigManager::Instance()->GetIntConfigItem("key_wubi_pinyin",configItemValue))
    {
        int index = ui->pinyin_wubi_combo->findText(QString::fromLatin1((char*)&configItemValue, 1));
        if (index == -1)
        {
            index = 0;
        }
        ui->pinyin_wubi_combo->setCurrentIndex(index);
    }

    if(ConfigManager::Instance()->GetIntConfigItem("use_key_shortcut_ocr",configItemValue))
    {
        if(configItemValue)
        {
            ui->ocr_checkbox->setChecked(true);
            ui->ocr_combo->setEnabled(true);
        }
        else
        {
            ui->ocr_checkbox->setChecked(false);
            ui->ocr_combo->setEnabled(false);
        }
    }
    if(ConfigManager::Instance()->GetIntConfigItem("key_shortcut_ocr",configItemValue))
    {
        int index = ui->ocr_combo->findText(QString::fromLatin1((char*)&configItemValue, 1));
        if (index == -1)
        {
            index = 0;
        }
        ui->ocr_combo->setCurrentIndex(index);
    }

    if(ConfigManager::Instance()->GetIntConfigItem("use_key_shortcut_voice",configItemValue))
    {
        if(configItemValue)
        {
            ui->voice_switch_checkbox->setChecked(true);
            ui->voice_switch_combo->setEnabled(true);
        }
        else
        {
            ui->voice_switch_checkbox->setChecked(false);
            ui->voice_switch_combo->setEnabled(false);
        }

    }
    if(ConfigManager::Instance()->GetIntConfigItem("key_shortcut_voice",configItemValue))
    {
        int index = ui->voice_switch_combo->findText(QString::fromLatin1((char*)&configItemValue, 1));
        if (index == -1)
        {
            index = 0;
        }
        ui->voice_switch_combo->setCurrentIndex(index);
    }

    if(ConfigManager::Instance()->GetIntConfigItem("use_key_name_mode",configItemValue))
    {
        if(configItemValue)
        {
            ui->name_mode_shortcut_checkbox->setChecked(true);
            ui->name_mode_shortcut_combo->setEnabled(true);
        }
        else
        {
            ui->name_mode_shortcut_checkbox->setChecked(false);
            ui->name_mode_shortcut_combo->setEnabled(false);
        }
    }

    if(ConfigManager::Instance()->GetIntConfigItem("key_name_mode",configItemValue))
    {
        int index = ui->name_mode_shortcut_combo->findText(QString::fromLatin1((char*)&configItemValue, 1));
        if (index == -1)
        {
            index = 0;
        }
        ui->name_mode_shortcut_combo->setCurrentIndex(index);
    }

    bool is_ocr_exist = AddonConfig::Instance()->IsAddonExist("OCR");
    bool is_voice_exist = AddonConfig::Instance()->IsAddonExist("AsrInput");
    if(!is_ocr_exist)
    {
        ui->ocr_checkbox->setVisible(false);
        ui->ocr_label->setVisible(false);
        ui->ocr_combo->setVisible(false);
    }
    if(!is_voice_exist)
    {
        ui->voice_switch_checkbox->setVisible(false);
        ui->voice_switch_label->setVisible(false);
        ui->voice_switch_combo->setVisible(false);
    }
}


void AdvancedConfigStackedWidget::SlotOpenPalnContent()
{
    QString server_url = ConfigManager::Instance()->GetServerUrl();
    QString forget_pwd_url = QString("%1%2").arg(server_url).arg("/privacy.html");
    QDesktopServices::openUrl(QUrl(forget_pwd_url));
}

void AdvancedConfigStackedWidget::ResetConfigInfo()
{

    QStringList key_list;
    key_list << "key_change_mode" << "key_candidate_2nd_3rd" << "key_pair_zi_from_word"
             << "scope_gbk" << "use_hz_tone" << "english_dot_follow_number" << "numpad_select_enabled"
             << "post_after_switch" <<"using_english_punctuation_in_chinese_mode" << "using_history_shortcut"
             << "use_english_input" <<  "punctuation_auto_complete" << "shortcutindex" << "zirate" << "cirate"
             << "key_pair_candidate_page" << "use_key_english_input" << "key_english_input"
             << "use_key_quan_shuang_pin" << "key_quan_shuang_pin" << "use_key_jian_fan" << "key_jian_fan"
             << "use_shift_space_switch_full_half" << "use_toolbar_hide_shortcut" << "key_toolbar_hide"
             << "use_wubi_pinyin" << "key_wubi_pinyin" << "use_key_shortcut_ocr" << "key_shortcut_ocr"
             << "use_key_shortcut_voice" << "key_shortcut_voice" << "join_the_plan"
             << "enable_name_mode" << "use_key_name_mode" << "key_name_mode";

    for(QString index :key_list)
    {
        ConfigManager::Instance()->ClearConfigItem(index);
    }
    LoadConfigInfo();
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


void AdvancedConfigStackedWidget::ConfigFcitxSlot()
{
#ifdef USE_UOS
    //Config::Instance()->OpenUOSFcitxConfig();
    ConfigBus::instance()->valueChanged("openFcitxConfig",1);
#else
    QFile fcitx_config("/usr/bin/fcitx-configtool");
    if(!fcitx_config.exists())
    {
        QMessageBox::warning(this,"警告","fcitx配置工具不存在！");
        return;
    }
    QProcess::startDetached("/usr/bin/fcitx-configtool");
#endif
}

void AdvancedConfigStackedWidget::onLineEditDomainTextChanged(QString text)
{
    //Config::Instance()->set_domain(text);
}


void AdvancedConfigStackedWidget::slot_on_combo_text_changed(QString comboText)
{
    QString objectName = sender()->objectName();
    QList<QComboBox*> child_combo_list;
    child_combo_list << ui->phrase_frequence_combo << ui->character_frequence_combo << ui->shortcut_combo;

    QComboBox* senderCombo = (QComboBox*)sender();
    int index = senderCombo->findText(comboText);

    if(child_combo_list.contains((QComboBox*)sender()))
    {
        ConfigManager::Instance()->SetIntConfigItem(objectName,index);
    }
    else
    {
        ConfigManager::Instance()->SetIntConfigItem(objectName,comboText.toLatin1().at(0));
    }
}

void AdvancedConfigStackedWidget::SlotShortCutSet(int id)
{
    QCheckBox* currentCheckBox = (QCheckBox*)m_shortcutGroup->button(id);
    int status = currentCheckBox->checkState();
    QString objectName = currentCheckBox->objectName();
    ConfigManager::Instance()->SetIntConfigItem(objectName,status);

    switch(id)
    {
    case 0:
        break;
    case 1:
       (status == Qt::Checked)? ui->cnen_switch_combo->setEnabled(true):ui->cnen_switch_combo->setEnabled(false);

        break;
    case 2:
        (status == Qt::Checked)? ui->quan_shuang_combo->setEnabled(true):ui->quan_shuang_combo->setEnabled(false);
        break;
    case 3:
        (status == Qt::Checked)? ui->simple_traditional_switch_combo->setEnabled(true):ui->simple_traditional_switch_combo->setEnabled(false);
        break;
    case 4:
        break;
    case 5:
        ui->toolbar_hide_switch_combo->setEnabled(status);
        break;
    case 6:
        (status == Qt::Checked)? ui->ocr_combo->setEnabled(true):ui->ocr_combo->setEnabled(false);
        break;
    case 7:
        (status == Qt::Checked)? ui->voice_switch_combo->setEnabled(true):ui->voice_switch_combo->setEnabled(false);
        break;
    case 8:
        (status == Qt::Checked)? ui->pinyin_wubi_combo->setEnabled(true):ui->pinyin_wubi_combo->setEnabled(false);

    case 9:
        (status == Qt::Checked)? ui->name_mode_shortcut_combo->setEnabled(true):ui->name_mode_shortcut_combo->setEnabled(false);
        break;
    }
}

void AdvancedConfigStackedWidget::slot_scroll_to_hotkey()
{
    int target_pos = this->height() * 105 / 300;
    ui->scrollArea->verticalScrollBar()->setSliderPosition(target_pos);
}
