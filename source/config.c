/*	输入法配置程序。
 *	配置程序从注册表中装载配置信息。当注册表中不存在配置信息时，设置
 *	默认的配置到注册表。
 */
#include <kernel.h>
#include <config.h>
#include <context.h>
#include <utility.h>
#include <pim_resource.h>
#include <windows.h>
#include <regstr.h>
#include <shlwapi.h>
#include <tchar.h>
#include <fontcheck.h>
#include <share_segment.h>
#include <libfunc.h>

//制作主题的函数名
typedef int (_stdcall *pMAKE_THEME) (PIMCONFIG *);
typedef int (_stdcall *pBACKUP) (const TCHAR *);

PIMCONFIG *pim_config;					//全局使用的config指针

#pragma data_seg(HYPIM_SHARED_SEGMENT)

//int	is_default_ime = 0;					//是否为默认输入法
//int global_config_update_time = 0;		//配置装载时间
//int config_loaded = 0;					//是否已经装载Config
//
//PIMCONFIG all_config = { 0 };			//系统正在使用的Config
PIMCONFIG default_config =				//默认的输入法配置数据
{
	//输入风格
	STYLE_CSTAR,
//	STYLE_ABC,

	//启动的输入方式：中文、英文
	STARTUP_CHINESE,
//	STARTUP_ENGLISH,

	//拼音模式
	PINYIN_QUANPIN,

	//是否显示双拼的提示
	0,

	//候选选择方式，字母、数字。
	SELECTOR_DIGITAL,
//	SELECTOR_LETTER,

	//总是清除上下文
	0,

	//支持IME感知程序
	1,

	//垂直显示候选
	0,

	//总是显示写作窗口
	0,

	//总是显示候选窗口
	0,

	//是否显示状态条
	1,

	//是否使用汉字音调辅助
	1,

	//是否使用ICW（智能组词）
	1,

	//是否保存ICW的结果到词库
	1,

	//是否光标跟随
	1,

	//是否直接输入网址
	1,

	//首字母输入的最小汉字数目
	4,

	//双向解析拼音串
	1,

	//在输入V后以英文方式进行输入
	0,			//允许在V后面输入空格

	//是否在数字键之后，输入英文符号
	0,

	//使用TAB扩展汉字的候选
	1,

	//一直展开候选窗口
	0,

	//汉字输出的方式：简体、繁体。同时繁体字包含未分类汉字。
	HZ_OUTPUT_SIMPLIFIED,
//	HZ_OUTPUT_HANZI_ALL,
//	HZ_TRADITIONAL_EQU_OTHERS,		//繁体字包含属性为其他的字（日文/韩文汉字）

	//输入字的选项
	HZ_RECENT_FIRST			|		//最近输入的字优先（默认）
	HZ_ADJUST_FREQ_FAST		|		//字输入调整字频（默认）
	HZ_USE_TAB_SWITCH_SET	|		//使用TAB切换汉字的集合
	HZ_USE_TWO_LEVEL_SET	|		//使用两种集合切分方式
	HZ_SYMBOL_CHINESE		|		//使用汉字符号
	HZ_SYMBOL_HALFSHAPE		|		//半角符号
	HZ_USE_FIX_TOP,					//使用固顶字

	//词输入选项
	CI_AUTO_FUZZY			|		//输入词的时候，自动使用z/zh, c/ch, s/sh的模糊（默认）
	CI_SORT_CANDIDATES		|		//候选词基于词频进行排序（默认）
	CI_ADJUST_FREQ_FAST		|		//快速调整词频（默认）
	CI_WILDCARD				|		//输入词的时候，使用通配符（默认）
	CI_USE_FIRST_LETTER		|		//使用首字母输入词（默认）
	CI_RECENT_FIRST			|		//最新输入的词优先（默认）
	CI_AUTO_VOW_FUZZY,				//使用韵母自动匹配

	//是否使用模糊音
	0,

	//模糊音选项
	0,								//全部不模糊

	//软键盘默认序号
	0,

	//词库文件名称
	{
		TEXT("unispim6\\wordlib\\user.uwl"),
		TEXT("unispim6\\wordlib\\sys.uwl"),
		TEXT("unispim6\\wordlib\\names.uwl"),
		TEXT("unispim6\\wordlib\\addr.uwl"),
		TEXT("unispim6\\wordlib\\life.uwl"),
		TEXT("unispim6\\wordlib\\rec.uwl"),
		TEXT("unispim6\\wordlib\\idiom.uwl"),
#ifdef	_VER_SIFA_
		TEXT("unispim6\\wordlib\\sifa.uwl"),
#endif
	},

	//词库数量
#ifndef	_VER_SIFA_
#pragma message("_VER_SIFA_ not defined")
	7,
#else
#pragma message("_VER_SIFA_ defined")
	8,
#endif

	//一次能够输入的最大汉字数目
	20,

	//显示在候选页中的候选的个数
	8,

	//候选翻页键对
	KEY_PAIR_1 |					//,.
	KEY_PAIR_3 |					//-=
	KEY_PAIR_6,						//PageUp/PageDown

	//候选漫游键对
	KEY_PAIR_5,						//<>

	//以词定字键对
	KEY_PAIR_2,						//[]

	//编辑键对
	KEY_PAIR_4,						//()

	//变换中英文输入方式键
	KEY_SWITCH_SHIFT,				//左SHIFT

	//输入第二个以及第三个候选的键
	KEY_2ND_3RD_CONTROL,			//左右CONTROL

	//在数字之后，"."作为英文符号
	1,

	//扩展候选的行数
	4,

	//将用户使用过的词汇记录到用户词库中，默认为0
	0,

	//主窗口最小高度、宽度
	0,
	0,

	0,
	0,

	//主窗口与状态窗口的透明度
	0,
	0,

	//主窗口的锚点（左上角位置），用于设定窗口与光标的相对位置
	0,
	0,

	0,
	0,

	//状态窗口上边界
	-1,					//-1表示居中

	//主题应用的范围
	1,
	1,
	1,

	//备份目录
	TEXT(""),

	//禁止组合键，用于游戏
	0,

	//配置版本
	CONFIG_VERSION,					//配置版本

	//记忆最近输入过的词
	0,

	//输入过程中锁定模式切换
	1,

	//输入过程中忽略全角模式
	1,

	//拼音纠错选项
	0,
	//CORRECT_GN_NG          |
	//CORRECT_MG_NG          |
	////CORRECT_ON_ONG         |
	////CORRECT_IOU_IU         |
	//CORRECT_UEI_UI,         //|
	////CORRECT_UEN_UN,

	//使用自定义短语
	1,

	//使用自定义符号
	1,

	//使用软键盘快捷键
	1,

	//软键盘快捷键
	'K',

	//使用简繁切换快捷键
	1,

	//简繁切换快捷键
	'F',

	//使用全拼双拼快捷键
	0,

	//全拼双拼切换快捷键
	'S',

	//使用状态栏切换快捷键
	1,

	//状态栏切换快捷键
	'Z',

	//候选包含英文单词
	0,

	//英文单词后自动补空格
	1,

	//使用英文输入快捷键
	1,

	//英文输入快捷键
	'E',

	//英文输入时竖排显示
	1,

	//启动时进入英文输入法
	0,

	//显示提示信息
	1,

	//功能提示右边界
	0,

	//竖排功能提示右边界
	0,

	//短语文件名称
	{
		SYS_SPW_FILE_NAME,
		/*TEXT("unispim6\\phrase\\表情符号.ini"),
		TEXT("unispim6\\phrase\\数字符号.ini"),
		TEXT("unispim6\\phrase\\图形符号.ini"),
		TEXT("unispim6\\phrase\\中文字符.ini"),
		TEXT("unispim6\\phrase\\外文符号.ini"),
		TEXT("unispim6\\phrase\\字符画.ini"),
		TEXT("unispim6\\phrase\\用户短语.ini")*/
	},

	//短语文件数量
	1,

	//使用英文输入
	1,

	//使用英文翻译
	1,

	//use url hint
	//1,
	
	//单个短语候选时，显示位置；使用上面废弃的配置项；默认为3
	//spw_position
	3,

	//使用词语联想
	1,

	//从第x个音节开始联想
	4,

	//联想词位于候选词第x位
	2,

	//联想词个数
	2,

	//use_u_hint
	1,

	//只输出GBK集合，scope_gbk
	HZ_SCOPE_GBK,

	//开启屏蔽天窗功能，默认不开启
	0,

	//全屏应用时隐藏状态条
	1,

	//B模式开关
	1,

	//D模式开关
	1,

	//I模式开关
	1,

	//H模式开关
	1,

	//i模式开关
	1,

	//u模式开关
	1,

	//v模式开关
	1,

	//小键盘数字选词
	0,

	//保留的配置选项，用于版本升级，不用重新覆盖注册表
	{ 0, },

	//状态条的类型：新版，传统
	STATUS_TRADITIONAL_STYLE,		//新版本

	//传统状态条上的按键
	STATUS_BUTTON_MODE		|
	STATUS_BUTTON_CHARSET	|
	STATUS_BUTTON_SYMBOL	|
	STATUS_BUTTON_SHAPE		|
	STATUS_BUTTON_SOFTKBD	|
	STATUS_BUTTON_HELP		|
	STATUS_BUTTON_SETUP,

	//主题名称，用于进行主题的寻找判断
	DEFAULT_THEME_NAME,

	//中文字体名称
	TEXT("宋体"),

	//英文以及符号的字体名称
	TEXT("Arial"),

	//字体高度
	16,

	//主窗口背景图象名称，如果为NULL，则使用默认颜色进行绘画
	MAIN_BK_IMAGE_NAME,

	//主窗体分割线图像名称
	MAIN_LINE_IMAGE_NAME,

	//主窗口背景图象名称，如果为NULL，则使用默认颜色进行绘画
	MAIN_BK_IMAGE_NAME,

	//主窗体分割线图像名称
	MAIN_LINE_IMAGE_NAME,

	//背景图像的设置参数
	3,							//背景图像的左边界，默认2
	26, 						//背景图像的右边界，默认2
	25,							//背景图像的上边界，默认2
	3,							//背景图像的下边界，默认2

	3,							//背景图像的左边界，默认2
	26, 						//背景图像的右边界，默认2
	25,							//背景图像的上边界，默认2
	3,							//背景图像的下边界，默认2

	6,							//文字左边界
	6,							//文字右边界
	6,							//文字上边界
	6,							//文字下边界
	6,							//中心高度，即：拼音串与候选串之间的高度
	16,							//候选剩余数目右边界

	6,							//文字左边界
	6,							//文字右边界
	6,							//文字上边界
	6,							//文字下边界
	6,							//中心高度，即：拼音串与候选串之间的高度
	16,							//候选剩余数目右边界

	6,							//中心线左边界
	6,							//中心线右边界

	6,							//中心线左边界
	6,							//中心线右边界

	4,							//背景圆角宽度
	4,							//背景圆角高度

	4,							//背景圆角宽度
	4,							//背景圆角高度

	DRAW_MODE_STRETCH, 			//背景中心绘制模式，默认：拉伸
	DRAW_MODE_STRETCH, 			//垂直绘制模式，默认：拉伸
	DRAW_MODE_STRETCH, 			//水平绘制模式，默认：拉伸
	DRAW_MODE_STRETCH,			//中心线绘制模式，默认：拉伸

	DRAW_MODE_STRETCH, 			//背景中心绘制模式，默认：拉伸
	DRAW_MODE_STRETCH, 			//垂直绘制模式，默认：拉伸
	DRAW_MODE_STRETCH, 			//水平绘制模式，默认：拉伸
	DRAW_MODE_STRETCH,			//中心线绘制模式，默认：拉伸

	0,							//tab扩展时使用竖排背景，默认为否

	0,							//状态背景图像左边界
	0,							//状态背景图像右边界
	DRAW_MODE_STRETCH,			//状态背景图像中心绘制模式
	8,							//状态背景图像左侧空白宽度（按钮起始点）
	4,							//状态背景图像右侧空白宽度
	0,							//状态栏最小宽度

	//主窗口效果--此功能已经去掉，不再使用
	0,							//是否显示阴影，默认：显示

	//状态窗口效果--此功能已经去掉，不再使用
	0,							//是否显示阴影，默认：显示

	//主窗口写作窗口以及候选窗口的字符串颜色
	TORGB(0x0080FF),

	//主窗口光标颜色
	TORGB(0xFF8000),

	//主窗口候选前面的数字颜色
	TORGB(0x0094EC),

	//主窗口当前选中的候选的颜色
	TORGB(0xFF0000),

	//主窗口候选颜色
	TORGB(0x0080FF),

	//主窗口ICW颜色
	TORGB(0x008040),

	//主窗口剩余候选数目颜色
	TORGB(0xFF0000),

	//双拼提示框颜色
	TORGB(0xa0a0a0),

	//双拼提示框背景颜色
	TORGB(0xf4f4f8),

	//双拼解释字符串的颜色
	TORGB(0x000000),

	//状态窗口背景图象名称
	STATUS_BK_IMAGE_NAME,

	//状态窗口按钮图像名称
	STATUS_BUTTONS_IMAGE_NAME,

	//状态窗口位置，默认-1, -1
	-1,	-1,

	//主窗口位置，默认500, 500
	500, 1800,
};

#pragma data_seg()

int no_main_show = 0;
int no_status_show = 0;
int	no_transparent = 0;
int no_ime_aware = 0;
int no_gdiplus_release = 0;
int no_multi_line = 0;
int no_virtual_key = 0;
int host_is_console = 0;
int no_end_composition = 0;
int no_ppjz = 0;//不是偏旁部首检字

int program_id;

/**	检查特定程序对配置的要求
 */
void CheckSpecialConfig()
{
	const TCHAR *p;
	int i;

	static const TCHAR p_name[][0x20] =
	{
		TEXT(""),
		TEXT("opera.exe"),
		TEXT("maxthon.exe"),
		TEXT("theworld.exe"),
		//TEXT("wow.exe"),
		TEXT("maplestory.exe"),
		TEXT("war3.exe"),
		TEXT("conime.exe"),
		TEXT("photoshop.exe"),
		TEXT("imageready.exe"),
		TEXT("mir1.dat"),
		TEXT("iexplore.exe"),
		TEXT("firefox.exe"),
		TEXT("windowsLiveWriter.exe"),
		TEXT("onenote.exe"),
		TEXT("acrobat.exe"),
		TEXT("msnmsgr.exe"),
		TEXT("visio.exe"),
		TEXT("tm.exe"),
		//TEXT("qq.exe"),
		TEXT("foxmail.exe"),
		TEXT("outlook.exe"),
		TEXT("msimn.exe"),		//outlook express
		TEXT("TTraveler.exe")/*,
		TEXT("POWERPNT.EXE")*/
	};

	static const int p_id[] =
	{
		PID_UNKNOWN,
		PID_OPERA,
		PID_MAXTHON,
		PID_WORLD,
		//PID_WOW,
		PID_MXD,
		PID_WAR3,
		PID_CONIME,
		PID_PHOTOSHOP,
		PID_IMAGEREADY,
		PID_CQ,
		PID_IE,
		PID_FIREFOX,
		PID_WRITER,
		PID_ONENOTE,
		PID_ACROBAT,
		PID_MSNMESSENGER,
		PID_VISO,
		PID_TM,
		//PID_QQ,
		PID_FOXMAIL,
		PID_OUTLOOK,
		PID_OUTLOOKEXPRESS,
		PID_TTRAVELER/*,
		PID_POWERPOINT*/
	};

	program_id = PID_UNKNOWN;
	p = GetProgramName();
	if (!p)
		return;

	for (i = 0; i < sizeof(p_name) / _SizeOf(p_name[0]) / sizeof(TCHAR); i++)
		if (!_tcsicmp(p_name[i], p))
		{
			program_id = p_id[i];
			break;
		}

	if (program_id == PID_UNKNOWN)
		return;

	switch(program_id)
	{
	//Opera浏览器
	case PID_OPERA:
		no_ppjz = 1;
		//no_ime_aware = 1;
		break;

	//maxthon浏览器
	case PID_MAXTHON:
		no_transparent = 1;
		break;

	//世界之窗浏览器
	case PID_WORLD:
		no_transparent = 1;
		no_ime_aware = 1;
		break;

	//魔兽世界
	//case PID_WOW:
	//	//no_main_show = 1;
	//	break;

	//冒险岛
	case PID_MXD:
		no_status_show = 1;
		break;

	//魔兽争霸
	case PID_WAR3:
		no_status_show = 1;
		//no_main_show = 1;
		break;

	//Conime
	case PID_CONIME:
		no_status_show = 1;
		host_is_console = 1;
		break;

	//PhotoShop
	case PID_PHOTOSHOP:
		no_multi_line = 1;
		break;

	//ImageReady
	case PID_IMAGEREADY:
		no_multi_line = 1;
		break;

	//传奇世界
	case PID_CQ:
		no_gdiplus_release = 1;
		break;

	//火狐浏览器
	case PID_FIREFOX:
		no_transparent = 1;
		no_ime_aware = 1;
		break;

	//IE
	case PID_IE:
		no_transparent = 1;
		no_gdiplus_release = 1;
		no_ime_aware = 1;
		break;

	//onenote
	case PID_ONENOTE:
		no_virtual_key = 1;
		break;

	//acrobat
	case PID_ACROBAT:
		no_end_composition = 1;
		break;

	//msn messenger
	case PID_MSNMESSENGER:
		no_ppjz = 1;
		break;
	
	case PID_VISO://viso
	case PID_TM://QQ
	//case PID_QQ://QQ
	case PID_FOXMAIL://FOXMAIL
	case PID_OUTLOOK://OUTLOOK
	case PID_OUTLOOKEXPRESS://OUTLOOK
	case PID_TTRAVELER://QQ BROWSER
//	case PID_POWERPOINT:
		no_ime_aware = 1;
		break;
	}
}

/*	将配置写入到注册表中。
 *	内部函数，用于本模块的调用，不进行比较以及Resource的重新装载
 */
void SaveConfigInternal(PIMCONFIG *config)
{
	HKEY		reg_key;		//注册表KEY
	LONG		ret;			//函数返回值
	HKEY		cur_user_key;

	if (!config)
		return;

	Log(LOG_ID, L"被调用");
	if (ERROR_SUCCESS != RegOpenCurrentUser(KEY_ALL_ACCESS, &cur_user_key))
	{
		Log(LOG_ID, L"!!! 无法获得当前用户的key, error=%d", GetLastError());
		cur_user_key = HKEY_CURRENT_USER;
	}

	Log(LOG_ID, L"save theme name:%s", config->theme_name);
	Log(LOG_ID, L"save main image:%s", config->main_image_name);

	ret = RegOpenKeyEx(cur_user_key, REG_CONFIG_KEY_NAME, 0, KEY_READ | KEY_WRITE, &reg_key);
	if (ret != ERROR_SUCCESS)
	{
		ret = RegCreateKey(cur_user_key, REG_CONFIG_KEY_NAME, &reg_key);
		if (ret != ERROR_SUCCESS)				//失败
		{
			Log(LOG_ID, L"配置注册表Key创建失败。key名=%s, errcode = %d", REG_CONFIG_KEY_NAME, ret);
			return;
		}
	}

	//将默认配置数据写入到注册表中
	ret = RegSetValueEx(reg_key, REG_CONFIG_ITEM_NAME, 0, REG_BINARY, (char*)config, sizeof(PIMCONFIG));
	if (ret != ERROR_SUCCESS)				//失败
		Log(LOG_ID, L"注册表项写入失败，项名=%s, errcode = %d", REG_CONFIG_ITEM_NAME, ret);
	else
		Log(LOG_ID, L"注册表项写入成功");

	RegCloseKey(reg_key);			//关闭注册表Key

	//保存config后，需要进行更新的操作。
	if (pim_config && pim_config != config)
		memcpy(pim_config, config, sizeof(PIMCONFIG));

	Log(LOG_ID, L"save theme name:%s", pim_config->theme_name);
	Log(LOG_ID, L"save main image:%s", pim_config->main_image_name);
}

void LoadThemeConfig(PIMCONFIG *config)
{
	MakeTheme(config);

	//HMODULE dll;
	//pMAKE_THEME make_theme;
	//int ret;

	//dll = LoadLibrary(UTILITY_DLL_NAME);
	//if (!dll)
	//	return;

	//Log(LOG_ID, L"装载DLL");
	//make_theme = (pMAKE_THEME) GetProcAddress(dll, "MakeTheme");
	//if (make_theme)
	//{
	//	ret = (*make_theme)(config);
	//	if (!ret)							//装载失败
	//	{
	//		_tcscpy_s(config->theme_name, _SizeOf(config->theme_name), DEFAULT_THEME_NAME);
	//		ret = (*make_theme)(config);
	//	}
	//}
	//Log(LOG_ID, L"调用MakeTheme");

	//FreeLibrary(dll);
	//Log(LOG_ID, L"卸载DLL");

	//Log(LOG_ID, L"theme name:%s", config->theme_name);
	//Log(LOG_ID, L"main image:%s", config->main_image_name);
}

/*	将配置写入到注册表中。
 *	外部DLL接口函数，进行Config比较以及Resource的重新装载
 */
void WINAPI SaveConfig(PIMCONFIG *config, int modify_flag)
{
	int diff,old_hideblackwindow;
	int expand_candidates_save = pim_config->always_expand_candidates;
	old_hideblackwindow = pim_config->hide_black_window;

	Log(LOG_ID, L"被调用");

	//完全相同，则不需要保存
	diff = memcmp(pim_config, config, sizeof(PIMCONFIG));
	if (!modify_flag && !diff)
		return;

	if (pim_config && diff)
		SaveConfigInternal(config);

	//释放输入法资源，以及重新加载输入法资源
	if (modify_flag & MODIFY_INI)
		PIM_ReloadINIResource();

	if ((expand_candidates_save != pim_config->always_expand_candidates) ||
		(modify_flag & MODIFY_THEME) ||		//只有主题才需要update
		(modify_flag & MODIFY_WORDLIB))
		share_segment->global_config_update_time = GetCurrentTicks();

	if (modify_flag & MODIFY_WORDLIB)
		PIM_ReloadWordlibResource();
	else if (modify_flag & MODIFY_WORDLIB_FORCE_RELOAD)
		PIM_ForceReloadWordlibResource();

	if (modify_flag & MODIFY_ZI)
		PIM_ReloadZiResource();

	if (modify_flag & MODIFY_ENGLISH)
		PIM_ReloadEnglishAllResource();

	/*if (modify_flag & MODIFY_URL)
		PIM_ReloadURLResource();*/

	if ( config->hide_black_window == 1 && old_hideblackwindow == 0 )
		PIM_ReloadFontMapResource();

	Log(LOG_ID, L"保存配置结束，完成时间:%d\n", GetCurrentTicks());
}

/**	装载默认配置
 */
const PIMCONFIG * WINAPI LoadDefaultConfig()
{
	LoadThemeConfig(&default_config);			//装载主题配置
	return &default_config;
}

/**	判断是否为默认输入法
 */
void CheckDefaultIME()
{	HKEY		reg_key; 		//注册表KEY
	DWORD		data_type;		//注册表项的类型
	DWORD		data_length;	//配置内容长度
	LONG		ret;			//函数返回值
	HKEY		cur_user_key;	//当前用户key
	TCHAR		ime_id[0x20];	//IME的标识
	TCHAR		ime_name[0x100];
	TCHAR		key_str[0x100];

	//如果已经进行过判断，则不需要重新进行。
	if (share_segment->is_default_ime)
		return;

	Log(LOG_ID, L"检查是否为默认输入法");
	if (ERROR_SUCCESS != RegOpenCurrentUser(KEY_ALL_ACCESS, &cur_user_key))
	{
		Log(LOG_ID, L"!!! 无法获得当前用户的key, error=%d", GetLastError());
		cur_user_key = HKEY_CURRENT_USER;
	}

	//打开注册表KEY
	ret = RegOpenKey(cur_user_key, REG_PRELOAD_KEY_NAME, &reg_key);
	if (ret != ERROR_SUCCESS)
	{	//失败
		Log(LOG_ID, L"默认输入法Key打开失败，可能不存在。errcode = %d", ret);
		return;
	}

	//读取配置信息。
	data_length = _SizeOf(ime_id);
	ret = RegQueryValueEx(reg_key, TEXT("1"), 0, &data_type, (LPBYTE)ime_id, &data_length);

	RegCloseKey(reg_key);			//关闭注册表Key
	if (ret != ERROR_SUCCESS || data_type != REG_SZ)	//读取失败
	{
		Log(LOG_ID, L"默认输入法项读取失败");
		return;
	}

	//检查是否为我们自己的输入法
	_stprintf_s(key_str, _SizeOf(key_str), REGSTR_PATH_CURRENT_CONTROL_SET TEXT("\\Keyboard Layouts\\%s"), ime_id);
	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, key_str, 0, KEY_READ, &reg_key))
		return;			//无法打开失败

	data_length = _SizeOf(ime_name) - 1;
	ret = RegQueryValueEx(reg_key, TEXT("Ime File"), 0, &data_type, (LPBYTE)ime_name, &data_length);
	RegCloseKey(reg_key);

	if (ret != ERROR_SUCCESS || _tcscmp(ime_name, IME_NAME))
	{
		Log(LOG_ID, L"本输入法非默认输入法");
		return;
	}

	share_segment->is_default_ime = 1;
	Log(LOG_ID, L"本输入法为默认输入法");
}

/**	检查皮肤的目录是否存在
 */
int ExistThemeDirectory(const TCHAR *theme_name)
{
	TCHAR name[MAX_PATH] = {0};
	TCHAR t_name[MAX_PATH] = {0};

	_tcscpy_s(t_name, _SizeOf(t_name), THEME_DIR_NAME);
	_tcscat_s(t_name, _SizeOf(t_name), theme_name);

	GetFileFullName(TYPE_ALLAPP, t_name, name);
	if (PathIsDirectory(name))
		return 1;

	return 0;
}

/**	检查皮肤的文件是否存在
 */
int ExistThemeFile(const TCHAR *file_name)
{
	TCHAR name[MAX_PATH] = {0};
	TCHAR t_name[MAX_PATH] = {0};

	_tcscat_s(t_name, _SizeOf(t_name), file_name);

	GetFileFullName(TYPE_ALLAPP, t_name, name);
	if (PathFileExists(name))
		return 1;

	return 0;
}

/*	从注册表中读入输入法配置信息。
 *	注册表项为一个内存数据，其大小为PIMCONFIG的SIZE。
 */
void WINAPI LoadConfig(PIMCONFIG *config)
{
	HKEY		reg_key; 		//注册表KEY
	DWORD		data_type;		//注册表项的类型
	DWORD		data_length;	//配置内容长度
	LONG		ret;			//函数返回值
	HKEY		cur_user_key;
	int			theme_exists = 0;

	Log(LOG_ID, L"加载用户Reg配置");
	//由于XP要求必须使用RegOpenCurrentUser函数来获取HKCU，否则
	//可能返回的结果不是当前用户
	//用于解决 默认输入法(default ime)的皮肤切换的错误
	if (ERROR_SUCCESS != RegOpenCurrentUser(KEY_ALL_ACCESS, &cur_user_key))
	{
		Log(LOG_ID, L"!!! 无法获得当前用户的key, error=%d", GetLastError());
		cur_user_key = HKEY_CURRENT_USER;
	}

	data_length = sizeof(PIMCONFIG);

	//打开注册表KEY
	ret = RegOpenKey(cur_user_key, REG_CONFIG_KEY_NAME, &reg_key);
	if (ret != ERROR_SUCCESS)
	{	//失败
		Log(LOG_ID, L"配置注册表Key打开失败，可能不存在。errcode = %d", ret);

		LoadThemeConfig(&default_config);			//装载主题配置

		//将当前默认的配置复制到注册表（创建KEY）
		SaveConfigInternal(&default_config);
		memcpy(config, &default_config, sizeof(PIMCONFIG));
		return;
	}

	//读取配置信息。
	ret = RegQueryValueEx(reg_key, REG_CONFIG_ITEM_NAME, 0, &data_type, (LPBYTE)config, &data_length);

	//兼容以往配置
	//为兼容以往的配置，spw_position的数值从11~19，使用时直接减10
	if(config->spw_position <= 10)
		config->spw_position = 13;
	//if(config->key_pair_candidate_page < 4)
	//	config->key_pair_candidate_page = KEY_PAIR_1 | KEY_PAIR_3;
	//if( config->scope_gbk <= 1 )
	//	config->scope_gbk = HZ_SCOPE_GBK;

	//if(config->key_pair_zi_from_word < 4)
	//	config->key_pair_zi_from_word = KEY_PAIR_2;

	if (ret == ERROR_SUCCESS && data_length == sizeof(PIMCONFIG) && data_type == REG_BINARY)	//成功读取
		Log(LOG_ID, L"配置文件成功装载");
	else
	{
		Log(LOG_ID, L"注册表项读取失败，errcode = %d", ret);		//失败
		LoadThemeConfig(&default_config);			//装载主题配置
		SaveConfigInternal(&default_config);
		memcpy(config, &default_config, sizeof(PIMCONFIG));	//使用默认的数据法配置
		RegCloseKey(reg_key);			//关闭注册表Key
		return;
	}

	RegCloseKey(reg_key);			//关闭注册表Key

	LoadThemeConfig(&default_config);

	if (ExistThemeDirectory(config->theme_name) && ExistThemeFile(config->main_image_name))
		theme_exists = 1;

	//皮肤不存在，或者当前使用的是默认主题，但是配置版本不同
	if ((!theme_exists) ||
		(theme_exists && (!_tcscmp(DEFAULT_THEME_NAME, config->theme_name)) && (config->config_version != CONFIG_VERSION)))
	{
		_tcscpy_s(config->theme_name, _SizeOf(config->theme_name), DEFAULT_THEME_NAME);
		LoadThemeConfig(config);
		config->config_version = CONFIG_VERSION;
	}

	Log(LOG_ID, L"Reg配置加载完毕");
}

/*	维护输入法系统配置。
 *	用户在使用拼音输入的时候，可能进行配置的修改，因此当配置发生改变后，
 *	输入法需要在第一时间获取最新的配置信息。
 *	编码方式：
 *		1. 系统配置文件更新后，调用输入法动态连接库中的HYPIM_ConfigModifed函数，
 *		更新配置文件的最新时间；
 *		2. 当核心进行输入处理以及IME处理输入的时候，调用MaintainConfig；
 *		3. 本函数则判断Config的最后修改时间是否晚于最后更新时间，如果晚于
 *		则重新将配置调入内存；
 *		4. 如果注册表中并没有输入法的配置项，则使用默认的输入法配置，创建注册表项。
 *
 *	参数：无
 *	返回：无（永远正确）
 */
void MaintainConfig()
{
	pim_config = &share_segment->all_config;
	CheckSpecialConfig();						//检查特定程序对配置的要求

	if (share_segment->config_loaded)
		return;

	CheckDefaultIME();
	LoadConfig(&share_segment->all_config);		//装载配置

	share_segment->config_loaded = 1;
	share_segment->global_config_update_time = GetCurrentTicks();
}

/**	获得输入法安装目录
 *	参数：
 *		dir_name	安装目录
 *	返回：
 *		1：成功
 *		0：失败
 */
int GetInstallDir(TCHAR *dir_name)
{
	HKEY		reg_key;		//注册表KEY
	DWORD		data_type;		//注册表项的类型
	DWORD		data_length;	//配置内容长度
	LONG		ret;			//函数返回值

	dir_name[0] = 0;

	//打开注册表KEY
	ret = RegOpenKey(HKEY_LOCAL_MACHINE, REG_CONFIG_KEY_NAME, &reg_key);
	if (ret != ERROR_SUCCESS)
	{	//失败
		Log(LOG_ID, L"配置注册表Key打开失败，可能不存在。errcode = %d", ret);
		return 0;
	}

	//读取配置信息。
	data_length = MAX_PATH;

	ret = RegQueryValueEx(reg_key, REG_INSTALL_DIR_NAME, 0, &data_type, (LPBYTE)dir_name, &data_length);

	if (!(ret == ERROR_SUCCESS && data_length > 0 && data_type == REG_SZ))	//读取失败
	{
		Log(LOG_ID, L"注册表项读取失败，errcode = %d", ret);		//失败
		return 0;
	}

	RegCloseKey(reg_key);			//关闭注册表Key
	return 1;
}

/**	获得扩展候选的行数
 */
int GetExpandCandidateLine()
{
	return pim_config->expand_candidate_lines;
}

/**	进行系统备份
 */
void RunBackup()
{
	static int first = 1;
	//HMODULE dll;
	//pBACKUP backup;
	//int ret;

	if (!first)
		return;

	first = 0;

	if (!pim_config->backup_dir[0])
		return;

	BackupUserData(pim_config->backup_dir);

	//dll = LoadLibrary(UTILITY_DLL_NAME);
	//if (!dll)
	//	return;

	//Log(LOG_ID, L"装载DLL");
	//backup = (pBACKUP) GetProcAddress(dll, "BackupUserData");
	//if (backup)
	//	ret = (*backup)(pim_config->backup_dir);

	//Log(LOG_ID, L"调用BackupUserData");

	//FreeLibrary(dll);
	//Log(LOG_ID, L"卸载DLL");
}

void ExecuteConfig(TCHAR *parameter)
{
	TCHAR cmd_line[MAX_PATH] = {0};
	STARTUPINFO	start_info;
	PROCESS_INFORMATION	process_info;

	if (!GetInstallDir(cmd_line))
		return;

	_tcscat_s(cmd_line, _SizeOf(cmd_line), TEXT("\\bin\\"));
	_tcscat_s(cmd_line, _SizeOf(cmd_line), CONFIG_EXE_NAME);
	_tcscat_s(cmd_line, _SizeOf(cmd_line), parameter);

	memset(&start_info, 0, sizeof(start_info));
	start_info.cb = sizeof(start_info);

	CreateProcess(0,
				  cmd_line,
				  0,
				  0,
				  0,
				  CREATE_DEFAULT_ERROR_MODE | NORMAL_PRIORITY_CLASS,
				  0,
				  0,
				  &start_info,
				  &process_info);
}

/**	运行配置程序，直接跳入到版本界面
 */
void RunConfigNormal()
{
	ExecuteConfig(TEXT(""));
}

/**	运行配置程序，直接跳入到版本界面
 */
void RunConfigVersion()
{
	ExecuteConfig(TEXT(" -ver"));
}

/**	运行配置程序，直接跳入到词库界面
 */
void RunConfigWordlib()
{
	ExecuteConfig(TEXT(" -wordlib"));
}


void RunAssistTools(HWND hwnd, TCHAR *exe_filename)
{
	TCHAR install_dir[MAX_PATH];	//安装路径
	TCHAR cmd_line[MAX_PATH];		//命令行参数
	STARTUPINFO	start_info;
	PROCESS_INFORMATION	process_info;
	TCHAR slog[0x100];

	_tcscpy_s(slog, _SizeOf(slog), exe_filename);
	_tcscat_s(slog, _SizeOf(slog), TEXT("被调用"));

	Log(LOG_ID, slog);

	if (!GetInstallDir(install_dir))
		return;

	if(!hwnd){
		_stprintf_s(cmd_line,
				_SizeOf(cmd_line),
				TEXT("%s\\bin\\%s"),
				install_dir,
				exe_filename);
	}else{
		_stprintf_s(cmd_line,
				_SizeOf(cmd_line),
				TEXT("%s\\bin\\%s %d"),
				install_dir,
				exe_filename,
				(no_ppjz) ? 0 : (INT_PTR)hwnd);
	}

	memset(&start_info, 0, sizeof(start_info));
	start_info.cb = sizeof(start_info);
	CreateProcess(0,
				  cmd_line,
				  0,
				  0,
				  0,
				  CREATE_DEFAULT_ERROR_MODE | NORMAL_PRIORITY_CLASS,
				  0,
				  0,
				  &start_info,
				  &process_info);

	return;
}

void RunPianPangJianZi(HWND hwnd)
{
	RunAssistTools(hwnd, PPJZ_EXE_NAME);
}

void RunSPWPlugin(HWND hwnd)
{
	RunAssistTools(hwnd, SPW_EXE_NAME);
}

//void RunURLManager(HWND hwnd)
//{
//	RunAssistTools(hwnd, URL_EXE_NAME);
//}

void RunCFGWizard(HWND hwnd)
{
	RunAssistTools(hwnd, CFGWIZARD_EXE_NAME);
}

void RunBatchWords(HWND hwnd)
{
	RunAssistTools(hwnd, BATCH_WORDS_EXE_NAME);
}

void RunZiEditor(HWND hwnd)
{
	RunAssistTools(hwnd, ZIEDITOR_EXE_NAME);
}

void RunThemeMaker(HWND hwnd)
{
	RunAssistTools(hwnd, THEME_MAKER_EXE_NAME);
}

void RunCompose(HWND hwnd)
{
	RunAssistTools(hwnd, COMPOSE_EXE_NAME);
}

/*void RunImeManager(HWND hwnd)
{
	RunAssistTools(hwnd, IMEMANAGER_EXE_NAME);
}*/