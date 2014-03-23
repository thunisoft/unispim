/*	输入法IME接口头文件
 */

#ifndef	_PIM_IME_H_
#define	_PIM_IME_H_

#include <windows.h>
#include <win32/immdev.h>
#include <context.h>

#ifdef __cplusplus
extern "C" {
#endif

#define	UIM_MODE_CHANGE				(WM_USER + 0x101)		//改变输入模式消息
#define	UIM_INPUT_HZ				(WM_USER + 0x102)		//输入汉字消息（偏旁部首检字）
#define	UIM_NOTIFY					(WM_USER + 0x103)		//内部窗口控制消息
//#define	UIM_UPDATE_NOTIFY			(WM_USER + 0x104)		//更新通知消息
#define	UIM_SEL_CANDIDATE			(WM_USER + 0x105)		//鼠标选择候选
#define	UIM_NEXT_PAGE				(WM_USER + 0x106)		//翻动到下一页
#define	UIM_PREV_PAGE				(WM_USER + 0x107)		//翻动到上一页
#define UIM_RECENT_CI				(WM_USER + 0x108)		//最近输入的词
#define UIM_ENGLISH_INPUT			(WM_USER + 0x109)		//切换英文输入法
#define UIM_STATUS_WINDOW_CHANGED	(WM_USER + 0x110)		//状态窗口位置改变

//#define	UPDATE_NOTIFY_FINISHED		0						//更新完成
//#define	UPDATE_NOTIFY_WAIT			1						//需要等待
//#define	UPDATE_RESOLVE_FAILED		2						//地址解析失败

#define	IMN_OPENSTATUSWINDOW_ONLY		0x101
#define	IMN_CLOSESTATUSWINDOW_ONLY		0x102

#define	UI_WINDOW_CLASS_NAME		TEXT("V6UNISPIM")
#define	UNISPIM_MUTEX				"UNISPIM6_MUTEX"
	
extern int last_key;

extern TCHAR ui_window_class_name[];
extern HINSTANCE global_instance;

//获得当前输入法上下文
extern PIMCONTEXT *LockContext(HIMC hIMC, LPINPUTCONTEXT *pIMC);
//释放当前输入法上下文
extern void UnlockContext(HIMC hIMC, LPINPUTCONTEXT pIMC);
//通过UI窗口句柄获得输入法上下文
extern HIMC GetIMC(HWND ui_window);

extern void SetContextToWindow(PIMCONTEXT *, HWND);

extern int  Ime_InitContext(HWND ui_window);
extern void Ime_DeleteContext(HWND ui_window);
extern void Ime_SetContext(HWND ui_window, WPARAM wParam, LPARAM lParam);
extern void Ime_Notify(HWND ui_window, WPARAM wParam, LPARAM lParam);
extern void Ime_Select(HWND ui_window, WPARAM wParam, LPARAM lParam);
extern void Ime_Control(HWND ui_window, WPARAM wParam, LPARAM lParam);

extern void RunConfigWordlib();
extern void RunConfigVersion();

//注册IME UI窗口类
extern int RegisterUIWindowClass(HINSTANCE instance);
//清除UI窗口类
extern void UnregisterUIWindowClass(HINSTANCE instance);

extern void SetConversionStatus(HIMC hIMC, LPINPUTCONTEXT pIMC, PIMCONTEXT *context);
extern void GetConversionStatus(HIMC hIMC, LPINPUTCONTEXT pIMC, PIMCONTEXT *context);

extern void GenerateImeMessage(HIMC hIMC, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern int SetIMCContext(LPINPUTCONTEXT pIMC);

#ifdef __cplusplus
}
#endif

#endif
