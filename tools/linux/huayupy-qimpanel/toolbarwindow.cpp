#include "toolbarwindow.h"

#include "maincontroller.h"
#include "skin/skinbase.h"
#include "toolbarmodel.h"
#include "windowconfig.h"
#include "../public/config.h"
#include "tooltip.h"
#include "uosmodemonitor.h"
#include "toolbarmenu.h"
#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>
#include <QSvgRenderer>
#include <QPoint>
#include <QMouseEvent>
#include <QSettings>
#include <QScreen>

ToolbarWindow::ToolbarWindow():m_main_controller(NULL),
                               m_skin_base(NULL),
                               m_toolbar_model(NULL),
                               m_current_down_index(-1),
                               m_current_move_index(-1),
                               m_last_move_index(-1),
                               mouse_has_moved(false),
                               m_has_showed(false)
{
    this->setAttribute(Qt::WA_TranslucentBackground, true);//设置窗口背景透明
    m_main_controller = MainController::Instance();
    m_skin_base = MainController::Instance()->GetSkinBase();
    m_toolbar_model = MainController::Instance()->GetToolbarModel();

    m_image_background = NULL;
    m_image_logo = NULL;

    m_image_cn = NULL;
    m_image_en = NULL;
    m_image_full_width = NULL;
    m_image_half_width = NULL;
    m_image_cn_punctuation = NULL;
    m_image_en_punctuation = NULL;
    m_image_simplified = NULL;
    m_image_traditional = NULL;
    m_image_special_symbol = NULL;
    m_image_settings = NULL;
    m_image_settings_update = NULL;
    m_image_pinyin = NULL;
    m_image_wubi = NULL;
    m_image_quan = NULL;
    m_image_shuang = NULL;
    m_image_ocr = NULL;
    m_image_voice = NULL;

    m_image_cn_hover = NULL;
    m_image_en_hover = NULL;
    m_image_full_width_hover = NULL;
    m_image_half_width_hover = NULL;
    m_image_cn_punctuation_hover = NULL;
    m_image_en_punctuation_hover = NULL;
    m_image_simplified_hover = NULL;
    m_image_traditional_hover = NULL;
    m_image_special_symbol_hover = NULL;
    m_image_settings_hover = NULL;
    m_image_settings_update_hover = NULL;
    m_image_pinyin_hover = NULL;
    m_image_wubi_hover = NULL;
    m_image_quan_hover = NULL;
    m_image_shuang_hover = NULL;
    m_image_ocr_hover = NULL;
    m_image_voice_hover = NULL;

    m_image_cn_press = NULL;
    m_image_en_press = NULL;
    m_image_full_width_press = NULL;
    m_image_half_width_press = NULL;
    m_image_cn_punctuation_press = NULL;
    m_image_en_punctuation_press = NULL;
    m_image_simplified_press = NULL;
    m_image_traditional_press = NULL;
    m_image_special_symbol_press = NULL;
    m_image_settings_press = NULL;
    m_image_settings_update_press = NULL;
    m_image_pinyin_press = NULL;
    m_image_wubi_press = NULL;
    m_image_quan_press = NULL;
    m_image_shuang_press = NULL;
    m_image_ocr_press = NULL;
    m_image_voice_press = NULL;


    m_toolbar_config_widget = new ToolbarConfigWidget(this);
    m_toolbar_main_menu = new ToolBarMainMenu(this);
    connect(m_toolbar_config_widget,&ToolbarConfigWidget::item_changed,this,[&](){this->RefreshWindow();});
    connect(m_toolbar_main_menu,&ToolBarMainMenu::simple_tra_state_changed,this,&ToolbarWindow::slot_simple_tra_changed);
    LoadImage();
}

#define DELETE_SAFE(p) if(p) { delete p; p = NULL; }
ToolbarWindow::~ToolbarWindow()
{
    DELETE_SAFE(m_image_background)
    DELETE_SAFE(m_image_logo)

    DELETE_SAFE(m_image_cn)
    DELETE_SAFE(m_image_en)
    DELETE_SAFE(m_image_full_width)
    DELETE_SAFE(m_image_half_width)
    DELETE_SAFE(m_image_cn_punctuation)
    DELETE_SAFE(m_image_en_punctuation)
    DELETE_SAFE(m_image_simplified)
    DELETE_SAFE(m_image_traditional)
    DELETE_SAFE(m_image_special_symbol)    
    DELETE_SAFE(m_image_settings)
    DELETE_SAFE(m_image_settings_update)
    DELETE_SAFE(m_image_pinyin)    
    DELETE_SAFE(m_image_wubi)
    DELETE_SAFE(m_image_quan)
    DELETE_SAFE(m_image_shuang)
	 DELETE_SAFE(m_image_ocr)
    DELETE_SAFE(m_image_voice)


    DELETE_SAFE(m_image_cn_hover)
    DELETE_SAFE(m_image_en_hover)
    DELETE_SAFE(m_image_full_width_hover)
    DELETE_SAFE(m_image_half_width_hover)
    DELETE_SAFE(m_image_cn_punctuation_hover)
    DELETE_SAFE(m_image_en_punctuation_hover)
    DELETE_SAFE(m_image_simplified_hover)
    DELETE_SAFE(m_image_traditional_hover)
    DELETE_SAFE(m_image_special_symbol_hover)
    DELETE_SAFE(m_image_settings_hover)
    DELETE_SAFE(m_image_settings_update_hover)
    DELETE_SAFE(m_image_pinyin_hover)
    DELETE_SAFE(m_image_wubi_hover)
    DELETE_SAFE(m_image_quan_hover)
    DELETE_SAFE(m_image_shuang_hover)
	DELETE_SAFE(m_image_ocr_hover)
    DELETE_SAFE(m_image_voice_hover)

    DELETE_SAFE(m_image_cn_press)
    DELETE_SAFE(m_image_en_press)
    DELETE_SAFE(m_image_full_width_press)
    DELETE_SAFE(m_image_half_width_press)
    DELETE_SAFE(m_image_cn_punctuation_press)
    DELETE_SAFE(m_image_en_punctuation_press)
    DELETE_SAFE(m_image_simplified_press)
    DELETE_SAFE(m_image_traditional_press)
    DELETE_SAFE(m_image_special_symbol_press)
    DELETE_SAFE(m_image_settings_press)
    DELETE_SAFE(m_image_settings_update_press)
    DELETE_SAFE(m_image_pinyin_press)
    DELETE_SAFE(m_image_wubi_press)
    DELETE_SAFE(m_image_quan_press)
    DELETE_SAFE(m_image_shuang_press)
	DELETE_SAFE(m_image_ocr_press)
    DELETE_SAFE(m_image_voice_press)
}

#define ADD_BUTTON(_button, _rect_index) if(_button & WC->toolbar_config) \
                                          { \
                                               m_button_type.push_back(_button);\
                                               Rect rect = { 43 + _rect_index * 28, (31 -29) / 2, 28, 29};\
                                               m_button_rects.push_back(rect);\
                                               _rect_index++;\
                                          }
void ToolbarWindow::CalWindow()
{
    m_button_rects.clear();
    m_button_type.clear();
    int width = 0;

    //logo area
    Rect rect_logo = {(42 - 23) / 2, (31 -23) / 2, 23, 23};
    m_button_rects.push_back(rect_logo);
    width+=44;

    WindowConfig* WC = WindowConfig::Instance();

    bool is_ocr_exist = AddonConfig::Instance()->IsAddonExist("OCR");
    bool is_voice_exist = AddonConfig::Instance()->IsAddonExist("AsrInput");

    int index = 0;
    ADD_BUTTON(USE_WUBI_PINYIN, index)
    ADD_BUTTON(USE_ENG_CN, index)
    ADD_BUTTON(USE_PUNCTUATION, index)
    ADD_BUTTON(USE_FULL_HALF_WIDTH, index)
    ADD_BUTTON(USE_FAN_JIAN, index)
    if(is_ocr_exist)
    {
      ADD_BUTTON(USE_OCR_ADDON,index)
    }
    if(is_voice_exist)
    {
        ADD_BUTTON(USE_VOICE_ADDON,index)
    }
    ADD_BUTTON(USE_SPECIAL_SYMBOL, index)
    ADD_BUTTON(USE_QUAN_SHUANG,index)
    ADD_BUTTON(USE_SETTINGS, index)

    width+= index * 28;
    m_whole_window.height = 31;
    m_whole_window.width = width;
}

#define LOAD_SVG(path, _default, _img)  if(_img) \
                                            return; \
                                        if(!path.isEmpty()) { \
                                                _img = new QSvgRenderer(path); \
                                        } \
                                        else { \
                                                _img = new QSvgRenderer(QString(_default)); \
                                        }
void ToolbarWindow::LoadImage()
{
    LOAD_SVG(QString(""), ":/image/bg_status.svg", m_image_background)
    LOAD_SVG(QString(""), ":/image/logo.svg", m_image_logo)

    LOAD_SVG(QString(""), ":/image/pinyin.svg", m_image_cn)
    LOAD_SVG(QString(""), ":/image/en.svg", m_image_en)
    LOAD_SVG(QString(""), ":/image/fullwidth_active.svg", m_image_full_width)
    LOAD_SVG(QString(""), ":/image/fullwidth_inactive.svg", m_image_half_width)
    LOAD_SVG(QString(""), ":/image/punc_active.svg", m_image_cn_punctuation)
    LOAD_SVG(QString(""), ":/image/punc_inactive.svg", m_image_en_punctuation)
    LOAD_SVG(QString(""), ":/image/chttrans_inactive.svg", m_image_simplified)
    LOAD_SVG(QString(""), ":/image/chttrans_active.svg", m_image_traditional)
    LOAD_SVG(QString(""), ":/image/symbols.svg", m_image_special_symbol)
    LOAD_SVG(QString(""), ":/image/setting.svg", m_image_settings)
    LOAD_SVG(QString(""), ":/image/setting_update.svg", m_image_settings_update)
    LOAD_SVG(QString(""), ":/image/pin.svg", m_image_pinyin)
    LOAD_SVG(QString(""), ":/image/wubi.svg", m_image_wubi)
    LOAD_SVG(QString(""), ":/image/quan.svg", m_image_quan)
    LOAD_SVG(QString(""), ":/image/shuang.svg", m_image_shuang)
	LOAD_SVG(QString(""), ":/image/ocr.svg", m_image_ocr)
    LOAD_SVG(QString(""), ":/image/voice.svg", m_image_voice)

    LOAD_SVG(QString(""), ":/image/pinyin_hover.svg", m_image_cn_hover)
    LOAD_SVG(QString(""), ":/image/en_hover.svg", m_image_en_hover)
    LOAD_SVG(QString(""), ":/image/fullwidth_active_hover.svg", m_image_full_width_hover)
    LOAD_SVG(QString(""), ":/image/fullwidth_inactive_hover.svg", m_image_half_width_hover)
    LOAD_SVG(QString(""), ":/image/punc_active_hover.svg", m_image_cn_punctuation_hover)
    LOAD_SVG(QString(""), ":/image/punc_inactive_hover.svg", m_image_en_punctuation_hover)
    LOAD_SVG(QString(""), ":/image/chttrans_inactive_hover.svg", m_image_simplified_hover)
    LOAD_SVG(QString(""), ":/image/chttrans_active_hover.svg", m_image_traditional_hover)
    LOAD_SVG(QString(""), ":/image/symbols_hover.svg", m_image_special_symbol_hover)
    LOAD_SVG(QString(""), ":/image/setting_hover.svg", m_image_settings_hover)
    LOAD_SVG(QString(""), ":/image/setting_hover_update.svg", m_image_settings_update_hover)
    LOAD_SVG(QString(""), ":/image/pin_hover.svg", m_image_pinyin_hover)
    LOAD_SVG(QString(""), ":/image/wubi_hover.svg", m_image_wubi_hover)
    LOAD_SVG(QString(""), ":/image/quan_hover.svg", m_image_quan_hover)
    LOAD_SVG(QString(""), ":/image/shuang_hover.svg", m_image_shuang_hover)
	LOAD_SVG(QString(""), ":/image/ocr_hover.svg", m_image_ocr_hover)
    LOAD_SVG(QString(""), ":/image/voice_hover.svg", m_image_voice_hover)

    LOAD_SVG(QString(""), ":/image/pinyin_press.svg", m_image_cn_press)
    LOAD_SVG(QString(""), ":/image/en_press.svg", m_image_en_press)
    LOAD_SVG(QString(""), ":/image/fullwidth_active_press.svg", m_image_full_width_press)
    LOAD_SVG(QString(""), ":/image/fullwidth_inactive_press.svg", m_image_half_width_press)
    LOAD_SVG(QString(""), ":/image/punc_active_press.svg", m_image_cn_punctuation_press)
    LOAD_SVG(QString(""), ":/image/punc_inactive_press.svg", m_image_en_punctuation_press)
    LOAD_SVG(QString(""), ":/image/chttrans_inactive_press.svg", m_image_simplified_press)
    LOAD_SVG(QString(""), ":/image/chttrans_active_press.svg", m_image_traditional_press)
    LOAD_SVG(QString(""), ":/image/symbols_press.svg", m_image_special_symbol_press)
    LOAD_SVG(QString(""), ":/image/setting_press.svg", m_image_settings_press)
    LOAD_SVG(QString(""), ":/image/setting_press_update.svg", m_image_settings_update_press)
    LOAD_SVG(QString(""), ":/image/pin_press.svg", m_image_pinyin_press)
    LOAD_SVG(QString(""), ":/image/wubi_press.svg", m_image_wubi_press)
    LOAD_SVG(QString(""), ":/image/quan_press.svg", m_image_quan_press)
    LOAD_SVG(QString(""), ":/image/shuang_press.svg", m_image_shuang_press)
	LOAD_SVG(QString(""), ":/image/ocr_press.svg", m_image_ocr_press)
    LOAD_SVG(QString(""), ":/image/voice_press.svg", m_image_voice_press)
}

void ToolbarWindow::DrawWindow()
{
    CalWindow();
    this->resize(m_whole_window.width, m_whole_window.height);
    this->update();
}

void ToolbarWindow::RefreshWindow()
{
    bool show = this->isVisible();
    m_toolbar_config_widget->LoadConfig();
    DrawWindow();
    if(!show)
        this->hide();

    QPoint position = pos();


    QRect deskRect = QApplication::desktop()->rect();
    if(this->rect().width()+pos().x() > deskRect.width())
    {
        int pos_y = position.y();
        if(pos_y + this->rect().height() + 30 > deskRect.height())
        {
            pos_y = deskRect.height() - 30 - this->rect().height();
        }
        this->move(deskRect.width() -width(), pos_y);
    }
}

void ToolbarWindow::ToShow()
{
    m_toolbar_main_menu->LoadConfig();
    CalWindow();
    this->resize(m_whole_window.width, m_whole_window.height);

    //get desttop width and height
    int desktop_width = QApplication::desktop()->width();
    int desktop_height = QApplication::desktop()->height();

    auto show_default_pos = [&](){
        int screen_height = QApplication::primaryScreen()->availableGeometry().height();
        int screen_width = QApplication::primaryScreen()->availableGeometry().width();
        if(screen_height <= 0) screen_height = desktop_height - 30;
        if(screen_width <= 0)  screen_width = desktop_width;

        int x = screen_width - m_whole_window.width - 10;
        int y = screen_height - m_whole_window.height;

        this->show();
        this->move(QPoint(x, y));
    };

    QString file_path = Config::Instance()->configDirPath() + "qimpanel_location.ini";
    if(QFile::exists(file_path))
    {
        QSettings settings(file_path,QSettings::IniFormat);
        int temp_x = settings.value("pos_x",-1).toInt();
        int temp_y = settings.value("pos_y",-1).toInt();

        int toolbar_width = this->rect().width();
        int toolbar_height = this->rect().height();

        int center_x = temp_x + toolbar_width/2;
        int center_y = temp_y + toolbar_height/2;

        //防止默认位置超出屏幕
        if(((center_x > 0) && (center_x < desktop_width)) && ((center_y > 0) && (center_y < (desktop_height -30))))
        {
            this->show();
            this->move(QPoint(temp_x, temp_y));
        }
        else
        {
            show_default_pos();
        }
    }
    else
    {
        show_default_pos();
    }

}

void ToolbarWindow::HideChildWindows()
{
    m_toolbar_main_menu->HideAllMenu();
}

void ToolbarWindow::DrawButton(int index, QSvgRenderer* normal, QSvgRenderer* hover, QSvgRenderer* press)
{
    QPainter p(this);
    Rect rect = m_button_rects.at(index);

    if (m_current_down_index == index)
        press->render(&p, ConvertRectToQRectF(rect));
    else if (m_current_move_index == index)
        hover->render(&p, ConvertRectToQRectF(rect));
    else
        normal->render(&p, ConvertRectToQRectF(rect));
}

void ToolbarWindow::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)
    if(!m_skin_base || !m_toolbar_model)
        return;
    QPainter p(this);
    p.setRenderHint(QPainter::HighQualityAntialiasing, true);
    p.setBrush(QColor(0xff, 0xff, 0xff, 0xff));
    p.fillRect(0, 0, m_whole_window.width, m_whole_window.height, p.brush());

    DrawBackgroud(&p);

    bool is_ocr_exist = AddonConfig::Instance()->IsAddonExist("OCR");
    bool is_voice_exist = AddonConfig::Instance()->IsAddonExist("AsrInput");

    WindowConfig* WC = WindowConfig::Instance();
    int index = 0;
    //logo
    {
        if(m_button_rects.size() <= index)
            return;
        Rect rect = m_button_rects.at(index);
        m_image_logo->render(&p, ConvertRectToQRectF(rect));
        index++;
    }
    if(WC->toolbar_config & USE_WUBI_PINYIN)
    {
        if(m_button_rects.size() <= index)
            return;

        if(m_toolbar_model->wb())
            DrawButton(index, m_image_wubi, m_image_wubi_hover, m_image_wubi_press);
        else
            DrawButton(index, m_image_pinyin, m_image_pinyin_hover, m_image_pinyin_press);

        index++;
    }
    if(WC->toolbar_config & USE_ENG_CN)
    {
        if(m_button_rects.size() <= index)
            return;

        if(m_toolbar_model->cn())
            DrawButton(index, m_image_cn, m_image_cn_hover, m_image_cn_press);
        else
            DrawButton(index, m_image_en, m_image_en_hover, m_image_en_press);

        index++;
    }
    if(WC->toolbar_config & USE_PUNCTUATION)
    {
        if(m_button_rects.size() <= index)
            return;

        if(m_toolbar_model->cnMark())
            DrawButton(index, m_image_cn_punctuation, m_image_cn_punctuation_hover, m_image_cn_punctuation_press);
        else
            DrawButton(index, m_image_en_punctuation, m_image_en_punctuation_hover, m_image_en_punctuation_press);

        index++;
    }
    if(WC->toolbar_config & USE_FULL_HALF_WIDTH)
    {
        if(m_button_rects.size() <= index)
            return;

        if(m_toolbar_model->fullwidth())
            DrawButton(index, m_image_full_width, m_image_full_width_hover, m_image_full_width_press);
        else
            DrawButton(index, m_image_half_width, m_image_half_width_hover, m_image_half_width_press);

        index++;
    }
    if(WC->toolbar_config & USE_FAN_JIAN)
    {
        if(m_button_rects.size() <= index)
            return;

        if(m_toolbar_model->trad())
            DrawButton(index, m_image_traditional, m_image_traditional_hover, m_image_traditional_press);
        else
            DrawButton(index, m_image_simplified, m_image_simplified_hover, m_image_simplified_press);

        index++;
    }
    if((WC->toolbar_config & USE_OCR_ADDON) && is_ocr_exist)
    {
        if(m_button_rects.size() <= index)
            return;
        DrawButton(index, m_image_ocr, m_image_ocr_hover, m_image_ocr_press);
        index++;
    }

    if((WC->toolbar_config & USE_VOICE_ADDON) && is_voice_exist)
    {
        if(m_button_rects.size() <= index)
            return;
        DrawButton(index, m_image_voice, m_image_voice_hover, m_image_voice_press);
        index++;
    }
    if(WC->toolbar_config & USE_SPECIAL_SYMBOL)
    {
        if(m_button_rects.size() <= index)
            return;

        DrawButton(index, m_image_special_symbol, m_image_special_symbol_hover, m_image_special_symbol_press);

        index++;
    }
    if(WC->toolbar_config & USE_QUAN_SHUANG)
    {
        if(m_button_rects.size() <= index)
            return;

        if(m_main_controller->GetPinyinMode() == 0)
            DrawButton(index, m_image_quan, m_image_quan_hover, m_image_quan_press);
        else
            DrawButton(index, m_image_shuang, m_image_shuang_hover, m_image_shuang_hover);

        index++;
    }

    if(WC->toolbar_config & USE_SETTINGS)
    {
        if(m_button_rects.size() <= index)
            return;
        if(!m_main_controller->HasUpdate())
            DrawButton(index, m_image_settings, m_image_settings_hover, m_image_settings_press);
        else
            DrawButton(index, m_image_settings_update, m_image_settings_update_hover, m_image_settings_update_press);

        index++;
    }

    ShowToolTip();
}

void ToolbarWindow::slot_simple_tra_changed(int state)
{
     m_main_controller->changeTrad(state);
     RefreshWindow();
}

void ToolbarWindow::mousePressEvent(QMouseEvent *event)
{
    QPoint p = event->pos();
    m_current_down_index = CalMousePosOnToolbar(&p);
    mouse_has_moved = false;    
    CustomizeQWidget::mousePressEvent(event);
    m_is_pressed = true;
    RefreshWindow();
}

void ToolbarWindow::mouseMoveEvent(QMouseEvent *event)
{
    QPoint p = event->pos();
    m_current_move_index = CalMousePosOnToolbar(&p);
    mouse_has_moved = true;

    setCursor(Qt::PointingHandCursor);

    if(m_is_pressed)
    {
        QPoint pos = this->pos();
        QPoint newPos(pos.x(),pos.y() - m_toolbar_config_widget->height() + 3);
        m_toolbar_config_widget->MoveToNewPos(this->mapToGlobal(QPoint(0,0)));

        int target_x = pos.x();
        int target_y = pos.y() - m_toolbar_main_menu->height();
        m_toolbar_main_menu->move(target_x,target_y);
        m_toolbar_main_menu->HideAllMenu();
    }

    CustomizeQWidget::mouseMoveEvent(event);

    if (m_current_move_index != m_last_move_index)
    {
        RefreshWindow();
        m_last_move_index = m_current_move_index;
    }
}

void ToolbarWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(mouse_has_moved)
    {
        //Record toolbar position
        ReordToolBarPos();
    }
    if(!mouse_has_moved)
    {
        QPoint p = event->pos();
        int index = CalMousePosOnToolbar(&p);
        if(event->button() == Qt::LeftButton)
        {
            if(index == m_current_down_index)
                OnButtonRelase(index);
        }
        else if(event->button() == Qt::RightButton)
        {
            if((index == m_current_down_index) && (index < 1)){return;}

            QPoint pos = this->pos();
            int target_x = pos.x();
            int target_y = pos.y() - m_toolbar_main_menu->height();
            m_toolbar_main_menu->move(target_x,target_y);

            bool show_flag = m_toolbar_main_menu->isVisible();
            m_toolbar_main_menu->setVisible(!show_flag);
            m_toolbar_config_widget->hide();
        }
    }
    CustomizeQWidget::mouseReleaseEvent(event);
    m_current_down_index = -1;
    m_is_pressed = false;
    RefreshWindow();
}

void ToolbarWindow::leaveEvent(QEvent* event)
{
    Q_UNUSED(event)

    m_current_move_index = -1;
    m_last_move_index = -1;
    m_current_down_index = -1;
    unsetCursor();
    RefreshWindow();
}

void ToolbarWindow::hideEvent(QHideEvent *event)
{
    m_toolbar_main_menu->HideAllMenu();
    m_toolbar_config_widget->hide();
    event->accept();
}

void ToolbarWindow::ReordToolBarPos()
{
    QString file_path = Config::Instance()->configDirPath() + "qimpanel_location.ini";
    QSettings settings(file_path,QSettings::IniFormat);
    QPoint position = this->pos();
    if(!QFile::exists(file_path))
    {
        settings.setValue("pos_x",position.x());
        settings.setValue("pos_y",position.y());
    }
    else
    {
        int pos_x = settings.value("pos_x",-1).toInt();
        int pos_y = settings.value("pos_y",-1).toInt();
        if(pos_x < 0 || pos_y < 0)
        {
            settings.setValue("pos_x",position.x());
            settings.setValue("pos_y",position.y());
        }
        else
        {
            if((pos_x != position.x()) || (pos_y != position.y()))
            {
                settings.setValue("pos_x",position.x());
                settings.setValue("pos_y",position.y());
            }
        }
    }

}

void ToolbarWindow::DrawBackgroud(QPainter* p)
{
    m_whole_window.x = m_whole_window.y = 0;
    m_image_background->render(p, ConvertRectToQRectF(m_whole_window));
}

QRectF ToolbarWindow::ConvertRectToQRectF(const Rect& rect)
{
    return QRectF(rect.x, rect.y, rect.width, rect.height);
}

int ToolbarWindow::CalMousePosOnToolbar(QPoint* p)
{
    for(int i = 0; i < m_button_rects.size(); i++)
    {
        if(m_button_rects[i].x + m_button_rects[i].width >= p->x())
            return i;
    }
    return -1;
}

#define SHOW_TIP(tips) tip->ShowTip(tips, GetBottom(), GetTop())
void ToolbarWindow::ShowToolTip()
{
    Config::Instance()->LoadConfig();
    auto tip = ToolTip::Instance();
    if (m_current_move_index == -1)
    {
        tip->Hide();
        return;
    }

    int index = m_current_move_index;

    if(index < 1)
    {
        SHOW_TIP("自定义状态栏");
        return;
    }
    index --; //跳过logo

    if(m_button_type[index] == USE_WUBI_PINYIN)
    {
        QString shortcut_info;
        int use_wubi_pinyin = Config::Instance()->getUse_wubi_pinyin();
        if(use_wubi_pinyin)
        {
            int key_value = Config::Instance()->getKey_wubi_pinyin();
            QString keyValue = QString(QChar(key_value));
            shortcut_info = QString("(Ctrl+Shift+%1)").arg(keyValue);
        }
        SHOW_TIP(QString("拼音/五笔%1").arg(shortcut_info));
    }
    else if(m_button_type[index] == USE_ENG_CN)
    {

       QString change_mode_info;
       int key_change = Config::Instance()->key_change_mode();
       if(key_change == 0)
       {
           change_mode_info = QString("(Shift)");
       }
       else if(key_change == 1)
       {
           change_mode_info = QString("(Ctrl)");
       }
       SHOW_TIP(QString("中/英文%1").arg(change_mode_info));

    }
    else if(m_button_type[index] == USE_PUNCTUATION)
    {
       SHOW_TIP("中/英文标点");
    }
    else if(m_button_type[index] == USE_FULL_HALF_WIDTH)
    {
        QString shortcut_info;
        int use_voice = Config::Instance()->getUse_shift_space_switch_full_half();
        if(use_voice)
        {
            shortcut_info = QString("(Shift+Space)");
        }
       tip->Hide();
       SHOW_TIP(QString("全/半角%1").arg(shortcut_info));
    }
    else if(m_button_type[index] == USE_FAN_JIAN)
    {
        QString shortcut_info;
        int use_jian_fan = Config::Instance()->use_key_jian_fan();
        if(use_jian_fan)
        {
            int key_value = Config::Instance()->key_jian_fan();
            QString keyValue = QString(QChar(key_value));
            shortcut_info = QString("(Ctrl+Shift+%1)").arg(keyValue);
        }
       tip->Hide();
       SHOW_TIP(QString("简/繁体%1").arg(shortcut_info));
    }
    else if(m_button_type[index] == USE_SPECIAL_SYMBOL)
    {
       SHOW_TIP("符号大全");
    }
    else if(m_button_type[index] == USE_QUAN_SHUANG)
    {
        QString shortcut_info;
        int use_quan_shuang = Config::Instance()->use_key_quan_shuang_pin();
        if(use_quan_shuang)
        {
            int key_value = Config::Instance()->key_quan_shuang_pin();
            QString keyValue = QString(QChar(key_value));
            shortcut_info = QString("(Ctrl+Shift+%1)").arg(keyValue);
        }
       SHOW_TIP(QString("全/双拼%1").arg(shortcut_info));
    }
    else if(m_button_type[index] == USE_SETTINGS)
    {
       SHOW_TIP("设置");
    }
	else if(m_button_type[index] == USE_OCR_ADDON)
    {
        QString shortcut_info;
        int use_ocr = Config::Instance()->getUse_ocr();
        if(use_ocr)
        {
            int key_value = Config::Instance()->getKey_ocr();
            QString keyValue = QString(QChar(key_value));
            shortcut_info = QString("(Ctrl+Shift+%1)").arg(keyValue);
        }
       SHOW_TIP(QString("截图识字%1").arg(shortcut_info));
    }
    else if(m_button_type[index] == USE_VOICE_ADDON)
    {
        QString shortcut_info;
        int use_voice = Config::Instance()->getUse_voice();
        if(use_voice)
        {
            int key_value = Config::Instance()->getKey_voice();
            QString keyValue = QString(QChar(key_value));
            shortcut_info = QString("(Ctrl+Shift+%1)").arg(keyValue);
        }
       SHOW_TIP(QString("语音识别%1").arg(shortcut_info));
    }
    else
    {
        tip->Hide();
    }
}

int ToolbarWindow::GetTop()
{
    return this->y();
}

int ToolbarWindow::GetBottom()
{
    return this->y() + this->height();
}

void ToolbarWindow::OnButtonRelase(int index)
{
    if(index < 1)
    {
        QPoint pos = this->pos();
        m_toolbar_config_widget->move(pos.x(),pos.y() - m_toolbar_config_widget->height() + 3);
        bool show_flag = m_toolbar_config_widget->isVisible();
        m_toolbar_config_widget->setVisible(!show_flag);
        m_toolbar_config_widget->MoveToNewPos(this->mapToGlobal(QPoint(0,0)));
        return;
    }
    index --; //跳过logo
    if(m_button_type.size() <= index)
        return;

    if(m_button_type[index] != USE_SETTINGS)
    {
        m_toolbar_main_menu->HideAllMenu();
    }
    if(m_button_type[index] == USE_WUBI_PINYIN)
    {
        m_main_controller->toggleWbOrPinyinMode(!m_toolbar_model->wb());
    }
    else if(m_button_type[index] == USE_ENG_CN)
    {
       m_main_controller->changeCnMode(!m_toolbar_model->cn());
    }
    else if(m_button_type[index] == USE_PUNCTUATION)
    {
       m_main_controller->changePunc(!m_toolbar_model->cnMark());
    }
    else if(m_button_type[index] == USE_FULL_HALF_WIDTH)
    {
       m_main_controller->changeFullWidth(!m_toolbar_model->fullwidth());
    }
    else if(m_button_type[index] == USE_FAN_JIAN)
    {
       m_main_controller->changeTrad(!m_toolbar_model->trad());
    }
    else if(m_button_type[index] == USE_SPECIAL_SYMBOL)
    {
       m_main_controller->showSymbolsDialog(true);
    }
    else if(m_button_type[index] == USE_QUAN_SHUANG)
    {
       bool pintyin_mode = m_main_controller->GetPinyinMode();
       m_main_controller->ChangePinyinMode(!pintyin_mode);

    }
    else if(m_button_type[index] == USE_SETTINGS)
    {
        QPoint local_pos(this->width(),0);
        QPoint target_pos = this->mapToGlobal(local_pos);
        m_main_controller->showToolBox(target_pos);
        m_toolbar_config_widget->hide();
    }
	else if(m_button_type[index] == USE_OCR_ADDON)
    {
        m_main_controller->execuate_ocr_addon();
    }
    else if(m_button_type[index] == USE_VOICE_ADDON)
    {
        m_main_controller->execuate_voice_addon();
    }
    else{}

    RefreshWindow();
}
