/*	主窗口头文件。
 */

#ifndef	_MAIN_WINDOW_H_
#define	_MAIN_WINDOW_H_

#include <commctrl.h>
#include <context.h>

typedef struct tagUICONTEXT
{
	PIMCONTEXT	*context;								//指向context的指针

	HWND	ui_window;									//输入法消息窗口
	HWND	main_window;								//写作窗口
	HWND	status_window;								//状态窗口
	HWND	hint_window;								//提示窗口
	HWND	tooltip_window;								//ToolTip窗口
	HWND	theme_main_window;							//main预览窗口
	HWND	theme_status_window;						//status预览窗口

	POINT	main_window_pos;							//写作窗口的位置
	SIZE	main_window_size;							//写作窗口的大小
	SIZE	hint_window_size;							//提示窗口的大小
	int		compose_frame_height;						//写作部分的高度
	int		candidate_line_height;						//候选行高度

	int		have_caret;									//是否已经有光标位置信息，
														//用于判断是否通过GetCaretPos进行光标定位
	POINT	caret_pos;									//光标位置

	int		menu_showing;								//是否正在显示菜单
	int		process_show_or_hide;						//处理状态条的显示或隐藏

#ifdef __cplusplus
	Font	*zi_font;                                   //中文字体
	Font    *ascii_font;						        //英文字体
	Font	*no_font;									//数字字体
	Font	*tone_font;									//音调字体
	Font	*hint_font;									//提示字体，指双拼的提示窗口
	Font	*guide_font;								//功能提示字体
	Bitmap  *image_main_bk;								//写作窗口背景图像
	Bitmap	*image_main_line;							//写作窗口中心图像
	Bitmap  *image_main_vert_bk;						//写作窗口竖排背景图像
	Bitmap	*image_main_vert_line;						//写作窗口竖排中心图像
	Bitmap	*image_status_bk;							//状态窗口背景图像
	Bitmap	*image_status_buttons;						//状态窗口按钮图像
#endif
}UICONTEXT;

#ifdef __cplusplus
extern "C" {
#endif

//光标跟随的偏移位置
#define	CARET_X_OFFSET		10
#define	CARET_Y_OFFSET		20
#define	CARET_HINT_Y_OFFSET	40

//IME使用的窗口标志
#define	IME_WINDOW_FLAG			(SWP_NOACTIVATE | SWP_NOCOPYBITS)
#define	IME_WINDOW_STYLE		(CS_IME | CS_VREDRAW | CS_HREDRAW)

//窗口显示标志
#define	SWF_PAINT				(1 << 0)	//重画窗口
#define	SWF_SETPOS				(1 << 1)	//设定窗口位置
#define	SWF_HIDE				(1 << 2)	//隐藏窗口
#define	SWF_SHOW				(1 << 3)	//显示窗口

//鼠标点击在主窗口上的区域
#define	ZONE_NONE				0
#define	ZONE_CANDIDATE			1
#define	ZONE_BUTTON				2			//状态窗口上的按钮图标
#define	ZONE_DRAG				4			//状态窗口拖动区
#define	ZONE_COMPOSE_LEFT		5			//写作框左边
#define	ZONE_COMPOSE_RIGHT		6			//写作框右边

#define	MIN_MAIN_WINDOW_WIDTH	100

//ToolTip窗口设置
#define	MAX_HINT_LENGTH				0x200	//Hint消息长度
#define	TOOLTIP_BASE_ID				0x100	//ToolTip的起始ID
#define	TOOLTIP_DELAYTIME_AUTOPOP	5000	//ToolTip显示时间5秒钟
#define	TOOLTIP_DELAYTIME_INITIAL	600		//开始显示
#define	TOOLTIP_DELAYTIME_RESHOW	200		//显示下一个tip的延迟时间（没有用处）

#define	MAX_STATUS_BUTTON_NUMBER	0x10	//最多的状态窗口按钮数目
//#define	CARET_TIMER_ID				0x100	//光标记时器标识

#define	ROUND_WIDTH					1		//圆角宽度
#define	ROUND_HEIGHT				1		//圆角高度

#define	CAND_NUMBER_GAP_WIDTH		2		//候选与数字之间的距离
#define	CAND_GAP_WIDTH				6		//候选之间的水平空隙
#define	CAND_GAP_HEIGHT				2		//候选行之间的垂直空隙
#define	COMPOSE_STATUS_WIDTH		80		//写作窗口的状态宽度
#define	COMPOSE_NO_GAP_WIDTH		40		//写作串与候选数目指示器之间的最小宽度
#define	COMPOSE_SP_HINT_GAP			20		//双拼提示与写作串之间的距离
#define	MIN_COMPOSE_NO_GAP			20		//剩余数字与写作串的最小距离
#define MIN_COMPOSE_HINT_GAP		20		//提示信息与写作串的最小距离
#define MIN_HINT_NO_GAP				10		//提示信息与剩余数字的最小距离
#define COMPOSE_CARET_GAP_WIDTH		3		//写作串与光标之间的距离

#define	STATUS_BUTTON_WIDTH			22		//状态窗口图标宽度
#define	STATUS_BUTTON_HEIGHT		22		//状态窗口图标高度

#define POS_DEFAULT_X				500		//默认位置X
#define POS_DEFAULT_Y				1800	//默认位置Y

#define CAND_POPUP_MENU_SET_TOP		1		//置顶
#define CAND_POPUP_MENU_DELETE		2		//删除
#define CAND_POPUP_MENU_BAIDU		3		//百度搜索

#define MAX_CAND_POPUP_MENU_LENGTH	0x200   //最大菜单项长度
#define MAX_SEARCH_URL_LENGTH		0x200	//最大搜索的URL长度

#define MAX_CAND_HINT_SIZE			60		//提示最大长度

#define CAND_HINT_TYPE_ENGLISH_CN		1	//英文输入提示_中文状态
#define CAND_HINT_TYPE_ENGLISH_EN		2	//英文输入提示_英文状态
#define CAND_HINT_TYPE_ENGLISH_INPUT	3	//英文输入提示_英文输入法
#define	CAND_HINT_TYPE_ABC				4	//智能ABC空格提示
#define CAND_HINT_TYPE_USE_TAB			5	//使用Tab扩展提示
#define CAND_HINT_TYPE_NUMBER_STRING	6	//数字串，i/i提示
#define CAND_HINT_TYPE_AT_CHAR			7   //@提示
#define CAND_HINT_TYPE_U_CHAR			8   //u提示
#define CAND_HINT_TYPE_RANDOM		   99   //随机提示

#define STATUS_BUTTON_HINT_INPUT_MODE		1   //中文/英文输入
#define STATUS_BUTTON_HINT_OUTPUT_MODE		2   //输出字符集
#define STATUS_BUTTON_HINT_MARK_MODE		3	//中文/英文标点
#define STATUS_BUTTON_HINT_SHAP_MODE		4	//全/半角
#define STATUS_BUTTON_HINT_SOFTKEYBOARD		5	//软键盘
#define STATUS_BUTTON_HINT_SETUP			6	//设置工具
#define STATUS_BUTTON_HINT_HELP				7	//帮助

#define MAX_STATUS_LIST_SIZE			100		//状态栏列表数

#define	MAIN_WINDOW_CLASS_NAME			TEXT("V6MAIN")
#define	STATUS_WINDOW_CLASS_NAME		TEXT("V6STATUS")
#define	HINT_WINDOW_CLASS_NAME			TEXT("V6HINT")
#define THEME_MAIN_WINDOW_CLASS_NAME	TEXT("V6THEMEMAIN")
#define THEME_STATUS_WINDOW_CLASS_NAME	TEXT("V6THEMESTATUS")

//#define	PIM_UPDATE_NOSTART				0	//尚未运行
//#define	PIM_UPDATE_RUNNING				1	//运行中
//#define	PIM_UPDATE_WAITING				2	//等待下次运行
//#define	PIM_UPDATE_FINISHED				3	//已经完成
//#define	PIM_UPDATE_ERROR				4	//错误（可能升级程序不存在）

//extern HWND main_window_list[MAX_STATUS_LIST_SIZE];
//extern HWND status_window_list[MAX_STATUS_LIST_SIZE];
//extern HWND hint_window_list[MAX_STATUS_LIST_SIZE];

void CreateIMEWindows(PIMCONTEXT *context, UICONTEXT *ui_context);
void DestroyIMEWindows(PIMCONTEXT *context, UICONTEXT *ui_context);

//创建、清除窗口
extern void PIM_CreateMainWindow(PIMCONTEXT *context, UICONTEXT *ui_context, HINSTANCE instance);
extern void PIM_CreateStatusWindow(PIMCONTEXT *context, UICONTEXT *ui_context, HINSTANCE instance);
extern void PIM_DestroyMainWindow(PIMCONTEXT *context, UICONTEXT *ui_context, HINSTANCE instance);
extern void PIM_DestroyStatusWindow(PIMCONTEXT *context, UICONTEXT *ui_context, HINSTANCE instance);
extern void PIM_DestroyHintWindow(PIMCONTEXT *context, UICONTEXT *ui_context, HINSTANCE instance);
extern void PIM_DestroyToolTipWindow(PIMCONTEXT *context, UICONTEXT *ui_context, HINSTANCE instance);
extern void PIM_DestroyThemeWindow(PIMCONTEXT *context, UICONTEXT *ui_context, HINSTANCE instance);

extern void MainWindowTraceCaret(PIMCONTEXT *context, UICONTEXT *ui_context, POINT point);

extern void LoadThemeResource(UICONTEXT *ui_context);
extern void DeleteThemeResource(UICONTEXT *ui_context);
extern void CheckThemeResource(UICONTEXT *ui_context);

extern void HideMainWindow(PIMCONTEXT *context, UICONTEXT *ui_context);			//隐藏主窗口
extern void ShowMainWindow(PIMCONTEXT *context, UICONTEXT *ui_context);			//显示主窗口

extern void HideStatusWindow(PIMCONTEXT *context, UICONTEXT *ui_context);		//隐藏状态窗口
extern void ShowStatusWindow(PIMCONTEXT *context, UICONTEXT *ui_context);		//显示状态窗口
extern void UpdateStatusWindow(PIMCONTEXT *context, UICONTEXT *ui_context);
extern void CheckHintState(PIMCONTEXT *context, UICONTEXT *ui_context);			//检查Hint窗口的状态
extern void HideHintWindow(PIMCONTEXT *context, UICONTEXT *ui_context);

extern void UpdateWinResource(PIMCONTEXT *context, UICONTEXT *ui_context);		//更新窗口资源

extern void InitGdiplus();
extern void FreeGdiplus();

extern void CheckMainWindowPosition(PIMCONTEXT *context, UICONTEXT *ui_context, HWND win);

extern PIMCONTEXT *GetPIMContextByWindow(HWND ui_window);

extern UICONTEXT *AllocateUIContext();
extern void FreeUIContext(UICONTEXT *context);

extern UICONTEXT default_ui_context;

extern void WINAPI PaintDemoWindow(PIMCONFIG *config, HDC dc_status, HDC dc_main, int *status_width, int *status_height, int *main_width, int *main_height,	int expand, int vertical, int draw_assist_line);

extern void ShowThemeWindow(PIMCONTEXT *context, UICONTEXT *ui_context);
extern void HideThemeWindow(PIMCONTEXT *context, UICONTEXT *ui_context);

extern int LoadRandomHintMessage();

extern void EncodeSearchURL(TCHAR* search_url, TCHAR* key, int is_baidu);

#ifdef __cplusplus
}
#endif

#endif