#ifndef TOOLBARWINDOW_H
#define TOOLBARWINDOW_H

#include "customize_qwidget.h"
#include "skin/skinbase.h"

#include "windowconfig.h"
#include "toolbarconfigwidget.h"

#include <QImage>
#include <QRectF>
#include <map>
#include <QHideEvent>
using namespace std;

class MainController;
class SkinBase;
class ToolBarModel;
class QSvgRenderer;
class ToolBarMainMenu;
class ToolbarWindow : public CustomizeQWidget
{
public:
    ToolbarWindow();
    ~ToolbarWindow();
    void RefreshWindow();
    void ToShow();
    void HideChildWindows();
    void DrawWindow();
private:
    void CalWindow();
    void LoadImage();

    void DrawBackgroud(QPainter* p);

    QRectF ConvertRectToQRectF(const Rect& rect);

    int CalMousePosOnToolbar(QPoint* p);
    void OnButtonRelase(int index);    
    void DrawButton(int index, QSvgRenderer* normal, QSvgRenderer* hover, QSvgRenderer* press);
    void ShowToolTip();
    int GetTop();
    int GetBottom();

private slots:
    void paintEvent(QPaintEvent *e);
    void slot_simple_tra_changed(int state);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void leaveEvent(QEvent* event);
    void hideEvent(QHideEvent *event);
    void ReordToolBarPos();

private:
    Rect m_whole_window;

    QList<Rect> m_button_rects;
    QList<int> m_button_type;

    MainController* m_main_controller;
    SkinBase* m_skin_base;
    ToolBarModel* m_toolbar_model;

    QSvgRenderer* m_image_background;
    QSvgRenderer* m_image_logo;

    QSvgRenderer* m_image_cn;
    QSvgRenderer* m_image_en;
    QSvgRenderer* m_image_full_width;
    QSvgRenderer* m_image_half_width;
    QSvgRenderer* m_image_cn_punctuation;
    QSvgRenderer* m_image_en_punctuation;
    QSvgRenderer* m_image_simplified;
    QSvgRenderer* m_image_traditional;
    QSvgRenderer* m_image_special_symbol;
    QSvgRenderer* m_image_settings;
    QSvgRenderer* m_image_settings_update;
    QSvgRenderer* m_image_pinyin;
    QSvgRenderer* m_image_wubi;
    QSvgRenderer* m_image_quan;
    QSvgRenderer* m_image_shuang;
	QSvgRenderer* m_image_ocr;
    QSvgRenderer* m_image_voice;

    QSvgRenderer* m_image_cn_hover;
    QSvgRenderer* m_image_en_hover;
    QSvgRenderer* m_image_full_width_hover;
    QSvgRenderer* m_image_half_width_hover;
    QSvgRenderer* m_image_cn_punctuation_hover;
    QSvgRenderer* m_image_en_punctuation_hover;
    QSvgRenderer* m_image_simplified_hover;
    QSvgRenderer* m_image_traditional_hover;
    QSvgRenderer* m_image_special_symbol_hover;
    QSvgRenderer* m_image_settings_hover;
    QSvgRenderer* m_image_settings_update_hover;
    QSvgRenderer* m_image_pinyin_hover;
    QSvgRenderer* m_image_wubi_hover;
    QSvgRenderer* m_image_quan_hover;
    QSvgRenderer* m_image_shuang_hover;
	QSvgRenderer* m_image_ocr_hover;
    QSvgRenderer* m_image_voice_hover;

    QSvgRenderer* m_image_cn_press;
    QSvgRenderer* m_image_en_press;
    QSvgRenderer* m_image_full_width_press;
    QSvgRenderer* m_image_half_width_press;
    QSvgRenderer* m_image_cn_punctuation_press;
    QSvgRenderer* m_image_en_punctuation_press;
    QSvgRenderer* m_image_simplified_press;
    QSvgRenderer* m_image_traditional_press;
    QSvgRenderer* m_image_special_symbol_press;
    QSvgRenderer* m_image_settings_press;
    QSvgRenderer* m_image_settings_update_press;
    QSvgRenderer* m_image_pinyin_press;
    QSvgRenderer* m_image_wubi_press;
    QSvgRenderer* m_image_quan_press;
    QSvgRenderer* m_image_shuang_press;
	QSvgRenderer* m_image_ocr_press;
    QSvgRenderer* m_image_voice_press;

    int m_current_down_index;
    int m_current_move_index;
    int m_last_move_index;
    bool mouse_has_moved;
    bool m_has_showed;
    ToolbarConfigWidget* m_toolbar_config_widget;
    ToolBarMainMenu* m_toolbar_main_menu;
    bool m_is_pressed = false;
};

#endif // TOOLBARWINDOW_H
