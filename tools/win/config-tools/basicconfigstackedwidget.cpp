#include "basicconfigstackedwidget.h"
#include "ui_basicconfigstackedwidget.h"
#include "spsetplandlg.h"
#include "suggestsetwidget.h"
#include "fuzzydialog.h"
#include "addoreditphrasedialog.h"
#include "toviewphrasedialog.h"
#include "config.h"
#include "fuzzydialog.h"
#include "suggestsetwidget.h"
#include "spsetplandlg.h"
#include "toviewphrasedialog.h"

#include "dpiadaptor.h"
#include "spschemeviewdlg.h"
#include <QButtonGroup>
#include <QListView>
#include <QDir>
#include <QScreen>
#include <QGraphicsEffect>
#include <QList>
#include <QStandardItemModel>

BasicConfigStackedWidget::BasicConfigStackedWidget(QWidget *parent) :
    BaseStackedWidget(parent),
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

    QList<QPushButton*> btn_list;
    btn_list << ui->word_suggest_btn << ui->fuzzy_set_btn << ui->modify_sp_btn << ui->fast_reply_btn;
    for(QPushButton* btn : btn_list){
        btn->setProperty("type","normal-set-btn");
        btn->setCursor(Qt::PointingHandCursor);
    }


    ui->super_simple_tip->setProperty("type","tip");
    ui->normal_set_label->setProperty("type","h1");
    ui->smart_option_label->setProperty("type","h1");
    ui->fast_reply_label->setProperty("type","h1");
    ui->pinyin_mode_label->setProperty("type","h1");
    this->setProperty("type","stackedwidget");
    ui->scrollArea->setBackgroundRole(QPalette::Light);
    QVector<int> labelLayout;
    bool isOk = Config::Instance()->GetLayoutVector("labellayout",labelLayout);
    if(isOk)
    {
        for(int index=0; index<labelLayout.size(); ++index)
        {
            ui->labelLayout->setStretch(index,labelLayout.at(index));
            ui->label_layout->setStretch(index,labelLayout.at(index));
            ui->label_layout_2->setStretch(index,labelLayout.at(index));
            ui->labelLayout_2->setStretch(index,labelLayout.at(index));
        }
    }
    ui->min_character_combo->setView(new QListView());
    ui->min_character_combo->setProperty("type","combo");
    ui->min_character_combo->setAutoFillBackground(true);
    ui->min_character_combo->setMaxVisibleItems(6);


    QFont font("微软雅黑",10);
    ui->min_characters_label->setFont(font);

    QStringList min_character_combo_list;
    min_character_combo_list << "2" << "3" << "4" << "5" << "6" << "7" << "8";
    ui->min_character_combo->addItems(min_character_combo_list);
    ui->min_character_combo->installEventFilter(this);


    QString superSimpleTip = "输入词时，键入每个字的拼音首字母即可,比声母组合又进一步";
    ui->super_simple_tip->setToolTip(superSimpleTip);

    m_chorenGroup = new QButtonGroup(this);
    m_chorenGroup->addButton(ui->ch_raidobtn,0);
    m_chorenGroup->addButton(ui->en_radiobutton,1);


    m_simpleOrTraditionalGroup = new QButtonGroup(this);
    m_simpleOrTraditionalGroup->addButton(ui->simple_radiobtn,0);
    m_simpleOrTraditionalGroup->addButton(ui->traditional_radiobtn,1);

    m_halfOrFullGroup = new QButtonGroup(this);
    m_halfOrFullGroup->addButton(ui->half_radiobtn,0);
    m_halfOrFullGroup->addButton(ui->full_radiobtn,1);


    m_inputModeGroup = new QButtonGroup(this);
    m_inputModeGroup->addButton(ui->shuangpin_radioBtn,0);
    m_inputModeGroup->addButton(ui->quanpin_radioBtn, 1);

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
    connect(ui->hide_status_bar_check, SIGNAL(stateChanged(int)), this, SLOT(SlotHideToolbarStateChanged(int)));
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
    m_halfOrFullGroup->disconnect();
    m_inputModeGroup->disconnect();
    m_simpleOrTraditionalGroup->disconnect();
    m_chorenGroup->disconnect();
    ui->modify_sp_btn->disconnect();
    ui->hide_status_bar_check->disconnect();

    ConfigItemStruct configInfo;

    if(Config::Instance()->GetConfigItemByJson("startup_with_english_input",configInfo))
    {
        (configInfo.itemCurrentIntValue)? ui->en_radiobutton->setChecked(true): ui->ch_raidobtn->setChecked(true);

    }
    else
    {
        ui->ch_raidobtn->setChecked(true);
    }

    if(Config::Instance()->GetConfigItemByJson("hz_output_mode",configInfo))
    {
       (configInfo.itemCurrentIntValue & HZ_OUTPUT_SIMPLIFIED)?ui->simple_radiobtn->setChecked(true):ui->traditional_radiobtn->setChecked(true);
    }
    else
    {
        ui->simple_radiobtn->setChecked(true);
    }

    //汉字配置
    if(Config::Instance()->GetConfigItemByJson("hz_option",configInfo))
    {
        (configInfo.itemCurrentIntValue & HZ_SYMBOL_HALFSHAPE)?ui->half_radiobtn->setChecked(true):ui->full_radiobtn->setChecked(true);
    }
    else
    {
        ui->half_radiobtn->setChecked(true);
    }

    //拼音模式
    if(Config::Instance()->GetConfigItemByJson("pinyin_mode",configInfo))
    {
        (configInfo.itemCurrentIntValue == PINYIN_QUANPIN)?ui->quanpin_radioBtn->setChecked(true):ui->shuangpin_radioBtn->setChecked(true);
        (configInfo.itemCurrentIntValue == PINYIN_QUANPIN)?ui->modify_sp_btn->setVisible(false):ui->modify_sp_btn->setVisible(true);
    }
    else
    {
        ui->quanpin_radioBtn->setChecked(true);
        ui->modify_sp_btn->setVisible(false);
    }

    //词语联想
    if(Config::Instance()->GetConfigItemByJson("use_word_suggestion",configInfo))
    {
        configInfo.itemCurrentIntValue?ui->suggest_checkbox->setCheckState(Qt::Checked):ui->suggest_checkbox->setCheckState(Qt::Unchecked);

        int flag = configInfo.itemCurrentIntValue?Qt::Checked:Qt::Unchecked;

        if(flag == Qt::Checked)
        {
            ui->word_suggest_btn->setVisible(true);
        }
        else
        {
            ui->word_suggest_btn->setVisible(false);
        }
    }
    else
    {
        ui->suggest_checkbox->setCheckState(Qt::Checked);
        ui->word_suggest_btn->setVisible(true);
    }

    //超级简拼
    if(Config::Instance()->GetConfigItemByJson("ci_option",configInfo))
    {
        if(configInfo.itemCurrentIntValue & CI_USE_FIRST_LETTER)
        {
            ui->super_simple_checkbox->setChecked(true);
            ui->min_character_combo->setEnabled(true);
        }
        else
        {
            ui->min_character_combo->setEnabled(false);
            ui->super_simple_checkbox->setChecked(false);
        }
    }
    else
    {
         ui->min_character_combo->setEnabled(true);
         ui->super_simple_checkbox->setChecked(true);
    }

    if(Config::Instance()->GetConfigItemByJson("first_letter_input_min_hz",configInfo))
    {
        QString currentStr = QString::number(configInfo.itemCurrentIntValue);
        int index = ui->min_character_combo->findText(currentStr);
        ui->min_character_combo->setCurrentIndex(index);
    }

    if(Config::Instance()->GetConfigItemByJson("use_fuzzy",configInfo))
    {
        configInfo.itemCurrentIntValue? ui->fuzzy_checkbox->setCheckState(Qt::Checked):ui->fuzzy_checkbox->setCheckState(Qt::Unchecked);
        int flag = configInfo.itemCurrentIntValue?Qt::Checked:Qt::Unchecked;
        ui->fuzzy_set_btn->setAttribute(Qt::WA_TranslucentBackground);
        if(flag == Qt::Checked)
        {

            ui->fuzzy_set_btn->setVisible(true);

        }
        else
        {
           ui->fuzzy_set_btn->setVisible(false);

        }

    }
    else
    {
        ui->fuzzy_checkbox->setCheckState(Qt::Unchecked);
        ui->fuzzy_set_btn->setAttribute(Qt::WA_TranslucentBackground);
        ui->fuzzy_set_btn->setVisible(false);
    }

    //自定义短语(快速回复)
    if(Config::Instance()->GetConfigItemByJson("use_special_word",configInfo))
    {
        configInfo.itemCurrentIntValue?ui->fast_reply_checkbox->setCheckState(Qt::Checked):ui->fast_reply_checkbox->setCheckState(Qt::Unchecked);
        int flag = configInfo.itemCurrentIntValue?Qt::Checked:Qt::Unchecked;

        if(flag == Qt::Checked)
        {
            ui->fast_reply_btn->setVisible(true);
        }
        else
        {
            ui->fast_reply_btn->setVisible(false);
        }
    }
    else
    {
        ui->fast_reply_checkbox->setCheckState(Qt::Checked);
        ui->fast_reply_btn->setVisible(true);
    }

    if(Config::Instance()->GetConfigItemByJson("toolbar_visible",configInfo))
    {
        int isVisible = configInfo.itemCurrentIntValue;
        if(isVisible == 0)
        {
            ui->hide_status_bar_check->setChecked(true);
        }
        else
        {
            ui->hide_status_bar_check->setChecked(false);
        }

    }

    ConnectSignalToSlot();

}

void BasicConfigStackedWidget::SlotShowFuzzyDlg()
{
    ConfigItemStruct configInfo;
    if(Config::Instance()->GetConfigItemByJson("fuzzy_mode",configInfo))
    {
        m_fuzzyDlg->SetInitValue(configInfo.itemCurrentIntValue);
    }
    else
    {
        m_fuzzyDlg->SetInitValue(0);
    }
//    QRect rect = QApplication::
//    //计算显示原点
//    int x = rect.x() + rect.width()/2 - this->width() /2;
//    int y = rect.y() + rect.height()/2 - this->height()/2;
//    m_fuzzyDlg->move(x, y);
    m_fuzzyDlg->exec();
}

void BasicConfigStackedWidget::SlotShowSPSetDlg()
{

}

void BasicConfigStackedWidget::SlotShowWordSuggestDlg()
{
    m_suggestSetDlg->ReloadWidget();
    m_suggestSetDlg->exec();
}

void BasicConfigStackedWidget::SlotShowAddSingleItem()
{
   // m_toviewPhraseDlg->exec();
}

void BasicConfigStackedWidget::SlotShowAddLotsItem()
{

}

void BasicConfigStackedWidget::SlotEnORChMode(int id, bool state)
{
    if(state == true)
    {
        ConfigItemStruct configInfo;
        configInfo.itemName = "startup_with_english_input";
        configInfo.itemCurrentIntValue = id;
        Config::Instance()->SetConfigItemByJson("startup_with_english_input",configInfo);
    }
}

void BasicConfigStackedWidget::SlotSimpleORTraditional(int id, bool state)
{
    if(state)
    {
        ConfigItemStruct configInfo;
        if(id == 0)
        {
            configInfo.itemName = "hz_output_mode";
            configInfo.itemCurrentIntValue = HZ_OUTPUT_SIMPLIFIED;
            Config::Instance()->SetConfigItemByJson("hz_output_mode",configInfo);

        }
        else{
            configInfo.itemName = "hz_output_mode";
            configInfo.itemCurrentIntValue = HZ_OUTPUT_TRADITIONAL;
            Config::Instance()->SetConfigItemByJson("hz_output_mode",configInfo);
        }
    }
}

void BasicConfigStackedWidget::SlotHalfORFullMode(int id, bool state)
{
    ConfigItemStruct configInfo;
    if(state)
    {
        if(Config::Instance()->GetConfigItemByJson("hz_option",configInfo))
        {
            int flag = configInfo.itemCurrentIntValue;
            if(id == 0)
            {
                flag |= HZ_SYMBOL_HALFSHAPE;
            }
            else
            {
                flag |= HZ_SYMBOL_HALFSHAPE;
                flag ^= HZ_SYMBOL_HALFSHAPE;
            }
            configInfo.itemCurrentIntValue = flag;
        }

        Config::Instance()->SetConfigItemByJson("hz_option",configInfo);
    }
}

void BasicConfigStackedWidget::OnInputModeGroupToggled(int id, bool state)
{
    ConfigItemStruct configInfo;
    if((id == 0) && state)
    {
        configInfo.itemName = "pinyin_mode";
        configInfo.itemCurrentIntValue = PINYIN_SHUANGPIN;
        Config::Instance()->SetConfigItemByJson("pinyin_mode",configInfo);
        ui->modify_sp_btn->setVisible(true);
    }
    if((id == 1) && state)
    {
        configInfo.itemName = "pinyin_mode";
        configInfo.itemCurrentIntValue = PINYIN_QUANPIN;
        Config::Instance()->SetConfigItemByJson("pinyin_mode",configInfo);
        ui->modify_sp_btn->setVisible(false);
    }
}



void BasicConfigStackedWidget::SlotEnableSuggestWord(int flag)
{
    ConfigItemStruct configInfo;
    if(flag == Qt::Checked)
    {
        configInfo.itemName = "use_word_suggestion";
        configInfo.itemCurrentIntValue = 1;
        Config::Instance()->SetConfigItemByJson("use_word_suggestion",configInfo);

        ui->word_suggest_btn->setVisible(true);
    }
    else
    {
        configInfo.itemName = "use_word_suggestion";
        configInfo.itemCurrentIntValue = 0;
        Config::Instance()->SetConfigItemByJson("use_word_suggestion",configInfo);
        ui->word_suggest_btn->setVisible(false);
    }
}

void BasicConfigStackedWidget::SlotEnableSuperSimple(int flag)
{

    ConfigItemStruct configInfo;
    if(Config::Instance()->GetConfigItemByJson("ci_option",configInfo))
    {
        if(flag == Qt::Checked)
        {
            configInfo.itemCurrentIntValue |= CI_USE_FIRST_LETTER;
            ui->min_character_combo->setEnabled(true);
        }
        else if( flag == Qt::Unchecked)
        {
            configInfo.itemCurrentIntValue &= ~CI_USE_FIRST_LETTER;
            ui->min_character_combo->setEnabled(false);
        }
    }

    Config::Instance()->SetConfigItemByJson("ci_option",configInfo);
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
}

void BasicConfigStackedWidget::SlotEnableSpwMode(int flag)
{
    ConfigItemStruct configInfo;
    if(flag == Qt::Checked)
    {
        ui->fast_reply_btn->setVisible(true);
        configInfo.itemName = "use_special_word";
        configInfo.itemCurrentIntValue = 1;
        Config::Instance()->SetConfigItemByJson("use_special_word",configInfo);
    }
    else
    {
        ui->fast_reply_btn->setVisible(false);
        configInfo.itemName = "use_special_word";
        configInfo.itemCurrentIntValue = 0;
        Config::Instance()->SetConfigItemByJson("use_special_word",configInfo);
    }
}

void BasicConfigStackedWidget::SlotFastReplySet()
{
    ToViewPhraseDialog dlg;
    dlg.ScaledToHighDpi();
    dlg.exec();
}

void BasicConfigStackedWidget::SlotFuzzyEnabled(int state)
{
    ConfigItemStruct configInfo;
    configInfo.itemName = "use_fuzzy";
    if(state == Qt::Checked)
    {

        configInfo.itemCurrentIntValue = 1;
        Config::Instance()->SetConfigItemByJson("use_fuzzy",configInfo);
    }
    else
    {
        configInfo.itemCurrentIntValue = 0;
        Config::Instance()->SetConfigItemByJson("use_fuzzy",configInfo);
    }
}

void BasicConfigStackedWidget::ResetConfigInfo()
{

    QStringList config_item_name_list;
    config_item_name_list << "startup_with_english_input"
                          << "hz_output_mode"
                          << "hz_option"
                          << "pinyin_mode"
                          << "use_word_suggestion"
                          << "use_fuzzy"
                          << "use_special_word"
                          << "sp_file_name"
                          << "suggest_syllable_location"
                          << "suggest_word_location"
                          << "suggest_word_count"
                          << "fuzzy_mode"
                          << "first_letter_input_min_hz"
                          << "spw_position"
                          << "toolbar_visible";

    for(int index=0; index<config_item_name_list.size(); ++index)
    {
        QString currentItem = config_item_name_list.at(index);
        ConfigItemStruct configInfo;
        if(Config::Instance()->GetDefualtConfigItem(currentItem,configInfo))
        {
            Config::Instance()->SetConfigItemByJson(currentItem,configInfo);
        }

    }
}

void BasicConfigStackedWidget::SlotSPSchemeChanged(int index)
{
    Q_UNUSED(index);
    ConfigItemStruct configInfo;
    configInfo.itemName = "sp_file_name";
    configInfo.itemType = "string";
    configInfo.itemCurrentStrValue = "";
    Config::Instance()->SetConfigItemByJson("sp_file_name",configInfo);
}
void BasicConfigStackedWidget::SlotChangeMinLetters(int index)
{
    ConfigItemStruct configInfo;
    int minLetter = ui->min_character_combo->itemText(index).toInt();
    configInfo.itemName = "first_letter_input_min_hz";
    configInfo.itemCurrentIntValue = minLetter;
    Config::Instance()->SetConfigItemByJson("first_letter_input_min_hz",configInfo);
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

void BasicConfigStackedWidget::SlotHideToolbarStateChanged(int checkStatus)
{
    int flag = (checkStatus == Qt::Checked)? 0:1;
    ConfigItemStruct configInfo;
    configInfo.itemName = "toolbar_visible";
    configInfo.itemCurrentIntValue = flag;
    Config::Instance()->SetConfigItemByJson("toolbar_visible",configInfo);
}
