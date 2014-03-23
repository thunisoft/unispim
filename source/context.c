/*	输入上下文管理
 */
#include <kernel.h>
#include <editor.h>
#include <context.h>
#include <config.h>
#include <utility.h>
#include <win32/pim_ime.h>
#include <win32/main_window.h>
#include <tchar.h>

//全局上下文数据，也是默认的数据
//PIMCONTEXT *context;			//全局（线程级别）Context，用于窗口的显示

PIMCONTEXT demo_context =
{
	STATE_START,

	//输入状态
	CHINESE_MODE | QP_MODE,		//输入模式
	0,							//软键盘
	HZ_LESS_USED,				//当前汉字集合的level
	0,							//扩展候选状态
	0,							//横排显示不全，强制竖排显示

	//窗口状态
	1, 							//是否显示写作窗口
	1,							//是否显示候选窗口
	1,							//是否显示状态窗口
	VIEW_MODE_HORIZONTAL,		//候选显示模式，默认横排

	//返回结果
	TEXT(""),					//返回结果字符串
	0,							//返回结果长度
	{ {0}, },					//结果音节数组
	0,							//结果音节计数
	0,							//当前选择的候选数字

	//上下文数据
	TEXT("hua'yu'pin'yin"),	//用户输入的文字
	14,							//输入串长度
	0,							//正在处理的输入串位置
	9,							//光标位置索引（以当前正在处理的syllable为准）
	0,							//用户输入的符号
	0,							//上次输入是否为数字
	0,							//剩余的字符
	0,							//上次输入的是否为点
	0,							//上上次输入的是否为点

	//当前音节
	{ 0, },						//Syllables
	//{ 0, },						//syllable correct flags
	//{ 0, },						//音节分隔符标记
	{ 0, },						//syllable start position
	4,							//音节计数
	0,							//当前处理音节位置
	0,
	{ 0, },                     //默认汉字串
	{ 0, },                     //默认汉字串的音节

	//已经选择的项目
	{ {0}, },					//已经进行了选择的数据
	0,							//数据项计数

	//智能编辑状态数据
	{ 0, },						//已经选好的汉字
	{ 0, }, 					//智能编辑状态的音节位置

	//写作
	TEXT("hua2'yu3'pin'yin"),	//写作窗口的文字
	TEXT(""),					//已经选择的写作窗口文字
	TEXT(""),					//双拼提示文字
	TEXT(""),					//特殊输入提示文字
	12,							//写作串长度
	8,							//光标位置

	//候选
	{
		{ CAND_TYPE_ICW },
	},							//候选集合
	42,							//候选数目
	0,							//显示候选的第一条索引
	1,	 						//被选中的候选索引

	//当前页需要显示的候选
	{
		TEXT("华宇拼音"),	TEXT("华宇"),	TEXT("化"), TEXT("话"), TEXT("花"), TEXT("画"), TEXT("华"), TEXT("滑"), TEXT("划"),
		TEXT("哗"), TEXT("桦"), TEXT("骅"), TEXT("婲"), TEXT("铧"),	TEXT("嬅"),	TEXT("婳"),	TEXT("婠"), TEXT("粿"), 
		TEXT("猾"), TEXT("埖"), TEXT("吪"), TEXT("蕐"), TEXT("姡"), TEXT("搳"),	TEXT("芲"),	TEXT("觟"), TEXT("螖"), 
		TEXT("摦"), TEXT("檴"), TEXT("磆"), TEXT("槬"), TEXT("蘳"), TEXT("杹"),	TEXT("夻"),	TEXT("黊"), TEXT("呚"), 
		TEXT("砉")
	},
	45,							//当前页中的候选数目
	{ 0, },

	0,							//IME消息标志
	0,							//当前输入的窗口句柄

	0,							//软键盘索引
	0,							//大写键按下
	-1,							//配置变换时间

	//英文输入相关选项
	0,							//英文补全模式
	0,							//是否有英文候选

	0,    						//u命令状态

	0,                          //指向uicontext的指针           
};

/**	清除上一次的结果
 */
void ClearResult(PIMCONTEXT *context)
{
	context->result_length		   = 0;
	context->result_string[0]	   = 0;
	context->result_syllable_count = 0;
}

/**	初始化输入上下文
 */
void ResetContext(PIMCONTEXT *context)
{
	context->candidate_count		= 0;
	context->candidate_index		= 0;
	context->candidate_page_count	= 0;
	context->compose_cursor_index	= 0;
	context->compose_length			= 0;
	context->compose_string[0]		= 0;
	context->cursor_pos				= 0;
	context->last_symbol			= 0;
	context->input_length			= 0;
	context->input_pos				= 0;
	context->input_string[0]		= 0;
	context->selected_digital		= 0;
	context->selected_item_count	= 0;
	context->syllable_count			= 0;
	context->syllable_pos			= 0;
	context->show_candidates		= 0;
	context->show_composition		= 0;
	context->spw_hint_string[0]		= 0;
	context->sp_hint_string[0]		= 0;
	context->has_english_candidate	= 0;
	context->force_vertical			= 0;
	//context->url_state				= URL_STATE_NONE;
	context->u_state				= 0;
	context->candidates_view_mode	= VIEW_MODE_HORIZONTAL;
	context->expand_candidate		= (pim_config->always_expand_candidates && CanSwitchToExpandMode(context)) ? 1 : 0;
	context->syllable_mode			= 0;
	context->last_dot				= 0;
	context->next_to_last_dot		= 0;

	if (context->soft_keyboard)
		context->state = STATE_SOFTKBD;
	else if (context->capital)
		context->state = STATE_CAPITAL;
	else if (context->input_mode & CHINESE_MODE)
		context->state = STATE_START;
	else if (context->input_mode & ENGLISH_MODE)
		context->state = STATE_ENGLISH;

	if (context->english_state == ENGLISH_STATE_CAND)
		context->english_state = ENGLISH_STATE_NONE;

	if (!pim_config->use_english_input)
		context->english_state = ENGLISH_STATE_NONE;
}

/**	第一次进行上下文的初始化
 */
void FirstTimeResetContext(PIMCONTEXT *context)
{
	BYTE key_state[0x100];

	memcpy(context, &demo_context, sizeof(PIMCONTEXT));
	GetKeyboardState(key_state);
	if (key_state[VK_CAPITAL] & 0x1)
		context->capital = 1;

	if (pim_config->startup_mode == STARTUP_ENGLISH)
		context->input_mode = ENGLISH_MODE;

	if (pim_config->use_english_input && pim_config->startup_with_english_input)
	{
		context->english_state = ENGLISH_STATE_INPUT;
		if (pim_config->hz_option & HZ_SYMBOL_CHINESE)
			pim_config->hz_option ^= HZ_SYMBOL_CHINESE;
	}
	//else
	//	pim_config->hz_option |= HZ_SYMBOL_CHINESE;

	//启动时默认半角输入
	pim_config->hz_option |= HZ_SYMBOL_HALFSHAPE;

	ResetContext(context);

	context->local_config_update_time = GetCurrentTicks();
	context->show_status			  = pim_config->show_status_window;
}

/*	翻转全角、半角符号
 */
void ToggleFullShape(PIMCONTEXT *context)
{
	Log(LOG_ID, L"翻转全角、半角");
	pim_config->hz_option ^= HZ_SYMBOL_HALFSHAPE;
	SaveConfigInternal(pim_config);
	context->modify_flag |= MODIFY_STATUS;
}

/*	翻转全拼、双拼
 */
void ToggleQuanShuang(PIMCONTEXT *context)
{
	Log(LOG_ID, L"翻转全拼双拼");
	pim_config->pinyin_mode = (PINYIN_QUANPIN + PINYIN_SHUANGPIN) - pim_config->pinyin_mode;
	SaveConfigInternal(pim_config);
	context->modify_flag |= MODIFY_STATUS;
}

/*  翻转横排、竖排显示
 */
void ToggleHorizonalVertical(PIMCONTEXT *context)
{
	Log(LOG_ID, L"翻转横排竖排");
	pim_config->show_vertical_candidate = !pim_config->show_vertical_candidate;
	SaveConfigInternal(pim_config);
	context->modify_flag |= MODIFY_STATUS;
}

void ToggleIgnoreFullShape(PIMCONTEXT *context)
{
	Log(LOG_ID, L"翻转全角输入");
	pim_config->ignore_fullshape = !pim_config->ignore_fullshape;
	SaveConfigInternal(pim_config);
	context->modify_flag |= MODIFY_STATUS;
}

/**	设定为繁体
 */
void SetToFanti(PIMCONTEXT *context)
{
	if (!context || !pim_config)
		return;

	pim_config->hz_output_mode = HZ_OUTPUT_TRADITIONAL;
	SaveConfigInternal(pim_config);
	context->modify_flag |= MODIFY_STATUS;
}

/**	设定为简体
 */
void SetToJianti(PIMCONTEXT *context)
{
	if (!context || !pim_config)
		return;

	pim_config->hz_output_mode = HZ_OUTPUT_SIMPLIFIED;
	SaveConfigInternal(pim_config);
	context->modify_flag |= MODIFY_STATUS;
}

/**	设定为全集
 */
void SetToLargeSet(PIMCONTEXT *context)
{
	if (!context || !pim_config)
		return;

	pim_config->hz_output_mode = HZ_OUTPUT_HANZI_ALL;
	SaveConfigInternal(pim_config);
	context->modify_flag |= MODIFY_STATUS;
}

/**	设定为双拼
 */
void SetToShuangPin(PIMCONTEXT *context)
{
	Log(LOG_ID, L"设定为双拼");
	if (pim_config->pinyin_mode == PINYIN_SHUANGPIN)
		return;

	pim_config->pinyin_mode = PINYIN_SHUANGPIN;
	SaveConfigInternal(pim_config);
	context->modify_flag |= MODIFY_STATUS;
}

/**	设定输入方式
 */
void SetInputStyle(PIMCONTEXT *context, int style)
{
	Log(LOG_ID, L"设定输入风格:%d", style);
	if (pim_config->input_style == style)
		return;

	pim_config->input_style = style;
	SaveConfigInternal(pim_config);
	context->modify_flag |= MODIFY_STATUS;
}

/**	设定为全拼
 */
void SetToQuanPin(PIMCONTEXT *context)
{
	Log(LOG_ID, L"设定为全拼");
	if (pim_config->pinyin_mode == PINYIN_QUANPIN)
		return;

	pim_config->pinyin_mode = PINYIN_QUANPIN;
	SaveConfigInternal(pim_config);
	context->modify_flag |= MODIFY_STATUS;
}

/**	设定接受、不接受组合键输入
 */
void SetCombineKeyMode(PIMCONTEXT *context, int yes)
{
	pim_config->disable_combined_key = !yes;
	SaveConfigInternal(pim_config);
}

/*	翻转简体与繁体
 */
void ToggleFanJian(PIMCONTEXT *context)
{
	int mode;

	if (!context || !pim_config)
		return;

	Log(LOG_ID, L"翻转简、繁、全集");

	mode = pim_config->hz_output_mode;
	if (mode & HZ_OUTPUT_HANZI_ALL)
		mode = HZ_OUTPUT_SIMPLIFIED;
	else if (mode & HZ_OUTPUT_SIMPLIFIED)
		mode = HZ_OUTPUT_TRADITIONAL;
	else
		mode = HZ_OUTPUT_HANZI_ALL;

	if (pim_config)
	{
		pim_config->hz_output_mode = mode;
		SaveConfigInternal(pim_config);
	}

	context->modify_flag |= MODIFY_STATUS;
}

/*	翻转英文、中文符号
 */
void ToggleEnglishSymbol(PIMCONTEXT *context)
{
	Log(LOG_ID, L"翻转英文、中文符号");

	//加此if是为了解决如下bug：
	//0.初始状态是中文+中文标点
	//1.按Shift切换为英文，点击标点按钮，按Shift切换为中文，
	//此时状态是中文+英文标点
	//2.按Shift切换为英文，点击标点按钮，按Shift切换为中文，
	//此时状态时中文+中文标点
	//2、3两次相同操作结果却不一致
	if (context->input_mode & CHINESE_MODE)
	{
		pim_config->hz_option ^= HZ_SYMBOL_CHINESE;
	}

	SaveConfigInternal(pim_config);
	context->modify_flag |= MODIFY_STATUS;
}

/*	翻转中英文输入
 */
void ToggleChineseMode(PIMCONTEXT *context)
{
	Log(LOG_ID, L"翻转英文、中文输入");
	if (context->input_mode & CHINESE_MODE)
		context->input_mode = ENGLISH_MODE;
	else
		context->input_mode = CHINESE_MODE;

	SaveConfigInternal(pim_config);

	context->ui_context->have_caret = -1;
	context->modify_flag |= MODIFY_ENDCOMPOSE;
	context->modify_flag |= MODIFY_STATUS;
	ResetContext(context);

	CheckHintState(context, context->ui_context);
}

/*	清除候选信息
 */
void ClearCandidateInfo(LPINPUTCONTEXT pIMC, PIMCONTEXT *context)
{
	LPCANDIDATEINFO candidate_info;
	LPCANDIDATELIST candidate_list;

	if (0 != pIMC && 0 != (candidate_info = (LPCANDIDATEINFO)ImmLockIMCC(pIMC->hCandInfo)) &&
		0 != (candidate_list = (LPCANDIDATELIST)((char*)candidate_info + candidate_info->dwOffset[0])))
	{
		candidate_list->dwCount		= 0;
		candidate_list->dwSelection = 0;
		candidate_list->dwPageSize	= 0;
		candidate_list->dwPageStart = 0;
	}

	if (0 != context)
	{
		context->candidate_count		  = 0;
		context->candidate_index		  = 0;
		context->candidate_selected_index = -1;
	}

	if (candidate_info)
		ImmUnlockIMCC(pIMC->hCandInfo);
}

/*	清除写作信息
 */
void ClearComposeInfo(LPINPUTCONTEXT pIMC, PIMCONTEXT *context)
{
	LPCOMPOSITIONSTRING compose_string;

	if (0 != pIMC &&
		0 != (compose_string = (LPCOMPOSITIONSTRING)ImmLockIMCC(pIMC->hCompStr)))
	{
		compose_string->dwCompStrLen = 0;
		compose_string->dwCursorPos  = 0;
	}

	if (0 != context)
	{
		context->compose_cursor_index = 0;
		context->compose_length		  = 0;
		context->compose_string[0]	  = 0;
	}

	if (compose_string)
		ImmUnlockIMCC(pIMC->hCompStr);
}

/**	处理上下文音节与字节之间的关系
 */
void MakeSyllableStartPosition(PIMCONTEXT *context)
{
	int i, index = 0;
	TCHAR pinyin[0x10];

	for (i = 0; i < context->syllable_count; i++)
	{
		context->syllable_start_pos[i] = index;

		if (pim_config->pinyin_mode == PINYIN_SHUANGPIN)
			index += GetSyllableStringSP(context->syllables[i], pinyin, _SizeOf(pinyin));
		else
			index += GetSyllableString(context->syllables[i], pinyin, _SizeOf(pinyin), /*context->syllable_correct_flag[i],*/ 0);

		if (context->input_string[index] == SYLLABLE_SEPARATOR_CHAR)
			index++;
	}

	context->syllable_start_pos[i] = index;
}

/**	获得用户选择过的写作窗口字符串
 */
TCHAR *GetSelectedComposeString(PIMCONTEXT *context)
{
	static TCHAR string[MAX_COMPOSE_LENGTH + 4];

	_tcscpy_s(string, _SizeOf(string), context->selected_compose_string);
	return string;
}

/**	获得当前的候选字符串
 */
TCHAR *GetCurrentCandidateString(PIMCONTEXT *context)
{
	static TCHAR string[MAX_COMPOSE_LENGTH + 4];
	int cand_index = context->candidate_selected_index + context->candidate_index;

	GetCandidateDisplayString(context, &context->candidate_array[cand_index], string, MAX_COMPOSE_LENGTH / 2, 0);
	return string;
}

/** 获得当前剩余的音节串
 */
TCHAR *GetReaminComposeString(PIMCONTEXT *context)
{
	static TCHAR string[MAX_COMPOSE_LENGTH + 4];
	int cand_index = context->candidate_selected_index + context->candidate_index;
	SYLLABLE syllables[MAX_SYLLABLE_PER_INPUT];
	TCHAR py[0x10];
	int syllable_length, i;

	string[0] = 0;
	//需要判断是否还有剩余的音节，特殊判断反向解析后的结果（长度与原始不相同）
	if (context->candidate_array[cand_index].type == CAND_TYPE_CI &&
		context->candidate_array[cand_index].word.type == CI_TYPE_OTHER)
		syllable_length = context->candidate_array[cand_index].word.origin_syllable_length;
	else
		syllable_length = GetCandidateSyllable(&context->candidate_array[cand_index], syllables, sizeof(syllables) / sizeof(syllables[0]));

	if (!syllable_length)	//SPW不需要再次计算
		return string;

	for (i = context->syllable_pos + syllable_length; i < context->syllable_count; i++)
	{
		if (_tcslen(string) > _SizeOf(string) - 8)
			break;

		GetSyllableString(context->syllables[i], py, _SizeOf(py), /*context->syllable_correct_flag[i],*/ 0);
		if (i != context->syllable_pos + syllable_length)
			_tcscat_s(string, _SizeOf(string), TEXT("'"));

		_tcscat_s(string, _SizeOf(string), py);
	}

	return string;
}

/**	显示、隐藏状态窗口
 */
void ToggleShowStatusWindow(PIMCONTEXT *context)
{

	if (!context || !context->ui_context)
		return;

	pim_config->show_status_window = !pim_config->show_status_window;
	Log(LOG_ID, L"显示状态栏:%d", pim_config->show_status_window);

	if (!pim_config->show_status_window)
		HideStatusWindow(context, context->ui_context);

	SaveConfigInternal(pim_config);
	PostMessage(context->ui_context->ui_window, UIM_MODE_CHANGE, 0, 0);
}

/*	切换到英文输入模式
 */
void ToggleEnglishInput(PIMCONTEXT *context)
{
	if (!context || !context->ui_context || !pim_config)
		return;

	Log(LOG_ID, L"切换英文输入模式");

	if (ENGLISH_STATE_INPUT == context->english_state)
		context->english_state = ENGLISH_STATE_NONE;
	else
		context->english_state = ENGLISH_STATE_INPUT;

	PostMessage(context->ui_context->ui_window, UIM_ENGLISH_INPUT, 0, 0);
}

/*	切换英文候选模式
 */
void ToggleEnglishCandidate(PIMCONTEXT *context)
{
	if (!context || !context->ui_context || !pim_config)
		return;

	Log(LOG_ID, L"切换英文候选模式");

	if (ENGLISH_STATE_CAND == context->english_state)
		context->english_state = ENGLISH_STATE_NONE;
	else
		context->english_state = ENGLISH_STATE_CAND;

	PostMessage(context->ui_context->ui_window, UIM_ENGLISH_INPUT, 0, 0);
}

//设置候选显示模式
void SetCandidatesViewMode(PIMCONTEXT *context)
{
	int view_mode = VIEW_MODE_HORIZONTAL;

	switch (pim_config->input_style)
	{
	case STYLE_CSTAR:
		//非英文模式
		if (context->english_state == ENGLISH_STATE_NONE)
		{
			//u模式 or i模式 竖排
			if (STATE_UINPUT == context->state || STATE_IINPUT == context->state)
				view_mode = VIEW_MODE_VERTICAL;
			//竖排显示候选
			else if (pim_config->show_vertical_candidate)
				view_mode = VIEW_MODE_VERTICAL;
			//强制竖排显示候选
			else if (context->force_vertical)
				view_mode = VIEW_MODE_VERTICAL;
			//扩展
			else if (pim_config->use_tab_expand_candidates && context->expand_candidate)
				view_mode = VIEW_MODE_EXPAND;
			else
				view_mode = VIEW_MODE_HORIZONTAL;
		}
		else
		{
			//英文竖排
			view_mode = VIEW_MODE_VERTICAL;
		}

		break;

	case STYLE_ABC:
		view_mode = VIEW_MODE_VERTICAL;
		break;
	}

	context->candidates_view_mode = view_mode;
}

int CanSwitchToExpandMode(PIMCONTEXT *context)
{
	if (STYLE_CSTAR == pim_config->input_style &&					//中文之星
		VIEW_MODE_HORIZONTAL == context->candidates_view_mode &&	//当前横排
		pim_config->use_tab_expand_candidates &&					//允许扩展
		pim_config->theme_use_for_expand_mode &&					//主题允许扩展
		!IsFullScreen())
		return 1;
	else
		return 0;
}