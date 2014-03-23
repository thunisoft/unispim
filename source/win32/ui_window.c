/*	输入法核心UI窗口。
 *	Maxthon2: Tooltip窗口造成Dialog不能立即显示，禁止Tooltip功能。2007-7-2
 */
#include <windows.h>
#include <config.h>
#include <utility.h>
#include <editor.h>
#include <win32/pim_ime.h>
#include <win32/main_window.h>
#include <time.h>
#include <tchar.h>
#include <share_segment.h>
#include <libfunc.h>

//#define	UI_TIMER_ID		0x1234				//输入法时间事件标识

//#pragma data_seg(HYPIM_SHARED_SEGMENT)
//int pim_update_flag = PIM_UPDATE_NOSTART;			//词库是否已经更新完毕
//int next_pim_update_time = 0;						//下一次的词库更新的时间
//#pragma data_seg()

static int app_active = 1;

PIMCONTEXT *GetPIMContextByWindow(HWND ui_window)
{
	UICONTEXT* ui_context;

	//需要判断ui_context的合法性（TestManager出错，Release版本没有这个问题，奇怪）
	if (!IsWindow(ui_window))
		return 0;

	//PhotoShop会出一个DDE的错误，可能此时ui_window已经不存在了
	//Release版本可能会过滤这个错误消息。
	ui_context = (UICONTEXT*)(LONG_PTR)GetWindowLongPtr(ui_window, IMMGWLP_PRIVATE);

	if (ui_context)
		return ui_context->context;

	return 0;
}

void SetUIContextToWindow(UICONTEXT *context, HWND ui_window)
{
	SetWindowLongPtr(ui_window, IMMGWLP_PRIVATE, (__int3264)(LONG_PTR)context);
}

UICONTEXT *GetUIContextByWindow(HWND ui_window)
{
	if (IsWindow(ui_window))
		return (UICONTEXT*)(LONG_PTR)GetWindowLongPtr(ui_window, IMMGWLP_PRIVATE);

	return 0;
}

void UpdateWinResource(PIMCONTEXT *context, UICONTEXT *ui_context)
{
	if (!context || !ui_context)
		return;

	if (context->local_config_update_time < share_segment->global_config_update_time)
	{
		int show_status, show_main;

		//重新设置使用的资源
		context->local_config_update_time = GetCurrentTicks();
		MaintainWordLibPointer();

		show_status = IsWindowVisible(ui_context->status_window);
		show_main	= IsWindowVisible(ui_context->main_window);

		ResetContext(context);
		HideStatusWindow(context, ui_context);
		HideMainWindow(context, ui_context);
		DestroyIMEWindows(context, ui_context);
		CreateIMEWindows(context, ui_context);

		if (show_status)
			ShowStatusWindow(context, ui_context);

		if (show_main)
			ShowMainWindow(context, ui_context);
	}

	CheckThemeResource(ui_context);
}

/*	通过UI窗口句柄，获得系统上下文句柄
 */
HIMC GetIMC(HWND ui_window)
{
	return (HIMC)(LONG_PTR)GetWindowLongPtr(ui_window, IMMGWLP_IMC);
}

void HideUnactivatedWindow(PIMCONTEXT *context, UICONTEXT  *ui_context)
{
	int i;

	//隐藏所有未激活的状态栏窗体(解决Maxthon下僵尸条的问题)
	if (!ui_context || !ui_context->status_window)
		return;

	for (i = 0; i < MAX_STATUS_LIST_SIZE; i++)
	{
		if (0 == share_segment->status_window_list[i] || 
			ui_context->status_window == (HWND)share_segment->status_window_list[i])
			continue;

		if (IsWindowVisible((HWND)share_segment->status_window_list[i]))
			ShowWindow((HWND)share_segment->status_window_list[i], SW_HIDE);

		if (share_segment->main_window_list[i] != 0 && IsWindowVisible((HWND)share_segment->main_window_list[i]))
			ShowWindow((HWND)share_segment->main_window_list[i], SW_HIDE);

		if (share_segment->hint_window_list[i] != 0 && IsWindowVisible((HWND)share_segment->hint_window_list[i]))
			ShowWindow((HWND)share_segment->hint_window_list[i], SW_HIDE);
	}

	if (pim_config->show_status_window && !IsWindowVisible(ui_context->status_window))
		ShowStatusWindow(context, ui_context);
}

void Ime_SetContext(HWND ui_window, WPARAM wParam, LPARAM lParam)
{
	HIMC hIMC;
	LPINPUTCONTEXT pIMC = 0;
	PIMCONTEXT *context = 0;
	static int process_show_or_hide = 0;	//是否处理Hide、Show消息

	Lock();
	Log(LOG_ID, L"设置Context, ui_window:%x, wParam:%d, lParam:%x", ui_window, wParam, lParam);

	if (!((hIMC = GetIMC(ui_window)) && (context = LockContext(hIMC, &pIMC))))
	{
		Log(LOG_ID, L"没有pIMC或Context。hIMC = %x, pIMC = %x, context = %x", hIMC, pIMC, context);
		if (context && context->ui_context)
			context->ui_context->process_show_or_hide = 1;
		process_show_or_hide = 1;

		if (wParam)
		{
			context = GetPIMContextByWindow(ui_window);
			if (context)
			{
				HideMainWindow(context, context->ui_context);
				HideStatusWindow(context, context->ui_context);
			}
		}

		Unlock();
		return;
	}

	Log(LOG_ID, L"hIME = %x, pIMC = %x, pIMC->hWnd = %x, context->hwnd = %x", hIMC, pIMC, pIMC->hWnd, context->current_input_window);

	Log(LOG_ID, L"selected:%d, status:%d", wParam, ImmGetOpenStatus(hIMC));

	if (context && !wParam)
	{
		pIMC->fOpen = 0;
		if (process_show_or_hide ||
			(context->ui_context && context->ui_context->process_show_or_hide))
		{
			HideMainWindow(context, context->ui_context);

			//在Vista+IE8保护模式下，会出现软键盘第一次输入后状态条被隐藏的情况，因此注释掉下面这句代码
			//by yuanzh 2009.3.19
			//HideStatusWindow(context, context->ui_context);
		}
	}
	else if (context && wParam)
	{
		BYTE key_state[0x100];
		GetKeyboardState(key_state);

		pIMC->fOpen = 1;		//为解决Microsoft live writer的切换后不能输入而添加。2008-1-17
		UpdateWinResource(context, context->ui_context);

		if (context)
		{
			if (((key_state[VK_CAPITAL] & 0x1) && !context->capital) ||	(!(key_state[VK_CAPITAL] & 0x1) && context->capital))
			{
				context->capital = key_state[VK_CAPITAL] & 1 ? 1 : 0;
				ResetContext(context);
			}
			else if (process_show_or_hide || (context->ui_context && context->ui_context->process_show_or_hide))
			{
				//TODO:由于Excel的输入造成Status条不显示，需要继续修改
				ShowStatusWindow(context, context->ui_context);
				ShowMainWindow(context, context->ui_context);
			}

			HideUnactivatedWindow(context, context->ui_context);

			//为了避免DreamWeaver死机，但是与conime冲突。
			//再没有输入的情况下，发送这个消息
			if (!context->input_length)
				GenerateImeMessage(hIMC, WM_IME_ENDCOMPOSITION, 0, 0);
		}
	}

	if (context && context->ui_context)
		context->ui_context->process_show_or_hide = 0;

	process_show_or_hide = 0;

	UnlockContext(hIMC, pIMC);
	Unlock();
}

/*	处理IME的通知消息
 */
void Ime_Notify(HWND ui_window, WPARAM wParam, LPARAM lParam)
{
	HIMC hIMC;
	LPINPUTCONTEXT pIMC;
	PIMCONTEXT *context;
	UICONTEXT  *ui_context;
	POINT point;
	static int last_set_font = 0;

	int st = clock();

	if (!((hIMC = GetIMC(ui_window)) && (context = LockContext(hIMC, &pIMC))))
	{
		Log(LOG_ID, L"No Context");
		return;
	}

	ui_context = context->ui_context;
	if (!ui_context)
		return;

	switch(wParam)			//子消息处理
	{
	case IMN_CLOSESTATUSWINDOW:		//关闭status窗口
		HideStatusWindow(context, ui_context);
		HideMainWindow(context, ui_context);				//必须跟着关闭Compose窗口
		break;

	case IMN_OPENSTATUSWINDOW:		//打开status窗口
		ShowMainWindow(context, ui_context);
		ShowStatusWindow(context, ui_context);
		break;

	case IMN_OPENCANDIDATE:			//打开candidate窗口
		CheckMainWindowPosition(context, ui_context, pIMC->hWnd);
		ShowMainWindow(context, ui_context);
		break;

	case IMN_CHANGECANDIDATE:
		CheckMainWindowPosition(context, ui_context, pIMC->hWnd);
		ShowMainWindow(context, ui_context);
		break;

	case IMN_CLOSECANDIDATE:		//关闭candidate窗口
		HideMainWindow(context, ui_context);
		break;

	case IMN_SETCONVERSIONMODE:		//转换方式改变，需要重画status窗口
		Log(LOG_ID, L"IMN_SETCONVERSIONMODE, ui:%x", ui_window);
		if (!((hIMC = GetIMC(ui_window)) && (pIMC = ImmLockIMC(hIMC))))
			break;

		GetConversionStatus(hIMC, pIMC, context);
		ImmUnlockIMC(hIMC);
		ResetContext(context);
		HideMainWindow(context, ui_context);
		//由于传奇世界造成的问题，必须显示这个窗口
		//ShowStatusWindow(context, ui_context);
		UpdateStatusWindow(context, ui_context);

		HideUnactivatedWindow(context, ui_context);
		break;

	case IMN_SETSENTENCEMODE:		//语句方式改变，不做操作
		Log(LOG_ID, L"IMN_SETSENTENCEMODE, ui:%x", ui_window);
		HideUnactivatedWindow(context, ui_context);
		break;

	case IMN_SETOPENSTATUS:			//状态窗口显示
		Log(LOG_ID, L"IMN_SETOPENSTATUS, ui:%x, hIMC:%x, open:%d", ui_window, hIMC, pIMC->fOpen);
		CheckHintState(context, ui_context);

		if (lParam)
		{
//			process_show_or_hide = 1;
			if (pIMC->fOpen)
			{
//				ShowStatusWindow(context, ui_context);
//				ShowMainWindow(context, ui_context);
			}
		}
		else
		{
			if (!app_active)
				pIMC->fOpen = 1;
//			process_show_or_hide = 1;
			if (!pIMC->fOpen && (program_id == PID_WRITER))
			{
				HideStatusWindow(context, ui_context);
				HideMainWindow(context, ui_context);
			}
		}

		break;

	case IMN_SETCANDIDATEPOS:
		Log(LOG_ID, L"IMN_SETCANDIDATEPOS, ui:%x", ui_window);

		if (pIMC->hWnd)			//有输入焦点窗口句柄
		{
			point = pIMC->cfCandForm[0].ptCurrentPos;
			ClientToScreen(pIMC->hWnd, &point);

			//此时不能进行边界的判断，不然拖动窗口无法处理。FireFox产生这个问题。2007-8-27
			context->ui_context->main_window_pos.x = 30000;
			MainWindowTraceCaret(context, ui_context, point);
			context->ui_context->have_caret = 1;
		}
		break;

	case IMN_SETCOMPOSITIONFONT:		//设置写作窗口字体
		Log(LOG_ID, L"IMN_SETCOMPOSITIONFONT, ui:%x", ui_window);

		//Excel会不停地发送SETCOMPOSITIONWINDOW消息，所以在右侧的避免抖动机制
		//无法正常，加入这个局部变量则可以改善。
		last_set_font = 1;
		break;

	case IMN_SETCOMPOSITIONWINDOW:		//设置写作窗口位置
		Log(LOG_ID, L"IMN_SETCOMPOSITIONWINDOW, ui:%x, hwnd=%x, xy=<%d,%d>", ui_window, pIMC->hWnd,
			pIMC->cfCompForm.ptCurrentPos.x, pIMC->cfCompForm.ptCurrentPos.y);

		if (pIMC->hWnd &&			//有输入焦点窗口句柄
			((pIMC->cfCompForm.dwStyle & (CFS_POINT | CFS_RECT | CFS_CANDIDATEPOS)) != 0))	//格式设定是点或矩形
		{
			point = pIMC->cfCompForm.ptCurrentPos;
			ClientToScreen(pIMC->hWnd, &point);
			Log(LOG_ID, L"x,y = <%d, %d>, style:%x", point.x, point.y, pIMC->cfCompForm.dwStyle);
			if (point.x >= -30000 && point.y >= -30000)
			{
				//此时不能进行边界的判断，不然拖动窗口无法处理。
				if (!last_set_font)
					context->ui_context->main_window_pos.x = 30000;

				MainWindowTraceCaret(context, ui_context, point);

				if (context->ui_context)
					context->ui_context->have_caret = 1;
			}
		}
		else if (context->ui_context)
			context->ui_context->have_caret = -1;

		last_set_font = 0;
		break;

	case IMN_GUIDELINE:					//显示提示信息
		Log(LOG_ID, L"IMN_GUIDELINE, ui:%x", ui_window);
		break;

	case IMN_OPENSTATUSWINDOW_ONLY:		//打开status窗口
		Log(LOG_ID, L"IMN_OPENSTATUSWINDOW_ONLY, ui:%x", ui_window);
		ShowStatusWindow(context, ui_context);
		break;

	case IMN_CLOSESTATUSWINDOW_ONLY:		//关闭status窗口
		Log(LOG_ID, L"IMN_CLOSESTATUSWINDOW_ONLY, ui:%x", ui_window);
		HideStatusWindow(context, ui_context);
		break;

	default:
		Log(LOG_ID, L"IMN_UNKNOWN, ui:%x, sub_msg:%X", ui_window, wParam);
		break;
	}

	UnlockContext(hIMC, pIMC);

	Log(LOG_ID, L"Cost:%d", clock() - st);
}

void Ime_Select(HWND ui_window, WPARAM wParam, LPARAM lParam)
{
	HIMC hIMC = 0;
	LPINPUTCONTEXT pIMC = 0;
	PIMCONTEXT *context = 0;

//	process_show_or_hide = 1;

//	Lock();
	Log(LOG_ID, L"UI选择IME, ui:0x%x, wparam:%d\n", ui_window, wParam);

	//如果进行了配置的变化，需要重新加载资源
	if (!((hIMC = GetIMC(ui_window)) && (context = LockContext(hIMC, &pIMC))))
	{
		Log(LOG_ID, L"hIMC=%x, pIMC=%p, context=%p", hIMC, pIMC, context);
		Unlock();
		return;
	}

	if (wParam)			//被选中
	{
		//if (pIMC && !pIMC->fOpen)
		//	ImmSetOpenStatus(hIMC, 1);
		UpdateWinResource(context, context->ui_context);
		context->current_input_window = pIMC->hWnd;
		GenerateImeMessage(hIMC, WM_IME_ENDCOMPOSITION, 0, 0);
		PostMessage(pIMC->hWnd, WM_IME_NOTIFY, IMN_SETCOMPOSITIONWINDOW, 1);

		//如果不是CMD窗口的话，可以显示
		if (!no_status_show)
			ShowStatusWindow(context, context->ui_context);
//		GenerateImeMessage(hIMC, WM_IME_NOTIFY, IMN_SETCOMPOSITIONWINDOW, 1);
	}

//	if (!wParam)		//被切换
//	{
//		//清除IME使用的三个窗口
//		//if (pIMC && pIMC->fOpen)
//		//	ImmSetOpenStatus(hIMC, 0);
////		HideStatusWindow(context, context->ui_context);
//	}

	UnlockContext(hIMC, pIMC);
//	Unlock();
}

void Ime_Control(HWND ui_window, WPARAM wParam, LPARAM lParam)
{
	HIMC hIMC;
	LPINPUTCONTEXT pIMC;
	PIMCONTEXT *context;
	UICONTEXT  *ui_context;

	Log(LOG_ID, L"WM_IME_CONTROL, ui:%X, wParam:%x, lParam:%x", ui_window, wParam, lParam);

//	Lock();
	if (!((hIMC = GetIMC(ui_window)) && (context = LockContext(hIMC, &pIMC))))
	{
		Unlock();
		return;
	}

	ui_context = context->ui_context;
	if (!ui_context)
		return;

	switch(wParam)
	{
	case IMC_CLOSESTATUSWINDOW:
//		HideStatusWindow(context, ui_context);
		break;

	case IMC_GETCANDIDATEPOS:
		break;

	case IMC_GETCOMPOSITIONFONT:
		break;

	case IMC_GETCOMPOSITIONWINDOW:
		break;

	case IMC_GETSTATUSWINDOWPOS:
		break;

	case IMC_OPENSTATUSWINDOW:
		ShowStatusWindow(context, ui_context);
		break;

	case IMC_SETCANDIDATEPOS:
		break;

	case IMC_SETCOMPOSITIONFONT:
		break;

	case IMC_SETCOMPOSITIONWINDOW:
		Log(LOG_ID, L"显示主窗口");
		ShowMainWindow(context, ui_context);
		break;

	case IMC_SETSTATUSWINDOWPOS:
		ShowStatusWindow(context, ui_context);
		break;
	}

	UnlockContext(hIMC, pIMC);
//	Unlock();
}

/**	执行更新程序
改为先调用dll里的CanRunUpdateApp方法，看看能否运行自动更新程序(允许更新并且可以检查)
解决输入过程中不允许更新也运行自动更新程序的问题 by yuanzh 2008.5.5
 */
/*int ExecuateUpdate(HWND ui_window)
{
    //typedef int (_stdcall *pCanRunUpdateApp) (INT_PTR);

	//HMODULE dll;
	//pCanRunUpdateApp canRunUpdateApp;
	int ret;

	char  cmd_line[MAX_PATH];
	TCHAR install_dir[MAX_PATH];		//安装路径
	char  install_dir_ansi[MAX_PATH];
	char  update_exe_name[MAX_PATH];

	ret = CanRunUpdateApp((INT_PTR)ui_window);
	//Log(LOG_ID, L"装载DLL");

	//dll = LoadLibrary(UTILITY_DLL_NAME);
	//if (!dll)
	//	return 0;

	//ret = 0;
	//canRunUpdateApp = (pCanRunUpdateApp) GetProcAddress(dll, "CanRunUpdateApp");
	//if (canRunUpdateApp)
	//	ret = (*canRunUpdateApp)((INT_PTR)ui_window);

	//Log(LOG_ID, L"调用CanRunUpdateApp");

	//FreeLibrary(dll);

	//Log(LOG_ID, L"卸载DLL");

	if (!ret)
		return 1;

	if (!GetInstallDir(install_dir))
		return 0;

	Utf16ToAnsi(install_dir, install_dir_ansi, MAX_PATH);
	Utf16ToAnsi(UPDATE_EXE_NAME, update_exe_name, MAX_PATH);
	sprintf_s(cmd_line, sizeof(cmd_line), "\"%s\\bin\\%s\" -ime %d", install_dir_ansi, update_exe_name, (INT_PTR)ui_window);

	return WinExec(cmd_line, SW_SHOWNOACTIVATE);
}*/

/**	检查更新程序
 */
/*void CheckUpdate(HWND ui_window)
{
	int cur_time;

	switch(share_segment->pim_update_flag)
	{
	case PIM_UPDATE_NOSTART:
		ExecuateUpdate(ui_window);
		return;

	case PIM_UPDATE_RUNNING:
		return;

	case PIM_UPDATE_WAITING:
		cur_time = GetCurrentTicks();
		if (cur_time < share_segment->next_pim_update_time)
			return;		//时间未到

		ExecuateUpdate(ui_window);
		return;

	case PIM_UPDATE_FINISHED:
		return;

	case PIM_UPDATE_ERROR:
		return;
	}
}*/

/*	UI窗口消息处理程序。
*/
LRESULT WINAPI UIWindowProcedure(HWND ui_window, UINT message, WPARAM wParam, LPARAM lParam)
{
	HIMC hIMC;
	LPINPUTCONTEXT pIMC;
	PIMCONTEXT *context;
	UICONTEXT *ui_context;
	static UINT x = 0;
	static WPARAM y = 0;
	static HIMCC candidate_handle = 0;

	Log(LOG_ID, L"UI消息：hwnd:0x%x, message:0x%x, wParam:0x%x, lParam:0x%x", ui_window, message, wParam, lParam);

	hIMC = GetIMC(ui_window);
	context = LockContext(hIMC, &pIMC);
	UnlockContext(hIMC, pIMC);

//	Log(LOG_ID, L"context Info: hwnd:0x%x, hIMC:0x%x, pIMC:0x%x, context:0x%x", ui_window, hIMC, pIMC, context);

	ui_context = 0;
	if (context)// && !context->ui_context)
	{
		__try
		{
			ui_context = GetUIContextByWindow(ui_window);
			if (ui_context)// && !ready_ui_context->context)
			{
//				Log(LOG_ID, L"Copy ui_context to context, ui_window:%x, himc=%x, context=%x, ui_context=%x", ui_window, hIMC, context, ui_context);
				context->ui_context = ui_context;
				ui_context->context = context;
			}
		}
		__except (1)
		{
			ui_context = 0;
			context	= 0;
		}
	}

    switch (message)
	{
		case WM_ACTIVATEAPP:
			app_active = wParam == 1;
			if (ui_context)
				ui_context->process_show_or_hide = 1;

//			process_show_or_hide = 1;
			if (!app_active)
			{
				Log(LOG_ID, L"Deactive!");
//				HideMainWindow(context, ui_context);
//				HideStatusWindow(context, ui_context);
			}
			else
			{
				Log(LOG_ID, L"Active!");
//				ShowMainWindow(context, ui_context);
//				ShowStatusWindow(context, ui_context);
			}

			return DefWindowProc(ui_window, message, wParam, lParam);

		//TODO:需要加入对“传奇世界”的判断
		//还有ActivateApp的判断
		case WM_DISPLAYCHANGE:
		case WM_PALETTECHANGED:
			if (ui_context)
				ui_context->process_show_or_hide = 1;

			if (app_active)
			{
				ShowStatusWindow(context, ui_context);
//				ShowMainWindow(context, ui_context);
			}

			return DefWindowProc(ui_window, message, wParam, lParam);

		case WM_DESTROY:
			Log(LOG_ID, L"UI_WINDOW DESTROY");
			if (ui_context)
				ui_context->process_show_or_hide = 1;

			if (context && context->ui_context)
			{
				DestroyIMEWindows(context, context->ui_context);
				FreeUIContext(context->ui_context);
				context->ui_context = 0;
				SetUIContextToWindow(0, ui_window);
			}

			//KillTimer(ui_window, UI_TIMER_ID);
			return DefWindowProc(ui_window, message, wParam, lParam);

		case WM_CREATE:
			hIMC = GetIMC(ui_window);
			Log(LOG_ID, L"UI_WINDOW CREATE");
			ui_context = AllocateUIContext();
			if (ui_context)
			{
				demo_context.ui_context = ui_context;
				memcpy(ui_context, &default_ui_context, sizeof(UICONTEXT));
				ui_context->ui_window = ui_window;
				CreateIMEWindows(&demo_context, ui_context);
				SetUIContextToWindow(ui_context, ui_window);
				demo_context.ui_context = 0;
			}

			//创建一个用于进行更新判断的TIMER。每次时间为1分钟。
			//SetTimer(ui_window, UI_TIMER_ID, 60000, 0);
			return DefWindowProc(ui_window, message, wParam, lParam);

		case WM_TIMER:
			//CheckUpdate(ui_window);
			//RunBackup();
			break;

		case WM_IME_SELECT:
			Ime_Select(ui_window, wParam, lParam);		//处理SelectIME消息
			break;

		case WM_IME_SETCONTEXT:
			Ime_SetContext(ui_window, wParam, lParam);		//设置上下文
			break;

		case WM_IME_NOTIFY:
			Ime_Notify(ui_window, wParam, lParam);
			break;

		case WM_IME_CONTROL:
			Ime_Control(ui_window, wParam, lParam);			//应用控制IME，虽不建议，但必须实现
			break;

		case WM_IME_ENDCOMPOSITION:
			Log(LOG_ID, L"WM_IME_ENDCOMPOSITION, ui:%X", ui_window);
			if (context && context->ui_context)
			{
				context->ui_context->main_window_pos.x += 0x400;
				Log(LOG_ID, L"have_caret = %d", context->ui_context->have_caret);
				if (context->ui_context->have_caret == -1)		//按下ESC后的操作
				{
//					context->ui_context->have_caret = 1;
					context->ui_context->main_window_pos.x += 0x400;
				}
//				else
//					context->ui_context->have_caret = 0;
			}
			break;

		case WM_IME_STARTCOMPOSITION:
			Log(LOG_ID, L"WM_IME_STARTCOMPOSITION, ui:%X", ui_window);
			if (!context || !context->ui_context)
				break;
			context->ui_context->main_window_pos.x = 999999;

			LoadRandomHintMessage();

			break;

		case WM_IME_COMPOSITION:
			Log(LOG_ID, L"WM_IME_COMPOSITION, ui:%X", ui_window);
			break;

		case WM_IME_COMPOSITIONFULL:
		case WM_IME_REQUEST:
		case WM_IME_CHAR:
		case WM_IME_KEYDOWN:
		case WM_IME_KEYUP:
			break;

		case UIM_MODE_CHANGE:
			if (!context)
				break;

			if (wParam != 1 || lParam != 1)			//在这种情况下，才显示（DOS处理要求）
			{
				SetConversionStatus(hIMC, pIMC, context);
				GetConversionStatus(hIMC, pIMC, context);
				HideMainWindow(context, ui_context);
				ResetContext(context);
				ShowStatusWindow(context, ui_context);
			}
			else
				ResetContext(context);

			CheckHintState(context, ui_context);
			break;

		case UIM_INPUT_HZ:		//偏旁部首输入的汉字
			if (!context)
				break;
			{
				int hz = (int) wParam;
				LPCOMPOSITIONSTRING compose_string;

				if ((compose_string = (LPCOMPOSITIONSTRING)ImmLockIMCC(pIMC->hCompStr)) == 0)
					break;

				memcpy((char*)compose_string + compose_string->dwResultStrOffset, &hz, sizeof(TCHAR));
				compose_string->dwResultStrLen = 1;

				GenerateImeMessage(hIMC, WM_IME_COMPOSITION, 0, GCS_RESULTSTR);
				compose_string->dwCompStrLen = 0;
				ImmUnlockIMCC(pIMC->hCompStr);
			}
			break;

		case UIM_RECENT_CI:     //通过CTRL+↑和CTRL+↓快速录入最近输入的字词
			Log(LOG_ID, L"UIM_RECENT_CI, ui:%X", ui_window);

			if (!context)
				break;
			{
				TCHAR* input_string = (TCHAR*)wParam;
				LPCOMPOSITIONSTRING compose_string;

				if ((compose_string = (LPCOMPOSITIONSTRING)ImmLockIMCC(pIMC->hCompStr)) == 0)
					break;

				memcpy((char *)compose_string + compose_string->dwResultStrOffset, input_string, _tcslen(input_string) * sizeof(TCHAR));
				compose_string->dwResultStrLen = (int)_tcslen(input_string);
				GenerateImeMessage(hIMC, WM_IME_COMPOSITION, 0, GCS_RESULTSTR);
				compose_string->dwCompStrLen = 0;
				ImmUnlockIMCC(pIMC->hCompStr);
			}
			break;

		//不支持IME自动感知的程序，通过这组消息进行控制
		case UIM_NOTIFY:
			Log(LOG_ID, L"UIM消息");
			Ime_Notify(ui_window, wParam, lParam);
			break;

		//更新程序通知
		//case UIM_UPDATE_NOTIFY:
		//	if (wParam == UPDATE_NOTIFY_FINISHED)
		//		share_segment->pim_update_flag = PIM_UPDATE_FINISHED;
		//	else if (wParam == UPDATE_NOTIFY_WAIT)
		//	{
		//		share_segment->pim_update_flag		= PIM_UPDATE_WAITING;
		//		share_segment->next_pim_update_time = GetCurrentTicks() + (int)lParam;
		//	}
		//	else if (wParam == UPDATE_RESOLVE_FAILED)
		//		share_segment->pim_update_flag = PIM_UPDATE_ERROR;

		//	break;

		//切换英文输入法
		case UIM_ENGLISH_INPUT:
			if (!context)
				break;

			if (host_is_console)
			{
				ResetContext(context);
				SendMessage(pIMC->hWnd, WM_IME_ENDCOMPOSITION, 0, 0);
				SendMessage(pIMC->hWnd, WM_IME_NOTIFY, IMN_CLOSECANDIDATE, 1);
			}
			else
			{
				ProcessContext(context);
//				MakeCandidate(context);
				ShowMainWindow(context, ui_context);

				if (!context->input_length)
					context->state = STATE_START;

				if (context->english_state != ENGLISH_STATE_NONE)
					HideHintWindow(context, ui_context);
				else
					CheckHintState(context, context->ui_context);
			}

			break;

		case WM_MENUCHAR:
			if (HIWORD(wParam) == MF_POPUP)
			{
				PostMessage(ui_window, WM_CANCELMODE, 0, 0);
				break;
			}

		default:
			return DefWindowProc(ui_window, message, wParam, lParam);
	}

	UnlockContext(hIMC, pIMC);
	return 0;
}

/*	注册IME UI窗口类
 */
int RegisterUIWindowClass(HINSTANCE instance)
{
	WNDCLASSEX	ui_class;

	ui_class.cbSize			= sizeof(WNDCLASSEX);
	ui_class.style			= CS_IME | CS_VREDRAW | CS_HREDRAW;
    ui_class.cbClsExtra		= 0;
    ui_class.cbWndExtra		= sizeof(LONG_PTR) * 2;
    ui_class.hIcon			= 0;
    ui_class.hIconSm		= 0;
	ui_class.hInstance		= instance;
    ui_class.hCursor		= NULL;
    ui_class.hbrBackground	= NULL;
    ui_class.lpszMenuName	= NULL;
	ui_class.lpfnWndProc	= UIWindowProcedure;
	ui_class.lpszClassName	= ui_window_class_name;

	if (!RegisterClassEx(&ui_class))
		return 0;

	return 1;
}

/*	清除UI窗口类
 */
void UnregisterUIWindowClass(HINSTANCE instance)
{
	UnregisterClass(ui_window_class_name, instance);
}