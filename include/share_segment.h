#ifndef	_SHARE_SEGMENT_H_
#define	_SHARE_SEGMENT_H_

#include <windows.h>
#include <config.h>
#include <ci.h>
#include <kernel.h>
#include <win32/main_window.h>
#include <spw.h>
#include <symbol.h>
#include <wordlib.h>
#include <zi.h>

#ifdef __cplusplus
extern "C" {
#endif

static TCHAR *segment_share_name = TEXT("HYPIM_SEGMENT_SHARED_NAME");

typedef struct tagSHARE_SEGMENT
{
	int	process_count;												//与IME连接的进程计数
	//int	first_use; 													//第一次使用IME

	int	is_default_ime;												//是否为默认输入法
	int global_config_update_time;									//配置装载时间
	int config_loaded;												//是否已经装载Config

	PIMCONFIG all_config;											//系统正在使用的Config

	int q1_index;													//单引号
	int q2_index;													//双引号
	int symbol_loaded;												//是否已经装载符号表
	SYMBOLITEM symbol_table[SYMBOL_NUMBER];							//符号表，默认配置

	int user_wordlib_id;											//用户词库标识
	int user_wl_modified;											//是否改变
	int can_save_user_wordlib;										//是否可以保存用户词库（向用户词库Import词汇可能被覆盖）
	TCHAR wordlib_name[MAX_WORDLIBS * 2][WORDLIB_FILE_NAME_LENGTH];	//词库的名称
	int	wordlib_length[MAX_WORDLIBS * 2];							//词库长度数组
	int wordlib_deleted[MAX_WORDLIBS * 2];							//词库是否已经被删除
	TCHAR wordlib_shared_name[MAX_WORDLIBS * 2][0x20];				//共享内存的标识

	int	bh_loaded;													//笔划是否已经在内存中

	CICACHE	ci_cache;												//词cache
	int	ci_cache_loaded;											//是否已经装入
	int	ci_cache_modified;											//是否改变

	//NEWCI new_ci;													//新词表
	//int	new_ci_loaded;												//是否装入
	//int	new_ci_modified;											//是否已经修改

	int english_loaded;												//英文词典是否已经在内存中
	int engtrans_loaded;											//英文翻译是否已经在内存中

	TCHAR szRecentResult[MAX_RECENT_LENGTH][MAX_WORD_LENGTH + 1];	//最近输入的词
	int nCurRecent;													//数组中最后一项的下标

	int	jf_loaded;													//简繁对照表是否已经在内存中
	int	jf_count;													//简繁对照项数目

	unsigned __int64 main_window_list[MAX_STATUS_LIST_SIZE];		//写作栏句柄集合
	unsigned __int64 status_window_list[MAX_STATUS_LIST_SIZE];		//状态栏句柄集合
	unsigned __int64 hint_window_list[MAX_STATUS_LIST_SIZE];		//提示栏句柄集合

	int	resource_loaded;											//资源尚未装载

	int	spw_index[SPW_MAX_ITEMS];									//短语索引表
	int	spw_count;													//短语数目
	int	spw_length;													//缓冲区数据长度
	int	spw_loaded;													//短语是否已经在内存中

	SYLLABLEMAP syllable_map[460];									//拼音－音节转换表
	int syllable_map_items;
	SYLLABLEV5MAP syllablev5_map[451];								//拼音－音节转换表(V5)
	int syllablev5_map_items;
	int sp_used_u;													//双拼是否使用u作为开头的字母
	int sp_used_v;													//双拼是否使用V作为开头的字母
	int sp_used_i;													//双拼是否使用I作为开头的字母
	int sp_loaded;													//双拼数据是否已加载

	TCHAR con_sp_string_save[26][8];								//双拼声母表
	TCHAR con_sp_string[26][8];
	TCHAR vow_sp_string_save[46][8];								//双拼韵母表
	TCHAR vow_sp_string[46][8];
	TCHAR vow_sp_string_single_save[94][8];							//双拼音节表
	TCHAR vow_sp_string_single[94][8];

	//int pim_update_flag;											//词库是否已经更新完毕
	//int next_pim_update_time;										//下一次的词库更新的时间

	//int url_loaded;													//url是否已经在内存中

	HZCACHE	hz_cache;												//汉字Cache结构
	int	zi_cache_loaded;											//是否已经装载了字Cache
	int	zi_cache_modified;											//是否改变
	TOPZIITEM topzi_table[MAX_SYLLABLES];							//置顶字表
	int	topzi_table_items;											//置顶字表项数目
	int	topzi_loaded;												//是否已经装入到内存
	int hz_data_loaded;												//汉字信息表是否已经装入

	int fontmap_loaded;                                             //font map loaded?
	int gbkmap_loaded;                                              //gbk map genernate?
}SHARE_SEGMENT;

extern SHARE_SEGMENT *share_segment;

extern int LoadSharedSegment();
extern int FreeSharedSegment();

#ifdef __cplusplus
}
#endif

#endif