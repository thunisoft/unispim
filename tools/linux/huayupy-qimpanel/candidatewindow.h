#ifndef CANDIDATEWINDOW_H
#define CANDIDATEWINDOW_H

#include "customize_qwidget.h"
#include "deletetipwidget.h"
#include "kimpanelagent.h"
#include <QMouseEvent>
#include <QDomDocument>
#include <QKeyEvent>
#include <QEvent>

#ifdef USE_UOS
#include "uosmodemonitor.h"
#endif


#define DRAW_MID_LINE (1 << 0)
#define DRAW_CANDIDATE (1 << 1)
#define DRAW_LEFT_RIGHT (1 << 2)

//#include <QSvgRenderer>
class QGraphicsDropShadowEffect;
class QSvgRenderer;
class QColor;
class QPainter;

class WindowConfig;
class KimpanelAgent;
class MainModel;

typedef struct tagCandiateRect {
    Rect candidate;
    Rect index;

    int use_index;
    int x;
    int y;
    int width;
    int height;
    int is_name_candidate;
}CandiateRect;

enum COMPOSE_TYPE {
    TYPE_EN = 0,
    TYPE_CN,
    TYPE_CART
};

typedef struct tagInputRect {
    COMPOSE_TYPE type = TYPE_EN;
    QString data;
    Rect rect = {0, 0, 0, 0};
}InputRect;

enum AREA_TYPE {
    Area_None = 0,
    Area_Candiate,
    Area_Button
};

typedef struct tagMouseState {
    AREA_TYPE type;
    int index;
} MouseState;

enum MouseEvent {
    Left_Button_Down,
    Left_Button_Release,
    Mouse_Move
};

enum VIEW_MODE {
    VIEW_MODE_HORIZONTAL,
    VIEW_MODE_VERTICAL,
    VIEW_MODE_EXPAND
};

enum SHOW_MODE {
   SHOW_WITH_CANDIDATE,
   SHOW_WITHOUT_CANDIDATE,
   SHOW_U_TIPS,
   SHOW_I_TIPS,
   SHOW_BH_TIPS //笔画提示
};

class ToolBarMainMenu;
class CandidateWindow : public CustomizeQWidget
{
    Q_OBJECT
public:
    CandidateWindow(KimpanelAgent* kimpanel_agent);
    ~CandidateWindow();
    void ToShow();    
    void DrawInputWindow();
    void RefreshWindow();

    void SetInputString(const QString& str);
    void SetCandidates(const KimpanelLookupTable &table);
    void SetIsHorizontal(bool is_horizontal);
    void SetCandidatesViewMode(VIEW_MODE inputMode);
    void SetHightIndex(int index);
    void SetCartIndex(int pos);
    void SetFontSize(const int font_size);
    void SetChineseFont(const QString& font_name);
    void SetEnglishFont(const QString& font_name);
    void ChangeLogoAndButtonColor(const int color);

    void SetSystemMode(const bool mode);
    bool IsSpecialMode();
    void SetPersonNameCandidatesIndex(const int index);
    void ConvertIntColorToString(int color,QString& low_color_str,QString& high_color_str);
signals:
    void toUpdate();


private slots:
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void leaveEvent(QEvent *event);
    void slot_remove_ci_by_index(const int ci_index);
    void slot_simulate_click_ci(const int ci_index);
    void slot_expand_candidates();
    void slot_simple_tra_changed(int state);
    void showEvent(QShowEvent* event);
    void hideEvent(QHideEvent *event);
    void slot_hide_main_menu();

private:
    void DrawShadow(const int shadow_offset, const int circle_radius);
    void CalStrWidthAndHeight(const QString& str, const QFont& font, Rect& rect);
    void CalStrWidthAndHeight(InputRect& rect, const QFont& font);
    int GetDotRightSpace();
    int CalInputViewWidthAndHeight();
    void Clear();
    void Init();
    void Paint();
    void InsertTestData();
    int GetScaleValue(int origin_value);

    void CalInputViewAreaByHorizontalMode();
    void CalInputViewAreaByVerticalMode();
    void CalInputViewAreaByExpandMode();
    void CalInputViewAreaByNoCandidate();
    void CalInputViewAreaByTipsMode();
    int CalScaleWithFont(int value);
    void LoadImage();
    void SwitchMode();
    void MakeHightAdjusted();
    void CalInputRect();
    int GetCiGap();
    bool IsChineseChar(const QChar& c);
    void MakeSelectedRect(Rect& rect, const int index);

    QColor ConvertIntToQColor(int color_value, const int transparent = 255);
    void ClearMouseState();

    bool ProcessMouseEvent(QPoint* p, MouseEvent event = Mouse_Move);
    void CallbackEngine(const MouseState mouse_state);

    VIEW_MODE SwitchShowModel();
    void MakeColor(int color);

    void CalWindowRectPos();
    void CalWholeWindowSize();
    void CalPersonNameIconSize();

private:

    KimpanelAgent* m_kimpanel_agent;
    QGraphicsDropShadowEffect* m_shadow_corner_maker;
    DeleteTipWidget* m_delete_tip_window;
    DeleteTipWidget* m_person_name_tip_window = nullptr;
    ToolBarMainMenu* m_toolbar_main_menu = nullptr;


    QString m_input_string;
    QStringList m_candidates;
    QStringList m_candidates_indexs;

    Rect m_window_rect;
    Rect m_rect_input;
    Rect m_rect_left;
    Rect m_rect_right;
    Rect m_rect_logo;
    Rect m_rect_tab;
    Rect m_rect_mid_line;
    Rect m_rect_whole_window;
    Rect m_expand_mode_current_row_rect;
    Rect m_person_name_candidate_rect;

    QList<InputRect> m_rect_inputs;
    QList<CandiateRect> m_rects_candidate;
    QList<int> m_candidate_height;
    int m_font_size;
    QFont* m_font_input;
    QFont* m_font_candidate;
    QFont* m_font_index;

    int m_top_margin;
    int m_left_margin;
    int m_right_margin;
    int m_bottom_margin;
    int m_center_margin;


    int m_gap_of_candiadtes;
    int m_max_candidate_width;
    int m_max_candidate_height;
    int m_candidate_line_width;
    int m_candidate_line_height;

    VIEW_MODE m_view_mode;
    QSvgRenderer* m_left_button_svg_low;
    QSvgRenderer* m_right_button_svg_low;
    QSvgRenderer* m_logo_svg;
    QSvgRenderer* m_left_button_svg_high;
    QSvgRenderer* m_right_button_svg_high;
    QSvgRenderer* m_tab_expand_button_svg;
    QSvgRenderer* m_person_name_candidate_svg;

    QDomDocument* m_left_button_dom;
    QDomDocument* m_right_button_dom;
    QDomDocument* m_logo_content_dom;
    QDomDocument* m_tab_button_dom;
    QDomDocument* m_person_name_dom;

    bool m_has_cal_done;
    int m_selected_index;

    bool m_has_pre;
    bool m_has_next;

    int m_pos_x;
    int m_pos_y;

    MouseState m_mouse_state;

    WindowConfig* WC;
    MainModel* m_main_model;

    QString m_low_color;
    QString m_high_color;

    int m_max_height; //input的最大高度

    SHOW_MODE m_show_mode;

    int m_element_should_draw;

    int m_cart_index;

    Rect m_rect_selected;

    bool m_is_spcial_mode;
    int m_candidates_lines = 1;
    bool m_receive_candidates_flag = false;
    int m_person_name_candidate_index = -1;

#ifdef USE_UOS
    UosModeMonitor* m_uos_mode_monitor;
#endif

};

#endif // CANDIDATEWINDOW_H
