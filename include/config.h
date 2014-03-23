/*	输入法配置头文件。
 */

//TODO: *****程序员符号模式，即在非中文的字符之间的/以及*符号一直为半角（需要进行很好的编程以及试验）

//注：本源代码在TAB设置为4，Indent设置为4的编辑器设置上显示才能正常。
/* Vim ts=4;sw=4 */

#ifndef	_CONFIG_H_
#define	_CONFIG_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <wordlib.h>
#include <windows.h>
#include <spw.h>

//配置程序运行后，改变的配置项
#define	MODIFY_NONE					0
#define	MODIFY_WORDLIB				(1 << 0)
#define	MODIFY_THEME				(1 << 1)
#define	MODIFY_INI					(1 << 2)
#define	MODIFY_ZI					(1 << 3)
#define MODIFY_WORDLIB_FORCE_RELOAD	(1 << 4) //词库有变动，强制重新装载词库(恢复数据的时候)
#define MODIFY_ENGLISH				(1 << 5)
//#define MODIFY_URL	     			(1 << 6)

//配置程序文件名
#define	IME_NAME					TEXT("UNISPIM6.IME")
//#define	UPDATE_EXE_NAME				TEXT("upimupd.exe")
#define	PPJZ_EXE_NAME				TEXT("upimrad.exe")
#define	SPW_EXE_NAME				TEXT("upimspw.exe")
//#define	URL_EXE_NAME				TEXT("upimurl.exe")
#define	COMPOSE_EXE_NAME			TEXT("upimcpz.exe")
#define	CONFIG_EXE_NAME				TEXT("upimcfg6.exe")
#define CFGWIZARD_EXE_NAME          TEXT("upimwiz6.exe")
#define THEME_MAKER_EXE_NAME        TEXT("ThemeEditor.exe")
//#define IMEMANAGER_EXE_NAME         TEXT("ImeTool.exe")
#define BATCH_WORDS_EXE_NAME        TEXT("BatchWords.exe")
#define ZIEDITOR_EXE_NAME           TEXT("ZiEditor.exe")

//注册表配置的Key名字
#define	REG_CONFIG_KEY_NAME			TEXT("Software\\Thunisoft\\Unispim\\6.9")
//#define REG_OLD_CONFIG_KEY_NAME		TEXT("Software\\Thunisoft\\Unispim\\6.6")
//#define	REG_INSTALL_DIR_KEY_NAME	TEXT("Software\\Thunisoft\\Unispim\\6.8")
#define REG_FULL_CONFIG_KEY_NAME	TEXT("HKEY_CURRENT_USER\\Software\\Thunisoft\\Unispim\\6.9")
#define	REG_PRELOAD_KEY_NAME		TEXT("Keyboard Layout\\Preload")

//注册表配置的项名字
#define	REG_CONFIG_ITEM_NAME		TEXT("Config")
#define	REG_INSTALL_DIR_NAME		TEXT("InstallDir")

#define	DEFAULT_THEME_NAME			TEXT("默认主题")
#define	THEME_DIR_NAME				TEXT("unispim6\\theme\\")
#define	MAIN_BK_IMAGE_NAME			TEXT("unispim6\\theme\\默认主题\\bg_main.png")
#define	MAIN_LINE_IMAGE_NAME		TEXT("unispim6\\theme\\默认主题\\line.png")
#define	STATUS_BK_IMAGE_NAME		TEXT("unispim6\\theme\\默认主题\\bg_status.png")
#define	STATUS_BUTTONS_IMAGE_NAME	TEXT("unispim6\\theme\\默认主题\\buttons.png")

#define	MAX_FILE_NAME_LENGTH		256				//文件名字的最大长度
#define	MAX_THEME_NAME_LENGTH		256				//主题名称的最大长度

#define	MAX_THEME_COUNT				64				//最大主题数目

//输入法风格
#define STYLE_CSTAR					0				//中文之星风格（默认）
#define STYLE_ABC					1				//智能ABC风格
#define	STYLE_MS2					2				//微软风格

//拼音方式
#define	PINYIN_QUANPIN				0				//全拼
#define	PINYIN_SHUANGPIN			1				//双拼

//启动输入法状态
#define	STARTUP_CHINESE				0				//启动时为英文输入状态
#define	STARTUP_ENGLISH				1				//启动时为中文输入状态（默认）

//候选选择方式
#define	SELECTOR_DIGITAL			0				//使用数字选择候选（默认）
#define	SELECTOR_LETTER				1				//使用字母选择候选

//简繁定义
#define	HZ_OUTPUT_SIMPLIFIED		(1 << 0)		//输出简体字（默认）
#define	HZ_OUTPUT_TRADITIONAL		(1 << 1)		//输出繁体字
#define	HZ_OUTPUT_HANZI_ALL 		(1 << 2)		//输出全集
#define	HZ_OUTPUT_ICW_ZI			(1 << 4)		//输出ICW使用的汉字集合
#define	HZ_SYMBOL_CHINESE			(1 << 5)		//中文符号
#define	HZ_SYMBOL_HALFSHAPE			(1 << 6)		//半角符号

#define HZ_SCOPE_GBK                (1 << 0)
#define HZ_SCOPE_UNICODE            (1 << 1)

//字选项
#define	HZ_RECENT_FIRST				(1 << 0)		//最近输入的字优先（默认）
#define	HZ_ADJUST_FREQ_FAST			(1 << 2)		//快速调整字频（默认）
#define	HZ_ADJUST_FREQ_SLOW			(1 << 3)		//慢速调整字频
#define	HZ_ADJUST_FREQ_NONE			(1 << 4)		//不调整字频
#define	HZ_USE_TAB_SWITCH_SET		(1 << 6)		//使用TAB切换汉字的集合（默认）
#define	HZ_USE_TWO_LEVEL_SET		(1 << 7)		//使用两种集合切分方式（默认）
#define	HZ_USE_THREE_LEVEL_SET		(1 << 8)		//使用三种集合切分方式
#define	HZ_USE_FIX_TOP				(1 << 9)		//使用固顶字（默认）

//词选项
#define	CI_AUTO_FUZZY				(1 << 0)		//输入词的时候，自动使用z/zh, c/ch, s/sh的模糊（默认）
#define	CI_SORT_CANDIDATES			(1 << 1)		//候选词基于词频进行排序（默认）
#define	CI_ADJUST_FREQ_FAST			(1 << 2)		//快速调整词频（默认）
#define	CI_ADJUST_FREQ_SLOW			(1 << 3)		//慢速调整词频（默认）
#define	CI_ADJUST_FREQ_NONE			(1 << 4)		//不调整词频（默认）
#define	CI_WILDCARD					(1 << 5)		//输入词的时候，使用通配符（默认）
#define	CI_RECENT_FIRST				(1 << 6)		//最新输入的词优先（默认）
#define	CI_USE_FIRST_LETTER			(1 << 7)		//使用首字母输入词（默认）
#define	CI_AUTO_VOW_FUZZY			(1 << 8)		//输入词的时候自动进行韵母首字母模糊匹配

//键盘对配置
#define	KEY_SWITCH_SHIFT			0				//SHIFT切换输入状态
#define	KEY_SWITCH_CONTROL			1				//CONTROL切换输入状态
#define	KEY_SWITCH_NONE				2				//不使用键盘切换输入状态
#define	KEY_2ND_3RD_NONE			0				//不使用第二/第三快速候选键
#define	KEY_2ND_3RD_CONTROL			1				//用CONTROL选择2、3号候选
#define	KEY_2ND_3RD_SHIFT			2				//用SHIFT选择2、3号候选
//#define KEY_2ND_3RD_SYMBOL          3               //使用;和'选择2、3号候选
#define KEY_ZI_FROM_CI_NONE			0				//不使用以词定字
#define	KEY_PAIR_1					(1 << 2)		//,.（候选翻页）
#define	KEY_PAIR_2					(1 << 3)		//[]（以词定字）
#define	KEY_PAIR_3					(1 << 4)		//-=
#define	KEY_PAIR_4					(1 << 5)		//()（智能编辑）
#define	KEY_PAIR_5					(1 << 6)		//<>（候选漫游）
#define KEY_PAIR_6					(1 << 7)		//PageUp/PageDown
#define KEY_PAIR_7					(1 << 8)		//Up/Down

//状态条配置选项
#define	STATUS_MORDEN_STYLE			0				//新的状态条
#define	STATUS_TRADITIONAL_STYLE	1				//传统的状态条

#define	STATUS_BUTTON_NUMBER		9				//状态窗口上Button的数目

#define	STATUS_BUTTON_MODE			(1 << 0)		//输入模式按钮
#define	STATUS_BUTTON_CHARSET		(1 << 1)		//常用、全集开关按钮
#define	STATUS_BUTTON_SYMBOL		(1 << 2)		//标点符号按钮
#define	STATUS_BUTTON_SHAPE			(1 << 3)		//全半角符号按钮
#define	STATUS_BUTTON_SOFTKBD		(1 << 4)		//软键盘开关按钮
#define	STATUS_BUTTON_SETUP			(1 << 5)		//配置工具按钮
#define	STATUS_BUTTON_HELP			(1 << 6)		//帮助按钮

//图象绘画方式
#define	DRAW_MODE_TILED				0				//平铺方式
#define	DRAW_MODE_STRETCH			1				//拉伸方式

//配置版本
#define CONFIG_VERSION              0x6500          //配置版本号

//配置选项采用在注册表中开辟内存DUMP区的方式
//进行存储。这样读取以及写入配置的过程可以得到很大的简化

//输入选项结构
typedef struct tagHYPIMCONFIG
{
	//输入风格
	int		input_style;							//默认STYLE_CSTAR

	//启动的输入方式：中文、英文
	int		startup_mode;							//默认为STARTUP_CHINESE

	//拼音模式
	int		pinyin_mode;							//默认为PINYIN_QUANPIN

	//是否显示双拼的提示
	int		show_sp_hint;							//默认为0

	//候选选择方式，字母、数字。
	int		select_sytle;							//默认为SELECTOR_DIGITAL

	//总是清除上下文
	int		clear_context_always;					//默认为1

	//IME自动感知相关设置。如果支持IME自动感知，应用程序可以自行
	//显示写作串以及候选串，输入法将不再显示这两个窗口。如果用户
	//还是要求显示两个窗口，则通过下面的选项进行设置。

	//支持IME感知程序
	int		support_ime_aware;						//默认为1

	//垂直显示候选
	int		show_vertical_candidate;				//默认为0

	//总是显示写作窗口
	int		show_composition_always;				//默认为0

	//总是显示候选窗口
	int		show_candidate_always;					//默认为0

	//是否隐藏状态条
	int		show_status_window;						//默认为1

	//是否使用汉字音调辅助
	int		use_hz_tone;							//默认为1

	//是否使用ICW（智能组词）
	int		use_icw;								//默认为1

	//是否保存ICW的结果到词库
	int		save_icw;								//默认为1

	//是否光标跟随
	int		trace_caret;							//默认为1

	//是否直接输入网址
	int		__url_direct_input;						//默认为1，已经废弃

	//首字母输入词汇。输入tagl，则产生候选“提案管理”。汉字数太少，有很大的问题。
	int		first_letter_input_min_hz;				//首字母输入的最小汉字数目，默认为3。

	//特殊方式解析拼音串。解决问题：fangan -> 反感、方案，以及xian->xi'an
	int		__special_parse_pin_yin;				//双向解析组合拼音串；废弃

	//在输入V后以英文方式进行输入
	int		input_space_after_v;					//允许在V模式下输入空格

	//是否在数字键之后，输入英文符号
	int		english_symbol_follow_number;			//默认为0

	//使用TAB切换汉字的集合
	int		use_tab_expand_candidates;				//默认为1

	//一直展开候选窗口
	int		always_expand_candidates;				//默认为0

	//汉字输出的方式：简体、繁体
	int		hz_output_mode;							//默认为HZ_OUTPUT_SIMPLIFIED

	//输入字的选项
	int		hz_option;								//默认为全部

	//词输入选项
	int		ci_option;								//默认为全部

	//是否使用模糊音，由于用户可能设置模糊音，不使用的时候，还应该保留，所以加入了这个选项。
	int		use_fuzzy;								//默认为0

	//模糊音选项
	int		fuzzy_mode;								//默认为0，全部不模糊

	//软键盘默认序号
	int		soft_kbd_index;							//默认为0

	//词库文件名称
	TCHAR	wordlib_name[MAX_WORDLIBS][MAX_FILE_NAME_LENGTH];
			//第一个默认为"wordlib/usr.uwl"(userapp)	用户词库
			//第二个默认为"wordlib/sys.uwl"(alluserapp)	系统词库
			//第二项以后才允许进行编辑

	//词库数量
	int		wordlib_count;							//默认为2，系统词库与用户词库不在本表中记录

	//一次能够输入的最大汉字数目
	int		max_hz_per_input;						//默认为20

	//显示在候选页中的候选的最多个数
	int		candidates_per_line;					//默认为5

	//候选翻页键对
	int		key_pair_candidate_page;				//默认为KEY_PAIR_1

	//候选漫游键对
	int		__key_pair_candidate_browse;				//默认为KEY_PAIR_5；好像没啥用。【废弃】

	//以词定字键对
	int		key_pair_zi_from_word;					//默认为KEY_PAIR_2

	//编辑键对
	int		key_prev_edit;							//默认为KEY_PAIR_4

	//变换中英文输入方式键
	int		key_change_mode;						//默认为KEY_SWICH_LSHIFT

	//输入第二个以及第三个候选的键
	int		key_candidate_2nd_3rd;					//默认为KEY_2ND_3RD_CONTROL

	//在数字之后，"."作为英文符号
	int		english_dot_follow_number;				//默认为1

	//扩展候选的行数
	int		expand_candidate_lines;					//默认为5

	//将用户使用过的词汇记录到用户词库中，
	int		insert_used_word_to_user_wl;			//默认为0

	//主窗口最小高度、宽度
	int		main_window_min_width;					//默认为0
	int		main_window_min_height;					//默认为0

	int		main_window_vert_min_width;				//默认为0
	int		main_window_vert_min_height;			//默认为0

	//主窗口与状态窗口的透明度
	int		main_window_transparency;				//默认为0
	int		status_window_transparency;				//默认为0

	//主窗口的锚点（左上角位置），用于设定窗口与光标的相对位置
	int		main_window_anchor_x;					//默认为0
	int		main_window_anchor_y;					//默认为0

	int		main_window_vert_anchor_x;				//默认为0
	int		main_window_vert_anchor_y;				//默认为0

	//状态窗口上边界
	int		status_window_top_margin;				//默认为0

	//主题应用的范围：水平、垂直、扩展
	int		theme_use_for_horizon_mode;				//默认为1
	int		theme_use_for_vertical_mode;			//默认为1
	int		theme_use_for_expand_mode;				//默认为1

	TCHAR	backup_dir[MAX_FILE_NAME_LENGTH];		//用户备份目录

	//禁止组合键，用于游戏
	int		disable_combined_key;					//默认为0

	int		config_version;							//配置版本

	int     trace_recent;                           //记忆最近输入过的词

	int     post_after_switch;                      //模式切换时输入内容上屏

	int     ignore_fullshape;                       //输入过程中忽略全角模式(全角模式下也可输入中文)

	//拼音纠错选项
	int     __correct_option;                       //默认全部纠错;废弃

	//使用自定义短语
	int		use_special_word;                       //默认为1

	//使用自定义中文符号
	//int     __use_chinese_symbol;                     //默认为1，已经废弃
	//双拼方案序号，使用旧的配置__use_chinese_symbol位置存储
	int shuangpin_selItem;

	//使用软键盘快捷键
	int		use_key_soft_kbd;                       //默认为1

	//软键盘快捷键
	int		key_soft_kbd;                           //默认为K

	//使用简繁切换快捷键
	int		use_key_jian_fan;						//默认为1

	//简繁切换快捷键
	int		key_jian_fan;							//默认为F

	//使用全拼双拼快捷键
	int		use_key_quan_shuang_pin;				//默认为0

	//全拼双拼切换快捷键
	int		key_quan_shuang_pin;					//默认为S

	//使用状态栏切换快捷键
	int		use_key_status_window;                  //默认为1

	//状态栏切换快捷键
	int		key_status_window;						//默认为Z

	//候选包含英文单词
	int		candidate_include_english;				//默认为1

	//英文单词后自动补空格
	int		space_after_english_word;				//默认为1

	//使用英文输入快捷键
	int		use_key_english_input;					//默认为1

	//英文输入快捷键
	int		key_english_input;						//默认为E

	//英文输入时竖排显示
	int		__english_candidate_vertical;				//默认为1；从6.8.0.23开始废弃

	//启动时进入英文输入法
	int		startup_with_english_input;				//默认为0

	//显示输入栏提示信息
	int		show_hint_msg;							//默认为1

	//横排功能提示右边界
	int		hint_right_margin;						//默认为0

	//竖排功能提示右边界
	int		hint_vert_right_margin;					//默认为0

	//短语文件名称
	TCHAR	spw_name[MAX_SPW_COUNT][MAX_FILE_NAME_LENGTH];

	//短语文件数量
	int		__spw_count;								//默认为1,可以废弃了

	//使用英文输入
	int		use_english_input;						//默认为1

	//使用英文翻译功能
	int		use_english_translate;					//默认为1

	//使用url hint功能
	//int		__use_url_hint;					        //默认为1,disuse

	//单个短语候选时，显示位置；使用上面废弃的配置项
	////为兼容以往的配置，spw_position的数值从11~19，使用时直接减10
	int    spw_position;                            //默认为13

	//使用词语联想
	int		use_word_suggestion;					//默认为1

	//从第x个音节开始联想
	int		suggest_syllable_location;				//默认为2

	//联想词位于候选词第x位
	int		suggest_word_location;					//默认为2

	//联想词个数
	int		suggest_word_count;						//默认为2

	//使用u command hint功能
	int		use_u_hint;			    		        //默认为1

	//输出字符集范围：gbk、unicode
	int		scope_gbk;							    //是否只输出gbk集合

	//显示Unicode Ext-B和Ext-C汉字
	int		hide_black_window;					    //开启屏蔽天窗功能

	//全屏应用时隐藏状态条
	int		hide_status_when_full_screen;			//默认为1

	//B模式开关
	int		B_mode_enabled;							//默认为1

	//D模式开关
	int		__D_mode_enabled;							//默认为1

	//I模式开关
	int		I_mode_enabled;							//默认为1

	//H模式开关
	int		__H_mode_enabled;							//默认为1

	//i模式开关
	int		i_mode_enabled;							//默认为1

	//u模式开关
	int		u_mode_enabled;							//默认为1

	//v模式开关
	int		v_mode_enabled;							//默认为1

	//小键盘数字选词
	int		numpad_select_enabled;					//默认为1

	//保留的配置选项，用于版本升级，不用重新覆盖注册表
	int		reserved[239];							//默认为{0}

	//状态条的类型：新版，传统
	int		status_style;							//默认为STATUS_MORDEN_STYLE

	//传统状态条上的按键
	int		status_buttons;							//默认为STATUS_BUTTON_MODE | STATUS_BUTTON_SYMBOL | STATUS_BUTTON_SHAPE | STATUS_BUTTON_SOFTKBD | STATUS_BUTTON_MENU

	//主题名称，用于进行主题的寻找判断
	TCHAR	theme_name[MAX_THEME_NAME_LENGTH];		//默认值为""

	//中文字体名称
	TCHAR	chinese_font_name[MAX_FILE_NAME_LENGTH];	//默认为“宋体”

	//英文以及符号的字体名称
	TCHAR	english_font_name[MAX_FILE_NAME_LENGTH];	//默认为"Arial"

	//字体高度
	int		font_height;							//默认为16

	//主窗口（写作、候选窗口）定义。在本输入法中，Compose、Candidate窗口没有区分，一直是同时显示
	//当应用窗口能够处理Compose或者Candidate时，通过设定来显示一部分内容即可。这个窗口命名为main_window。
	//双拼使用的窗口设置与main窗口完全相同。

	//主窗口背景图象名称，如果为NULL，则使用默认颜色进行绘画
	TCHAR	main_image_name[MAX_FILE_NAME_LENGTH];					//默认为""

	TCHAR	main_vert_image_name[MAX_FILE_NAME_LENGTH];				//默认为""

	//主窗体分割线图像名称
	TCHAR	main_line_image_name[MAX_FILE_NAME_LENGTH];				//默认为""

	TCHAR	main_vert_line_image_name[MAX_FILE_NAME_LENGTH];		//默认为""

	//背景图像的设置参数
	int		main_left_margin;						//背景图像的左边界，默认2
	int		main_right_margin;						//背景图像的右边界，默认2
	int		main_top_margin;						//背景图像的上边界，默认2
	int		main_bottom_margin;						//背景图像的下边界，默认2

	int		main_vert_left_margin;					//背景图像的左边界，默认2
	int		main_vert_right_margin;					//背景图像的右边界，默认2
	int		main_vert_top_margin;					//背景图像的上边界，默认2
	int		main_vert_bottom_margin;				//背景图像的下边界，默认2

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

	int		main_line_left_margin;					//中心线左边界
	int		main_line_right_margin;					//中心线右边界

	int		main_vert_line_left_margin;				//中心线左边界
	int		main_vert_line_right_margin;			//中心线右边界

	int		main_round_width;						//背景圆角宽度
	int		main_round_height;						//背景圆角高度

	int		main_vert_round_width;					//背景圆角宽度
	int		main_vert_round_height;					//背景圆角高度

	int		main_center_mode;						//背景中心绘制模式，默认：拉伸
	int		main_vertical_mode;						//垂直绘制模式，默认：拉伸
	int		main_horizontal_mode;					//水平绘制模式，默认：拉伸
	int		main_line_mode;							//中心线绘制模式，默认：拉伸

	int		main_vert_center_mode;					//背景中心绘制模式，默认：拉伸
	int		main_vert_vertical_mode;				//垂直绘制模式，默认：拉伸
	int		main_vert_horizontal_mode;				//水平绘制模式，默认：拉伸
	int		main_vert_line_mode;					//中心线绘制模式，默认：拉伸

	int		use_vert_bk_when_expanding;				//tab扩展时使用竖排背景

	int		status_left_margin;						//状态背景图像左边界，默认：0
	int		status_right_margin;					//状态背景图像右边界，默认：0
	int		status_center_mode;						//状态背景图像中心绘制模式，默认：拉伸
	int		status_left_space;						//状态背景图像左侧空白宽度（按钮起始点），默认：8
	int		status_right_space;						//状态背景图像右侧空白宽度，默认：4
	int		status_min_width;						//状态栏最小宽度

	int		main_effect;							//主窗口特殊效果：NONE, SHADOW, TRANSPARENT
	int		status_effect;							//状态条窗口效果

	//主窗口写作窗口以及候选窗口的字符串颜色
	int		main_string_color;						//默认为0x000000

	//主窗口光标颜色
	int		main_caret_color;						//默认为0xd00000

	//主窗口候选前面的数字颜色
	int		main_number_color;						//默认为0x000000

	//主窗口当前选中的候选的颜色
	int		main_selected_color;					//默认为0xd000a0

	//候选颜色
	int		main_candidate_color;					//默认为0x0040c0

	//主窗口ICW颜色
	int		main_icw_color;							//默认为0x008000

	//主窗口剩余候选数目颜色
	int		main_remains_number_color;				//默认为0xff0000

	//双拼提示框颜色
	int		main_sp_hint_border_color;				//默认为0xa0a0a0

	//双拼提示框背景颜色
	int		main_sp_hint_back_color;				//默认为0xf4f4f8

	//双拼解释字符串的颜色
	int		main_sp_full_string_color;				//默认为0x000000

	//状态窗口背景图象名称
	TCHAR	status_image_name[MAX_FILE_NAME_LENGTH];	//默认为""

	//状态窗口按钮图像名称
	TCHAR	status_buttons_image_name[MAX_FILE_NAME_LENGTH];	//默认为""

	int		status_window_x;						//状态窗口X位置，默认-1
	int		status_window_y;						//状态窗口Y位置，默认-1

	int		main_window_x;							//主窗口x位置，默认500
	int		main_window_y;							//主窗口y位置，默认500

} PIMCONFIG;

//全局共享配置信息
extern PIMCONFIG *pim_config;
extern PIMCONFIG default_config;
//extern int global_config_update_time;		//全局配置装载时间
//extern int is_default_ime;				//是否为默认输入法
extern int no_transparent;
extern int no_ime_aware;
extern int no_gdiplus_release;
extern int no_status_show;
extern int no_main_show;
extern int no_multi_line;
extern int no_virtual_key;
extern int host_is_console;
extern int no_end_composition;
extern int no_ppjz;

extern void MaintainConfig();
extern int  ConfigTest();
extern int  GetInstallDir(TCHAR *dir_name);
extern void SaveConfigInternal(PIMCONFIG *config);
extern void LoadThemeConfig(PIMCONFIG *config);
extern int  GetExpandCandidateLine();
extern void RunBackup();

extern void RunConfigNormal();
extern void RunConfigVersion();
extern void RunConfigWordlib();

extern void RunPianPangJianZi(HWND hwnd);
extern void RunSPWPlugin(HWND hwnd);
extern void RunCompose(HWND hwnd);
extern void RunCFGWizard(HWND hwnd);
//extern void RunURLManager(HWND hwnd);
extern void RunThemeMaker(HWND hwnd);
//extern void RunImeManager(HWND hwnd);
extern void RunBatchWords(HWND hwnd);
extern void RunZiEditor(HWND hwnd);

//定义程序标识
extern int program_id;
#define	PID_UNKNOWN			0
#define	PID_OPERA			1
#define	PID_MAXTHON			2
#define	PID_WORLD			3
//#define	PID_WOW				4
#define	PID_MXD				5
#define	PID_WAR3			6
#define	PID_CONIME			7
#define	PID_PHOTOSHOP		8
#define	PID_IMAGEREADY		9
#define	PID_CQ				10
#define	PID_IE				11
#define	PID_FIREFOX			12
#define	PID_WRITER			13
#define PID_ONENOTE			14
#define PID_ACROBAT			15
#define PID_MSNMESSENGER	16
#define PID_VISO			17
#define PID_TM				18
//#define PID_QQ				19
#define PID_FOXMAIL			20
#define PID_OUTLOOK         21
#define PID_OUTLOOKEXPRESS  22
#define PID_TTRAVELER       23
//#define PID_POWERPOINT      24

#ifdef __cplusplus
}
#endif

#endif