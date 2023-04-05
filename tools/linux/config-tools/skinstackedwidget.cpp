#include "skinstackedwidget.h"
#include "ui_skinconfigstackedwidget.h"
#include <QFontDatabase>

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
#include "stylepreviewwidght.h"
#include <QSpacerItem>
#include <QStandardItemModel>
#include <QRegExp>
#include <QLineEdit>

#include "../public/addon_config.h"
#include "../public/configmanager.h"



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

SkinStackedWidget::SkinStackedWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SkinConfigStackedWidget)
{
    ui->setupUi(this);
    InitWidget();
    LoadConfigInfo();

}

SkinStackedWidget::~SkinStackedWidget()
{
    delete ui;
}

void SkinStackedWidget::InitWidget()
{
    ui->status_bar_set_label->setProperty("type","h1");
    ui->show_btn_label->setProperty("type","h2");
    ui->input_bar_set_label->setProperty("type","h1");
    ui->show_style_label->setProperty("type","h2");
    ui->skin_font_style_label->setProperty("type","h2");
    this->setProperty("type","stackedwidget");

    ui->label_3->setProperty("type","h2");
    ui->label_4->setProperty("type","h2");
    ui->label_5->setProperty("type","h2");
    ui->label_6->setProperty("type","h2");
    ui->label_7->setProperty("type","h2");
    ui->scrollArea->setBackgroundRole(QPalette::Light);

    QVector<int> labelLayout ={21,16,1,77,454};

    for(int index=0; index<labelLayout.size(); ++index)
    {
        ui->label_layout->setStretch(index,labelLayout.at(index));
        ui->label_layout_2->setStretch(index,labelLayout.at(index));
    }

    QStringList candidateNumlist;
    candidateNumlist << "3" << "4" << "5" << "6" << "7" << "8" << "9";
    ui->candidates_combo->addItems(candidateNumlist);

    QStringList expandCandidatesLine;
    expandCandidatesLine <<"2行" << "3行" << "4行";
    ui->expand_candidates_combo->addItems(expandCandidatesLine);
    ui->expand_candidates_combo->setItemData(0,2);
    ui->expand_candidates_combo->setItemData(1,3);
    ui->expand_candidates_combo->setItemData(2,4);

    m_candidatesGroup = new QButtonGroup(this);
    m_candidatesGroup->addButton(ui->horizon_radiobtn,0);
    m_candidatesGroup->addButton(ui->vertical_radiobtn,1);

    m_toolbar_group = new QButtonGroup(this);
    m_toolbar_group->addButton(ui->simple_tradition_checkbox, USE_FAN_JIAN);
    m_toolbar_group->addButton(ui->cn_en_checkbox, USE_ENG_CN);
    m_toolbar_group->addButton(ui->full_half_checkbox, USE_FULL_HALF_WIDTH);
    m_toolbar_group->addButton(ui->punctuation_checkbox, USE_PUNCTUATION);
    m_toolbar_group->addButton(ui->special_symbol_checkbox, USE_SPECIAL_SYMBOL);
    m_toolbar_group->addButton(ui->pinyin_wubi_checkbox, USE_WUBI_PINYIN);
    m_toolbar_group->addButton(ui->quan_shuang_checkbox,USE_QUAN_SHUANG);
    m_toolbar_group->addButton(ui->ocr_addon_checkbox,USE_OCR_ADDON);
    m_toolbar_group->addButton(ui->voice_addon_checkbox,USE_VOICE_ADDON);
    m_toolbar_group->setExclusive(false);

    bool is_ocr_exist = AddonConfig::Instance()->IsAddonExist("OCR");
    bool is_voice_exist = AddonConfig::Instance()->IsAddonExist("AsrInput");
    if(!is_ocr_exist)
    {
        ui->ocr_addon_checkbox->setVisible(false);
    }
    if(!is_voice_exist)
    {
        ui->voice_addon_checkbox->setVisible(false);
    }

    if(!is_ocr_exist && !is_voice_exist)
    {
        //ui->gridLayout->addWidget(ui->hide_toobar_checkbox,3,0);
    }
    else if(!is_ocr_exist && is_voice_exist)
    {
        ui->gridLayout->addWidget(ui->voice_addon_checkbox,3,0);        
    }
    else if(is_ocr_exist && !is_voice_exist)
    {
        ui->gridLayout->addWidget(ui->ocr_addon_checkbox,3,0);
    }

    QStringList chineseFontList = GetChineseFontList();
    QStringList englishFontList = GetEnglishFontList();

    QStandardItemModel *chineseFontmodel = new QStandardItemModel();//添加提示tootip
    for(int i = 0; i < chineseFontList.size(); ++i){
        QStandardItem *item = new QStandardItem(chineseFontList.at(i));
        item->setToolTip(chineseFontList.at(i));
        chineseFontmodel->appendRow(item);
    }
    ui->chinese_font_combo->setModel(chineseFontmodel);

    QVector<QString> unuse_fonts;
    unuse_fonts.push_back("Webdings");
    unuse_fonts.push_back("Wingdings");
    unuse_fonts.push_back("Wingdings 2");
    unuse_fonts.push_back("Wingdings 3");
    QStandardItemModel* englishFontModel = new QStandardItemModel();
    for(int i = 0; i < englishFontList.size(); ++i){
        if(unuse_fonts.contains(englishFontList.at(i)))
            continue;
        QStandardItem *item = new QStandardItem(englishFontList.at(i));
        item->setToolTip(englishFontList.at(i));
        englishFontModel->appendRow(item);
    }
    ui->english_font_combo->setModel(englishFontModel);


    QStringList fontSizeList;
    fontSizeList << "14" << "16" << "18" << "20" << "24" << "28" << "32" << "36" << "42" << "48";
    ui->font_size_combo->addItems(fontSizeList);

    ui->set_checkbox->setChecked(true);
    ui->set_checkbox->setEnabled(false);

    //添加了配置颜色列表
    resetColors();

    ui->select_color_btn->setStyleSheet("QPushButton{background-color:#FFFFFF;border:1px solid #CCCCCC;outline:none;}"
                                        "QPushButton:hover{background-color:#4d9cf8;}");

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

    ui->font_size_combo->setEditable(true);
    ui->font_size_combo->setValidator(new QRegExpValidator(QRegExp("[1-9][0-9]{0,1}")));

}

void SkinStackedWidget::ConnectSignalToSlot()
{
    connect(ui->candidates_combo, SIGNAL(currentIndexChanged(int)),this, SLOT(OnCandidatesChangedSlot(int)));
    connect(ui->always_expand_checkbox, SIGNAL(stateChanged(int)), this, SLOT(SlotAlwaysKeepExpandMode(int)));
    connect(ui->expand_candidates_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(SlotSetExpandModeLine(int)));
    connect(ui->expand_checkbox, SIGNAL(stateChanged(int)), this, SLOT(SlotAllowTabExpand(int)));
    connect(ui->chinese_font_combo, SIGNAL(currentIndexChanged(int)),this, SLOT(SlotCurrentChineseFontIndexChanged(int)));

    connect(ui->english_font_combo, SIGNAL(currentIndexChanged(int)),this, SLOT(SlotCurrentEnglishFontIndexChanged(int)));

    connect(m_toolbar_group, SIGNAL(buttonToggled(int, bool)), this, SLOT(SlotToolbarConfigToggled(int, bool)));
    connect(m_candidatesGroup,SIGNAL(buttonToggled(int, bool)),this,SLOT(SlotCandidatesDirectChanged(int,bool)));

    connect(ui->select_color_btn, SIGNAL(clicked()),this,SLOT(SlotOnColorSelectBtnClicked()));
    connect(ui->color_list_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(SlotColorItemChanged(int)));
    connect(ui->font_size_combo, SIGNAL(currentTextChanged(const QString&)),this,SLOT(SlotFontSizeChanged(const QString&)));

    connect(ui->font_size_combo, SIGNAL(focusOut()), this, SLOT(OnFontSizeComboFocusOUt()));
}

QColor SkinStackedWidget::TranshFromIntToColor(int inputColorValue)
{
    int red = inputColorValue & 255;
    int green = inputColorValue >> 8 & 255;
    int blue = inputColorValue >> 16 & 255;
    return QColor(red,green,blue);
}

int SkinStackedWidget::TransFormColorToInt(QColor inputColor)
{
    int redValue = inputColor.red();
    int greenValue = inputColor.green();
    int blueValue = inputColor.blue();

    return (int)((blueValue << 16)|(greenValue << 8)|(redValue));
}

void SkinStackedWidget::LoadConfigInfo()
{
    ui->candidates_combo->disconnect();
    ui->always_expand_checkbox->disconnect();
    ui->expand_candidates_combo->disconnect();
    ui->expand_checkbox->disconnect();
    ui->chinese_font_combo->disconnect();
    ui->font_size_combo->disconnect();
    m_toolbar_group->disconnect();
    m_candidatesGroup->disconnect();
    ui->color_list_combo->disconnect();
    ui->select_color_btn->disconnect();

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
                          << "compose_string_color"
                          << "candidate_selected_color"
                          << "candidate_string_color"
                          << "main_background_color"
                          << "main_line_color"
                          << "compose_caret_color"
                          << "line_background_color"
                          << "cell_background_color"
                          << "hide_tray";
    int toolbarConfig;
    if(ConfigManager::Instance()->GetIntConfigItem("toolbar_config",toolbarConfig))
    {
        (toolbarConfig & USE_FAN_JIAN)? ui->simple_tradition_checkbox->setChecked(true):ui->simple_tradition_checkbox->setChecked(false);
        (toolbarConfig & USE_ENG_CN)? ui->cn_en_checkbox->setChecked(true):ui->cn_en_checkbox->setChecked(false);
        (toolbarConfig & USE_FULL_HALF_WIDTH)? ui->full_half_checkbox->setChecked(true):ui->full_half_checkbox->setChecked(false);
        (toolbarConfig & USE_PUNCTUATION)? ui->punctuation_checkbox->setChecked(true):ui->punctuation_checkbox->setChecked(false);
        (toolbarConfig & USE_SPECIAL_SYMBOL)? ui->special_symbol_checkbox->setChecked(true):ui->special_symbol_checkbox->setChecked(false);
        (toolbarConfig & USE_WUBI_PINYIN)
                ? ui->pinyin_wubi_checkbox->setChecked(true)
                : ui->pinyin_wubi_checkbox->setChecked(false);
        (toolbarConfig & USE_QUAN_SHUANG)?ui->quan_shuang_checkbox->setChecked(true) : ui->quan_shuang_checkbox->setChecked(false);
        (toolbarConfig & USE_OCR_ADDON)? ui->ocr_addon_checkbox->setChecked(true):ui->ocr_addon_checkbox->setChecked(false);
        (toolbarConfig & USE_VOICE_ADDON)? ui->voice_addon_checkbox->setChecked(true):ui->voice_addon_checkbox->setChecked(false);
    }

    int configItemValue;
    if(ConfigManager::Instance()->GetIntConfigItem("show_vertical_candidate",configItemValue))
    {
        configItemValue? ui->vertical_radiobtn->setChecked(true): ui->horizon_radiobtn->setChecked(true);

        if(configItemValue)
        {
            ui->expand_checkbox->setEnabled(false);
            ui->always_expand_checkbox->setEnabled(false);
            ui->expand_candidates_combo->setEnabled(false);
        }
        else
        {
            ui->expand_checkbox->setEnabled(true);
            ui->always_expand_checkbox->setEnabled(true);
            ui->expand_candidates_combo->setEnabled(true);
        }
    }



    if(ConfigManager::Instance()->GetIntConfigItem("candidates_per_line",configItemValue))
    {
        int candidatesIndex = ui->candidates_combo->findText(QString::number(configItemValue));
        ui->candidates_combo->setCurrentIndex(candidatesIndex);
    }

    if(ConfigManager::Instance()->GetIntConfigItem("use_tab_expand_candidates",configItemValue))
    {
        configItemValue?ui->expand_checkbox->setChecked(true):ui->expand_checkbox->setChecked(false);
        configItemValue?ui->expand_candidates_combo->setEnabled(true):ui->expand_candidates_combo->setEnabled(false);
    }

    if(ConfigManager::Instance()->GetIntConfigItem("always_expand_candidates",configItemValue))
    {
       ui->always_expand_checkbox->setChecked(configItemValue);
    }

    if(ConfigManager::Instance()->GetIntConfigItem("expand_candidate_lines",configItemValue))
    {
        QRegExp regexp(QString("[^\\d]*%1[^\\d]*").arg(configItemValue));
        for (int i = 0; i < ui->expand_candidates_combo->count(); ++i)
        {
            if (regexp.exactMatch(ui->expand_candidates_combo->itemText(i)))
            {
                ui->expand_candidates_combo->setCurrentIndex(i);
                break;
            }
        }
    }


    //中文字体
    QString configItemStr;
    if(ConfigManager::Instance()->GetStrConfigItem("chinese_font_name",configItemStr))
    {
        int chineseFontIndex = ui->chinese_font_combo->findText(configItemStr);
        ui->chinese_font_combo->setCurrentIndex(chineseFontIndex);
    }
    if(ConfigManager::Instance()->GetStrConfigItem("english_font_name",configItemStr))
    {
        int englishFontIndex = ui->english_font_combo->findText(configItemStr);
        ui->english_font_combo->setCurrentIndex(englishFontIndex);
    }

    if(ConfigManager::Instance()->GetIntConfigItem("font_height",configItemValue))
    {
        int fontHeightIndex = ui->font_size_combo->findText(QString::number(configItemValue));
        if(fontHeightIndex == -1)
        {
            ui->font_size_combo->addItem(QString::number(configItemValue));
            ui->font_size_combo->setCurrentText(QString::number(configItemValue));
        }
        else
        {
            ui->font_size_combo->setCurrentIndex(fontHeightIndex);
        }
    }

    if(ConfigManager::Instance()->GetIntConfigItem("compose_string_color",configItemValue))
    {
        QColor stringColor = TranshFromIntToColor(configItemValue);
        SlotDrawColorToTheBtn(ui->select_color_btn,stringColor);
    }

    ui->color_list_combo->setCurrentIndex(0);
    m_stylePreviewWidght->UpdateTheWidght();
    ConnectSignalToSlot();

}

QStringList SkinStackedWidget::GetEnglishFontList()
{
    QFontDatabase database;
    QStringList allfontList = database.families(QFontDatabase::Any);
    return allfontList;

}

QStringList SkinStackedWidget::GetChineseFontList()
{
    QStringList result;
    QFontDatabase database;
    result += database.families(QFontDatabase::SimplifiedChinese);
    result += database.families(QFontDatabase::TraditionalChinese);
    QList<QString> resultList = result.toSet().toList();

    QStringList  returnList = QStringList::fromSet(resultList.toSet());
    returnList.sort();
    return result;
}

//获取候选词个数
void SkinStackedWidget::OnCandidatesChangedSlot(int index)
{
    int candidatesNum = ui->candidates_combo->itemText(index).toInt();
    ConfigManager::Instance()->SetIntConfigItem("candidates_per_line",candidatesNum);
    m_stylePreviewWidght->UpdateTheWidght();

}

void SkinStackedWidget::SlotAlwaysKeepExpandMode(int checkStatus)
{
    ConfigManager::Instance()->SetIntConfigItem("always_expand_candidates",checkStatus);
}

void SkinStackedWidget::SlotSetExpandModeLine(int index)
{
    if (index != -1)
    {
        QString text = ui->expand_candidates_combo->itemText(index);
        QRegExp regexp("[^\\d]*(\\d+)[^\\d]*");
        int index = regexp.indexIn(text);
        if (index != -1)
        {
            QString numberStr = regexp.cap(1);
            ConfigManager::Instance()->SetIntConfigItem("expand_candidate_lines",numberStr.toInt());
        }
    }
}

void SkinStackedWidget::SlotCandidatesDirectChanged(int id,bool status)
{
    if((id == 0) && (status == true))
    {
        ConfigManager::Instance()->SetIntConfigItem("show_vertical_candidate",0);
        ui->always_expand_checkbox->setEnabled(true);
        ui->expand_checkbox->setEnabled(true);
        ui->expand_candidates_combo->setEnabled(true);

    }
    else if((id == 1) && (status == true))
    {
        ConfigManager::Instance()->SetIntConfigItem("show_vertical_candidate",1);
        ui->always_expand_checkbox->setEnabled(false);
        ui->expand_checkbox->setEnabled(false);
        ui->expand_candidates_combo->setEnabled(false);
    }

}

void SkinStackedWidget::SlotAllowTabExpand(int flag)
{
    bool use_tab_expand = (flag == Qt::Checked) ? 1 : 0;
    ConfigManager::Instance()->SetIntConfigItem("use_tab_expand_candidates",use_tab_expand);
    ui->expand_candidates_combo->setEnabled(use_tab_expand);
}

void SkinStackedWidget::SlotToolbarConfigToggled(int id, bool checked)
{
    int configValue;
    if(ConfigManager::Instance()->GetIntConfigItem("toolbar_config",configValue))
    {
        if (checked)
        {
            configValue |= id;
        }
        else
        {
            configValue &= ~id;
        }
        ConfigManager::Instance()->SetIntConfigItem("toolbar_config",configValue);
    }

}

void SkinStackedWidget::ResetConfigInfo()
{

    QStringList config_item_name_list;
    config_item_name_list << "candidates_per_line"
                          << "show_sp_hint"
                          << "show_vertical_candidate"
                          << "use_tab_expand_candidates"
                          << "always_expand_candidates"
                          << "expand_candidate_lines"
                          << "toolbar_config"
                          << "chinese_font_name"
                          << "english_font_name"
                          << "font_height"
                          << "compose_string_color"
                          << "candidate_selected_color"
                          << "candidate_string_color"
                          << "main_background_color"
                          << "main_line_color"
                          << "compose_caret_color"
                          << "line_background_color"
                          << "cell_background_color"
                          << "hide_tray";

    for(QString indexkey : config_item_name_list)
    {
        ConfigManager::Instance()->ClearConfigItem(indexkey);
    }
    LoadConfigInfo();
    m_stylePreviewWidght->UpdateTheWidght();
}

void SkinStackedWidget::SlotCurrentChineseFontIndexChanged(int index)
{
    QString chineseFontName = ui->chinese_font_combo->itemText(index);
    ConfigManager::Instance()->SetStrConfigItem("chinese_font_name",chineseFontName);
    m_stylePreviewWidght->UpdateTheWidght();
}

void SkinStackedWidget::SlotCurrentFontHeightIndexChanged(int index)
{
    int fontHeight = ui->font_size_combo->itemText(index).toInt();
    ConfigManager::Instance()->SetIntConfigItem("font_height",fontHeight);
    m_stylePreviewWidght->UpdateTheWidght();
}

void SkinStackedWidget::SlotCurrentEnglishFontIndexChanged(int index)
{
    QString englishFontName = ui->english_font_combo->itemText(index);
    ConfigManager::Instance()->SetStrConfigItem("english_font_name",englishFontName);
    m_stylePreviewWidght->UpdateTheWidght();
}

void SkinStackedWidget::SlotHideToolbarStateChanged(int checkStatus)
{
    ConfigManager::Instance()->SetIntConfigItem("hide_tray",checkStatus);
}

void SkinStackedWidget::SlotOnColorSelectBtnClicked()
{
    QColorDialog *pColorDialog = new QColorDialog(this);
    pColorDialog->setWindowTitle("选择颜色");
    pColorDialog->setStyleSheet("QDialog{background-color:#FFFFFF;}QColorShowLabel{border:1px solid #666666;}");
    //修改配置工具的样式
    if (QDialogButtonBox *btnBox = pColorDialog->findChild<QDialogButtonBox *>())
    {
        if(QPushButton *btnOk = btnBox->button(QDialogButtonBox::Ok))
        {
            btnOk->setStyleSheet(normalBtnStyle);
            btnOk->setText("确定");
        }

        if (QPushButton *btnCancel = btnBox->button(QDialogButtonBox::Cancel))
        {
            btnCancel->setStyleSheet(normalBtnStyle);
            btnCancel->setText("取消");
        }

    }
    QList<QPushButton*> childBtn = pColorDialog->findChildren<QPushButton*>();
    for(QPushButton* tempBtn:childBtn)
    {
        tempBtn->setStyleSheet(normalBtnStyle);
    }

    //设置默认选中的颜色
    QMap<QString,int> colorMap;
    QStringList color_key_list;
    color_key_list << "compose_string_color"
                   << "candidate_selected_color"
                   << "candidate_string_color"
                   << "main_background_color"
                   << "main_line_color"
                   << "compose_caret_color"
                   << "line_background_color"
                   << "cell_background_color";

    for(QString key: color_key_list)
    {
        int key_value;
        if(ConfigManager::Instance()->GetIntConfigItem(key,key_value))
        {
            colorMap.insert(key,key_value);
        }
    }

    QString colorItem = ui->color_list_combo->currentData().toString();
    QColor currentColor;
    currentColor = TranshFromIntToColor(colorMap.value(colorItem,0));

    pColorDialog->setCurrentColor(currentColor);
    pColorDialog->exec();

    QColor selectedColor = pColorDialog->selectedColor();

    if(selectedColor.isValid())
    {
        SlotDrawColorToTheBtn(ui->select_color_btn, pColorDialog->selectedColor());
        //修改对应的颜色样式
        int colorValue = TransFormColorToInt(pColorDialog->selectedColor());
        colorMap.insert(colorItem,colorValue);
        ConfigManager::Instance()->SetIntConfigItem(colorItem,colorValue);
        m_stylePreviewWidght->UpdateTheWidght();
    }
    delete pColorDialog;
    pColorDialog = nullptr;

}

void SkinStackedWidget::SlotDrawColorToTheBtn(QPushButton *inputBtn, QColor color)
{
    inputBtn->blockSignals(true);
    QPixmap pix(22,22);
    pix.fill(Qt::transparent);
    QPainter shapePainter(&pix);
    shapePainter.setPen(Qt::transparent);
    shapePainter.setBrush(color);
    shapePainter.setRenderHint(QPainter::SmoothPixmapTransform);
    shapePainter.drawRect(pix.rect());
    inputBtn->setIcon(QIcon(pix));
    inputBtn->setIconSize(QSize(18,18));
    inputBtn->blockSignals(false);
}

void SkinStackedWidget::SlotColorItemChanged(int currentIndex)
{
    QString selecteItem = ui->color_list_combo->itemData(currentIndex).toString();
    if(selecteItem.isEmpty())
    {
        return;
    }
    QMap<QString,int> colorMap;
    QStringList color_key_list;
    color_key_list << "compose_string_color"
                   << "candidate_selected_color"
                   << "candidate_string_color"
                   << "main_background_color"
                   << "main_line_color"
                   << "compose_caret_color"
                   << "line_background_color"
                   << "cell_background_color";

    for(QString key: color_key_list)
    {
        int key_value;
        if(ConfigManager::Instance()->GetIntConfigItem(key,key_value))
        {
            colorMap.insert(key,key_value);
        }
    }
    QColor stringColor = TranshFromIntToColor(colorMap.value(selecteItem));
    SlotDrawColorToTheBtn(ui->select_color_btn,stringColor);

}

void SkinStackedWidget::SlotFontSizeChanged(const QString &fontSize)
{
    int fontSizeInt = fontSize.toUInt();
    ConfigManager::Instance()->SetIntConfigItem("font_height",fontSizeInt);
    m_stylePreviewWidght->UpdateTheWidght();
}

void SkinStackedWidget::OnFontSizeComboFocusOUt()
{
    if (ui->font_size_combo->currentText().isEmpty())
    {
        ui->font_size_combo->setCurrentText("16");
    }
}

bool SkinStackedWidget::eventFilter(QObject *obj, QEvent *event)
{
    qDebug() << obj->metaObject()->className();
    if(obj->metaObject()->className() == QStringLiteral("QComboBox"))
    {
        if (event->type() == QEvent::Wheel)//鼠标滚轮事件
        {
            return true;//禁用下拉框的滚轮改变项的功能
        }
    }
    return false;
}

void SkinStackedWidget::resetColors()
{
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
    ui->color_list_combo->clear();
    for(int index=0; index<itemSize; ++index)
    {
        QPair<QString,QString> currentPair = colorConfigItemVector.at(index);
        ui->color_list_combo->addItem(currentPair.first,currentPair.second);
    }
}
