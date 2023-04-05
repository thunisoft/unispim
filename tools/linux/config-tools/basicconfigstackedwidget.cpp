#include "basicconfigstackedwidget.h"
#include "ui_basicconfigstackedwidget.h"
#include "suggestsetwidget.h"
#include "fuzzydialog.h"
#include "addoreditphrasedialog.h"
#include "toviewphrasedialog.h"
#include "fuzzydialog.h"
#include "suggestsetwidget.h"
#include "toviewphrasedialog.h"

#include "spschemeviewdlg.h"
#include <QButtonGroup>
#include <QListView>
#include <QDir>
#include <QScreen>
#include <QGraphicsEffect>
#include <QList>
#include <QStandardItemModel>
#include "../public/configmanager.h"

BasicConfigStackedWidget::BasicConfigStackedWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BasicConfigStackedWidget)
{
    ui->setupUi(this);
    InitWidget();
    LoadConfigInfo();
}

BasicConfigStackedWidget::~BasicConfigStackedWidget()
{
    delete ui;
}

void BasicConfigStackedWidget::InitWidget()
{
    //ui->sp_set_btn->setProperty("type","normal-set-btn");
    ui->word_suggest_btn->setProperty("type","normal-set-btn");
    ui->fuzzy_set_btn->setProperty("type","normal-set-btn");
    ui->fast_reply_btn->setProperty("type","normal-set-btn");
    ui->modify_sp_btn->setProperty("type","normal-set-btn");
    ui->super_simple_tip->setProperty("type","tip");
    ui->normal_set_label->setProperty("type","h1");
    ui->smart_option_label->setProperty("type","h1");
    ui->fast_reply_label->setProperty("type","h1");
    ui->pinyin_mode_label->setProperty("type","h1");
    ui->min_characters_label->setProperty("type", "h2");

    this->setProperty("type","stackedwidget");
    ui->scrollArea->setBackgroundRole(QPalette::Light);


    QVector<int> labelLayout= {21,16,1,77,454};

    for(int index=0; index<labelLayout.size(); ++index)
    {
        ui->labelLayout->setStretch(index,labelLayout.at(index));
        ui->label_layout->setStretch(index,labelLayout.at(index));
        ui->label_layout_2->setStretch(index,labelLayout.at(index));
        ui->labelLayout_2->setStretch(index,labelLayout.at(index));
    }


    ui->min_character_combo->setView(new QListView());
    ui->min_character_combo->setProperty("type","combo");
    ui->min_character_combo->setAutoFillBackground(true);
    ui->min_character_combo->setMaxVisibleItems(6);


    QStringList min_character_combo_list;
    min_character_combo_list << "2" << "3" << "4" << "5" << "6" << "7" << "8";
    ui->min_character_combo->addItems(min_character_combo_list);
    ui->min_character_combo->installEventFilter(this);


    QString superSimpleTip = "输入词时，键入每个字的拼音首字母即可,比声母组合又进一步";
    ui->super_simple_tip->setToolTip(superSimpleTip);

    m_chorenGroup = new QButtonGroup(this);
    m_chorenGroup->addButton(ui->ch_raidobtn, 0);
    m_chorenGroup->addButton(ui->en_radiobutton, 1);

    m_simpleOrTraditionalGroup = new QButtonGroup(this);
    m_simpleOrTraditionalGroup->addButton(ui->radio_button_hz_output_simplified, HZ_OUTPUT_SIMPLIFIED);
    m_simpleOrTraditionalGroup->addButton(ui->radio_button_hz_output_traditional, HZ_OUTPUT_TRADITIONAL);


    m_button_group_default_chinese_input_mode = new QButtonGroup(this);
    m_button_group_default_chinese_input_mode->addButton(
                ui->radio_button_default_chinese_input_mode_pinyin,
                DEFAULT_CHINESE_INPUT_MODE_PINYIN);
    m_button_group_default_chinese_input_mode->addButton(
                ui->radio_button_default_chinese_input_mode_wubi,
                DEFAULT_CHINESE_INPUT_MODE_WUBI);
    m_button_group_default_chinese_input_mode->setExclusive(true);

    m_inputModeGroup = new QButtonGroup(this);
    m_inputModeGroup->addButton(ui->shuangpin_radioBtn,0);
    m_inputModeGroup->addButton(ui->quanpin_radioBtn, 1);

    m_halfOrFullGroup = new QButtonGroup(this);
    m_halfOrFullGroup->addButton(ui->half_raidobtn,0);
    m_halfOrFullGroup->addButton(ui->full_radiobutton,1);

    m_fuzzyDlg = new FuzzyDialog(this);
    m_suggestSetDlg = new SuggestsetWidget(this);
    m_spview_dlg = new SPSchemeViewDlg(this);




    ui->super_simple_checkbox->setEnabled(false);

}

void BasicConfigStackedWidget::ConnectSignalToSlot()
{
    connect(ui->fuzzy_set_btn, SIGNAL(clicked()), this, SLOT(SlotShowFuzzyDlg()));

    connect(ui->word_suggest_btn, SIGNAL(clicked()), this, SLOT(SlotShowWordSuggestDlg()));
    connect(ui->fast_reply_btn, SIGNAL(clicked()), this, SLOT(SlotFastReplySet()));

    connect(ui->suggest_checkbox, SIGNAL(stateChanged(int)), this, SLOT(SlotEnableSuggestWord(int)));
    connect(ui->super_simple_checkbox, SIGNAL(stateChanged(int)), this, SLOT(SlotEnableSuperSimple(int)));
    connect(ui->min_character_combo, SIGNAL(currentIndexChanged(int)),this,SLOT(SlotChangeMinLetters(int)));

    connect(ui->fuzzy_checkbox,SIGNAL(stateChanged(int)),this, SLOT(SlotEnableFuzzyMode(int)));
    connect(ui->fast_reply_checkbox, SIGNAL(stateChanged(int)), this, SLOT(SlotEnableSpwMode(int)));
    connect(ui->fuzzy_checkbox, SIGNAL(stateChanged(int)),this,SLOT(SlotFuzzyEnabled(int)));

    connect(m_halfOrFullGroup, SIGNAL(buttonToggled(int, bool)), this, SLOT(SlotHalfORFullMode(int,bool)));
    connect(m_inputModeGroup, SIGNAL(buttonToggled(int, bool)), this,SLOT(OnInputModeGroupToggled(int,bool)));
    connect(m_simpleOrTraditionalGroup, SIGNAL(buttonToggled(int, bool)), this, SLOT(SlotSimpleORTraditional(int,bool)));
    connect(m_chorenGroup,SIGNAL(buttonToggled(int, bool)), this, SLOT(SlotEnORChMode(int,bool)));
    connect(ui->modify_sp_btn, SIGNAL(clicked()), this, SLOT(SlotEditSPScheme()));

    connect(m_button_group_default_chinese_input_mode, SIGNAL(buttonToggled(int,bool)),
            this, SLOT(on_button_group_default_chinese_input_mode_button_toggled(int, bool)));
    connect(ui->hide_toobar_checkbox, SIGNAL(stateChanged(int)), this, SLOT(SlotHideToolbarStateChanged(int)));
}

void BasicConfigStackedWidget::LoadConfigInfo()
{
    ui->fuzzy_set_btn->disconnect();
    ui->word_suggest_btn->disconnect();
    ui->fast_reply_btn->disconnect();
    ui->suggest_checkbox->disconnect();
    ui->super_simple_checkbox->disconnect();
    ui->fuzzy_checkbox->disconnect();
    ui->fast_reply_checkbox->disconnect();
    ui->fuzzy_checkbox->disconnect();
    ui->min_character_combo->disconnect();
    m_inputModeGroup->disconnect();
    m_simpleOrTraditionalGroup->disconnect();
    m_chorenGroup->disconnect();
    ui->modify_sp_btn->disconnect();

    int startup_with_english_input;
    if(ConfigManager::Instance()->GetIntConfigItem("startup_with_english_input",startup_with_english_input))
    {
        startup_with_english_input? ui->en_radiobutton->setChecked(true) : ui->ch_raidobtn->setChecked(true);
    }

    int hz_output_mode;
    if(ConfigManager::Instance()->GetIntConfigItem("hz_output_mode",hz_output_mode))
    {
        if (hz_output_mode == HZ_OUTPUT_SIMPLIFIED)
        {
            ui->radio_button_hz_output_simplified->setChecked(true);
        }
        else if (hz_output_mode == HZ_OUTPUT_TRADITIONAL)
        {
            ui->radio_button_hz_output_traditional->setChecked(true);
        }
    }

    int pinyinMode;
    if(ConfigManager::Instance()->GetIntConfigItem("pinyin_mode",pinyinMode))
    {
        (pinyinMode == 0)?ui->quanpin_radioBtn->setChecked(true):ui->shuangpin_radioBtn->setChecked(true);
        (pinyinMode == 0)?ui->modify_sp_btn->setVisible(false):ui->modify_sp_btn->setVisible(true);
    }


    int suggesetFlag;
    if(ConfigManager::Instance()->GetIntConfigItem("enablesuggestion",suggesetFlag))
    {
        suggesetFlag?ui->suggest_checkbox->setCheckState(Qt::Checked):ui->suggest_checkbox->setCheckState(Qt::Unchecked);
        (suggesetFlag)?ui->word_suggest_btn->setVisible(true):ui->word_suggest_btn->setVisible(false);
    }

    //超级简拼
    ui->min_character_combo->setEnabled(true);
    ui->super_simple_checkbox->setChecked(true);

    int first_letter_input_min_hz;
    if(ConfigManager::Instance()->GetIntConfigItem("first_letter_input_min_hz",first_letter_input_min_hz))
    {
        int index = ui->min_character_combo->findText(QString::number(first_letter_input_min_hz));
        ui->min_character_combo->setCurrentIndex(index);
    }
    int configItemValue;
    if(ConfigManager::Instance()->GetIntConfigItem("hide_tray",configItemValue))
    {
        ui->hide_toobar_checkbox->setChecked(configItemValue ? Qt::Checked : Qt::Unchecked);
    }
    int enablefuzzy;
    if(ConfigManager::Instance()->GetIntConfigItem("enablefuzzy",enablefuzzy))
    {
        enablefuzzy? ui->fuzzy_checkbox->setCheckState(Qt::Checked):ui->fuzzy_checkbox->setCheckState(Qt::Unchecked);
        ui->fuzzy_set_btn->setAttribute(Qt::WA_TranslucentBackground);
        (enablefuzzy)?ui->fuzzy_set_btn->setVisible(true):ui->fuzzy_set_btn->setVisible(false);
    }

    int useCustomizePhrase;
    if(ConfigManager::Instance()->GetIntConfigItem("enableCustomPhrase",useCustomizePhrase))
    {
        useCustomizePhrase?ui->fast_reply_checkbox->setCheckState(Qt::Checked):ui->fast_reply_checkbox->setCheckState(Qt::Unchecked);
        useCustomizePhrase?ui->fast_reply_btn->setVisible(true):ui->fast_reply_btn->setVisible(false);
    }


    int default_chinese_input_mode;
    if(ConfigManager::Instance()->GetIntConfigItem("default_chinese_input_mode",default_chinese_input_mode))
    {
        if (default_chinese_input_mode == DEFAULT_CHINESE_INPUT_MODE_PINYIN)
        {
            ui->radio_button_default_chinese_input_mode_pinyin->setChecked(true);
        }
        else
        {
            ui->radio_button_default_chinese_input_mode_wubi->setChecked(true);
        }
    }

    int hz_option;
    if(ConfigManager::Instance()->GetIntConfigItem("hz_option",hz_option))
    {
        if(hz_option & HZ_SYMBOL_HALFSHAPE)
        {
            ui->half_raidobtn->setChecked(true);
        }
        else
        {
            ui->full_radiobutton->setChecked(true);
        }
    }

    ConnectSignalToSlot();

}

void BasicConfigStackedWidget::SlotShowFuzzyDlg()
{
    int fuzzyConfig;
    ConfigManager::Instance()->GetIntConfigItem("fuzzyconfig",fuzzyConfig);
    m_fuzzyDlg->SetInitValue(fuzzyConfig);
    m_fuzzyDlg->exec();
}


void BasicConfigStackedWidget::SlotShowWordSuggestDlg()
{
    m_suggestSetDlg->ReloadWidget();
    m_suggestSetDlg->exec();
}


void BasicConfigStackedWidget::SlotEnORChMode(int id, bool state)
{
    if (state)
    {
        ConfigManager::Instance()->SetIntConfigItem("startup_with_english_input",id);
    }
}

void BasicConfigStackedWidget::SlotSimpleORTraditional(int id, bool state)
{
    if (state)
    {
        ConfigManager::Instance()->SetIntConfigItem("hz_output_mode",id);
    }
}

void BasicConfigStackedWidget::SlotHalfORFullMode(int id, bool state)
{
    if (state)
    {
        int hz_option;
        if(ConfigManager::Instance()->GetIntConfigItem("hz_option",hz_option))
        {
            if (id == 0)
            {
                hz_option |= HZ_SYMBOL_HALFSHAPE;
            }
            else
            {
                hz_option &= ~HZ_SYMBOL_HALFSHAPE;
            }
            ConfigManager::Instance()->SetIntConfigItem("hz_option",hz_option);
        }

    }
}

void BasicConfigStackedWidget::OnInputModeGroupToggled(int id, bool state)
{
    if((id == 0) && state)
    {
        ConfigManager::Instance()->SetIntConfigItem("pinyin_mode",1);
        ui->modify_sp_btn->setVisible(true);
    }
    if((id == 1) && state)
    {
         ConfigManager::Instance()->SetIntConfigItem("pinyin_mode",0);
        ui->modify_sp_btn->setVisible(false);
    }
}



void BasicConfigStackedWidget::SlotEnableSuggestWord(int flag)
{
    if(flag == Qt::Checked)
    {
        ConfigManager::Instance()->SetIntConfigItem("enablesuggestion",1);
        ui->word_suggest_btn->setVisible(true);
    }
    else
    {
        ConfigManager::Instance()->SetIntConfigItem("enablesuggestion",0);
        ui->word_suggest_btn->setVisible(false);
    }
}

void BasicConfigStackedWidget::SlotEnableSuperSimple(int flag)
{

}

void BasicConfigStackedWidget::SlotEnableFuzzyMode(int flag)
{
    ui->fuzzy_set_btn->setAttribute(Qt::WA_TranslucentBackground);
    if(flag == Qt::Checked)
    {

        ui->fuzzy_set_btn->setVisible(true);

    }
    else
    {
        ui->fuzzy_set_btn->setVisible(false);

    }
    ConfigManager::Instance()->SetIntConfigItem("enablefuzzy",(flag==Qt::Checked));
}

void BasicConfigStackedWidget::SlotEnableSpwMode(int flag)
{
    if(flag == Qt::Checked)
    {
        ui->fast_reply_btn->setVisible(true);
    }
    else
    {
        ui->fast_reply_btn->setVisible(false);
    }
    ConfigManager::Instance()->SetIntConfigItem("enableCustomPhrase",(flag==Qt::Checked));
}

void BasicConfigStackedWidget::SlotFastReplySet()
{
    ToViewPhraseDialog dlg(this);
    dlg.exec();
}

void BasicConfigStackedWidget::SlotFuzzyEnabled(int state)
{
    ConfigManager::Instance()->SetIntConfigItem("enablefuzzy",(state==Qt::Checked));
}

void BasicConfigStackedWidget::ResetConfigInfo()
{
    QStringList key_list;
    key_list << "first_letter_input_min_hz" << "startup_with_english_input" << "hz_output_mode"
             << "pinyin_mode" << "enablesuggestion" << "enablefuzzy" << "enableCustomPhrase"
             << "default_chinese_input_mode" << "hz_option" << "fuzzyconfig"
             << "suggestsyllablelocation" << "suggestwordlocation" << "suggestwordcount"
             << "hide_tray";

    for(QString key : key_list)
    {
        ConfigManager::Instance()->ClearConfigItem(key);
    }
    LoadConfigInfo();
}

void BasicConfigStackedWidget::SlotChangeMinLetters(int index)
{
    QString text = ui->min_character_combo->itemText(index);
    ConfigManager::Instance()->SetIntConfigItem("first_letter_input_min_hz",text.toInt());
}

void BasicConfigStackedWidget::SlotEditSPScheme()
{
    m_spview_dlg->RefreshTheSchemeList();
    m_spview_dlg->LoadTheConfig();
    m_spview_dlg->exec();
}

bool BasicConfigStackedWidget::eventFilter(QObject *obj, QEvent *event)
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

void BasicConfigStackedWidget::on_button_group_default_chinese_input_mode_button_toggled(int id, bool checked)
{
    if (checked)
    {
        ConfigManager::Instance()->SetIntConfigItem("default_chinese_input_mode",id);
    }
}


void BasicConfigStackedWidget::SlotHideToolbarStateChanged(int checkStatus)
{
    ConfigManager::Instance()->SetIntConfigItem("hide_tray",checkStatus);
}
