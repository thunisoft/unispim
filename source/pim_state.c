/*	输入法状态控制函数
 */
#include <config.h>
#include <symbol.h>
#include <utility.h>
#include <editor.h>
#include <zi.h>
#include <win32/softkbd.h>
#include <win32/pim_ime.h>
//#include <url.h>
#include <tchar.h>
#include <share_segment.h>

/**	判断是否为非法输入状态
 */
int IllegalState(PIMCONTEXT *context)
{
	if (!context->syllable_count)
		return 1;

	return 0;
}

/**	判断是否为符号键
 */
int IsSymbolKey(int virtual_key)
{
	return 1;
}

/**	是否为URL的起始字符串
 */
int IsURLPrefix(const TCHAR *input, TCHAR ch)
{
	static TCHAR *direct_inputs[] =
	{
		TEXT(".www"),	TEXT(".ftp"),	TEXT(".bbs"),	TEXT(".forum"),		TEXT(".mail"),	TEXT(".blog"),
		TEXT(":ftp"),	TEXT(":http"),	TEXT(":https"),	TEXT(":mailto"),	TEXT(":file"),
	};

	static TCHAR *direct_inputs1[] =
	{
		TEXT("www."),	TEXT("ftp."),	TEXT("bbs."),	TEXT("forum."),		TEXT("mail."),	TEXT("blog."),
		TEXT("ftp:"),	TEXT("http:"),	TEXT("https:"),	TEXT("mailto:"),	TEXT("file:"),
	};

	int i;

	for (i = 0; i < sizeof(direct_inputs) / sizeof(direct_inputs[0]); i++)
	{
		if (ch && !_tcscmp(direct_inputs[i] + 1, input) && direct_inputs[i][0] == ch)
			return 1;

		if (!_tcsnccmp(input,direct_inputs1[i],_tcslen(direct_inputs1[i])))
			return 1;
	}

	return 0;
}

int NeedKeyStart(UINT virtual_key, UINT_PTR scan_code, LPBYTE key_state)
{
	if (key_state[VK_CONTROL] & 0x80)		//control被按下
		return 0;

	if (virtual_key == VK_CONTROL || virtual_key == VK_SHIFT)
		return 0;

	switch(virtual_key)
	{
		case 0xc0:	// '~' | '`'
		case 0xbd:	// '_' | '-'
		case 0xbb:	// '+' | '='
		case 0xdc:	// '|' | '\\'
		case 0xdb:	// '{' | '['
		case 0xdd:	// '}' | ']'
		case 0xba:	// ':' | ';'
		case 0xde:	// '"' | '\''
		case 0xbc:	// '<' | ','
		case 0xbe:	// '>' | '.'
		case 0xbf:	// '?' | '/'
			if ((pim_config->hz_option & HZ_SYMBOL_CHINESE) ||
				!(pim_config->hz_option & HZ_SYMBOL_HALFSHAPE))
				return 1;

			return 0;

		//数字
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			if (key_state[VK_SHIFT] & 0x80)				//shift键按下，输入符号
				return 1;

			if (!(pim_config->hz_option & HZ_SYMBOL_HALFSHAPE))
				return 1;

			return 0;

		//小键盘区
		case 0x60:	case 0x61:	case 0x62:	case 0x63:	case 0x64:
		case 0x65:	case 0x66:	case 0x67:	case 0x68:	case 0x69:
			if (!(pim_config->hz_option & HZ_SYMBOL_HALFSHAPE))
				return 1;

			return 0;

		case ' ':
			if (!(pim_config->hz_option & HZ_SYMBOL_HALFSHAPE))
				return 1;

			return 0;

		case 'A':	case 'B':	case 'C':	case 'D':	case 'E':	case 'F':	case 'G':
		case 'H':	case 'I':	case 'J':	case 'K':	case 'L':	case 'M':	case 'N':
		case 'O':	case 'P':	case 'Q':	case 'R':	case 'S':	case 'T':	case 'U':
		case 'V':	case 'W':	case 'X':	case 'Y':	case 'Z':
			return 1;
	}

	return 0;
}

/*	基于当前状态判断是否需要对按键进行处理
 */
int IsNeedKey(PIMCONTEXT *context, UINT virtual_key, UINT_PTR scan_code, LPBYTE key_state)
{
	if (!context)
		return 0;

	if (context->state == STATE_START && (key_state[virtual_key] & 0x80)) //virtual_key == VK_BACK)
		CheckDeleteNewCi(virtual_key);

	if (virtual_key == VK_CAPITAL)
		return 1;

	if (pim_config->disable_combined_key &&
		(context->state == STATE_START || context->state == STATE_ENGLISH) &&
		(virtual_key == VK_SHIFT || virtual_key == VK_CONTROL ||
		 (key_state[VK_SHIFT] & 0x80) || (key_state[VK_CONTROL] & 0x80)))
		return 0;

	//shift键，并且是抬起的时候
	if (virtual_key == VK_SHIFT && (key_state[VK_SHIFT] & 0x80) == 0 && last_key == VK_SHIFT)
		return 1;

	//control键，并且是抬起的时候，我们要响应它，但注意不用这个键的时候，不需要
	//魔兽世界中，会因为这个造成问题。
	//所以必须要全部响应，不然会造成CTRL键一直按下的现象
	if (virtual_key == VK_CONTROL && (key_state[VK_CONTROL] & 0x80) == 0 && last_key == VK_CONTROL)
		return 1;

	//当按下Ctrl+↑或者CTRL+↓时，需要响应
	//if ((key_state[VK_CONTROL] & 0x80) && (virtual_key == VK_UP || virtual_key ==VK_DOWN))
	if ((last_key == VK_CONTROL) && (virtual_key == VK_UP || virtual_key == VK_DOWN))
	{
		if ((pim_config->trace_recent && (context->state == STATE_START)) || !(key_state[VK_CONTROL] & 0x80))
			return 1;
		else
			return 0;
	}

	//除shift外，其它键抬起，不做处理
	//在Vista+IE8保护模式下，key_state状态有误，需要对扫描码进行判断  by yuanzh 2009.2.10
	if (((key_state[virtual_key] & 0x80) == 0) && (scan_code & 0xC0000000))
		return 0;

	if ((key_state[VK_SHIFT] & 0x80) && (key_state[VK_CONTROL] & 0x80))
	{
		//软键盘快捷键
		if ((pim_config->use_key_soft_kbd) && (virtual_key == pim_config->key_soft_kbd))
			return 1;

		//简繁体快捷键
		if ((pim_config->use_key_jian_fan) && (virtual_key == pim_config->key_jian_fan))
			return 1;

		//全双拼快捷键
		if ((pim_config->use_key_quan_shuang_pin) && (virtual_key == pim_config->key_quan_shuang_pin))
			return 1;

		//状态栏切换快捷键
		if ((pim_config->use_key_status_window) && (virtual_key == pim_config->key_status_window))
			return 1;

		//英文输入法快捷键
		if ((pim_config->use_english_input) && (pim_config->use_key_english_input) && (virtual_key == pim_config->key_english_input))
			return 1;
	}

	////粘贴不能响应，否则可能我们的短语无法上屏
	//if (virtual_key == 'V' && (key_state[VK_CONTROL] & 0x80))
	//	return 0;

	if (virtual_key == VK_CONTROL)
		return 0;

	switch(context->state)
	{
	case STATE_START:
		return NeedKeyStart(virtual_key, scan_code, key_state);

	case STATE_ENGLISH:
		return 0;

	case STATE_EDIT:
		return 1;

	case STATE_RESULT:
		return 0;

	case STATE_SELECT:
	case STATE_VINPUT:
	case STATE_IINPUT:
	case STATE_UINPUT:
	case STATE_ILLEGAL:
		return 1;

	case STATE_SOFTKBD:
		//pc键盘
		if (context->softkbd_index == SOFTKBD_NUMBER - 1)
		{
			if (key_state[VK_CAPITAL] & 0x1)
				return 0;

			if (context->input_mode == ENGLISH_MODE)
				return 0;

			if (0 == context->input_length)
				return NeedKeyStart(virtual_key, scan_code, key_state);

			return 1;
		}

		if (key_state[VK_CONTROL] & 0x80)
			return 0;

		switch(virtual_key)
		{
		case 0xc0:	// '~' | '`'
		case 0xbd:	// '_' | '-'
		case 0xbb:	// '+' | '='
		case 0xdc:	// '|' | '\\'
		case 0xdb:	// '{' | '['
		case 0xdd:	// '}' | ']'
		case 0xba:	// ':' | ';'
		case 0xde:	// '"' | '\''
		case 0xbc:	// '<' | ','
		case 0xbe:	// '>' | '.'
		case 0xbf:	// '?' | '/'
			return 1;
		}

		if (virtual_key >= '0' && virtual_key <= '9')
			return 1;

		if (virtual_key >= 'A' && virtual_key <= 'Z')
			return 1;

		return 0;

	case STATE_CAPITAL:
		if (pim_config->hz_option & HZ_SYMBOL_HALFSHAPE)
			return 0;

		switch(virtual_key)
		{
		case 0xc0:	// '~' | '`'
		case 0xbd:	// '_' | '-'
		case 0xbb:	// '+' | '='
		case 0xdc:	// '|' | '\\'
		case 0xdb:	// '{' | '['
		case 0xdd:	// '}' | ']'
		case 0xba:	// ':' | ';'
		case 0xde:	// '"' | '\''
		case 0xbc:	// '<' | ','
		case 0xbe:	// '>' | '.'
		case 0xbf:	// '?' | '/'
		//小键盘区
		case 0x60:	case 0x61:	case 0x62:	case 0x63:	case 0x64:
		case 0x65:	case 0x66:	case 0x67:	case 0x68:	case 0x69:
			return 1;
		}

		if (virtual_key >= '0' && virtual_key <= '9')
			return 1;

		if (virtual_key >= 'A' && virtual_key <= 'Z')
			return 1;

		return 0;

	case STATE_ABC_SELECT:
		return 1;

	case STATE_IEDIT:
		return 1;
	}

	return 0;
}

void ProcessKeyStart(PIMCONTEXT *context, int key_flag, int virtual_key, TCHAR ch)
{
	const TCHAR *symbol_string = 0;

 	ClearResult(context);
	ResetContext(context);

	//全角或者输入的是符号
	//增加输入过程中忽略全角模式的选项，全角模式也可以正常输入 changed by yuanzh 2008.7.17
	if ((!(pim_config->hz_option & HZ_SYMBOL_HALFSHAPE) && !pim_config->ignore_fullshape) || STATE_CAPITAL == context->state ||
	    IsSymbolChar(ch) || (!symbol_string && ((ch >= TEXT('0') && ch <= TEXT('9')) || ch == TEXT(' '))))
	{
		symbol_string = GetSymbol(context, ch);
		if (symbol_string)
		{
			_tcscpy(context->result_string, symbol_string);
			context->result_length = (int)_tcslen(symbol_string);
			CheckQuoteInput(*(HZ*)symbol_string);
		}
		else
		{
			context->result_string[0] = ch;
			context->result_string[1] = 0;
			context->result_length = 1;
		}

		context->result_syllable_count = 0;
		context->modify_flag |= MODIFY_RESULT;
		return;
	}

	//大写字母输入
	if (ch >= 'A' && ch <= 'Z')
	{
		AddChar(context, ch, 0);

		context->state = (ch == 'U' || ch == 'I') ? STATE_IINPUT : STATE_ILLEGAL;

		if (ch == 'E' && (context->state != STATE_ENGLISH) && !IsFullScreen())
			context->state = STATE_UINPUT;

		context->modify_flag |= MODIFY_STARTCOMPOSE;
		context->show_composition = context->show_candidates = 1;
		return;
	}

	//进行正常字符的输入
	if (ch >= 'a' && ch <= 'z')
	{
		if (context->english_state != ENGLISH_STATE_NONE)
		{
			AddChar(context, ch, 0);
			context->state = STATE_EDIT;
		}
		else if (ch == 'v' && (!share_segment->sp_used_v || pim_config->pinyin_mode != PINYIN_SHUANGPIN))
		{
			AddChar(context, ch, 0);
			context->state = STATE_VINPUT;
		}
		else if (ch == 'i' && (!share_segment->sp_used_i || pim_config->pinyin_mode != PINYIN_SHUANGPIN))
		{
			context->state = STATE_IINPUT;
			AddChar(context, ch, 0);
			context->state = STATE_IINPUT;
		}
		else if (ch == 'u' && (!share_segment->sp_used_u || pim_config->pinyin_mode != PINYIN_SHUANGPIN) && !IsFullScreen())
		{
			AddChar(context, ch, 0);
			context->state = STATE_UINPUT;
		}
		else
		{
			AddChar(context, ch, 0);
			context->state = STATE_EDIT;
		}

		context->modify_flag |= MODIFY_STARTCOMPOSE;
		context->show_composition = context->show_candidates = 1;
		return;
	}
}

void ProcessKeyEdit(PIMCONTEXT *context, int key_flag, int virtual_key, TCHAR ch)
{
	if (!ch)		//控制键
	{
		switch(virtual_key)
		{
		case '1':	case '2':	case '3':	case '4':	case '5':
		case '6':	case '7':	case '8':	case '9':
			if ((key_flag & KEY_CONTROL) && (key_flag & KEY_SHIFT))			//Ctrl-Shift-数字
			{	//设置为固顶字
				if (pim_config->input_style != STYLE_ABC)
					SetFixTopZi(context, virtual_key - '1');

				break;
			}

			if (key_flag & KEY_CONTROL)
				DeleteCi(context, virtual_key - '1');

			break;

		case VK_NEXT:
			NextCandidatePage(context);
			break;

		case VK_PRIOR:
			PrevCandidatePage(context);
			break;

		case VK_DOWN:
			if ((pim_config->input_style != STYLE_ABC || context->english_state == ENGLISH_STATE_INPUT) &&
				(pim_config->key_pair_candidate_page & KEY_PAIR_7))
				NextCandidatePage(context);
			else
				NextCandidateItem(context);

			break;

		case VK_UP:
			if ((pim_config->input_style != STYLE_ABC || context->english_state == ENGLISH_STATE_INPUT) &&
				(pim_config->key_pair_candidate_page & KEY_PAIR_7))
				PrevCandidatePage(context);
			else
				PrevCandidateItem(context);

			break;

		case VK_HOME:
			MoveCursorHead(context);
			break;

		case VK_END:
			MoveCursorTail(context);
			break;

		case VK_LEFT:
			PrevSyllable(context);
			context->selected_digital = 0;
			break;

		case VK_RIGHT:
			if (key_flag & KEY_CONTROL)
				NextSyllable(context);
			else
				MoveCursorRight(context);

			context->selected_digital = 0;
			break;

		case VK_DELETE:
			DelChar(context);
			break;

		case VK_CONTROL:
			if (pim_config->key_candidate_2nd_3rd != KEY_2ND_3RD_CONTROL)
				break;

			if (key_flag & KEY_LCONTROL)
				SelectCandidate(context, 1);
			else if (key_flag & KEY_RCONTROL)
				SelectCandidate(context, 2);

			break;

		case VK_SHIFT:
			if (pim_config->key_candidate_2nd_3rd != KEY_2ND_3RD_SHIFT)
				break;

			if (IllegalState(context))		//处于非法拼音状态，则加入字符
			{
				AddChar(context, ch, 0);
				break;
			}

			if (key_flag & KEY_LSHIFT)
				SelectCandidate(context, 1);
			else if (key_flag & KEY_RSHIFT)
				SelectCandidate(context, 2);

			break;
		}

		context->show_composition = context->show_candidates = 1;

		if (!context->compose_length)
		{
			context->show_composition = context->show_candidates = 0;
			context->state = STATE_START;
			context->modify_flag |= MODIFY_ENDCOMPOSE;
		}

		return;
	}

	//Ctrl + a..z 光标按音节快速定位
	if ((key_flag & KEY_CONTROL) && ch >= 1 && ch <= 26)
	{
		MoveCursorByLetter(context, ch + 'a' - 1);

		context->show_composition = context->show_candidates = 1;
		if (!context->compose_length)
		{
			context->show_composition = context->show_candidates = 0;
			context->state = STATE_START;
			context->modify_flag |= MODIFY_ENDCOMPOSE;
		}

		return;
	}

	switch(ch)
	{
	case 0x1e:		//莫名其妙：Ctrl-Shift-6会产生这个Ascii代码，其他没有
		if ((key_flag & KEY_CONTROL) && (key_flag & KEY_SHIFT))			//Ctrl-Shift-数字
			if (pim_config->input_style != STYLE_ABC)
				SetFixTopZi(context, 5);			//设置为固顶字
		break;
/*
	case 4:		//CTRL-D, 删除词汇
		DeleteCi(context, -1);
		break;
*/
		//候选移动键、以词定字键处理
	case '<':
		if (pim_config->input_style != STYLE_ABC && (pim_config->key_pair_candidate_page & KEY_PAIR_1))		//,.
			PrevCandidateItem(context);
		else
			AddChar(context, ch, 0);
		break;

	case '>':
		if (pim_config->input_style != STYLE_ABC && (pim_config->key_pair_candidate_page & KEY_PAIR_1))		//,.
			NextCandidateItem(context);
		else
			AddChar(context, ch, 0);
		break;

	case ',':
		if (context->has_english_candidate && context->candidate_index <= 0 &&
			(context->english_state != ENGLISH_STATE_INPUT) && (!IsFullScreen()) &&
			(pim_config->input_style == STYLE_CSTAR || context->state == STATE_ABC_SELECT))
		{
			ToggleEnglishCandidate(context);
			break;
		}
		else if ((pim_config->input_style != STYLE_ABC || context->english_state == ENGLISH_STATE_INPUT) &&
				 (pim_config->key_pair_candidate_page & KEY_PAIR_1))		//,.
		{
			PrevCandidatePage(context);
			break;
		}
/*
		else if (pim_config->key_pair_zi_from_word & KEY_PAIR_1)
			if (SelectZFWCandidate(context, context->candidate_selected_index, ZFW_LEFT))
				break;
*/
		AddChar(context, ch, 0);
		break;

	case '.':
		if ((virtual_key & 0xFF) == 0x6E)
		{
			AddChar(context, ch, 1);
		}
		else
		{
			if (IsURLPrefix(context->input_string, ch) || !context->candidate_count)
			{
				context->state = STATE_ILLEGAL;
				//context->url_state = URL_STATE_INPUT;
				AddChar(context, ch, 0);
				break;
			}
			else if ((pim_config->input_style != STYLE_ABC || context->english_state == ENGLISH_STATE_INPUT) &&
					 (pim_config->key_pair_candidate_page & KEY_PAIR_1))		//,.
			{
				NextCandidatePage(context);

				context->next_to_last_dot = context->last_dot;
				context->last_dot		  = 1;

				break;
			}
/*
		else if (pim_config->input_style != STYLE_ABC && (pim_config->key_pair_zi_from_word & KEY_PAIR_1))
			if (SelectZFWCandidate(context, context->candidate_selected_index, ZFW_RIGHT))
				break;
*/
			AddChar(context, ch, 0);
		}

		break;

	case '{':	//智能编辑键
		if ((pim_config->input_style != STYLE_ABC || context->english_state == ENGLISH_STATE_INPUT) &&
			(pim_config->key_pair_candidate_page & KEY_PAIR_2))		//[]
			PrevCandidateItem(context);
		else
			AddChar(context, ch, 0);
		break;

	case '}':
		if ((pim_config->input_style != STYLE_ABC || context->english_state == ENGLISH_STATE_INPUT) &&
			(pim_config->key_pair_candidate_page & KEY_PAIR_2))		//[]
			PrevCandidateItem(context);
		else
			AddChar(context, ch, 0);
		break;

	case '[':
		if ((pim_config->input_style != STYLE_ABC || context->english_state == ENGLISH_STATE_INPUT) &&
			(pim_config->key_pair_candidate_page & KEY_PAIR_2))		//[]
		{
			PrevCandidatePage(context);
			break;
		}
		else if (pim_config->input_style != STYLE_ABC && (pim_config->key_pair_zi_from_word & KEY_PAIR_2))
			if (SelectZFWCandidate(context, context->candidate_selected_index, ZFW_LEFT))
				break;

		AddChar(context, ch, 0);
		break;

	case ']':
		if ((pim_config->input_style != STYLE_ABC || context->english_state == ENGLISH_STATE_INPUT) &&
			(pim_config->key_pair_candidate_page & KEY_PAIR_2))		//[]
		{
			NextCandidatePage(context);
			break;
		}
		else if (pim_config->input_style != STYLE_ABC && (pim_config->key_pair_zi_from_word & KEY_PAIR_2))
			if (SelectZFWCandidate(context, context->candidate_selected_index, ZFW_RIGHT))
				break;

		AddChar(context, ch, 0);
		break;

	case '-':
		if ((virtual_key & 0xFF) == 0x6D)
		{
			AddChar(context, ch, 1);
		}
		else
		{
			if ((pim_config->input_style != STYLE_ABC || context->english_state == ENGLISH_STATE_INPUT) &&
				(pim_config->key_pair_candidate_page & KEY_PAIR_3))		//-=
			{
				PrevCandidatePage(context);
				break;
			}
			else if (pim_config->input_style != STYLE_ABC && (pim_config->key_pair_zi_from_word & KEY_PAIR_3))
				if (SelectZFWCandidate(context, context->candidate_selected_index, ZFW_LEFT))
					break;

			AddChar(context, ch, 0);
		}

		break;

	case '=':
		if ((pim_config->input_style != STYLE_ABC || context->english_state == ENGLISH_STATE_INPUT) &&
			(pim_config->key_pair_candidate_page & KEY_PAIR_3))		//-=
		{
			NextCandidatePage(context);
			break;
		}
		else if (pim_config->input_style != STYLE_ABC && (pim_config->key_pair_zi_from_word & KEY_PAIR_3))
			if (SelectZFWCandidate(context, context->candidate_selected_index, ZFW_RIGHT))
				break;

		AddChar(context, ch, 0);
		break;

	//上屏键处理
	case ' ':
		if (pim_config->input_style == STYLE_ABC && context->english_state != ENGLISH_STATE_INPUT)
		{
			context->expand_candidate = 0;
			context->candidate_index = 0;
			MakeCandidate(context);
			if (context->candidate_count)
				context->state = STATE_ABC_SELECT;
		}
		else if (context->english_state != ENGLISH_STATE_NONE && 0 == context->candidate_page_count)
			SelectInputString(context, 0);
		else
		{
			if (0 == context->candidate_count && context->selected_item_count)
				SelectInputStringWithSelectedItem(context);
			else
				SelectCandidate(context, context->candidate_selected_index);

			//下面语句用于右侧窗口移动处理，不能删除
			if ((context->modify_flag & MODIFY_RESULT) && context->ui_context)
				context->ui_context->have_caret = -1;
		}
		break;

	case 0xd:	//回车
		if (key_flag & KEY_SHIFT)
			SelectCandidate(context, context->candidate_selected_index);
		else if (context->selected_item_count)
			SelectInputStringWithSelectedItem(context);
		else
			SelectInputString(context, 0);

		break;

	case 9:		//TAB
		if (context->force_vertical)
		{
			if (key_flag & KEY_SHIFT)
				PrevCandidatePage(context);
			else
				NextCandidatePage(context);

			break;
		}

		//已经处于扩展状态，则作为翻页键处理
		if (context->expand_candidate)
		{
			if (key_flag & KEY_SHIFT)		//shift-tab
				PrevCandidatePage(context);
			else
				NextCandidatePage(context);

			break;
		}

		if (CanSwitchToExpandMode(context))
		{
			context->expand_candidate = 1;
			ProcessCandidateStuff(context);
			break;
		}

		if (key_flag & KEY_SHIFT)		//shift-tab
			PrevCandidatePage(context);
		else
			NextCandidatePage(context);

		break;

	case 27:		//ESC
		ResetContext(context);
		context->modify_flag |= MODIFY_ENDCOMPOSE;
		break;

	case 8:		//回退键
		if (pim_config->input_style != STYLE_ABC &&	(key_flag & KEY_SHIFT))
		{
			IEditModeStart(context);
			break;
		}

		if (context->selected_item_count && context->input_length)
		{
			context->syllable_pos = context->selected_items[context->selected_item_count - 1].syllable_start_pos;
			context->selected_item_count--;
			context->input_pos = GetInputPos(context);

			ProcessContext(context);
		}
		else if (context->selected_digital)
			ClearSelectedDigital(context);
		else
		{
			BackspaceChar(context);
			if (!context->input_length)
			{
				ResetContext(context);
				context->modify_flag = MODIFY_ENDCOMPOSE;
			}
		}
		break;

	case '`':			//变换音调键
	case '~':
		if (!pim_config->use_hz_tone)		//不使用Tone的情况下，增加一个符号
			AddChar(context, ch, 0);
		else
			ChangeTone(context, ch);					//变换音调
		break;

	case '0':	//直接跳转到字
		{
			if (IllegalState(context))		
				AddChar(context, ch, 0);
			else if (pim_config->input_style == STYLE_ABC)
				AddChar(context, ch, 0);
			else if (context->selected_digital)
				ClearSelectedDigital(context);
			else if (0x60 == (virtual_key & 0xFF) && !pim_config->numpad_select_enabled)
				AddChar(context, ch, 0);
			else
				JumpToZiOrNextPage(context);

			break;
		}

	//处理候选选择键
	case '1':	case '2':	case '3':	case '4':	case '5':
	case '6':	case '7':	case '8':	case '9':
		{
			int is_numpad = IsNumpadKey(virtual_key);

			//处于非法拼音状态，则加入字符
			if ((pim_config->input_style == STYLE_ABC || 
				(IllegalState(context) && context->english_state == ENGLISH_STATE_NONE)|| 
				(!pim_config->numpad_select_enabled && is_numpad) ||
				(ch - '0') > context->candidate_page_count) /*&& (context->english_state == ENGLISH_STATE_NONE)*/)
				AddChar(context, ch, 0);
			else
				ProcessDigitalKey(context, ch);

			break;
		}

	case ':':
		if (IsURLPrefix(context->input_string, ch))
		{
			context->state = STATE_ILLEGAL;
			//context->url_state = URL_STATE_INPUT;
			AddChar(context, ch, 0);
			break;
		}

		AddChar(context, ch, 0);
		break;

	case 127:		//CTRL-BACKSPACE，启动IEdit模式
		if (pim_config->input_style != STYLE_ABC)
			IEditModeStart(context);
		break;

	default:
/*
		if (key_flag & KEY_CONTROL && (ch == 'd' || ch == 'D'))
		{
			DeleteCi(context, -1);
			break;
		}
*/

		//插入字符（在光标位置）
		if (ch >= 0x20 && ch <= 0x80)
		{
			switch (virtual_key & 0xFF)
			{
			case 0x6B:	// +
			case 0x6F:	// /
				AddChar(context, ch, 1);
				break;

			default:
				AddChar(context, ch, 0);
			}
		}

		break;
	}

	context->show_composition = context->show_candidates = 1;
	if (!context->compose_length)
	{
		context->show_composition = context->show_candidates = 0;
		context->state			  = STATE_START;
		context->modify_flag	 |= MODIFY_ENDCOMPOSE;
	}
}

void ProcessKeyEnglish(PIMCONTEXT *context, int key_flag, int virtual_key, char ch)
{
	return;
}

void ProcessKeySelect(PIMCONTEXT *context, int key_flag, int virtual_key, TCHAR ch)
{
	return;
}

void ProcessKeyVInput(PIMCONTEXT *context, int key_flag, int virtual_key, TCHAR ch)
{

	if (context->english_state != ENGLISH_STATE_NONE)
	{
		ProcessKeyEdit(context, key_flag, virtual_key, ch);
		return;
	}

	if (!ch)		//控制键
	{
		switch(virtual_key)
		{
		case VK_HOME:
			MoveCursorHead(context);
			break;

		case VK_END:
			MoveCursorTail(context);
			break;

		case VK_LEFT:
			MoveCursorLeft(context);
			break;

		case VK_RIGHT:
			MoveCursorRight(context);
			break;

		case VK_DELETE:
			DelChar(context);
			break;

		default:
			return;
		}

		if (context->input_length)
		{
			context->show_composition = context->show_candidates = 1;
			context->modify_flag	 |= MODIFY_COMPOSE;
			return;
		}

		//用户输入没有了
		context->show_composition = context->show_candidates = 0;
		context->state			  = STATE_START;
		context->modify_flag	 |= MODIFY_ENDCOMPOSE;
		return;
	}

	switch(ch)
	{
	case 0xd:	//回车
		SelectInputString(context, 0);
		break;

	case 9:		//TAB
		AddChar(context, ' ', 0);
		break;

	case 8:		//BS
		BackspaceChar(context);
		break;

	case 27:		//ESC
		ResetContext(context);
		if (context->ui_context)
			context->ui_context->have_caret = -1;

		context->modify_flag |= MODIFY_ENDCOMPOSE;
		break;

	case 32:		//SPACE
		if (pim_config->input_space_after_v)
		{
			AddChar(context, ch, 0);
		}
		else if (!context->candidate_count)
		{
			SelectInputString(context, 1);
		}
		else if (pim_config->input_style == STYLE_ABC)
		{
			context->expand_candidate = 0;
			context->candidate_index = 0;
			ProcessContext(context);
			MakeCandidate(context);
			context->state = STATE_ABC_SELECT;
		}
		else
			SelectCandidate(context, context->candidate_selected_index);

		break;

	//处理候选选择键
	case '1':	case '2':	case '3':	case '4':	case '5':
	case '6':	case '7':	case '8':	case '9':
		{
			int is_numpad = IsNumpadKey(virtual_key);

			if (!context->candidate_count || (!pim_config->numpad_select_enabled && is_numpad) ||
				(context->candidate_array[0].type == CAND_TYPE_SPW &&
				 context->candidate_array[0].spw.type != SPW_STRING_NORMAL))
			{
				AddChar(context, ch, 0);
				break;
			}

			if (ch - '0' <= context->candidate_page_count)
				ProcessDigitalKey(context, ch);
			else
				AddChar(context, ch, 0);

			break;
		}

	case ',':
		if ((context->has_english_candidate && context->candidate_index <= 0) &&
			(context->english_state != ENGLISH_STATE_INPUT) && (!IsFullScreen()) &&
			(pim_config->input_style == STYLE_CSTAR || context->state == STATE_ABC_SELECT || context->state == STATE_VINPUT))
			ToggleEnglishCandidate(context);
		else
			AddChar(context, ch, 0);

		break;

	default:
		//插入字符（在光标位置）
		if (ch >= 0x20 && ch <= 0x80)
			AddChar(context, ch, 0);

		break;
	}

	if (context->input_length)
	{
		context->modify_flag |= MODIFY_COMPOSE;
		context->show_composition = context->show_candidates = 1;
		return;
	}

	//没有输入
	ResetContext(context);
	if (context->ui_context)
		context->ui_context->have_caret = -1;

	context->show_composition = context->show_candidates = 0;
	context->state			  = STATE_START;
	context->modify_flag	 |= MODIFY_ENDCOMPOSE;

	return;
}

/**	处理错误输入的情况
 */
void ProcessKeyIllegal(PIMCONTEXT *context, int key_flag, int virtual_key, TCHAR ch)
{
	if (!ch)		//控制键
	{
		switch(virtual_key)
		{
		case VK_NEXT:
			NextCandidatePage(context);
			break;

		case VK_PRIOR:
			PrevCandidatePage(context);
			break;

		case VK_DOWN:
			NextCandidateItem(context);
			break;

		case VK_UP:
			PrevCandidateItem(context);
			break;

		case VK_HOME:
			MoveCursorHead(context);
			break;

		case VK_END:
			MoveCursorTail(context);
			break;

		case VK_LEFT:
			if (key_flag & KEY_SHIFT)
				PrevCandidateItem(context);
			else
				MoveCursorLeft(context);
			break;

		case VK_RIGHT:
			if (key_flag & KEY_SHIFT)
				NextCandidateItem(context);
			else
				MoveCursorRight(context);
			break;

		case VK_DELETE:
			DelChar(context);
			break;

		default:
			return;
		}

		if (context->input_length)
		{
			context->show_composition = context->show_candidates = 1;
			context->modify_flag |= MODIFY_COMPOSE;
			return;
		}

		//用户输入没有了
		context->show_composition = context->show_candidates = 0;
		context->state			  = STATE_START;
		context->modify_flag	 |= MODIFY_ENDCOMPOSE;
		return;
	}


	switch(ch)
	{
	case ',':
		if (context->has_english_candidate && context->candidate_index <= 0 &&
			(context->english_state != ENGLISH_STATE_INPUT) && (!IsFullScreen()) &&
			(pim_config->input_style == STYLE_CSTAR || context->state == STATE_ABC_SELECT))
		{
			ToggleEnglishCandidate(context);
			break;
		}
		else if (pim_config->input_style != STYLE_ABC && (IsURLPrefix(context->input_string, ch)))
		{
			context->state = STATE_ILLEGAL;
			//context->url_state = URL_STATE_INPUT;
			AddChar(context, ch, 0);
		}
		else if (pim_config->input_style != STYLE_ABC && (pim_config->key_pair_candidate_page & KEY_PAIR_1) &&
				 (context->candidate_count && context->candidate_count != 1)) //不是一个候选
		{
			PrevCandidatePage(context);
		}
		else
			AddChar(context, ch, 0);

		break;

	case '.':
		if ((virtual_key & 0xFF) == 0x6E)
			AddChar(context, ch, 1);
		else if (pim_config->input_style != STYLE_ABC && (IsURLPrefix(context->input_string, ch)))
		{
			context->state = STATE_ILLEGAL;
			//context->url_state = URL_STATE_INPUT;
			AddChar(context, ch, 0);
		}
		else if (STATE_IINPUT == context->state && context->english_state == ENGLISH_STATE_NONE)
		{
			AddChar(context, ch, 0);
		}
		else if (pim_config->input_style != STYLE_ABC && (pim_config->key_pair_candidate_page & KEY_PAIR_1) &&
				 (context->candidate_count && context->candidate_count != 1)) //不是一个候选
		{
			NextCandidatePage(context);
		}
		else
			AddChar(context, ch, 0);

		break;

	case '-':
		if ((virtual_key & 0xFF) == 0x6D)
			AddChar(context, ch, 1);
		else if (pim_config->input_style != STYLE_ABC &&	(pim_config->key_pair_candidate_page & KEY_PAIR_3) &&
			(context->candidate_count && context->candidate_count != 1))		//-=
			PrevCandidatePage(context);
		else
			AddChar(context, ch, 0);

		break;

	case '=':
		if (pim_config->input_style != STYLE_ABC &&	(pim_config->key_pair_candidate_page & KEY_PAIR_3) &&
			(context->candidate_count && context->candidate_count != 1))		//-=
			NextCandidatePage(context);
		else
			AddChar(context, ch, 0);

		break;

	case '[':
		if (pim_config->input_style != STYLE_ABC &&	(pim_config->key_pair_candidate_page & KEY_PAIR_2) &&
			(context->candidate_count && context->candidate_count != 1))		//[]
			PrevCandidatePage(context);
		else
			AddChar(context, ch, 0);

		break;

	case ']':
		if (pim_config->input_style != STYLE_ABC &&	(pim_config->key_pair_candidate_page & KEY_PAIR_2) &&
			(context->candidate_count && context->candidate_count != 1))		//[]
			NextCandidatePage(context);
		else
			AddChar(context, ch, 0);

		break;

	case 9:		//TAB
		//已经处于扩展状态，则作为翻页键处理
		if (pim_config->use_tab_expand_candidates && context->expand_candidate)
		{
			if (key_flag & KEY_SHIFT)		//shift-tab
				PrevCandidatePage(context);
			else
				NextCandidatePage(context);
			break;
		}

		//处理命令行方式
		if (context->candidate_count < 2)			//只有很少的候选情况下，将Tab作为空格使用
		{
			AddChar(context, ' ', 0);
			break;
		}

		if (CanSwitchToExpandMode(context))
		{
			context->expand_candidate = 1;
			ProcessCandidateStuff(context);
			break;
		}

		if (key_flag & KEY_SHIFT)		//shift-tab
			PrevCandidatePage(context);
		else
			NextCandidatePage(context);

		break;

	case 0xd:	//回车
		if ((key_flag & KEY_SHIFT) && context->candidate_count)
			SelectCandidate(context, context->candidate_selected_index);
		else
			SelectInputString(context, 0);

		break;

	case 8:		//BS
		BackspaceChar(context);
		if (!context->input_length)
		{
			ResetContext(context);
			if (context->ui_context)
				context->ui_context->have_caret = -1;
			context->modify_flag |= MODIFY_ENDCOMPOSE;
		}

		/*if (!IsURLPrefix(context->input_string, ch))
		{
			context->url_state = URL_STATE_NONE;
		}*/

		break;

	case 27:		//ESC
		ResetContext(context);
		if (context->ui_context)
			context->ui_context->have_caret = -1;

		context->modify_flag |= MODIFY_ENDCOMPOSE;
		break;

	case ' ':		//上屏
		if (!context->candidate_count)
		{
			SelectInputString(context, 1);
		}
		else if (pim_config->input_style == STYLE_ABC)
		{
			context->expand_candidate = 0;
			context->candidate_index = 0;
			ProcessContext(context);
			MakeCandidate(context);
			context->state = STATE_ABC_SELECT;
		}
		else
			SelectCandidate(context, context->candidate_selected_index);

		break;

	//处理候选选择键
	case '1':	case '2':	case '3':	case '4':	case '5':
	case '6':	case '7':	case '8':	case '9':
		{
			int is_numpad = IsNumpadKey(virtual_key);

			if (!context->candidate_count || (!pim_config->numpad_select_enabled && is_numpad) ||
				/*context->candidate_array[0].type == CAND_TYPE_URL ||*/
				(context->candidate_array[0].type == CAND_TYPE_SPW &&
				 context->candidate_array[0].spw.type != SPW_STRING_NORMAL &&
				 context->candidate_array[0].spw.type != SPW_STRING_BH &&
				 context->candidate_array[0].spw.type != SPW_STIRNG_ENGLISH &&
				context->candidate_array[0].spw.type != SPW_STRING_EXEC))
			{
				AddChar(context, ch, 0);
				break;
			}

			if (ch - '0' <= context->candidate_page_count)
				ProcessDigitalKey(context, ch);
			else
				AddChar(context, ch, 0);

			break;
		}

	case 'a':	case 'b':	case 'c':	case 'd':	case 'e':
	case 'f':	case 'g':	case 'h':	case 'i':	case 'j':
	case 'k':	case 'l':	case 'm':	case 'n':	case 'o':
	case 'p':	case 'q':	case 'r':	case 's':	case 't':
	case 'u':	case 'v':	case 'w':	case 'x':	case 'y':
	case 'z':
		if (STATE_IINPUT == context->state && context->english_state == ENGLISH_STATE_NONE &&
			(context->candidate_count > 1) && (ch - 'a' < context->candidate_count))
			SelectCandidate(context, ch - 'a');
		else
			AddChar(context, ch, 0);

		break;

	default:
		//插入字符（在光标位置）
		if (ch >= 0x20 && ch <= 0x80)
		{
			switch (virtual_key & 0xFF)
			{
			case 0x6B:	// +
			case 0x6F:	// /
				AddChar(context, ch, 1);
				break;

			default:
				AddChar(context, ch, 0);
			}
		}

		break;
	}

	if (context->input_length)
	{
		context->modify_flag	 |= MODIFY_COMPOSE;
		context->show_composition = context->show_candidates = 1;
		return;
	}

	//没有输入
	context->show_composition = context->show_candidates = 0;
	context->state			  = STATE_START;
	context->modify_flag	 |= MODIFY_ENDCOMPOSE;

	return;
}

void ProcessKeySoftKeyboard(PIMCONTEXT *context, int key_flag, int virtual_key, TCHAR ch)
{
	TCHAR symbol[0x10];

	context->result_string[0] = 0;
	if (!ch || (key_flag & KEY_CONTROL) || !GetSoftKeyboardSymbol(context, virtual_key, key_flag, symbol))
	{
		context->result_string[0]		= ch;
		context->result_string[1]		= 0;
		context->result_length			= 1;
		context->result_syllable_count	= 0;
		context->modify_flag		   |= MODIFY_RESULT;
		return;
	}

	context->result_string[0]		= symbol[0];
	context->result_string[1]		= symbol[1];
	context->result_string[2]		= 0;
	context->result_length			= (int)_tcslen(context->result_string);
	context->result_syllable_count	= 0;
	context->modify_flag		   |= MODIFY_RESULT;
}

void ProcessKeyABCSelect(PIMCONTEXT *context, int key_flag, int virtual_key, TCHAR ch)
{
	int select_style;

	if (!ch)		//控制键
	{
		switch(virtual_key)
		{
		case '1':	case '2':	case '3':	case '4':	case '5':
		case '6':	case '7':	case '8':	case '9':
			if ((key_flag & KEY_CONTROL) && (key_flag & KEY_SHIFT))			//Ctrl-Shift-数字
			{	//设置为固顶字
				SetFixTopZi(context, virtual_key - '1');
				break;
			}

			if (key_flag & KEY_CONTROL)
				DeleteCi(context, virtual_key - '1');

			break;

		case VK_HOME:
			HomeCandidateItem(context);
			break;

		case VK_END:
			EndCandidateItem(context);
			break;

		case VK_NEXT:
			NextCandidatePage(context);
			break;

		case VK_PRIOR:
			PrevCandidatePage(context);
			break;

		case VK_DOWN:
			NextCandidateItem(context);
			break;

		case VK_UP:
			PrevCandidateItem(context);
			break;

		case VK_LEFT:
			PrevCandidatePage(context);
			break;

		case VK_RIGHT:
			NextCandidatePage(context);
			break;

		case VK_CONTROL:
			if (pim_config->key_candidate_2nd_3rd != KEY_2ND_3RD_CONTROL)
				break;

			if (key_flag & KEY_LCONTROL)
				SelectCandidate(context, 1);
			else if (key_flag & KEY_RCONTROL)
				SelectCandidate(context, 2);

			break;

		case VK_SHIFT:
			if (pim_config->key_candidate_2nd_3rd != KEY_2ND_3RD_SHIFT)
				break;

			if (IllegalState(context))		//处于非法拼音状态，则加入字符
			{
				AddChar(context, ch, 0);
				break;
			}

			if (key_flag & KEY_LSHIFT)
				SelectCandidate(context, 1);
			else if (key_flag & KEY_RSHIFT)
				SelectCandidate(context, 2);

			break;
		}

		context->show_composition = context->show_candidates = 1;

		if (!context->compose_length)
		{
			context->show_composition = context->show_candidates = 0;
			context->state			  = STATE_START;
			context->modify_flag	 |= MODIFY_ENDCOMPOSE;
		}

		return;
	}


	switch(ch)
	{
	case 0x1e:		//莫名其妙：Ctrl-Shift-6会产生这个Ascii代码，其他没有
		if ((key_flag & KEY_CONTROL) && (key_flag & KEY_SHIFT))			//Ctrl-Shift-数字
			SetFixTopZi(context, 5);			//设置为固顶字

		break;

	case 9:		//TAB
		if (key_flag & KEY_SHIFT)		//shift-tab
			PrevCandidatePage(context);
		else
			NextCandidatePage(context);
		break;
/*
	case 4:		//CTRL-D, 删除词汇
		DeleteCi(context, -1);
		break;
*/
	case ',':
		if (context->has_english_candidate && context->candidate_index <= 0 &&
			(context->english_state != ENGLISH_STATE_INPUT) && (!IsFullScreen()) &&
			(pim_config->input_style == STYLE_CSTAR || context->state == STATE_ABC_SELECT))
		{
			ToggleEnglishCandidate(context);
			break;
		}
		else if (pim_config->key_pair_candidate_page & KEY_PAIR_1)		//,.
		{
			PrevCandidatePage(context);
			break;
		}
/*
		else if (pim_config->key_pair_zi_from_word & KEY_PAIR_1)
		{
			if (SelectZFWCandidate(context, context->candidate_selected_index, ZFW_LEFT))
				break;
		}
*/
		else
			AddChar(context, ch, 0);

		break;

	case '.':
		if (pim_config->key_pair_candidate_page & KEY_PAIR_1)		//,.
		{
			NextCandidatePage(context);
			break;
		}
/*
		else if (pim_config->key_pair_zi_from_word & KEY_PAIR_1)
		{
			if (SelectZFWCandidate(context, context->candidate_selected_index, ZFW_RIGHT))
				break;
		}
*/
		else
			AddChar(context, ch, 0);

		break;

	//候选移动键、以词定字键处理
	case '<':	case '>':	case '{':	case '}':
		AddChar(context, ch, 0);
		break;

	case '[':
		if (pim_config->key_pair_candidate_page & KEY_PAIR_2)		//[]
		{
			PrevCandidatePage(context);
			break;
		}
		else if (pim_config->key_pair_zi_from_word & KEY_PAIR_2)
		{
			if (SelectZFWCandidate(context, context->candidate_selected_index, ZFW_LEFT))
				break;
		}

		break;

	case ']':
		if (pim_config->key_pair_candidate_page & KEY_PAIR_2)		//[]
		{
			NextCandidatePage(context);
			break;
		}
		else if (pim_config->key_pair_zi_from_word & KEY_PAIR_2)
		{
			if (SelectZFWCandidate(context, context->candidate_selected_index, ZFW_RIGHT))
				break;
		}

		break;

	case '-':
		if (pim_config->key_pair_candidate_page & KEY_PAIR_3)		//-=
		{
			PrevCandidatePage(context);
			break;
		}
		else if (pim_config->key_pair_zi_from_word & KEY_PAIR_3)
		{
			if (SelectZFWCandidate(context, context->candidate_selected_index, ZFW_LEFT))
				break;
		}
		else
			AddChar(context, ch, 0);

		break;

	case '=':
		if (pim_config->key_pair_candidate_page & KEY_PAIR_3)		//-=
		{
			NextCandidatePage(context);
			break;
		}
		else if (pim_config->key_pair_zi_from_word & KEY_PAIR_3)
		{
			if (SelectZFWCandidate(context, context->candidate_selected_index, ZFW_RIGHT))
				break;
		}
		else
			AddChar(context, ch, 0);

		break;

	//上屏键处理
	case 0xd:
	case ' ':
		SelectCandidate(context, context->candidate_selected_index);
		break;

	case 27:		//ESC
		ResetContext(context);
		context->modify_flag |= MODIFY_ENDCOMPOSE;
		break;

	case 8:			//回退键
		if (BackSelectedCandidate(context))
			break;

		context->candidate_count	  = 0;
		context->candidate_page_count = 0;
		context->modify_flag		 |= MODIFY_COMPOSE;
		context->state				  = STATE_EDIT;

		ProcessContext(context);
		break;

	case '0':	//直接跳转到字
		if (!IllegalState(context))		//处于非法拼音状态，则加入字符
			JumpToZiOrNextPage(context);
		else
			AddChar(context, ch, 0);
		break;

	//处理候选选择键
	case '1':	case '2':	case '3':	case '4':	case '5':
	case '6':	case '7':	case '8':	case '9':
		if (pim_config->select_sytle == SELECTOR_DIGITAL || host_is_console ||
			context->english_state != ENGLISH_STATE_NONE)
			SelectCandidate(context, ch - '1');

		break;

	case 'a':	case 'b':	case 'c':	case 'd':	case 'e':
	case 'f':	case 'g':	case 'h':	case 'i':	case 'j':
		if (host_is_console)
		{
			select_style = pim_config->select_sytle;
			pim_config->select_sytle = SELECTOR_DIGITAL;
		}

		if (pim_config->select_sytle == SELECTOR_LETTER && context->english_state == ENGLISH_STATE_NONE &&
			context->candidate_page_count > ch - 'a')
			SelectCandidate(context, ch - 'a');
		else
		{
			if (pim_config->select_sytle != SELECTOR_LETTER)
			{
				//需要判断是否全部音节都有候选
				int sc = GetCandidateSyllableCount(&context->candidate_array[context->candidate_selected_index]);
				if (context->syllable_pos + sc >= context->syllable_count)
				{
					SelectCandidate(context, context->candidate_selected_index);
					context->last_char = ch;
					context->modify_flag = MODIFY_RESULT_CONTINUE;
				}
			}
		}

		if (host_is_console)
			pim_config->select_sytle = select_style;

		break;

	default:
		if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
		{
			if (host_is_console)
			{
				select_style = pim_config->select_sytle;
				pim_config->select_sytle = SELECTOR_DIGITAL;
			}

			if (pim_config->select_sytle != SELECTOR_LETTER)
			{
				//需要判断是否全部音节都有候选
				int sc = GetCandidateSyllableCount(&context->candidate_array[context->candidate_selected_index]);
				if (context->syllable_pos + sc >= context->syllable_count)
				{
					SelectCandidate(context, context->candidate_selected_index);
					context->last_char = ch;
					context->modify_flag = MODIFY_RESULT_CONTINUE;
				}
			}

			if (host_is_console)
				pim_config->select_sytle = select_style;
		}
		else
			AddChar(context, ch, 0);

		break;
	}

	context->show_composition = context->show_candidates = 1;
	if (!context->compose_length)
	{
		context->show_composition = context->show_candidates = 0;
		context->state			  = STATE_START;
		context->modify_flag	 |= MODIFY_ENDCOMPOSE;
	}
}

void ProcessKeyIEdit(PIMCONTEXT *context, int key_flag, int virtual_key, TCHAR ch)
{
	if (!ch)		//控制键
	{
		switch(virtual_key)
		{
		case '1':	case '2':	case '3':	case '4':	case '5':
		case '6':	case '7':	case '8':	case '9':
			if (key_flag & KEY_CONTROL)
				DeleteCi(context, virtual_key - '1');

			break;

		case VK_NEXT:
			NextCandidatePage(context);
			break;

		case VK_PRIOR:
			PrevCandidatePage(context);
			break;

		case VK_DOWN:
			if (VIEW_MODE_VERTICAL == context->candidates_view_mode)
				NextCandidateItem(context);
			else
				NextCandidateLine(context);

			//if (pim_config->show_vertical_candidate || context->url_state ||
			//	(context->english_state != ENGLISH_STATE_NONE && pim_config->english_candidate_vertical)
			//	|| context->state==STATE_UINPUT || context->state==STATE_IINPUT)
			//	NextCandidateItem(context);
			//else
			//	NextCandidateLine(context);

			break;

		case VK_UP:
			if (VIEW_MODE_VERTICAL == context->candidates_view_mode)
				PrevCandidateItem(context);
			else
				PrevCandidateLine(context);

			//if (pim_config->show_vertical_candidate || context->url_state ||
			//	(context->english_state != ENGLISH_STATE_NONE && pim_config->english_candidate_vertical)
			//	|| context->state==STATE_UINPUT || context->state==STATE_IINPUT)
			//	PrevCandidateItem(context);
			//else
			//	PrevCandidateLine(context);

			break;

		case VK_HOME:
			IEditMoveCursorHead(context);
			break;

		case VK_END:
			IEditMoveCursorTail(context);
			break;

		case VK_LEFT:
			IEditPrevSyllable(context);
			break;

		case VK_RIGHT:
			IEditNextSyllable(context);
			break;

		case VK_DELETE:
			IEditModeEnd(context);
			context->state = STATE_EDIT;
			ProcessKeyEdit(context, key_flag, virtual_key, ch);
			return;

		case VK_CONTROL:
			if (pim_config->key_candidate_2nd_3rd != KEY_2ND_3RD_CONTROL)
				break;

			if (key_flag & KEY_LCONTROL)
				IEditSelectCandidate(context, 1);
			else if (key_flag & KEY_RCONTROL)
				IEditSelectCandidate(context, 2);

			break;

		case VK_SHIFT:
			if (pim_config->key_candidate_2nd_3rd != KEY_2ND_3RD_SHIFT)
				break;

			if (key_flag & KEY_LSHIFT)
				IEditSelectCandidate(context, 1);
			else if (key_flag & KEY_RSHIFT)
				IEditSelectCandidate(context, 2);

			break;
		}
		return;
	}

	switch(ch)
	{
/*
	case 4:		//CTRL-D, 删除词汇
		DeleteCi(context, -1);
		break;
*/
	case '{':	//智能编辑键
		IEditPrevSyllable(context);
		break;

	case '}':
		IEditNextSyllable(context);
		break;

		//候选移动键、以词定字键处理
	case '<':
		if (pim_config->input_style != STYLE_ABC && (pim_config->key_pair_candidate_page & KEY_PAIR_1))		//,.
			PrevCandidateItem(context);
		break;

	case '>':
		if (pim_config->input_style != STYLE_ABC && (pim_config->key_pair_candidate_page & KEY_PAIR_1))		//,.
			NextCandidateItem(context);
		break;

	case ',':
		if (pim_config->input_style != STYLE_ABC && (pim_config->key_pair_candidate_page & KEY_PAIR_1))		//,.
			PrevCandidatePage(context);
		break;

	case '.':
		if (pim_config->input_style != STYLE_ABC && (pim_config->key_pair_candidate_page & KEY_PAIR_1))		//,.
			NextCandidatePage(context);
		break;

	case '[':
		if (pim_config->input_style != STYLE_ABC && (pim_config->key_pair_candidate_page & KEY_PAIR_2))		//[]
			PrevCandidatePage(context);
		break;

	case ']':
		if (pim_config->input_style != STYLE_ABC && (pim_config->key_pair_candidate_page & KEY_PAIR_2))		//[]
			NextCandidatePage(context);
		break;

	case '-':
		if (pim_config->input_style != STYLE_ABC && (pim_config->key_pair_candidate_page & KEY_PAIR_3))		//-=
			PrevCandidatePage(context);
		break;

	case '=':
		if (pim_config->input_style != STYLE_ABC && (pim_config->key_pair_candidate_page & KEY_PAIR_3))		//-=
			NextCandidatePage(context);
		break;

	//上屏键处理
	case ' ':
		if (context->iedit_syllable_index == context->syllable_count)
			IEditSelectResult(context);
		else
			IEditSelectCandidate(context, context->candidate_selected_index);

		break;

	case 0xd:	//回车
		IEditSelectResult(context);
		break;

	case 9:		//TAB
		//已经处于扩展状态，则作为翻页键处理
		if (context->expand_candidate)
		{
			if (key_flag & KEY_SHIFT)		//shift-tab
				PrevCandidatePage(context);
			else
				NextCandidatePage(context);

			break;
		}

		if (CanSwitchToExpandMode(context))
		{
			context->expand_candidate = 1;
			ProcessCandidateStuff(context);
			break;
		}

		if (key_flag & KEY_SHIFT)		//shift-tab
			PrevCandidatePage(context);
		else
			NextCandidatePage(context);

		break;

	case 27:		//ESC
		ResetContext(context);
		if (context->ui_context)
			context->ui_context->have_caret = -1;

		context->modify_flag |= MODIFY_ENDCOMPOSE;

		break;

	case '0':	//直接跳转到字
		if (context->selected_digital)
			ClearSelectedDigital(context);
		else
			JumpToZiOrNextPage(context);

		break;

	//处理候选选择键
	case '1':	case '2':	case '3':	case '4':	case '5':
	case '6':	case '7':	case '8':	case '9':
		{
			int is_numpad = IsNumpadKey(virtual_key);

			if (!is_numpad || (pim_config->numpad_select_enabled && is_numpad))
				ProcessDigitalKey(context, ch);

			break;
		}

	case 127:			//CTRL-BACKSPACE
		IEditPrevSyllable(context);
		return;

	case 0x8:			//BackSpace
		if (key_flag & KEY_SHIFT)
		{
			IEditPrevSyllable(context);
			return;
		}

		if (context->selected_digital)
		{
			ClearSelectedDigital(context);
			return;
		}

	default:
		IEditModeEnd(context);
		context->state = STATE_EDIT;
		ProcessKeyEdit(context, key_flag, virtual_key, ch);

		return;
	}
}

/*	处理所有状态下的按键。
 *	如果处理了键，则返回1，否则返回0
 */
void ProcessKey(PIMCONTEXT *context, int key_flag, int virtual_key, TCHAR ch)
{
	SetCandidatesViewMode(context);//设置竖排显示模式

	if(!ch)
	{
		//TCHAR *url_string;
		TCHAR search_url[MAX_SEARCH_URL_LENGTH] = {0};

		switch(virtual_key)
		{
			case VK_F9:
				/*if(context->url_state)
				{
					if(context->candidate_count)
						ExecuateProgram(context->candidate_string[context->candidate_selected_index], 0, 1);
					else
						ExecuateProgram(context->input_string, 0, 1);

					ResetContext(context);
					context->modify_flag |= MODIFY_ENDCOMPOSE;

					break;
				}

				url_string = GetURL();

				if (url_string && _tcslen(url_string))
				{
					ExecuateProgram(url_string, 0, 1);
				}
				else */if (context->english_state != ENGLISH_STATE_NONE && context->candidate_count)
				{
					EncodeSearchURL(search_url, context->candidate_string[context->candidate_selected_index], 0);
					ExecuateProgram(search_url, 0, 1);

					ResetContext(context);
					context->modify_flag |= MODIFY_ENDCOMPOSE;

					break;
				}
				else if (context->candidate_count)
				{
					EncodeSearchURL(search_url, context->candidate_string[context->candidate_selected_index], 1);
					ExecuateProgram(search_url, 0, 1);
				}

				ResetContext(context);
				context->modify_flag |= MODIFY_ENDCOMPOSE;

				break;

			/*case VK_F10:
				url_string = GetURL();

				if (url_string && _tcslen(url_string))
					OutURLString(context);

				break;*/

			default:
				break;
		}
	}

	switch(context->state)
	{
	case STATE_START:
		ProcessKeyStart(context, key_flag, virtual_key, ch);
		break;

	case STATE_RESULT:
		break;

	case STATE_EDIT:
		ProcessKeyEdit(context, key_flag, virtual_key, ch);
		break;

	case STATE_ENGLISH:
		break;

	case STATE_SELECT:
		ProcessKeySelect(context, key_flag, virtual_key, ch);
		break;

	case STATE_VINPUT:
		ProcessKeyVInput(context, key_flag, virtual_key, ch);
		break;

	case STATE_ILLEGAL:
	case STATE_IINPUT:
		context->u_state = 0;
		ProcessKeyIllegal(context, key_flag, virtual_key, ch);
		break;

	case STATE_UINPUT:
		context->u_state = 1;
		ProcessKeyIllegal(context, key_flag, virtual_key, ch);
		break;

	case STATE_SOFTKBD:
		//pc键盘
		if (context->softkbd_index == SOFTKBD_NUMBER - 1)
		{
			if (0 == context->input_length)
				ProcessKeyStart(context, key_flag, virtual_key, ch);
			else
				ProcessKeyEdit(context, key_flag, virtual_key, ch);
		}
		else
			ProcessKeySoftKeyboard(context, key_flag, virtual_key, ch);

		break;

	case STATE_CAPITAL:
		ProcessKeyStart(context, key_flag, virtual_key, ch);
		break;

	case STATE_ABC_SELECT:
		ProcessKeyABCSelect(context, key_flag, virtual_key, ch);
		break;

	case STATE_IEDIT:
		ProcessKeyIEdit(context, key_flag, virtual_key, ch);
		break;
	}

	SetCandidatesViewMode(context);
}