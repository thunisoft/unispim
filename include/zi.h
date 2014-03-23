/*	汉字处理头文件。
 */

#ifndef	_ZI_H_
#define	_ZI_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <kernel.h>
#include <context.h>

#define	MAX_ZI_FREQ			((1 << 23) - 1)						//最大字频
#define	BASE_ZI_FREQ		(8000000)							//字频基准

#define	ZICACHE_FILE_NAME	TEXT("unispim6\\zi\\hz_cache.dat")	//汉字Cache文件名称
#define	HZDATA_FILE_NAME	TEXT("unispim6\\zi\\hzpy.dat")		//汉字数据文件名称
#define HZBH_FILE_NAME		TEXT("unispim6\\zi\\hzbh.dat")		//汉字笔划数据文件名称
#define	TOPZI_FILE_NAME		TEXT("unispim6\\ini\\固顶字.ini")	//置顶字定义文件名称
#define	HZJF_FILE_NAME		TEXT("unispim6\\zi\\hzjf.dat")		//汉字简繁对照表数据文件名称
#define	ZICACHE_SIGNATURE	0x20000918							//字缓冲区的标志

#define _CanInLibrary(x)	((x >= 0x00080 && x <= 0x0D7FF) || (x >= 0x0E000 && x <= 0x0FFEF) || \
							 (x >= 0x20000 && x <= 0x2A6D6) || (x >= 0x2A700 && x <= 0x2B734) || \
							 (x >= 0x2F800 && x <= 0x2FA1D))

//#define IsHanZiInExtA(x)	(x >= 0x3400  && x <= 0x4DB5)
//#define IsHanZiInExtB(x)	(x >= 0x20000 && x <= 0x2A6D6)
//#define IsHanZiInExtC(x)	(x >= 0x2A700 && x <= 0x2B73F)

typedef struct tagHZCACHE
{
	int		signature;							//字缓冲区的标志ZICACHE_SIGNATURE（用于版本判断）
	int		max_id;								//当前最大的汉字cache标号(如果Cache未满时就是Cache中字的个数))
	int		cache[MAX_HZ_IN_PIM];				//ZiCache比较独特，和CiCache不同，文件里并不存储汉字，cache[i]的i是字库里HZITEM的hz_id，而cache[i]的值是该字加入Cache时的顺序号，详见AddHzToCache
	int		used_count[MAX_HZ_IN_PIM];			//用户汉字使用度表，和cache成员类似，used_count[i]的i是字库里HZITEM的hz_id，而used_count[i]的值是该字的使用度
}HZCACHE;

//置顶字定义表
typedef struct tagTOPZITABLE
{
	SYLLABLE	syllable;						//音节
	HZ			top_zi[MAX_TOPZI];				//置顶字个数
}TOPZIITEM;

//笔划信息项
typedef struct tagBHITEM
{
	UC		zi;									//汉字
	byte	count;								//一个字有几套笔顺方案
	byte	order;								//笔顺方案的序号，从1开始计数
	byte	custom;								//是否自定义笔顺：0表示规范写法，1表示兼容笔顺
	byte	length;								//笔画数目，即笔画字符([1-5])串的长度
	int  	bh;									//当前笔顺方案的字符串指针
	int     next;                               // 本汉字的其它笔顺方案数据地址
                                                // ppNext为空，表示本结构已经是本汉字的最后笔顺方案
												// 本结构的存储按照笔画数排序，而一个汉字的多种笔顺
												// 串长度可能不同，那么，在存储器中就不能保证它们是
												// 连续存放的，所以用这个指针形成链式结构
}BHITEM;

//bh file header struct
typedef struct headBHFile
{
	int itemcount;                              //item amount,汉字笔顺条目数(一个汉字可有多种笔顺方案)
	int charcount;                              //不同汉字的数目
	UC  minmcp;                                 // 本文件包含笔顺数据的汉字中，Unicode编码点的最小值
	UC  maxmcp;                                 // 本文件包含笔顺数据的汉字中，Unicode编码点的最大值
                                                // 按照编码点的索引区长度计算方法如下行所示
                                                // ( MaxUCP - MinUCP + 1 ) * SizeOf( TStrokeItem )
	int maxstrockes;                            // 最大笔画数，即最长笔顺串的长度
                                                // “笔画长度数据起始地址”长度的计算方法如下行所示
                                                // ( MaxStrokes + 1 ) * SizeOf( PStrokeItem )
	int createdate;                             //创建日期(暂为0)
	int modifydate;                             //修改日期(暂为0)
	int checksum;                               //校验和(暂为0)
	int index1[1];                              //index1 = maxstrockes
	//int index2[1];							//index2 = maxmcp - minmcp + 1
	//BHITEM data[1];                           //数据   = itemcount
}BHDATA;

//简繁对照项
typedef struct tagJFITEM
{
	UC		JianTi;								//简体汉字
	UC		FanTi;								//繁体汉字
}JFITEM;

//汉字信息表
extern HZDATAHEADER	*hz_data;

//判断是否为汉字
extern int IsAllCanInLibrary(HZ *hz, int length);

//处理汉字被选择后进行Cache以及字频的更新
extern void ProcessZiSelected(HZITEM *item);

//处理以词定字方式选择汉字之后的更新
extern void ProcessZiSelectedByWord(HZ hz, SYLLABLE syllable);

//获得汉字在Cache中的使用标识（越大越近使用）
extern int GetZiCachePos(HZITEM *item);

//处理音节到汉字候选的一系列工作
extern int ProcessZiCandidates(SYLLABLE syllable, CANDIDATE *candidate_array, int array_length, int zi_level);

//在汉字候选中排重
extern int UnifyZiCandidates(CANDIDATE *candidate_array, int count);

//在小音节词后选中排重
extern int UnifySmallCiCandidates(CANDIDATE *candidate_array, int count);

//对汉字候选进行排序
extern void SortZiCandidates(CANDIDATE *candidate_array, int count);

//基于汉字的音节与字检索汉字全集中的汉字项
extern HZITEM *GetHzItemBySyllable(SYLLABLE syllable, HZ hz);

//获得汉字候选。
extern int GetZiCandidates(SYLLABLE syllable, CANDIDATE *candidate_array, int array_length, int fuzzy_mode, int set_mode, int output_mode);

//判断汉字是否包含特定的音调。
extern int ZiContainTone(HZ hz, SYLLABLE syllable, int tone);

//装载、释放置顶字文件
extern int LoadTopZiData(const TCHAR *name);
extern int FreeTopZiData();

//装载、保存、释放汉字Cache数据
extern int LoadHZData(const TCHAR *hz_data_name);
extern int FreeHZData();
extern int LoadZiCacheData(const TCHAR *zi_cache_name);
extern int FreeZiCacheData(const TCHAR *zi_cache_name);
extern int SaveZiCacheData(const TCHAR *zi_cache_name);

//判断是否为BG汉字
extern int IsGB(HZ hz);
extern int AdjustWordFreqToHZFreq(int ci_freq);
extern int ConvertToRealHZFreq(int zi_freq);
extern int ConvertToRealCIFreq(int ci_freq);

//获得汉字笔划拼音串
extern void GetZiBHPinyin(UC zi, TCHAR *buffer, int length);
//获得笔划的候选显示
extern void GetBHDisplayString(CANDIDATE *candidate, TCHAR *buffer, int length);

//获得笔划输入的候选
extern int GetBHCandidates(const TCHAR *input_string, CANDIDATE *candidates, int array_length);
//模糊模式
//extern int GetBHCandidatesFuzzy(const char *input_string, CANDIDATE *candidates, int array_length);

//设定为固顶字
extern void SetFixTopZi(PIMCONTEXT *context, int index);

//装载、释放笔划数据文件
extern int LoadBHData(const TCHAR *file_name);
extern int FreeBHData();

extern HZITEM* GetSingleZiCandidate(TCHAR zi);

#ifdef __cplusplus
}
#endif

#endif