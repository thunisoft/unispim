/*	工具头文件。
 *	装载log，file，以及内存管理的头文件。
 */

#ifndef	_UTILITY_H_
#define	_UTILITY_H_

#include <stdio.h>
#include <kernel.h>
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#define _SizeOf(x)		(sizeof((x)) / sizeof((x)[0]))
#define _IsNoneASCII(x)	((WORD)x >= 0x2B0)
#define _HanZiLen		1

//最多申请的共享内存数量
#define	MAX_SHARED_MEMORY_COUNT			1024			//最多的共享对象

//键盘定义
#define	KEY_LSHIFT				(1 << 0)
#define	KEY_RSHIFT				(1 << 1)
#define	KEY_SHIFT				(1 << 2)
#define	KEY_LCONTROL			(1 << 3)
#define	KEY_RCONTROL			(1 << 4)
#define	KEY_CONTROL				(1 << 5)
#define	KEY_LALT				(1 << 6)
#define	KEY_RALT				(1 << 7)
#define	KEY_ALT					(1 << 8)
#define	KEY_CAPITAL				(1 << 9)

//共享内存的信息
typedef struct tagSHAREDMEMORYINFO
{
	HANDLE	handle;
	void	*pointer;
} SHAREDMEMORYINFO;

#define	TYPE_USERAPP		0				//documents and setting/{username}/application data
#define	TYPE_ALLAPP			1				//documents and setting/all users/application data
#define	TYPE_PROGRAM		2				//program files/unispim6
#define	TYPE_TEMP			3				//临时文件目录

//LOG相关
//声明LOG记录的标识，由程序文件名字与函数名称组合而成。
#define WIDEN(x)    L ## x
#define WIDEN2(x)   WIDEN(x)
#define LOG_ID		WIDEN2(__FUNCTION__)

//#define	LOG_ID		__FUNCTION__

extern void	Log(const TCHAR *id, const TCHAR *format, ...);
extern int	LogInit(int restart);
extern int FreeLog();

//LOG相关结束

//文件处理相关
//从文件读入数据
extern int LoadFromFile(const TCHAR *file_name, void *buffer, int buffer_length);

//保存数据文件
extern int SaveToFile(const TCHAR *file_name, void *buffer, int buffer_length);

//获得文件长度
extern int GetFileLength(const TCHAR *file_name);

//文件处理结束

//获得当前系统tick
extern int GetCurrentTicks();

//复制部分字符串
extern void CopyPartString(TCHAR *target, const TCHAR *source, int length);

//输出汉字
extern void OutputHz(HZ hz);

//获得共享内存区域指针
extern void *GetSharedMemory(const TCHAR *shared_name);
extern void *GetReadOnlySharedMemory(const TCHAR *shared_name);

//创建共享内存区
extern void *AllocateSharedMemory(const TCHAR *shared_name, int length);

//释放共享内存区
extern void FreeSharedMemory(const TCHAR *shared_name, void *pointer);

//窗口移动开始
extern void DragStart(HWND window);

//窗口移动
extern void DragMove(HWND window);

//窗口移动结束
extern void DragEnd(HWND window);

//获得当前屏幕的坐标
extern RECT GetMonitorRectFromPoint(POINT point);

//将第一个矩形放入第二个矩形中
extern void MakeRectInRect(RECT *in_rect, RECT out_rect);

//转换VK
extern void TranslateKey(UINT virtual_key, UINT scan_code, CONST LPBYTE key_state, int *key_flag, TCHAR *ch, int no_virtual_key);

//判断文件是否存在
extern int FileExists(const TCHAR *file_name);

//获得当前program files目录
extern TCHAR *GetProgramDirectory(TCHAR *dir);

//获得当前用户的Application目录
extern TCHAR *GetUserAppDirectory(TCHAR *dir);

//获得所有用户的Application目录
extern TCHAR *GetAllUserAppDirectory(TCHAR *dir);

//组合目录与文件名
extern TCHAR *CombineDirAndFile(const TCHAR *dir, const TCHAR *file, TCHAR *result);

//获得文件的路径，App/allApp/program
TCHAR *GetFileFullName(int type, const TCHAR *file_name, TCHAR *result);

//Ansi字符串转换到UTF16
extern void AnsiToUtf16(const char *name, wchar_t *wname, int nSize);

extern void Utf16ToAnsi(const wchar_t *wchars, char *chars, int nSize);

extern void UCS32ToUCS16(const UC UC32Char, TCHAR *buffer);

//判断一个4字节TChar数组，是由几个汉字组成的。返回值：0，1，2
extern int UCS16Len(TCHAR *buffer);

//从ucs16转为ucs32，只转一个汉字，多于一个汉字，返回0。
extern UC UCS16ToUCS32(TCHAR *buffer);

//在文件中读取一行数据
extern int GetLineFromFile(FILE *fr, TCHAR *line, int length);

//在文件中读取一个字符串（没有分隔符）
extern int GetStringFromFile(FILE *file, TCHAR *string, int length);

//除掉字符串首尾的空白符号
extern void TrimString(TCHAR *line);

//使字符串符合缓冲区大小的限制
extern void MakeStringFitLength(TCHAR *string, int length);

//获得当前系统时间
extern void GetTimeValue(int *year, int *month, int *day, int *hour, int *minute, int *second, int *msecond);

//执行程序
extern void ExecuateProgram(const TCHAR *program_name, const TCHAR *args, const int is_url);
extern void ExecuateProgramWithArgs(const TCHAR *cmd_line);

//获得操作系统的版本号
extern int GetWindowVersion();

//显示系统信息窗口
extern void ShowWaitingMessage(const TCHAR *message, HINSTANCE instance, int min_time);

//对GZip文件进行解压缩
extern int UncompressFile(const TCHAR *name, const TCHAR *tag_name, int stop_length);
extern int CompressFile(const char *name, const char *tag_name);

//字符串/16进制字符串转换函数
extern int ArrayToHexString(const char *src, int src_length, char *tag, int tag_length);
extern int HexStringToArray(const char *src, char *tag, int tag_length);
extern int GetSign(const char *buffer, int buffer_length);

extern int IsFullScreen();

extern const TCHAR *GetProgramName();

extern void Lock();
extern void Unlock();

extern int PackStringToBuffer(TCHAR *str, int str_len, TCHAR *buffer, int buf_len);
extern int IsNumberString(TCHAR *candidate_str);
extern int LastCharIsAtChar(TCHAR *str);
extern char strMatch(char *src,char * pattern);

extern int IsNumpadKey(int virtual_key);

//判断是否为64位系统
extern BOOL IsIME64();

#ifdef __cplusplus
}
#endif

#endif