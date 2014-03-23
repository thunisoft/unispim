/*	输入上下文头文件
 */

#ifndef	_INPUTCONTEXT_H_
#define	_INPUTCONTEXT_H_

#include <kernel.h>
#include <windows.h>
#include <win32/immdev.h>

#ifdef __cplusplus
extern "C" {
#endif

#define		CHINESE_MODE		(1 << 0)		//中文输入模式
#define		ENGLISH_MODE		(1 << 1)		//英文输入模式
#define		CAPTICAL_MODE		(1 << 2)		//大写方式
#define		QP_MODE				(1 << 3)		//全拼模式
//#define		SP_MODE				(1 << 4)		//双拼模式

//状态改变设置，用于发送IME_NOTIFY消息
#define		MODIFY_COMPOSE			(1 << 0)	//写作串改变
#define		MODIFY_CANDIDATE		(1 << 1)	//候选串改变
#define		MODIFY_STATUS			(1 << 2)	//状态改变
#define		MODIFY_STARTCOMPOSE		(1 << 3)	//开始写作
#define		MODIFY_ENDCOMPOSE		(1 << 4)	//结束写作
#define		MODIFY_RESULT			(1 << 5)	//返回结果
#define		MODIFY_RESULT_CONTINUE	(1 << 6)	//返回结果并且继续
#define		MODIFY_SENDBACK_CTRL	(1 << 7)	//返回Control键（魔兽世界问题）
#define		MODIFY_SENDBACK_SHIFT	(1 << 8)	//返回Shift键
#define		MODIFY_DONT_SEND_CLEAR	(1 << 9)	//shift切换的时候，不能发送COMPOSE消息（Dreamweaver需要）

#define		STATE_START			0				//起始状态
#define		STATE_EDIT			1				//编辑状态
#define		STATE_SELECT		2				//选择候选状态
#define		STATE_ENGLISH		3				//英文状态
#define		STATE_RESULT		4				//候选上屏状态
#define		STATE_ILLEGAL		5				//非法输入状态
#define		STATE_VINPUT		6				//V输入状态
#define		STATE_UINPUT		7				//U输入状态
#define		STATE_IINPUT		8				//I输入状态
#define		STATE_SOFTKBD		9				//软键盘输入状态
#define		STATE_CAPITAL		10				//大写键按下状态
#define		STATE_ABC_SELECT	11				//ABC风格选择状态
#define		STATE_IEDIT			12				//智能编辑状态

#define		ENGLISH_STATE_NONE	0				//非英文模式
#define		ENGLISH_STATE_INPUT	1				//英文输入模式
#define		ENGLISH_STATE_CAND	2				//英文候选模式

//#define     URL_STATE_NONE      0               //not url state
//#define     URL_STATE_INPUT     1               //url state

#define		VIEW_MODE_HORIZONTAL	0			//横排显示
#define		VIEW_MODE_VERTICAL		1			//垂直显示
#define		VIEW_MODE_EXPAND		2			//扩展显示

typedef struct tagEditItem
{
	int			syllable_start_pos;				//起始音节位置
	int			syllable_length;				//占用的音节数目
	CANDIDATE	candidate;						//本项的候选信息
	int			left_or_right;					//是否为以词定字，并标识左侧、右侧
}SELECT_ITEM;

struct tagUICONTEXT;

//输入上下文结构
typedef struct tagPIMCONTEXT
{
	int			state;													//输入法当前状态

	//输入状态
	int			input_mode;												//输入模式
	int			soft_keyboard;											//软键盘
	int			zi_set_level;											//当前汉字集合的level
	int			expand_candidate;										//扩展候选状态
	int			force_vertical;											//横排显示不全，强制竖排显示

	//窗口状态
	int			show_composition;										//是否显示写作窗口
	int			show_candidates;										//是否显示候选窗口
	int			show_status;											//是否显示状态窗口
	int			candidates_view_mode;									//候选显示模式（横排、竖排、扩展）

	//返回结果
	TCHAR		result_string[MAX_RESULT_LENGTH + 1];					//上屏的结果
	int			result_length;											//结果长度（以字节为单位）
	SYLLABLE	result_syllables[MAX_SYLLABLE_PER_INPUT];				//结果音节
	int			result_syllable_count;									//音节计数
	int			selected_digital;										//当前选择的候选数字，0标识没有选择

	//上下文数据
	TCHAR		input_string[MAX_INPUT_LENGTH + 0x10];					//用户输入的文字
	int			input_length;											//输入串长度
	int			input_pos;												//正在处理的输入串位置
	int			cursor_pos;												//光标位置索引（以当前正在处理的syllable为准）
	TCHAR		last_symbol;											//用户输入的符号
	int			last_digital;											//上次输入的是否为数字
	int			last_char;												//上次输入剩余的字母（用于立即进行输入）
	int			last_dot;												//上次输入的是否为点
	int			next_to_last_dot;										//上上次输入的是否为点

	//当前音节
	SYLLABLE	syllables[MAX_SYLLABLE_PER_INPUT + 0x10];				//音节表
	//int         syllable_correct_flag[MAX_SYLLABLE_PER_INPUT + 0x10];   //音节纠错标记
	//int		  syllable_separator_flag[MAX_SYLLABLE_PER_INPUT + 0x10]; //音节分隔符标记
	int			syllable_start_pos[MAX_SYLLABLE_PER_INPUT + 0x10];		//音节在输入串中的起始位置表
	int			syllable_count;											//音节计数
	int			syllable_pos;											//当前处理到的音节位置
	int			syllable_mode;                                          //音节模式，当候选为短语时、输入拼音串中包括通配符时，此状态为0
	HZ			default_hz[MAX_SYLLABLE_PER_INPUT + 0x10];              //默认汉字串
	SYLLABLE    default_hz_syllables[MAX_SYLLABLE_PER_INPUT + 0x10];    //默认汉字串的音节(与默认汉字串中的每个汉字由正确的一一对应关系)

	//已经选择的项目
	SELECT_ITEM	selected_items[MAX_SYLLABLE_PER_INPUT];					//已经进行了选择的数据
	int			selected_item_count;									//数据项计数

	//智能编辑状态数据
	HZ			iedit_hz[MAX_SYLLABLE_PER_INPUT + 0x10];				//已经选好的汉字
	int			iedit_syllable_index;									//智能编辑状态的音节位置

	//写作
	TCHAR		compose_string[MAX_COMPOSE_LENGTH];						//写作窗口的文字
	TCHAR		selected_compose_string[MAX_COMPOSE_LENGTH];			//已经选择过的写作窗口文字
	TCHAR		sp_hint_string[MAX_COMPOSE_LENGTH * 2];					//双拼提示文字
	TCHAR		spw_hint_string[MAX_SPW_HINT_STRING];					//特殊输入提示文字
	int			compose_length;											//写作串长度
	int			compose_cursor_index;									//光标所在位置

	//候选
	CANDIDATE	candidate_array[MAX_CANDIDATES];						//候选集合
	int			candidate_count;										//候选数目
	int			candidate_index;										//显示候选的第一条索引
	int			candidate_selected_index;								//被选中的候选索引

	//当前页需要显示的候选
	TCHAR		candidate_string[MAX_CANDIDATES_PER_LINE * MAX_CANDIDATE_LINES][MAX_CANDIDATE_STRING_LENGTH + 2];
	int			candidate_page_count;									//当前页中的候选数目
	TCHAR		candidate_trans_string[MAX_CANDIDATES_PER_LINE * MAX_CANDIDATE_LINES][MAX_TRANSLATE_STRING_LENGTH + 2];

	int			modify_flag;											//操作修改状态
	HWND		current_input_window;									//当前输入的窗口

	int			softkbd_index;											//软键盘索引
	int			capital;												//大写键按下
	int			local_config_update_time;								//配置文件改变时间

	//英文输入相关选项
	int			english_state;											//英文补全模式
	int			has_english_candidate;									//是否有英文候选

	//int         url_state;                      						//1:input string is url
	int			u_state;												//u命令状态

	struct tagUICONTEXT *ui_context;									//指向uicontext的指针
}PIMCONTEXT;

extern PIMCONTEXT demo_context;

//extern void ClearContext();
extern void ClearResult(PIMCONTEXT *context);
extern void ResetContext(PIMCONTEXT *context);
extern void FirstTimeResetContext(PIMCONTEXT *context);
extern void ToggleChineseMode(PIMCONTEXT *context);
extern void SetToFanti(PIMCONTEXT *context);
extern void SetToJianti(PIMCONTEXT *context);
extern void SetToLargeSet(PIMCONTEXT *context);
extern void SetToShuangPin(PIMCONTEXT *context);
extern void SetToQuanPin(PIMCONTEXT *context);
extern void SetCombineKeyMode(PIMCONTEXT *context, int yes);
extern void SetInputStyle(PIMCONTEXT *context, int style);
extern void ToggleFanJian(PIMCONTEXT *context);
extern void ToggleFullShape(PIMCONTEXT *context);
extern void ToggleEnglishSymbol(PIMCONTEXT *context);
extern void ToggleEnglishInput(PIMCONTEXT *context);
extern void ToggleEnglishCandidate(PIMCONTEXT *context);
extern void ToggleShowStatusWindow(PIMCONTEXT *context);
extern void ToggleQuanShuang(PIMCONTEXT *context);
extern void ToggleHorizonalVertical(PIMCONTEXT *context);
extern void ToggleIgnoreFullShape(PIMCONTEXT *context);
extern void ProcessInputChar(PIMCONTEXT *context, char ch);
extern void ClearCandidateInfo(LPINPUTCONTEXT pIMC, PIMCONTEXT *context);
extern void ClearComposeInfo(LPINPUTCONTEXT pIMC, PIMCONTEXT *context);
extern void MakeSyllableStartPosition(PIMCONTEXT *context);

extern TCHAR *GetSelectedComposeString(PIMCONTEXT *context);
extern TCHAR *GetReaminComposeString(PIMCONTEXT *context);
extern TCHAR *GetCurrentCandidateString(PIMCONTEXT *context);

extern void SetCandidatesViewMode(PIMCONTEXT *context);
extern int CanSwitchToExpandMode(PIMCONTEXT *context);

#ifdef __cplusplus
}
#endif

#endif