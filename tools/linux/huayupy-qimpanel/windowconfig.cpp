#include "windowconfig.h"

#include <QSettings>
#include "../public/utils.h"
#include "../public/config.h"
#include "../public/configmanager.h"

WindowConfig* window_config = nullptr;

WindowConfig::WindowConfig()
{
    MakeDefault();
}

#define GET_VALUE_STR(name) name = config.value(#name, name).toString()
#define GET_VALUE_INT(name) name = config.value(#name, name).toInt()
void WindowConfig::LoadConfig()
{
    ConfigManager::Instance()->ReloadUserConfig();
    //字体大小
    ConfigManager::Instance()->GetIntConfigItem("font_height",font_height);
    //中文字体
    ConfigManager::Instance()->GetStrConfigItem("chinese_font_name",chinese_font_name);
    //英文字体
    ConfigManager::Instance()->GetStrConfigItem("english_font_name",english_font_name);
    //工具栏的样式
    ConfigManager::Instance()->GetIntConfigItem("toolbar_config",toolbar_config);

    ConfigManager::Instance()->GetIntConfigItem("compose_string_color",compose_string_color);
    ConfigManager::Instance()->GetIntConfigItem("candidate_selected_color",candidate_selected_color);
    ConfigManager::Instance()->GetIntConfigItem("candidate_string_color",candidate_string_color);
    ConfigManager::Instance()->GetIntConfigItem("main_background_color",main_background_color);
    ConfigManager::Instance()->GetIntConfigItem("main_line_color",main_line_color);
    ConfigManager::Instance()->GetIntConfigItem("compose_caret_color",compose_caret_color);
    ConfigManager::Instance()->GetIntConfigItem("line_background_color",line_background_color);
    ConfigManager::Instance()->GetIntConfigItem("cell_background_color",cell_background_color);
    ConfigManager::Instance()->GetIntConfigItem("show_vertical_candidate",show_vertical_candidate);

}

void WindowConfig::MakeDefault()
{
    main_text_left_margin = 12;                 //文字左边界
    main_text_right_margin = 8;					//文字右边界
    main_text_top_margin = 5;					//文字上边界
    main_text_bottom_margin = 5;				//文字下边界
    main_center_gap_height = 0;					//中心高度，即：拼音串与候选串之间的高度
    main_remain_number_margin = 16;				//候选剩余数目右边界

    main_vert_text_left_margin = 6;				//文字左边界
    main_vert_text_right_margin = 6;			//文字右边界
    main_vert_text_top_margin = 10;				//文字上边界
    main_vert_text_bottom_margin = 4;			//文字下边界
    main_vert_center_gap_height = 10;			//中心高度，即：拼音串与候选串之间的高度
    main_vert_remain_number_margin = 16;			//候选剩余数目右边界

    //主窗口最小高度、宽度
    main_window_min_width = 0;					//默认为0
    main_window_min_height = 0;					//默认为0

    main_window_vert_min_width = 0;				//默认为0
    main_window_vert_min_height = 0;			//默认为0

    hint_right_margin = 0;						//横排功能提示右边界，默认为0

    //竖排功能提示右边界
    hint_vert_right_margin = 0;					//默认为0

    //当鼠标划过候选词的颜色
    on_mouse_hover_color = 0x00ff00;					//默认先来个绿色

    use_vert_bk_when_expanding = 1;				//tab扩展时使用竖排背景

    //主题应用的范围：水平、垂直、扩展
    theme_use_for_horizon_mode = 1;				//默认为1
    theme_use_for_vertical_mode = 1;			//默认为1
    theme_use_for_expand_mode = 1;				//默认为1

    //主窗口的锚点（左上角位置），用于设定窗口与光标的相对位置
    main_window_anchor_x = 0;					//默认为0
    main_window_anchor_y = 0;					//默认为0

    main_window_vert_anchor_x = 0;				//默认为0
    main_window_vert_anchor_y = 0;				//默认为0

    main_line_left_margin = 8;					//中心线左边界
    main_line_right_margin = 8;					//中心线右边界

    main_vert_line_left_margin = 6;				//中心线左边界
    main_vert_line_right_margin = 6;			//中心线右边界

                                                    //背景图像的设置参数
    main_left_margin = 2;						//背景图像的左边界，默认2
    main_right_margin = 2;						//背景图像的右边界，默认2
    main_top_margin = 2;						//背景图像的上边界，默认2
    main_bottom_margin = 2;						//背景图像的下边界，默认2

    main_vert_left_margin = 2;					//背景图像的左边界，默认2
    main_vert_right_margin = 2;					//背景图像的右边界，默认2
    main_vert_top_margin = 2;					//背景图像的上边界，默认2
    main_vert_bottom_margin = 2;				//背景图像的下边界，默认2

    toolbar_config = USE_WUBI_PINYIN | USE_ENG_CN | USE_PUNCTUATION | USE_FAN_JIAN | USE_SETTINGS, //工具栏显示的图标,;

    border_width = 1;                           //候选栏边框宽度
    font_height = DEFAULT_FONT_SIZE;       //字体大小
    chinese_font_name =  QString::fromLocal8Bit(DEFAULT_FONT);    //中文字体
    english_font_name =  QString::fromLocal8Bit(DEFAULT_FONT);    //英文字体

    compose_string_color = INPUT_STRING_COLOR;
    candidate_selected_color = SELECTED_PEN_COLOR;
    candidate_string_color = CANDIDATE_STRING_COLOR;
    main_background_color = MAIN_BACKGROUND_COLOR;
    main_line_color = MAIN_LINE_COLOR;
    compose_caret_color = INPUT_CART_COLOR;
    line_background_color = LINE_BACKGROUND_COLOR;
    cell_background_color = CELL_BACKGROUND_COLOR;

//    candidates_per_line = 5;
    show_vertical_candidate = 0;
}

WindowConfig* WindowConfig::Instance()
{
    if(!window_config)
    {
       window_config = new WindowConfig;
       window_config->LoadConfig();
    }

    return window_config;
}

bool WindowConfig::SetStrValue(const QString& name, const QString& value)
{
    do {
        if(name.compare("chinese_font_name") == 0)
        {
            chinese_font_name = value;
        }
        else if(name.compare("english_font_name") == 0)
        {
            english_font_name = value;
        }
        else
        {
            break;
        }
        return true;
    }while(false);

    return false;

}

bool WindowConfig::SetIntValue(const QString& name, const int value)
{
    do {
        if(name.compare("font_height") == 0)
        {
            font_height = value;
        }
        else if(name.compare("tool_bar_config") == 0)
        {
            toolbar_config = value;
        }
        else if(name.compare("compose_string_color") == 0)
        {
            compose_string_color = value;
        }
        else if(name.compare("candidate_selected_color") == 0)
        {
            candidate_selected_color = value;
        }
        else if(name.compare("candidate_string_color") == 0)
        {
            candidate_string_color = value;
        }
        else if(name.compare("main_background_color") == 0)
        {
            main_background_color = value;
        }
        else if(name.compare("main_line_color") == 0)
        {
            main_line_color = value;
        }
        else if(name.compare("compose_caret_color") == 0)
        {
            compose_caret_color = value;
        }
        else if(name.compare("line_background_color") == 0)
        {
            line_background_color = value;
        }
        else if(name.compare("cell_background_color") == 0)
        {
            cell_background_color = value;
        }
        else if(name.compare("show_vertical_candidate") == 0)
        {
            show_vertical_candidate = value;
        }
        else
        {
            break;
        }
        return true;
    }while(false);

    return false;
}
