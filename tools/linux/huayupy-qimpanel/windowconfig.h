#ifndef WINDOWCONFIG_H
#define WINDOWCONFIG_H

#include <QString>


#define DEFAULT_OFFSET 2
#define DEFAULT_RADIUS 5
#define DEFAULT_FONT_SIZE 18
#ifdef USE_UOS
    #define DEFAULT_FONT "Noto Sans CJK SC"
#else
    #define DEFAULT_FONT "Droid Sans Fallback"
#endif
#define DEFAULT_TOP_MARGIN 5
#define DEFAULT_LEFT_MARGIN 5
#define DEFAULT_RIGHT_MARGIN 5
#define DEFAULT_BOTTOM_MARGIN 5
#define DEFAULT_CENTER_MARGIN 5
#define DEFAULT_GAP_OF_CANDIDATES 3

#define CAND_GAP_HEIGHT 2
#define	MIN_MAIN_WINDOW_WIDTH	100
#define MAIN_LINE_COLOR 0xe3d6c6
#define CELL_BACKGROUND_COLOR 0xeeeefb
#define SELECTED_PEN_COLOR 	0x0000ff				//选中的候选颜色, candidate_selected_color
#define CANDIDATE_STRING_COLOR 0x895a1e			    //普通候选颜色, candidate_string_color
#define INPUT_STRING_COLOR 0xc97a00				//组合编辑框的拼音串颜色, compose_string_color
#define INPUT_CART_COLOR 0x0000ff					//组合编辑框的光标颜色, compose_caret_color
#define MAIN_BACKGROUND_COLOR 0xfffcf8
#define LINE_BACKGROUND_COLOR 0xfff6ec

class WindowConfig
{
public:
    void LoadConfig();

    static WindowConfig* Instance();

    bool SetStrValue(const QString& name, const QString& value);
    bool SetIntValue(const QString& name, const int value);

    int		main_text_left_margin;					//文字左边界
    int		main_text_right_margin;					//文字右边界
    int		main_text_top_margin;					//文字上边界
    int		main_text_bottom_margin;				//文字下边界
    int		main_center_gap_height;					//中心高度，即：拼音串与候选串之间的高度
    int		main_remain_number_margin;				//候选剩余数目右边界

    int		main_vert_text_left_margin;				//文字左边界
    int		main_vert_text_right_margin;			//文字右边界
    int		main_vert_text_top_margin;				//文字上边界
    int		main_vert_text_bottom_margin;			//文字下边界
    int		main_vert_center_gap_height;			//中心高度，即：拼音串与候选串之间的高度
    int		main_vert_remain_number_margin;			//候选剩余数目右边界

    //主窗口最小高度、宽度
    int		main_window_min_width;					//默认为0
    int		main_window_min_height;					//默认为0

    int		main_window_vert_min_width;				//默认为0
    int		main_window_vert_min_height;			//默认为0

    int		hint_right_margin;						//横排功能提示右边界，默认为0

    //竖排功能提示右边界
    int		hint_vert_right_margin;					//默认为0

    //当鼠标划过候选词的颜色
    int     on_mouse_hover_color;					//默认先来个绿色

    int		use_vert_bk_when_expanding;				//tab扩展时使用竖排背景

    //主题应用的范围：水平、垂直、扩展
    int		theme_use_for_horizon_mode;				//默认为1
    int		theme_use_for_vertical_mode;			//默认为1
    int		theme_use_for_expand_mode;				//默认为1

    //主窗口的锚点（左上角位置），用于设定窗口与光标的相对位置
    int		main_window_anchor_x;					//默认为0
    int		main_window_anchor_y;					//默认为0

    int		main_window_vert_anchor_x;				//默认为0
    int		main_window_vert_anchor_y;				//默认为0

    int		main_line_left_margin;					//中心线左边界
    int		main_line_right_margin;					//中心线右边界

    int		main_vert_line_left_margin;				//中心线左边界
    int		main_vert_line_right_margin;			//中心线右边界

                                                    //背景图像的设置参数
    int		main_left_margin;						//背景图像的左边界，默认2
    int		main_right_margin;						//背景图像的右边界，默认2
    int		main_top_margin;						//背景图像的上边界，默认2
    int		main_bottom_margin;						//背景图像的下边界，默认2

    int		main_vert_left_margin;					//背景图像的左边界，默认2
    int		main_vert_right_margin;					//背景图像的右边界，默认2
    int		main_vert_top_margin;					//背景图像的上边界，默认2
    int		main_vert_bottom_margin;				//背景图像的下边界，默认2

    int		main_center_mode;						//背景中心绘制模式，默认：拉伸
    int		main_vertical_mode;						//垂直绘制模式，默认：拉伸
    int		main_horizontal_mode;					//水平绘制模式，默认：拉伸
    int		main_line_mode;							//中心线绘制模式，默认：拉伸

    int		main_vert_center_mode;					//背景中心绘制模式，默认：拉伸
    int		main_vert_vertical_mode;				//垂直绘制模式，默认：拉伸
    int		main_vert_horizontal_mode;				//水平绘制模式，默认：拉伸
    int		main_vert_line_mode;					//中心线绘制模式，默认：拉伸

    int     toolbar_config;

    int     border_width;                           //候选栏边框宽度

    int     font_height;
    QString     chinese_font_name;
    QString     english_font_name;
    int     compose_string_color;
    int     candidate_selected_color;
    int     candidate_string_color;
    int     main_background_color;
    int     main_line_color;
    int     compose_caret_color;
    int     line_background_color;
    int     cell_background_color;
    int     candidates_per_line;
    int     show_vertical_candidate;

private:
    WindowConfig();
    void MakeDefault();

};

#endif // WINDOWCONFIG_H
