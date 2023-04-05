#include "toolbarconfigwidget.h"
#include "../public/config.h"
#include "windowconfig.h"
#include "ui_toolbarconfigwidget.h"
#include "../public/utils.h"
#include "../public/addon_config.h"
#include "../public/configmanager.h"
#include <QGraphicsDropShadowEffect>
#include <QApplication>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDesktopWidget>

ToolbarConfigWidget::ToolbarConfigWidget(QWidget *parent) :
    CustomizeQWidget(parent),
    ui(new Ui::ToolbarConfigWidget)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
                         Qt::WindowDoesNotAcceptFocus | Qt::Tool | Qt::X11BypassWindowManagerHint);
    this->setAttribute(Qt::WA_TranslucentBackground,true);
    InitWidget();
    LoadConfig();
}

ToolbarConfigWidget::~ToolbarConfigWidget()
{
    delete ui;
}

void ToolbarConfigWidget::InitWidget()
{
    ui->label->setProperty("type","h1");
    ui->label_2->setProperty("type","h2");
    ui->label_3->setProperty("type","h2");
    ui->username_label->setProperty("type","username");
    QPixmap pix(":/image/toolbarConfig/default_user.png");
    ui->default_user_label->setPixmap(pix.scaled(32,32));
    ui->login_btn->setCursor(Qt::PointingHandCursor);
    ui->reset_btn->setCursor(Qt::PointingHandCursor);
    ui->close_btn->setCursor(Qt::PointingHandCursor);

    ui->cn_en_check->setIcon(QIcon(QPixmap(":/image/toolbarConfig/left_btn_pinyin.png").scaled(20,20)));
    ui->cn_en_punctuation_checkbox->setIcon(QIcon(QPixmap(":/image/toolbarConfig/left_btn_punc.png").scaled(20,20)));
    ui->full_half_checkbox->setIcon(QIcon(QPixmap(":/image/toolbarConfig/left_btn_fullwidth.png").scaled(20,20)));
    ui->simple_tradtion_checkbox->setIcon(QIcon(QPixmap(":/image/toolbarConfig/left_btn_simple_trad.png").scaled(20,20)));
    ui->wubi_pinyin_checkbox->setIcon(QIcon(QPixmap(":/image/toolbarConfig/left_btn_wubi.png").scaled(20,20)));
    ui->symbol_checkbox->setIcon(QIcon(QPixmap(":/image/toolbarConfig/left_btn_symbol.png").scaled(20,20)));
    ui->setting_checkbox->setIcon(QIcon(QPixmap(":/image/toolbarConfig/left_btn_setting.png").scaled(20,20)));
    ui->quan_shuang_checkbox->setIcon(QIcon(QPixmap(":/image/toolbarConfig/left_btn_quan.png").scaled(20,20)));
    ui->ocr_checkbox->setIcon(QIcon(QPixmap(":/image/toolbarConfig/left_btn_ocr.png").scaled(20,20)));
    ui->voice_checkbox->setIcon(QIcon(QPixmap(":/image/toolbarConfig/left_btn_voice.png").scaled(20,20)));

    QList<QCheckBox*> check_list;
    check_list << ui->cn_en_check << ui->cn_en_punctuation_checkbox << ui->full_half_checkbox << ui->simple_tradtion_checkbox
               << ui->wubi_pinyin_checkbox << ui->symbol_checkbox << ui->setting_checkbox << ui->quan_shuang_checkbox
               << ui->ocr_checkbox << ui->voice_checkbox;
    for(QCheckBox* index : check_list)
    {
        index->setIconSize(QSize(20,20));
    }

#ifdef TOOL_BOOX
    ui->quan_shuang_checkbox->setIcon(QIcon(QPixmap(":/image/toolbarConfig/left_btn_quan.png").scaled(20,20)));
    ui->ocr_check_box->setIcon(QIcon(QPixmap(":/image/toolbarConfig/left_btn_ocr.png").scaled(20,20)));
    ui->voice_checkbox->setIcon(QIcon(QPixmap(":/image/toolbarConfig/left_btn_voice.png").scaled(20,20)));
    m_checkbox_value_map.insert(ui->ocr_check_box,USE_OCR_ADDON);
    m_checkbox_value_map.insert(ui->voice_checkbox,USE_VOICE_ADDON);
    ui->quan_shuang_checkbox->setVisible(false);
#endif

    m_checkbox_value_map.insert(ui->cn_en_check,USE_ENG_CN);
    m_checkbox_value_map.insert(ui->cn_en_punctuation_checkbox,USE_PUNCTUATION);
    m_checkbox_value_map.insert(ui->full_half_checkbox,USE_FULL_HALF_WIDTH);
    m_checkbox_value_map.insert(ui->simple_tradtion_checkbox,USE_FAN_JIAN);
    m_checkbox_value_map.insert(ui->wubi_pinyin_checkbox,USE_WUBI_PINYIN);
    m_checkbox_value_map.insert(ui->symbol_checkbox,USE_SPECIAL_SYMBOL);
    m_checkbox_value_map.insert(ui->setting_checkbox,USE_SETTINGS);
    m_checkbox_value_map.insert(ui->quan_shuang_checkbox,USE_QUAN_SHUANG);
    m_checkbox_value_map.insert(ui->ocr_checkbox,USE_OCR_ADDON);
    m_checkbox_value_map.insert(ui->voice_checkbox,USE_VOICE_ADDON);


    ui->reset_btn->setProperty("type","puretextbtn");
    ui->login_btn->setProperty("type","normal");
    ui->close_btn->setProperty("type","closebtn");

    QList<QPushButton*> btn_list;
    btn_list << ui->reset_btn << ui->login_btn << ui->close_btn;
    for(QPushButton* btn : btn_list)
    {
        btn->setFlat(true);
        btn->setCheckable(false);
        btn->setFocusPolicy(Qt::NoFocus);
        btn->autoFillBackground();
        btn->setCursor(QCursor(Qt::PointingHandCursor));
    }
    connect(ui->close_btn, &QPushButton::clicked,this, [&](){this->close();});
    connect(ui->reset_btn, &QPushButton::clicked,this,&ToolbarConfigWidget::slot_on_reset_item);
    connect(ui->login_btn,&QPushButton::clicked,this,&ToolbarConfigWidget::slot_on_login_clicked);
}

void ToolbarConfigWidget::showEvent(QShowEvent *event)
{
    AddonConfig::Instance()->LoadUserAddon();
    bool is_ocr_exist = AddonConfig::Instance()->IsAddonExist("OCR");
    bool is_voice_exist = AddonConfig::Instance()->IsAddonExist("AsrInput");
    ui->ocr_checkbox->setVisible(true);
    ui->voice_checkbox->setVisible(true);
    if(!is_ocr_exist)
    {
        ui->ocr_checkbox->setVisible(false);
    }
    if(!is_voice_exist)
    {
        ui->voice_checkbox->setVisible(false);
    }
    if(!is_ocr_exist && is_voice_exist)
    {
        QGridLayout* layout = (QGridLayout*)(ui->grid_widget->layout());
        layout->addWidget(ui->voice_checkbox,3,0);
    }
    else if(is_ocr_exist && !is_voice_exist)
    {
        QGridLayout* layout = (QGridLayout*)(ui->grid_widget->layout());
        layout->addWidget(ui->ocr_checkbox,3,0);
    }
    else if(is_ocr_exist && is_voice_exist)
    {
        QGridLayout* layout = (QGridLayout*)(ui->grid_widget->layout());
        layout->addWidget(ui->ocr_checkbox,3,0);
        layout->addWidget(ui->voice_checkbox,3,1);
    }
    event->accept();
}

void ToolbarConfigWidget::LoadConfig()
{
    WindowConfig::Instance()->LoadConfig();
    int toolbarConfig;
    ConfigManager::Instance()->GetIntConfigItem("toolbar_config",toolbarConfig);
    QList<QCheckBox*> checkbox_list = m_checkbox_value_map.keys();
    for(QCheckBox* index : checkbox_list)
    {
        index->disconnect();
        if(toolbarConfig & m_checkbox_value_map.value(index))
        {
            index->setChecked(true);
        }
        else
        {
            index->setChecked(false);
        }
        connect(index,&QCheckBox::stateChanged,this,&ToolbarConfigWidget::slot_on_item_checked);
    }
    int islogin;
    ConfigManager::Instance()->GetIntConfigItem("login",islogin);
    if(islogin)
    {
        QString login_id = Config::Instance()->GetLoginId();
        ui->username_label->setText(login_id);
        ui->username_label->setVisible(true);
        ui->login_btn->setVisible(false);
        ui->default_user_label->setVisible(true);
    }
    else
    {
        ui->username_label->setVisible(false);
        ui->login_btn->setVisible(true);
        ui->default_user_label->setVisible(false);
    }

}

void ToolbarConfigWidget::SetShowFlag(bool flag)
{
    this->setVisible(flag);
    if(flag)
    {
        MoveToNewPos(this->pos());
    }
}

void ToolbarConfigWidget::MoveToNewPos(QPoint clickPoint)
{
    const int spacer = 3;
    QPoint cursor_pos = clickPoint;
    int top_border = cursor_pos.y() - this->height();
    int rightBorder = cursor_pos.x() + this->width();

    QRect desk_rect = QApplication::desktop()->availableGeometry();

    if (rightBorder > desk_rect.width())
    {
        cursor_pos.setX(desk_rect.width() - this->width());
    }

    if (top_border < 0)
    {
        int raw_y = cursor_pos.y() + 31 - spacer;
        cursor_pos.setY(raw_y);
    }
    else
    {
        int raw_y = cursor_pos.y() - this->height() + spacer;
        cursor_pos.setY(raw_y);
    }
    this->move(cursor_pos);
}


void ToolbarConfigWidget::slot_on_item_checked(int state)
{
    QCheckBox* sender_check = (QCheckBox*)sender();
    ConfigManager::Instance()->ReloadUserConfig();
    int toolbarConfig;
    ConfigManager::Instance()->GetIntConfigItem("toolbar_config",toolbarConfig);

    int config_value = m_checkbox_value_map.value(sender_check);
    if(state == Qt::Checked)
    {
      toolbarConfig |= config_value;
    }
    else if(state == Qt::Unchecked)
    {
      toolbarConfig &= ~config_value;
    }
    WindowConfig::Instance()->SetIntValue("tool_bar_config",toolbarConfig);
    ConfigManager::Instance()->SetIntConfigItem("toolbar_config",toolbarConfig);
    emit item_changed();
}

void ToolbarConfigWidget::slot_on_reset_item()
{
    QStringList config_item_name_list;
    config_item_name_list << "toolbar_config";

    for(QString indexkey : config_item_name_list)
    {
        ConfigManager::Instance()->ClearConfigItem(indexkey);
    }

    LoadConfig();
    emit item_changed();
}

void ToolbarConfigWidget::slot_on_login_clicked()
{
    if(!Utils::IsProcessOn(QString("huayupy-config-tools-fcitx")))
    {
        FILE* ptr2 = NULL;
        string command = Utils::GetBinFilePath().append("huayupy-config-tools-fcitx 82717623-mhe4-0293-aduh-ku87wh6328ne login_page &").toStdString();
        if((ptr2 = popen(command.c_str(), "r")) != NULL)
        {
            pclose(ptr2);
        }
    }
    else
    {
        // 最小化的时候激活
        QDBusMessage msg = QDBusMessage::createSignal("/com/thunisoft/huayupy", "com.thunisoft.huayupy", QString("to_show"));
        QDBusConnection::sessionBus().send(msg);
    }
}
