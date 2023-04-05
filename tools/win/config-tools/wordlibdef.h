#ifndef WORDLIBDEF_H
#define WORDLIBDEF_H

#include <tchar.h>

///////////////////////////////音节相关///////////////////////////////////////

#define		MAX_SYLLABLE_PER_INPUT	32								//一次能够输入的音节数目
//音调字符
#define	TONE_CHAR_1			'!'				//一声
#define	TONE_CHAR_2			'@'				//二声
#define	TONE_CHAR_3			'#'				//三声
#define	TONE_CHAR_4			'$'				//四声
#define	TONE_CHAR_CHANGE	'~'				//变换音调
#define	IS_TONE(ch)						((ch) == TONE_CHAR_1 || (ch) == TONE_CHAR_2 || (ch) == TONE_CHAR_3 || (ch) == TONE_CHAR_4)

#define	TONE_0				0
#define	TONE_1				(1 << 0)
#define	TONE_2				(1 << 1)
#define	TONE_3				(1 << 2)
#define	TONE_4				(1 << 3)

//判断音节是否以HU开头
#define	IS_START_WITH_HU(syllable)		(syllable.con == CON_H && syllable.vow >= VOW_U && syllable.vow <= VOW_UO)

//音节类型
typedef struct tagSYLLABLE
{
    unsigned short	con : 5;				//声母标识
    unsigned short	vow : 6;				//韵母标识
    unsigned short	tone : 5;				//音调0-5，按位设置音调。1-4位分别标识四个音调，5标识轻声
} SYLLABLE;

///////////////////////////////词库相关///////////////////////////////////////
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

//候选类型定义
#define		CAND_TYPE_ICW			1								//智能组词候选
#define		CAND_TYPE_SPW			2								//自定义词候选
#define		CAND_TYPE_CI			3								//词候选
#define		CAND_TYPE_ZI			4								//字候选
#define		CAND_TYPE_ZFW			5								//以词定字候选


#define	WORDLIB_MAX_FREQ				((1 << 19) - 1)				//词库中的最大词频
#define	BASE_CI_FREQ					(500000)					//词频基准
#define	USER_BASE_FREQ					100000						//用户自定词的默认词频

//词库相关定义
#define	DEFAULT_USER_WORDLIB_NAME	TEXT("用户词库")
#define	DEFAULT_USER_WORDLIB_AUTHOR	TEXT("华宇拼音输入法")
#define	WORDLIB_USER_SHORT_NAME		TEXT("user.uwl")					//用户词库文件的短名称
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
//#define	WORDLIB_CREATE_EXTRA_LENGTH		0x1000000					//创建新词库时，增加额外的10M数据。
#define	WORDLIB_NORMAL_EXTRA_LENGTH		0x40000						//普通增加容量便于添加词汇

#define		MAX_WORD_LENGTH			32								//词的最大汉字数目
#define _CanInLibrary(x)	((x >= 0x00080 && x <= 0x0D7FF) || (x >= 0x0E000 && x <= 0x0FFEF) || \
                             (x >= 0x20000 && x <= 0x2A6D6) || (x >= 0x2A700 && x <= 0x2B734) || \
                             (x >= 0x2F800 && x <= 0x2FA1D))

//汉字类型
typedef	unsigned short	HZ;					//标准汉字编码
typedef unsigned int	UC;					//Unicode编码
typedef unsigned short	BIG5;				//台湾汉字编码

//声母定义
#define	CON_NUMBER			24			//合法声母的数目

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

//词库指针数组，等于0的时候，为该ID没有被分配。
static WORDLIB *wordlib_buffer[MAX_WORDLIBS * 2] = { 0, };

#define		MAX_ICW_LENGTH			16								//智能组词的最大汉字数目

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

    };
}CANDIDATE;

#define		MAX_CANDIDATES			3000							//最大的候选项个数

#define		HYPIM_VERSION					6						//输入法大版本号
#define		HYPIM_WORDLIB_V66_SIGNATURE		0x14091994				//输入法词库V66标志
#define		HYPIM_WORDLIB_V6_SIGNATURE		0x14081994				//输入法词库V6标志
#define		HYPIM_WORDLIB_V6B2_SIGNATURE	0x14071994				//输入法词库V6B2标志
#define		HYPIM_WORDLIB_V6B1_SIGNATURE	0x14061994				//输入法词库V6B1标志
#define		HYPIM_WORDLIB_V5_SIGNATURE		0x19990604				//输入法词库V5标志
#define		HYPIM_WORDLIB_V3_SIGNATURE		0x19990604				//输入法词库V3标志
#define		HYPIM_SHARED_SEGMENT			".upim_shm"				//全局共享段的名称

#define		WORDLIB_WRONG				0							//词库非华宇词库
#define		WORDLIB_V3					3							//词库为V3词库
#define		WORDLIB_V5					5							//词库为V3、V5词库
#define		WORDLIB_V6					6							//词库为V6词库
#define		WORDLIB_V6B1				7							//词库为V6B1词库
#define		WORDLIB_V3_BKUP				8							//V3的备份词库文件
#define		WORDLIB_V5_BKUP				9							//V5的备份词库文件
#define		WORDLIB_V6B2				10							//词库为V6B2词库
#define		WORDLIB_V66					66							//词库为V6.6词库

#define _SizeOf(x)		(sizeof((x)) / sizeof((x)[0]))
#define _IsNoneASCII(x)	((WORD)x >= 0x2B0)
#define _HanZiLen		1

#define	CON_NULL			0			//没有声母
#define	CON_END				24

#define	PAGE_END						-1							//结束页

#define	DEFAULT_USER_WORDLIB_NAME	TEXT("用户词库")
#define	DEFAULT_USER_WORDLIB_AUTHOR	TEXT("华宇拼音输入法")

//词库维护外部接口
#define	ERR_FILE_NAME		TEXT("err.log")
#define	DEFAULT_FREQ		0

typedef	void (_stdcall *progress_indicator) (int total_number, int current_number);


//拼音方式
#define	PINYIN_QUANPIN				0				//全拼
#define	PINYIN_SHUANGPIN			1				//双拼

#define MAX_SP_FUZZY_SYLLABLE_COUNT    4

//切分音符号
#define	SYLLABLE_SEPARATOR_CHAR		'\''			//音节分隔符号（用户输入）
#define	SYLLABLE_SEPARAOTR_CHAR		9				//音节分隔符号（系统添加）

typedef struct tagSPFUZZYMAPITEM
{
    SYLLABLE sp_fuzzy_syllables[MAX_SP_FUZZY_SYLLABLE_COUNT];    //相互模糊的音节
    int      syllable_map_index[MAX_SP_FUZZY_SYLLABLE_COUNT];    //当前音节在share_segment->syllable_map中的下标
    int      sp_fuzzy_syllable_count;                            //fuzzy_syllable中实际音节个数
} SPFUZZYMAPITEM;


#define		MAX_PINYIN_LENGTH		8								//最长的拼音串长度

//音节通配符字符
#define	SYLLABLE_ANY_CHAR	'*'
#define		MAX_INPUT_LENGTH		64								//最大用户输入字符长度

#define		ENGLISH_STATE_NONE	0				//非英文模式

#define		STATE_START			0				//起始状态
#define		STATE_EDIT			1				//编辑状态
#define		STATE_SELECT		2				//选择候选状态
#define		STATE_ENGLISH		3				//英文状态
#define		STATE_RESULT		4				//候选上屏状态
#define		STATE_ILLEGAL		5				//非法输入状态

#define		CI_TYPE_NORMAL			1								//正常的词汇输入(逆向解析输入)
#define		CI_TYPE_LETTER			2								//单字母输入(超级简拼)
#define		CI_TYPE_OTHER			3								//正向解析输入
#define		CI_TYPE_WILDCARD		4								//词语联想输入



#endif // WORDLIBDEF_H
