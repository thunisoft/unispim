/*	输入法核心模块。完成输入法最核心的功能。
 *
 *	1。写作串生成
 *	2。候选生成包括用户的命令生成
 *
 *	Kernel是一个无状态的函数组，只对完整的输入串进行处理，不会记录上次
 *	按键的状态。
 *	Kernel的任务就是基于当前的输入，产生写作串以及候选，其他的操作由
 *	IME模块完成，如：上屏，进行输入选择等。
 */

#ifndef	_KERNEL_H_
#define	_KERNEL_H_

//汉字类型
typedef	unsigned short	HZ;					//标准汉字编码
typedef unsigned int	UC;					//Unicode编码
typedef unsigned short	BIG5;				//台湾汉字编码

#include <platform.h>
#include <syllable.h>
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int	window_logon;

#define		TORGB(x)		((((x) & 0xff0000) >> 16) | (((x) & 0xff) << 16) | ((x) & 0x00ff00))

#define		WORDLIB_WRONG				0							//词库非华宇词库
#define		WORDLIB_V3					3							//词库为V3词库
#define		WORDLIB_V5					5							//词库为V3、V5词库
#define		WORDLIB_V6					6							//词库为V6词库
#define		WORDLIB_V6B1				7							//词库为V6B1词库
#define		WORDLIB_V3_BKUP				8							//V3的备份词库文件
#define		WORDLIB_V5_BKUP				9							//V5的备份词库文件
#define		WORDLIB_V6B2				10							//词库为V6B2词库
#define		WORDLIB_V66					66							//词库为V6.6词库

#define		HYPIM_VERSION					6						//输入法大版本号
#define		HYPIM_WORDLIB_V66_SIGNATURE		0x14091994				//输入法词库V66标志
#define		HYPIM_WORDLIB_V6_SIGNATURE		0x14081994				//输入法词库V6标志
#define		HYPIM_WORDLIB_V6B2_SIGNATURE	0x14071994				//输入法词库V6B2标志
#define		HYPIM_WORDLIB_V6B1_SIGNATURE	0x14061994				//输入法词库V6B1标志
#define		HYPIM_WORDLIB_V5_SIGNATURE		0x19990604				//输入法词库V5标志
#define		HYPIM_WORDLIB_V3_SIGNATURE		0x19990604				//输入法词库V3标志
#define		HYPIM_SHARED_SEGMENT			".upim_shm"				//全局共享段的名称

//候选类型定义
#define		CAND_TYPE_ICW			1								//智能组词候选
#define		CAND_TYPE_SPW			2								//自定义词候选
#define		CAND_TYPE_CI			3								//词候选
#define		CAND_TYPE_ZI			4								//字候选
#define		CAND_TYPE_ZFW			5								//以词定字候选
//#define     CAND_TYPE_URL           6           					//url candidate

#define     ZI_TYPE_NORMAL          1                               //正常的字输入(非小音节拆分)
#define     ZI_TYPE_OTHER           2                               //小音节拆分的字，如xian中的xi

#define		CI_TYPE_NORMAL			1								//正常的词汇输入(逆向解析输入)
#define		CI_TYPE_LETTER			2								//单字母输入(超级简拼)
#define		CI_TYPE_OTHER			3								//正向解析输入
#define		CI_TYPE_WILDCARD		4								//词语联想输入

//不同类别的候选的长度定义（默认以汉字为单位）
#define		MAX_HZ_IN_PIM			65536							//本输入法中的汉字数目最大值
#define		MAX_TOPZI				8								//最多置顶字个数
#define		MAX_PINYIN_LENGTH		8								//最长的拼音串长度
#define		MAX_CANDIDATES			3000							//最大的候选项个数
#define		MAX_ICW_CANDIDATES		10								//最大的智能组词个数
#define		MAX_CANDIDATES_PER_LINE	9								//一行最大候选数目
#define		MAX_CANDIDATE_LINES		5								//最大候选行数

#define		MAX_CANDIDATE_STRING_LENGTH	32							//最大候选字符串的长度（用于显示）
#define		MAX_TRANSLATE_STRING_LENGTH	200							//最大英文翻译字符串的长度

#define		MAX_SYLLABLE_PER_INPUT	32								//一次能够输入的音节数目
#define		MAX_INPUT_LENGTH		64								//最大用户输入字符长度
#define		MAX_COMPOSE_LENGTH		128								//最大写作串长度
#define		MAX_RESULT_LENGTH		2048							//最大结果串长度
#define		MAX_WORD_LENGTH			32								//词的最大汉字数目
#define		MAX_ICW_LENGTH			16								//智能组词的最大汉字数目
#define		MAX_SPW_LENGTH			2000							//特殊词汇的最大长度（以Char为单位）
#define		MAX_SPW_HINT_STRING		64								//SPW提示字符串长度
#define		MAX_RECENT_LENGTH		16								//最近输入的词数
#define     MAX_OTHER_PARSE_SYLLABLE  32                            //最大正向解析音节数(用于GetCandidates::GetOtherSyllableParse)
#define     MAX_SMALL_SYLLABLES       4                             //一个拼音串最多可解析的小音节(由字的音节拆分成的音节，如xian->xi'an)数
                                                                    //其最大值不应超过MAX_SYLLABLE_PER_INPUT/2(否则拆分后的音节数溢出)

//汉字使用度标识
#define		HZ_ALL_USED				(HZ_MOST_USED | HZ_MORE_USED | HZ_LESS_USED)	//全集汉字
#define		HZ_MOST_USED			(1 << 0)										//常用字
#define		HZ_MORE_USED			(1 << 1)										//次常用字
#define		HZ_LESS_USED			(1 << 2)										//非常用字

//汉字Cache设置
#define		HZ_CACHE_MAX_ID			0x70000000	//最大的汉字cache标志。为避免出现符号错误，不设定为7fffffff

#pragma		pack(1)

//汉字项类型
typedef struct tagHZITEM
{
	UC				hz;							//字(UNICODE)
	unsigned short	hz_id;						//汉字条目的ID
	SYLLABLE		syllable;					//音节
	int				freq;						//字频
	int				simplified : 1;				//简体
	int				traditional : 1;			//繁体
	int				other : 1;					//非繁体/简体的字（一般为日本/韩文汉字）
	int				effective : 1;				//有效(支持将来条目变化)
	int				show_syllable : 1;			//难字(在候选栏中提示拼音)
	int				other_data : 26;			//其他数据(暂时空白)
	int				icw_hz : 1;					//ICW用(单字词)
}HZITEM;

//汉字信息文件结构(hzpy.dat)
typedef struct tagHZDATAHEADER
{
	unsigned int	signature;					//标识
	unsigned int	create_date;				//创建日期
	unsigned int	modify_date;				//修改日期
	unsigned int	check_sum;					//校验和
	int				hz_count;					//汉字数目

	HZITEM			hz_item[1];						//汉字项
//	char			reserved[(MAX_HZ_IN_PIM - 1) * sizeof(HZITEM) - sizeof(int) * 5];	//保留区
}HZDATAHEADER;

//词项类型
typedef struct tagWORDITEM
{
	unsigned int	effective : 1;				//有效（没有被删除）
	unsigned int	ci_length : 6;				//词长度
	unsigned int	syllable_length : 6;		//音节长度
	unsigned int	freq : 19;					//词频（Q: 词频和使用度有什么区别？A: 词频是词库里的，使用度是CiCache里的，词频和使用度的计算方式都是使用1次累加1
												//(见AddCi和InsertCiToCache)，其实准确地说，在装好输入法开始使用以后，使用度才是使用1次加1，而词库中的次品是不变的，
												//(用户词库除外)，最初的词库中的词频信息是利用原始语料库中的词频换算得到的(通常不是0，使用度的初始值才是0)，换算公
												//式见AdjustWordFreqToHZFreq))

	//后面两项为变长结构，与length相关
	SYLLABLE	syllable[1];					//音节, C++要求数组必须有size
//	HZ			hz[length];						//汉字
}WORDLIBITEM;

//词项类型
typedef struct tagWORDITEMV6B2
{
	unsigned int	effective : 1;				//有效（没有被删除）
	unsigned int	sys_word : 1;				//系统词条
	unsigned int	ci_length : 5;				//词长度
	unsigned int	syllable_length : 5;		//音节长度
	unsigned int	freq : 20;					//词频
	//后面两项为变长结构，与length相关
	SYLLABLE	syllable[1];					//音节, C++要求数组必须有size
//	HZ			hz[length];						//汉字
}WORDLIBITEMV6B2;

//词项类型
typedef struct tagWORDITEMV6B1
{
	unsigned int	effective : 1;				//有效（没有被删除）
	unsigned int	sys_word : 1;				//系统词汇
	unsigned int	length : 7;					//词汇长度
	unsigned int	freq : 20;					//词频
	//后面两项为变长结构，与length相关
	SYLLABLE	syllable[1];					//音节, C++要求数组必须有size
//	HZ			hz[length];						//汉字
}WORDLIBITEMV6B1;

//字候选类型
typedef struct tagHZCANDIDATE
{
	WORDLIBITEM	*word_item;					//词项指针
	HZITEM		*item;						//在字表中的汉字项
	int         hz_type;                    //候选字类型
	int			top_pos;					//固顶字标识，0：非固顶字，1-7为固顶字位置。
	int			cache_pos;					//在CACHE中的位置，用于判断最新输入的汉字
	int			is_word;					//是否为词(如：西安)
	SYLLABLE    origin_syllable;            //小音节词对应的原始音节，如xi'an的原始音节是xian
}HZCANDIDATE;

//词候选类型
typedef struct tagWORDCANDIDATE
{
	WORDLIBITEM		*item;					//词库中词条指针
	int				type;					//类型
	int				source;					//词汇来源词库标识
	int				cache_pos;				//本词条在cache中的位置
	SYLLABLE		*syllable;				//指向词库词条的指针
	HZ				*hz;					//指向词条汉字的指针
	int				origin_syllable_length;	//原始音节长度(用于双向解析造成的音节长度不符合)
}WORDCANDIDATE;

//以词定字候选
typedef struct tagZIFROMWORDCANDIDATE
{
	WORDCANDIDATE	*word;					//原来的词
	SYLLABLE		syllable;				//音节
	HZ				hz;						//汉字
}ZFWCANDIDATE;


//智能组词项类型
typedef struct tagICWCANDIDATE
{
	char		length;						//ICW的长度（以汉字为单位）
	SYLLABLE	syllable[MAX_ICW_LENGTH];	//ICW的音节数组，用于以词定字的字频更新以及保存词到词库。
	HZ			hz[MAX_ICW_LENGTH + 1];		//ICW的汉字内容
}ICWCANDIDATE;

//自定义词项类型
typedef struct tagSPWCANDIDATE
{
	int			length;						//长度
	const void	*string;					//特殊词指针
	const void	*hint;						//提示信息
	int			type;						//短语的类型
	UC			hz;							//笔划使用的汉字
}SPWCANDIDATE;

//url candidate
//typedef struct tagURLCANDIDATE
//{
//	int			length;						//长度
//	const void	*string;					//特殊词指针
//	const void	*hint;						//提示信息
//}URLCANDIDATE;

//候选项结构
typedef struct tagCANDIDATE
{
	char		type;						//候选类别
	union
	{
		ICWCANDIDATE		icw;			//智能组词候选
		SPWCANDIDATE		spw;			//自定义词候选
		WORDCANDIDATE		word;			//词候选
		HZCANDIDATE			hz;				//汉字候选
		ZFWCANDIDATE		zfw;			//以词定字候选
		//URLCANDIDATE        url;            //url candidate
	};
}CANDIDATE;

#pragma	pack()

struct tagPIMCONTEXT;

//获取候选
extern int GetCandidates(struct tagPIMCONTEXT *context, const TCHAR *input_string, SYLLABLE *syllables, int syllable_count, CANDIDATE *candidate_array, int array_length, int need_spw);

#ifdef __cplusplus
}
#endif

#endif
