/* 词库工具头文件
 */

#ifndef	_WORDLIB_H_
#define	_WORDLIB_H_

#include <kernel.h>
#include <syllable.h>

#ifdef __cplusplus
extern "C" {
#endif

//extern int user_wl_modified;
//extern int can_save_user_wordlib;

#define	WORDLIB_MAX_FREQ				((1 << 19) - 1)				//词库中的最大词频
#define	BASE_CI_FREQ					(500000)					//词频基准
#define	USER_BASE_FREQ					100000						//用户自定词的默认词频

//词库相关定义
#define	DEFAULT_USER_WORDLIB_NAME	TEXT("用户词库")
#define	DEFAULT_USER_WORDLIB_AUTHOR	TEXT("华宇拼音输入法")
#define	J2F_FILE_NAME				TEXT("\\unispim6\\zi\\j2f.dat")		//简体繁体转换数据文件名称
#define	WORDLIB_NAME_PREFIX			TEXT("\\unispim6\\wordlib\\")		//词库文件的前缀
#define	WORDLIB_USER_SHORT_NAME		TEXT("user.uwl")					//用户词库文件的短名称
#define WORDLIB_SYS_FILE_NAME		TEXT("unispim6\\wordlib\\sys.uwl")	//系统词库文件名
#define WORDLIB_V3V5_FILE_NAME		TEXT("unispim6\\wordlib\\v3v5.uwl") //V3V5导入词库文件名
#define WORDLIB_V3V5_NAME			TEXT("V3V5词库")					//V3V5导入词库名称

#define MAX_WORDLIBS					32							//内存中最多的词库数目
#define	WORDLIB_PAGE_SIZE				1024						//词库页大小
#define	WORDLIB_NAME_LENGTH				16							//词库名称长度
#define	WORDLIB_AUTHOR_LENGTH			16							//词库作者名称
#define	PAGE_END						-1							//结束页
#define	WORDLIB_EXTRA_LENGTH			0x100000					//用户词库的扩充空间
#define	WORDLIB_PAGE_DATA_LENGTH		(WORDLIB_PAGE_SIZE - 4 * sizeof(int))	//页中可用数据长度
#define	WORDLIB_FILE_NAME_LENGTH		256							//词库文件名称的最大长度
#define	WORDLIB_FEATURE_LENGTH			sizeof(int)					//词项中的特性描述长度，目前用一个整数
#define	WORDLIB_CREATE_EXTRA_LENGTH		0x100000					//创建新词库时，增加额外的1M数据。
#define	WORDLIB_NORMAL_EXTRA_LENGTH		0x40000						//普通增加容量便于添加词汇

//词库头定义
typedef struct tagWORDLIBHEADER
{
	int			signature;							//词库的签名
	TCHAR		name[WORDLIB_NAME_LENGTH];			//词库的名称
	TCHAR		author_name[WORDLIB_AUTHOR_LENGTH];	//词库作者名称
	int			word_count;							//词汇数目
	int			page_count;							//已分配的页数
	int			can_be_edit;						//是否可以编辑
	int			pim_version;						//输入法版本号（兼容性考虑）
	int			index[CON_NUMBER][CON_NUMBER];		//索引表
} WORDLIBHEADER;

//页定义
typedef	struct tagPAGE
{
	int			page_no;							//页号
	int			next_page_no;						//下一个页号，-1标识结束
	int			length_flag;						//本页包含的词汇长度的标志
	int			data_length;						//已经使用的数据长度
	char		data[WORDLIB_PAGE_DATA_LENGTH];		//数据开始
} PAGE;

//词库定义
typedef	struct tagWORDLIB
{	
	union
	{
		//词库文件头数据
		WORDLIBHEADER	header;	

		//用于对齐页边界
		PAGE			header_data[sizeof(WORDLIBHEADER) / WORDLIB_PAGE_SIZE + 1];
	};
	PAGE				pages[1];					//页数据
} WORDLIB;

//V5词库定义
typedef struct tagWORDLIBV5
{
	int		magic_number;
	int		index_table[8][18][18];
	int		win_time;
	int		free_address;
}WORDLIBV5;

#define	WLUP_OPER_ADD		1				//增加词条
#define	WLUP_OPER_DEL		2				//删除词条
#define	WLUP_OPER_UPDATE	3				//更新词条

typedef struct tagUPDATEITEM 
{
	const TCHAR *ci_string;					//词字符串
	const TCHAR *py_string;					//拼音字符串
	int freq;								//词频
	int operation;							//操作：OPER_ADD (1), OPER_DELETE(2), OPER_UPDATE(3),
}UPDATEITEM;

#define	ENCODERV5		0xfdef				//V5汉字的加密数字

#define	WLF_CONTINUE_ON_ERROR		(1 << 0)
#define	WLF_HALT_ON_ERROR			(1 << 1)

#define	WLF_CLEAR_WORDLIB			(1 << 2)
#define	WLF_DELETE_WORDLIB			(1 << 3)

#define	WLE_CI						(-1)
#define	WLE_YIN						(-2)
#define	WLE_FREQ					(-3)
#define	WLE_NAME					(-4)
#define	WLE_ITEMS					(-5)
#define	WLE_NOCI					(-6)
#define	WLE_OTHER					(-10)

//装载词库文件。
extern int LoadWordLibrary(const TCHAR *wordlib_name);

//保存词库文件
extern int SaveWordLibrary(int wordlib_id);

//获得词库的指针
extern WORDLIB *GetWordLibrary(int wordlib_id);

//建新页面
extern int NewWordLibPage(int wordlib_id);

//向词库中增加词项
extern int AddCiToWordLibrary(int wordlib_id, HZ *hz, int hz_length, SYLLABLE *syllable, int syllable_length, int freq);

//确定词是否在词库中
extern WORDLIBITEM *GetCiInWordLibrary(int wordlib_id, HZ *hz, int hz_length, SYLLABLE *syllable, int syllable_length);

//在词库中删除词汇。
extern int DeleteCiFromWordLib(int wordlib_id, HZ *hz, int hz_length, SYLLABLE *syllable, int syllable_length, int clear_syllable);

//装载词库文件，附带额外的数据长度（一般用于创建词库文件或者装载用户词库文件）
extern int LoadWordLibraryWithExtraLength(const TCHAR *lib_name, int extra_length, int check_exist);

//装载用户词库文件
extern int LoadUserWordLibrary(const TCHAR *wordlib_name);

//基于词条数据文件，创建词库。
extern int CreateWordLibraryWithData(const char *wordlib_file_name, const char *data_file_name, const char *name, const char *author, int can_be_edit);

//释放词库数据。
extern void CloseWordLibrary(int wordlib_id);

//释放所有的词库数据。
extern void CloseAllWordLibrary();

//创建空的词库文件
extern int CreateEmptyWordLibFile(const TCHAR *wordlib_file_name, const TCHAR *name, const TCHAR *author, int can_be_edit);

//获得下一个词库标识，用于词库的遍历。
int GetNextWordLibId(int cur_id);
const TCHAR *GetWordLibFileName(int wl_id);

//获得用户词库标识
int GetUserWordLibId();

//词库维护函数组
int __stdcall UpdateWordLibrary(const TCHAR *wl_name, UPDATEITEM *update_items, int item_count, int flag);
int __stdcall CheckAndUpdateWordLibrary(const TCHAR *wl_name);
int CheckAndUpdateWordLibraryFast(const TCHAR *wl_name);
int ExportWordLibrary(const TCHAR *wordlib_file_name, const TCHAR *text_file_name, int *ci_count, TCHAR *err_file_name, int export_all, void *call_back);
int ImportWordLibrary(const TCHAR *wordlib_file_name, const TCHAR *text_file_name, int *ok_count, int *err_count, TCHAR *err_file_name, void *call_back);
int CreateWordLibrary(const TCHAR *wordlib_file_name, const TCHAR *text_file_name, int *ok_count, int *err_count, TCHAR *err_file_name, void *call_back);
int CheckAndUpgrandWordLibFromV5BKToV6(const TCHAR *wl_name);
int UpgradeWordLibFromV5ToV6(const TCHAR *wl_file_name);
int LoadAllWordLibraries();
void MaintainWordLibPointer();
int GetWordLibraryLoaded(const TCHAR *lib_name);

//Word J2F
void WordJ2F(TCHAR *zi_word);

//删词
int __stdcall DeleteCiFromAllWordLib(TCHAR *ci_str, int ci_length, TCHAR *py_str, int py_length);

#ifdef __cplusplus
}
#endif

#endif