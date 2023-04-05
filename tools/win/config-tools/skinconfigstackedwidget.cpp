#include "skinconfigstackedwidget.h"
#include "ui_skinconfigstackedwidget.h"
#include "config.h"
#include "../../../toolBox/addon_config.h"

#include <QButtonGroup>
#include <QFontDatabase>
#include <QVBoxLayout>
#include <QListView>
#include <QScreen>
#include <QDebug>
#include <QColorDialog>
#include <QDialogButtonBox>
#include <QLayout>
#include <QVector>
#include <QPair>
#include <QPainter>
#include "utils.h"
#include "stylepreviewwidght.h"
#include <QSpacerItem>
#include <QStandardItemModel>

QString normalBtnStyle = "/*普通按钮的样式*/"
"QPushButton{"
"width:80px;"
"height:30px;"
"font-size:14px;"
"font-family:Microsoft YaHei;"
"border:1px solid rgb(77,156,248);"
"border-radius:2px;"
"color:#4D9CF8;"
"background:rgb(255,255,255);"
"padding-bottom : 2px;"
"outline:none;"
"}"
"QPushButton:hover{"
"width:80px;"
"height:30px;"
"background:#4D9CF8;"
"border-radius:2px;"
"color:white;"
"}"
"QPushButton:checked{"
"width:80px;"
"height:30px;"
"background:#4D9CF8;"
"border-radius:2px;"
"color:white;"
"}"
"QPushButton:pressed{"
"width:80px;"
"height:30px;"
"background:#2D7BD5;"
"border-radius:2px;"
"}"
"QPushButton:!enabled{"
"color:rgb(128,128,128);"
"background:rgb(191,191,191);"
"border:2px solid rgb(191,191,191);"
"}";

SkinConfigStackedWidget::SkinConfigStackedWidget(QWidget *parent) :
    BaseStackedWidget(parent),
    ui(new Ui::SkinConfigStackedWidget)
{
    ui->setupUi(this);
    InitWidget();
    LoadConfigInfo();

}

SkinConfigStackedWidget::~SkinConfigStackedWidget()
{
    if(m_font_lineedit != nullptr)
    {
        delete m_font_lineedit;
        m_font_lineedit = nullptr;
    }
    delete ui;
}

void SkinConfigStackedWidget::InitWidget()
{
    m_file_watcher = new QFileSystemWatcher(this);
    ui->status_bar_set_label->setProperty("type","h1");
    ui->show_btn_label->setProperty("type","h2");
    ui->input_bar_set_label->setProperty("type","h1");
    ui->show_style_label->setProperty("type","h2");
    ui->skin_font_style_label->setProperty("type","h2");
    this->setProperty("type","stackedwidget");
    ui->chinese_font_combo->setProperty("type","expandingcombo");
    ui->font_size_combo->setProperty("type","expandingcombo");
    ui->scrollArea->setBackgroundRole(QPalette::Light);

    ui->hide_toobar_checkbox->setVisible(false);

    QVector<int> labelLayout;
    bool isOk = Config::Instance()->GetLayoutVector("labellayout",labelLayout);
    if(isOk)
    {
        for(int index=0; index<labelLayout.size(); ++index)
        {
            ui->label_layout->setStretch(index,labelLayout.at(index));
            ui->label_layout_2->setStretch(index,labelLayout.at(index));
        }
    }

    QStringList candidateNumlist;
    candidateNumlist << "3" << "4" << "5" << "6" << "7" << "8" << "9";
    ui->candidates_combo->addItems(candidateNumlist);

    QStringList expandCandidatesLine;
    expandCandidatesLine <<"2行" << "3行" << "4行" << "5行";
    ui->expand_candidates_combo->addItems(expandCandidatesLine);

    m_candidatesGroup = new QButtonGroup(this);
    m_candidatesGroup->addButton(ui->horizon_radiobtn,0);
    m_candidatesGroup->addButton(ui->vertical_radiobtn,1);


    m_toolbar_group = new QButtonGroup(this);
    m_toolbar_group->addButton(ui->simple_tradition_checkbox,0);
    m_toolbar_group->addButton(ui->cn_en_checkbox,1);
    m_toolbar_group->addButton(ui->full_half_checkbox,2);
    m_toolbar_group->addButton(ui->punctuation_checkbox,3);
    m_toolbar_group->addButton(ui->special_symbol_checkbox,4);
    m_toolbar_group->addButton(ui->soft_keyboard_checkbox,5);
    m_toolbar_group->addButton(ui->set_checkbox,6);
    m_toolbar_group->addButton(ui->quan_shuang_checkbox,7);
    m_toolbar_group->addButton(ui->ocr_checkbox,8);
    m_toolbar_group->addButton(ui->voice_checkbox,9);
    m_toolbar_group->setExclusive(false);


    QFontDatabase database;

    //ui->chinese_font_combo->addItems(GetChineseFontList());
    //ui->english_font_combo->addItems(GetEnglishFontList());

    QStringList chineseFontList = GetChineseFontList();
    QStringList englishFontList = GetEnglishFontList();

    QStandardItemModel *chineseFontmodel = new QStandardItemModel();//添加提示tootip
    for(int i = 0; i < chineseFontList.size(); ++i){
        QStandardItem *item = new QStandardItem(chineseFontList.at(i));
        item->setToolTip(chineseFontList.at(i));
        chineseFontmodel->appendRow(item);
    }
    ui->chinese_font_combo->setModel(chineseFontmodel);

    QStandardItemModel* englishFontModel = new QStandardItemModel();
    for(int i = 0; i < englishFontList.size(); ++i){
        QStandardItem *item = new QStandardItem(englishFontList.at(i));
        item->setToolTip(englishFontList.at(i));
        englishFontModel->appendRow(item);
    }
    ui->english_font_combo->setModel(englishFontModel);


    QStringList fontSizeList;
    fontSizeList << "14" << "16" << "18" << "20" << "24" << "28" << "32" << "36" << "42" << "48";
    ui->font_size_combo->addItems(fontSizeList);

//    ui->set_checkbox->setChecked(true);
//    ui->set_checkbox->setEnabled(false);

    //添加了配置颜色列表
    QVector<QPair<QString,QString> > colorConfigItemVector =
    {{"拼音串颜色","compose_string_color"},
    {"选中候选词颜色","candidate_selected_color"},
    {"非选中候选词颜色","candidate_string_color"},
    {"光标颜色","compose_caret_color"},
    {"背景颜色","main_background_color"},
    {"选中词背景颜色","cell_background_color"},
    {"选中行背景颜色","line_background_color"},
    {"分割线及边框颜色","main_line_color"}};

    int itemSize = colorConfigItemVector.size();
    for(int index=0; index<itemSize; ++index)
    {
         QPair<QString,QString> currentPair = colorConfigItemVector.at(index);
         ui->color_list_combo->addItem(currentPair.first,currentPair.second);
    }

    ui->select_color_btn->setStyleSheet("QPushButton{background-color:#FFFFFF;border:1px solid #CCCCCC;}"
                                        "QPushButton:hover{background-color:#4d9cf8;}");
    ui->select_color_btn->setCursor(Qt::PointingHandCursor);

    QList<QComboBox*> comboList;
    comboList.push_back(ui->candidates_combo);
    comboList.push_back(ui->expand_candidates_combo);
    comboList.push_back(ui->font_size_combo);
    comboList.push_back(ui->chinese_font_combo);
    comboList.push_back(ui->color_list_combo);
    comboList.push_back(ui->english_font_combo);
    for(int index=0; index<comboList.size(); ++index)
    {
        comboList.at(index)->setMaxVisibleItems(6);
        comboList.at(index)->setAutoFillBackground(true);
        comboList.at(index)->setView(new QListView);
        comboList.at(index)->installEventFilter(this);
    }
    QVBoxLayout* previewLayout = new QVBoxLayout(ui->preview_widght);
    m_stylePreviewWidght = new StylePreviewWidght(ui->preview_widght);
    m_stylePreviewWidght->setFixedWidth(20);
    m_stylePreviewWidght->setFixedHeight(20);
    previewLayout->addWidget(m_stylePreviewWidght);
    previewLayout->setAlignment(m_stylePreviewWidght,Qt::AlignTop);
    ui->preview_widght->setLayout(previewLayout);
    ui->preview_Area->setStyleSheet("QScrollArea{border:1px solid #CCCCCC;}");

    m_font_lineedit = new MyLineEdit();
    style()->unpolish(m_font_lineedit);
    ui->font_size_combo->setLineEdit(m_font_lineedit);

    ui->font_size_combo->setEditable(true);
    ui->font_size_combo->setValidator(new QRegExpValidator(QRegExp("[1-9][0-9]{0,1}")));

    ui->chinese_font_combo->view()->setFixedWidth(155);
    ui->english_font_combo->view()->setFixedWidth(155);

}

void SkinConfigStackedWidget::ConnectSignalToSlot()
{
    connect(ui->candidates_combo, SIGNAL(currentIndexChanged(int)),this, SLOT(OnCandidatesChangedSlot(int)));
    connect(ui->always_expand_checkbox, SIGNAL(stateChanged(int)), this, SLOT(SlotAlwaysKeepExpandMode(int)));
    connect(ui->expand_candidates_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(SlotSetExpandModeLine(int)));
    connect(ui->expand_checkbox, SIGNAL(stateChanged(int)), this, SLOT(SlotAllowTabExpand(int)));
    connect(ui->chinese_font_combo, SIGNAL(currentIndexChanged(int)),this, SLOT(SlotCurrentChineseFontIndexChanged(int)));
    //connect(ui->font_size_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(SlotCurrentFontHeightIndexChanged(int)));
    connect(ui->english_font_combo, SIGNAL(currentIndexChanged(int)),this, SLOT(SlotCurrentEnglishFontIndexChanged(int)));

    connect(m_toolbar_group, SIGNAL(buttonClicked(int)),this, SLOT(SlotToolbarStatusChanged(int)));
    connect(m_candidatesGroup,SIGNAL(buttonToggled(int, bool)),this,SLOT(SlotCandidatesDirectChanged(int,bool)));
    connect(ui->hide_toobar_checkbox, SIGNAL(stateChanged(int)), this, SLOT(SlotHideToolbarStateChanged(int)));
    connect(ui->select_color_btn, SIGNAL(clicked()),this,SLOT(SlotOnColorSelectBtnClicked()));
    connect(ui->color_list_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(SlotColorItemChanged(int)));
    connect(ui->font_size_combo, SIGNAL(currentTextChanged(const QString&)),this,SLOT(SlotFontSizeChanged(const QString&)));
    connect(m_file_watcher, &QFileSystemWatcher::fileChanged,this,&SkinConfigStackedWidget::SlotConfigFileChanged);
    auto checkContent = [this](){
        if(ui->font_size_combo->currentText().isEmpty())
        {
            ConfigItemStruct configInfo;
            if(Config::Instance()->GetDefualtConfigItem("font_height",configInfo))
            {
                Config::Instance()->SetConfigItemByJson("font_height",configInfo);
                m_font_lineedit->setText(QString::number(configInfo.itemCurrentIntValue));
                m_stylePreviewWidght->UpdateTheWidght();
            }
        }
    };

    connect(m_font_lineedit, &MyLineEdit::focus_out, checkContent);
}

QColor SkinConfigStackedWidget::TranshFromIntToColor(int inputColorValue)
{
    int red = inputColorValue & 255;
    int green = inputColorValue >> 8 & 255;
    int blue = inputColorValue >> 16 & 255;
    return QColor(red,green,blue);
}

int SkinConfigStackedWidget::TransFormColorToInt(QColor inputColor)
{
    int redValue = inputColor.red();
    int greenValue = inputColor.green();
    int blueValue = inputColor.blue();

    return (int)((blueValue << 16)|(greenValue << 8)|(redValue));
}

void SkinConfigStackedWidget::LoadConfigInfo()
{
    ui->candidates_combo->disconnect();
    ui->always_expand_checkbox->disconnect();
    ui->expand_candidates_combo->disconnect();
    ui->expand_checkbox->disconnect();
    ui->chinese_font_combo->disconnect();
    ui->font_size_combo->disconnect();
    ui->hide_toobar_checkbox->disconnect();
    m_toolbar_group->disconnect();
    m_candidatesGroup->disconnect();
    ui->color_list_combo->disconnect();
    ui->select_color_btn->disconnect();
    ui->english_font_combo->disconnect();
    m_file_watcher->disconnect();

    bool ocr_exist = AddonConfig::Instance()->IsAddonExist("OCR");
    bool voice_exist = AddonConfig::Instance()->IsAddonExist("AsrInput");
    ui->voice_checkbox->setVisible(voice_exist);
    ui->ocr_checkbox->setVisible(ocr_exist);

    ConfigItemStruct configInfo;
    if(Config::Instance()->GetConfigItemByJson("candidates_per_line",configInfo))
    {
        int candidatesNum = configInfo.itemCurrentIntValue;
        int index = ui->candidates_combo->findText(QString::number(candidatesNum));
        ui->candidates_combo->setCurrentIndex(index);
    }

    if(Config::Instance()->GetConfigItemByJson("use_tab_expand_candidates",configInfo))
    {
        int useTabExpand = configInfo.itemCurrentIntValue;
        useTabExpand?ui->expand_checkbox->setChecked(true):ui->expand_checkbox->setChecked(false);
        useTabExpand?ui->expand_candidates_combo->setEnabled(true):ui->expand_candidates_combo->setEnabled(false);
    }

    if(Config::Instance()->GetConfigItemByJson("always_expand_candidates",configInfo))
    {
        int flag = configInfo.itemCurrentIntValue;
        ui->always_expand_checkbox->setChecked(flag);
    }

    if(Config::Instance()->GetConfigItemByJson("expand_candidate_lines",configInfo))
    {
        int expandlindes = configInfo.itemCurrentIntValue;
        ui->expand_candidates_combo->setCurrentIndex(expandlindes - 2);
    }

    if(Config::Instance()->GetConfigItemByJson("show_vertical_candidate",configInfo))
    {
        int candidatesFlag = configInfo.itemCurrentIntValue;
        candidatesFlag ? ui->vertical_radiobtn->setChecked(true): ui->horizon_radiobtn->setChecked(true);
        candidatesFlag ? ui->expand_checkbox->setEnabled(false): ui->expand_checkbox->setEnabled(true);
    }


    if(Config::Instance()->GetConfigItemByJson("tool_bar_config",configInfo))
    {
        int toolbarConfig = configInfo.itemCurrentIntValue;
        (toolbarConfig & USE_FAN_JIAN)? ui->simple_tradition_checkbox->setChecked(true):ui->simple_tradition_checkbox->setChecked(false);
        (toolbarConfig & USE_ENG_CN)? ui->cn_en_checkbox->setChecked(true):ui->cn_en_checkbox->setChecked(false);
        (toolbarConfig & USE_FULL_HALF_WIDTH)? ui->full_half_checkbox->setChecked(true):ui->full_half_checkbox->setChecked(false);
        (toolbarConfig & USE_PUNCTUATION)? ui->punctuation_checkbox->setChecked(true):ui->punctuation_checkbox->setChecked(false);
        (toolbarConfig & USE_SPECIAL_SYMBOL)? ui->special_symbol_checkbox->setChecked(true):ui->special_symbol_checkbox->setChecked(false);
        (toolbarConfig & USE_SOFT_KEYBOARD)? ui->soft_keyboard_checkbox->setChecked(true):ui->soft_keyboard_checkbox->setChecked(false);
        (toolbarConfig & USE_SETTINGS)? ui->set_checkbox->setChecked(true):ui->set_checkbox->setChecked(false);
        (toolbarConfig & USE_QUAN_SHUANG)? ui->quan_shuang_checkbox->setChecked(true):ui->quan_shuang_checkbox->setChecked(false);
        (toolbarConfig & USE_OCR_ADDON)? ui->ocr_checkbox->setChecked(true):ui->ocr_checkbox->setChecked(false);
        (toolbarConfig & USE_VOICE_ADDON)? ui->voice_checkbox->setChecked(true):ui->voice_checkbox->setChecked(false);
    }
    if(Config::Instance()->GetConfigItemByJson("chinese_font_name",configInfo))
    {
        QString tempChineseFontName = configInfo.itemCurrentStrValue;
        int index = ui->chinese_font_combo->findText(tempChineseFontName);
        ui->chinese_font_combo->setCurrentIndex(index);
    }
    if(Config::Instance()->GetConfigItemByJson("english_font_name",configInfo))
    {
        QString tempEnglishFontName = configInfo.itemCurrentStrValue;
        int index = ui->english_font_combo->findText(tempEnglishFontName);
        ui->english_font_combo->setCurrentIndex(index);
    }

    if(Config::Instance()->GetConfigItemByJson("font_height",configInfo))
    {
        int configFontHeight = configInfo.itemCurrentIntValue;
        int index = ui->font_size_combo->findText(QString::number(configFontHeight));
        if(index == -1)
        {
            ui->font_size_combo->addItem(QString::number(configFontHeight));
            ui->font_size_combo->setCurrentText(QString::number(configFontHeight));
        }
        else
        {
             ui->font_size_combo->setCurrentIndex(index);
        }

    }
    if(Config::Instance()->GetConfigItemByJson("toolbar_visible",configInfo))
    {
        int isVisible = configInfo.itemCurrentIntValue;
        if(isVisible == 0)
        {
            ui->hide_toobar_checkbox->setChecked(true);
        }
        else
        {
            ui->hide_toobar_checkbox->setChecked(false);
        }

    }

    if(Config::Instance()->GetConfigItemByJson("compose_string_color",configInfo))
    {
        QColor stringColor = TranshFromIntToColor(configInfo.itemCurrentIntValue);
        SlotDrawColorToTheBtn(ui->select_color_btn,stringColor);
    }
    ui->color_list_combo->setCurrentIndex(0);
    m_stylePreviewWidght->UpdateTheWidght();
    ConnectSignalToSlot();

}

QStringList SkinConfigStackedWidget::GetEnglishFontList()
{
    QFontDatabase database;
    QStringList allfontList = database.families(QFontDatabase::Any);
    QStringList filterFont;
    filterFont << "Bookshelf Sym" << "Euclid Extra" << "Euclid Math"  << "Euclid Symbol"
               << "Fences" << "Marlett" << "MS Outlook" << "MS Reference"
               << "MT Extra" << "Symbol" << "Symbol Tiger" << "Webdings" << "Wingdings"
               << "Courier" << "Fixedsys" << "Small Fonts" << "System" << "Terminal";
    QStringList result;
    for(QString index : allfontList)
    {
        bool isValid = true;
        for(QString filterIndex:filterFont)
        {
            if(index.startsWith(filterIndex))
            {
                isValid = false;
                break;
            }
        }
        if(isValid)
        {
            result.append(index);
        }
    }
    return result;

}

QStringList SkinConfigStackedWidget::GetChineseFontList()
{
    QStringList result;
    QFontDatabase database;
    result += database.families(QFontDatabase::SimplifiedChinese);
    result += database.families(QFontDatabase::TraditionalChinese);
    result.removeAll("Malgun Gothic Semilight");
    result.removeAll("Malgun Gothic");
    result.removeOne("Fixedsys");
    result.removeOne("System");
    result.removeOne("Terminal");
    result.sort();
    return result;
}

//获取候选词个数
void SkinConfigStackedWidget::OnCandidatesChangedSlot(int index)
{

    int candidatesNum = ui->candidates_combo->itemText(index).toInt();
    ConfigItemStruct configInfo;
    configInfo.itemName = "candidates_per_line";
    configInfo.itemCurrentIntValue = candidatesNum;
    Config::Instance()->SetConfigItemByJson("candidates_per_line",configInfo);
    m_stylePreviewWidght->UpdateTheWidght();

}

void SkinConfigStackedWidget::SlotAlwaysKeepExpandMode(int checkStatus)
{
    int flag = (checkStatus == Qt::Checked)? 1:0;
    ConfigItemStruct configInfo;
    configInfo.itemName = "always_expand_candidates";
    configInfo.itemCurrentIntValue = flag;
    Config::Instance()->SetConfigItemByJson("always_expand_candidates",configInfo);

}

void SkinConfigStackedWidget::SlotSetExpandModeLine(int lines)
{
    ConfigItemStruct configInfo;
    int expandLines = lines + 2;
    configInfo.itemName = "expand_candidate_lines";
    configInfo.itemCurrentIntValue = expandLines;
    Config::Instance()->SetConfigItemByJson("expand_candidate_lines",configInfo);

}

void SkinConfigStackedWidget::SlotCandidatesDirectChanged(int id,bool status)
{
    ConfigItemStruct configInfo;
    if((id == 0) && (status == true))
    {
        configInfo.itemName = "show_vertical_candidate";
        configInfo.itemCurrentIntValue = 0;
        Config::Instance()->SetConfigItemByJson("show_vertical_candidate",configInfo);
        ui->always_expand_checkbox->setEnabled(true);
        ui->expand_checkbox->setEnabled(true);
        ui->expand_candidates_combo->setEnabled(true);

    }
    else if((id == 1) && (status == true))
    {
        configInfo.itemName = "show_vertical_candidate";
        configInfo.itemCurrentIntValue = 1;
        Config::Instance()->SetConfigItemByJson("show_vertical_candidate",configInfo);
        ui->always_expand_checkbox->setEnabled(false);
        ui->expand_checkbox->setEnabled(false);
        ui->expand_candidates_combo->setEnabled(false);
    }
}

void SkinConfigStackedWidget::SlotAllowTabExpand(int flag)
{
    ConfigItemStruct configInfo;
    if(flag == Qt::Checked)
    {
        configInfo.itemName = "use_tab_expand_candidates";
        configInfo.itemCurrentIntValue = 1;
        Config::Instance()->SetConfigItemByJson("use_tab_expand_candidates",configInfo);
        ui->expand_candidates_combo->setEnabled(true);

    }
    else
    {
        configInfo.itemName = "use_tab_expand_candidates";
        configInfo.itemCurrentIntValue = 0;
        Config::Instance()->SetConfigItemByJson("use_tab_expand_candidates",configInfo);
        ui->expand_candidates_combo->setEnabled(false);

    }
}

void SkinConfigStackedWidget::SlotToolbarStatusChanged(int id)
{
    bool isChecked =(QCheckBox*)m_toolbar_group->button(id)->isChecked();
    int toolbarConfig =0;
    ConfigItemStruct configInfo;
    if(Config::Instance()->GetConfigItemByJson("tool_bar_config",configInfo))
    {
        toolbarConfig = configInfo.itemCurrentIntValue;
    }
    if(id>=0 && id<=9)
    {
        if(isChecked)
        {
            toolbarConfig |= (1 << id);
        }
        else
        {
            toolbarConfig &= ~(1 << id);
        }
        configInfo.itemCurrentIntValue = toolbarConfig;
        Config::Instance()->SetConfigItemByJson("tool_bar_config",configInfo);
    }
}

void SkinConfigStackedWidget::ResetConfigInfo()
{

    QStringList config_item_name_list;
    config_item_name_list << "candidates_per_line"
                          << "show_sp_hint"
                          << "show_vertical_candidate"
                          << "use_tab_expand_candidates"
                          << "always_expand_candidates"
                          << "expand_candidate_lines"
                          << "tool_bar_config"
                          << "chinese_font_name"
                          << "english_font_name"
                          << "font_height"
                          <<"compose_string_color"
                          <<"candidate_selected_color"
                          <<"candidate_string_color"
                          <<"main_background_color"
                          <<"main_line_color"
                          <<"compose_caret_color"
                          <<"line_background_color"
                          <<"cell_background_color";


    for(int index=0; index<config_item_name_list.size(); ++index)
    {
        QString currentItem = config_item_name_list.at(index);
        ConfigItemStruct configInfo;
        if(Config::Instance()->GetDefualtConfigItem(currentItem,configInfo))
        {
            Config::Instance()->SetConfigItemByJson(currentItem,configInfo);
        }

    }
    ui->expand_checkbox->setEnabled(true);

}

void SkinConfigStackedWidget::SlotCurrentChineseFontIndexChanged(int index)
{
    ConfigItemStruct configInfo;
    configInfo.itemName = "chinese_font_name";
    configInfo.itemType = "string";
    configInfo.itemCurrentStrValue = ui->chinese_font_combo->itemText(index);
    Config::Instance()->SetConfigItemByJson("chinese_font_name",configInfo);
    m_stylePreviewWidght->UpdateTheWidght();
}

void SkinConfigStackedWidget::SlotCurrentFontHeightIndexChanged(int index)
{
    ConfigItemStruct configInfo;
    configInfo.itemName = "font_height";
    configInfo.itemCurrentIntValue = ui->font_size_combo->itemText(index).toInt();
    Config::Instance()->SetConfigItemByJson("font_height",configInfo);
    m_stylePreviewWidght->UpdateTheWidght();
}

void SkinConfigStackedWidget::SlotCurrentEnglishFontIndexChanged(int index)
{
    ConfigItemStruct configInfo;
    configInfo.itemName = "english_font_name";
    configInfo.itemType = "string";
    configInfo.itemCurrentStrValue = ui->english_font_combo->itemText(index);
    Config::Instance()->SetConfigItemByJson("english_font_name",configInfo);
    m_stylePreviewWidght->UpdateTheWidght();
}

void SkinConfigStackedWidget::SlotHideToolbarStateChanged(int checkStatus)
{
    int flag = (checkStatus == Qt::Checked)? 0:1;
    ConfigItemStruct configInfo;
    configInfo.itemName = "toolbar_visible";
    configInfo.itemCurrentIntValue = flag;
    Config::Instance()->SetConfigItemByJson("toolbar_visible",configInfo);
}

void SkinConfigStackedWidget::SlotOnColorSelectBtnClicked()
{
    QColorDialog *pColorDialog = new QColorDialog(this);
    pColorDialog->setWindowTitle("选择颜色");
    pColorDialog->setStyleSheet("QDialog{background-color:#FFFFFF;}QColorShowLabel{border:1px solid #666666;}");
    //修改配置工具的样式
    if (QDialogButtonBox *btnBox = pColorDialog->findChild<QDialogButtonBox *>())
    {
        if(QPushButton *btnOk = btnBox->button(QDialogButtonBox::Ok))
            btnOk->setStyleSheet(normalBtnStyle);
        if (QPushButton *btnCancel = btnBox->button(QDialogButtonBox::Cancel))
            btnCancel->setStyleSheet(normalBtnStyle);
    }
    QList<QPushButton*> childBtn = pColorDialog->findChildren<QPushButton*>();
    for(QPushButton* tempBtn:childBtn)
    {
        tempBtn->setStyleSheet(normalBtnStyle);
    }

    //设置默认选中的颜色
    QString colorItem = ui->color_list_combo->currentData().toString();
    ConfigItemStruct configInfo;
    QColor currentColor;
    if(Config::Instance()->GetConfigItemByJson(colorItem,configInfo))
    {
        currentColor = TranshFromIntToColor(configInfo.itemCurrentIntValue);
    }

    pColorDialog->setCurrentColor(currentColor);
    pColorDialog->exec();

    QColor selectedColor = pColorDialog->selectedColor();

    if(selectedColor.isValid())
    {
        SlotDrawColorToTheBtn(ui->select_color_btn, pColorDialog->selectedColor());
        //修改对应的颜色样式
        configInfo.itemName = colorItem;
        configInfo.itemCurrentIntValue = TransFormColorToInt(pColorDialog->selectedColor());
        Config::Instance()->SetConfigItemByJson(colorItem,configInfo);
        m_stylePreviewWidght->UpdateTheWidght();
    }
    delete pColorDialog;
    pColorDialog = nullptr;
}

void SkinConfigStackedWidget::SlotDrawColorToTheBtn(QPushButton *inputBtn, QColor color)
{
    inputBtn->blockSignals(true);
    QPixmap pix(25,25);
    pix.fill(Qt::transparent);
    QPainter shapePainter(&pix);
    shapePainter.setPen(Qt::transparent);
    shapePainter.setBrush(color);
    shapePainter.setRenderHint(QPainter::SmoothPixmapTransform);
    shapePainter.drawRect(pix.rect());
    inputBtn->setIcon(QIcon(pix));
    inputBtn->setIconSize(QSize(20,20));
    inputBtn->blockSignals(false);
}

void SkinConfigStackedWidget::SlotColorItemChanged(int currentIndex)
{

    QString selecteItem = ui->color_list_combo->itemData(currentIndex).toString();
    if(selecteItem.isEmpty())
    {
        return;
    }
    ConfigItemStruct configInfo;
    if(Config::Instance()->GetConfigItemByJson(selecteItem,configInfo))
    {
        QColor stringColor = TranshFromIntToColor(configInfo.itemCurrentIntValue);
        SlotDrawColorToTheBtn(ui->select_color_btn,stringColor);
    }
}

void SkinConfigStackedWidget::SlotFontSizeChanged(const QString &fontSize)
{
    int fontSizeInt = fontSize.toUInt();
    ConfigItemStruct configInfo;
    configInfo.itemName = "font_height";
    configInfo.itemCurrentIntValue = fontSizeInt;
    Config::Instance()->SetConfigItemByJson("font_height",configInfo);
    m_stylePreviewWidght->UpdateTheWidght();
}

void SkinConfigStackedWidget::SlotConfigFileChanged(QString file_path)
{
    //文件发生变化
}

bool SkinConfigStackedWidget::eventFilter(QObject *obj, QEvent *event)
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
