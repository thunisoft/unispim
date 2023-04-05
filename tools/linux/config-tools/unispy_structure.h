#ifndef UNISPY_STRUCTURE_H
#define UNISPY_STRUCTURE_H

#include "unispydef.h"
#include "stdlib.h"
#include "stdio.h"
#include "stddef.h"
#include "stdint.h"
#include <string.h>

//**********************************Define*****************************************************************//
//**********************************CI*******************************************************************//
//词Cache定义，一般一个词占用5-7个字节
//256K空间，可以放置256000/5 ~= 51200条词汇，应该可以用了。
#define	CI_CACHE_SIZE				0x80000				//词cache的size
#define	CI_MOVE_SIZE				(CI_CACHE_SIZE / 4)	//Cache满后的移动幅度
#define	CI_MAX_USED_COUNT			0x700000			//词的最大使用度
#define CI_TOP_USED_COUNT			0x5FFFFF			//置顶词的使用度
#define	CI_CACHE_EXTRA_LENGTH		0x100				//词Cache的临界缓冲区长度
#define CI_CACHE_V66_SIGNATURE		0x20000919			//词缓冲区的标识（用于版本判断）
#define	CI_CACHE_SIGNATURE			0x20000918			//词缓冲区的标识（用于版本判断）
#define	CI_NEW_BUFFER_LENGTH		0x8000				//32K的新词缓冲区
#define	CI_NEW_EXTRA_LENGTH			0x100				//新词的缓冲区

#define CICACHE_FILE_NAME "ci_cache.dat"
//**********************************ICW***********************************************************************//
#define	BIGRAM_FILE_NAME    "bigram.dat"

#define	ICW_MAX_ITEMS			1024					//每一个ICW项的最大候选数目
#define	ICW_MAX_CI_ITEMS		256						//每项最大的词数目
#define	ICW_MAX_PART_SYLLABLES	5						//最多5个非全音节

#define	ENCODE(x)		((((x) >> 7) | ((x) << 9)) ^ 0xaa55)
#define	DECODE(x)		((((x) ^ 0xaa55) << 7) | (((x) ^ 0xaa55) << 7))

//*************************************WORDLIB*************************************************************//
#define	WORDLIB_MAX_FREQ				((1 << 19) - 1)				//词库中的最大词频
#define	BASE_CI_FREQ					(500000)					//词频基准
#define	USER_BASE_FREQ					100000						//用户自定词的默认词频

#define	DEFAULT_USER_WORDLIB_NAME   "用户词库"
#define	DEFAULT_USER_WORDLIB_AUTHOR	"华宇拼音输入法"

#define	WORDLIB_NAME_LENGTH     16  //词库名称长度
#define	WORDLIB_AUTHOR_LENGTH   16  //词库作者名称
#define	WORDLIB_PAGE_SIZE   1024    //词库页大小
#define	PAGE_END    -1      //结束页
#define	WORDLIB_PAGE_DATA_LENGTH    (WORDLIB_PAGE_SIZE - 4 * sizeof(int))   //页中可用数据长度
#define MAX_WORDLIBS    32  //内存中最多的词库数目
#define	WORDLIB_EXTRA_LENGTH    0x100000    //用户词库的扩充空间
#define	WORDLIB_NORMAL_EXTRA_LENGTH 0x40000 //普通增加容量便于添加词汇
#define	WORDLIB_FEATURE_LENGTH sizeof(int)  //词项中的特性描述长度，目前用一个整数

#define WORDLIB_SYS_FILE_NAME   "sys.uwl"   //系统词库文件名
//************************************************SYLLABLE****************************************************//
//判断音节是否以HU开头
#define	IS_START_WITH_HU(syllable)		(syllable.con == CON_H && syllable.vow >= VOW_U && syllable.vow <= VOW_UO)


#define	MAX_SYLLABLES		512			//最多的音节数目

//切分音符号
#define	SYLLABLE_SEPARATOR_CHAR '\''			//音节分隔符号（用户输入）

//音节通配符字符
#define	SYLLABLE_ANY_CHAR	'*'

//音调字符
#define	TONE_CHAR_1			'!'				//一声
#define	TONE_CHAR_2			'@'				//二声
#define	TONE_CHAR_3			'#'				//三声
#define	TONE_CHAR_4			'$'				//四声
#define	TONE_CHAR_CHANGE	'~'				//变换音调

#define	TONE_0				0
#define	TONE_1				(1 << 0)
#define	TONE_2				(1 << 1)
#define	TONE_3				(1 << 2)
#define	TONE_4				(1 << 3)

//声母定义
#define	CON_NUMBER 24   //合法声母的数目
#define	CON_ERROR
//#include "syllable.h"-1			//错误的声母
#define	CON_NULL			0			//没有声母
#define	CON_B				1
#define CON_C				2
#define CON_CH				3
#define	CON_D				4
#define CON_F				5
#define CON_G				6
#define CON_H				7
#define	CON_J				8
#define CON_K				9
#define CON_L				10
#define	CON_M				11
#define CON_N				12
#define	CON_P				13
#define	CON_Q				14
#define	CON_R				15
#define CON_S				16
#define CON_SH				17
#define	CON_T				18
#define	CON_W				19
#define	CON_X				20
#define	CON_Y				21
#define CON_Z				22
#define CON_ZH				23
#define	CON_END				24
//声母通配符
#define	CON_ANY				25

//韵母定义
#define	VOW_NUMBER			34			//合法韵母的数目
//切分音符号
#define	SYLLABLE_SEPARATOR_CHAR		'\''			//音节分隔符号（用户输入）
#define SYLLABLE_SEPARATOR_STR "\'"

//#define	SYLLABLE_SEPARAOTR_CHAR		9				//音节分隔符号（系统添加）
#define	VOW_ERROR			-1			//错误的韵母
#define	VOW_NULL			0			//没有韵母
#define	VOW_A				1
#define	VOW_AI				2
#define	VOW_AN				3
#define	VOW_ANG				4
#define	VOW_AO				5
#define	VOW_E				6
#define	VOW_EI				7
#define	VOW_EN				8
#define	VOW_ENG				9
#define	VOW_ER				10
#define	VOW_I				11
#define	VOW_IA				12
#define	VOW_IAN				13
#define	VOW_IANG			14
#define	VOW_IAO				15
#define	VOW_IE				16
#define	VOW_IN				17
#define	VOW_ING				18
#define	VOW_IONG			19
#define	VOW_IU				20
#define	VOW_O				21
#define	VOW_ONG				22
#define	VOW_OU				23
#define	VOW_U				24
#define	VOW_UA				25
#define	VOW_UAI				26
#define	VOW_UAN				27
#define	VOW_UANG			28
#define	VOW_UE				29
#define	VOW_UI				30
#define	VOW_UN				31
#define	VOW_UO				32
#define	VOW_V				33
#define	VOW_END				34
//韵母通配符
#define	VOW_ANY				35


//模糊音选项（TODO: 需要补上ian <-> iang，uan <-> uang）
//命名规则：后面为发音困难的，前面为替代的
#define	FUZZY_Z_ZH					(1 << 0)
#define	FUZZY_C_CH					(1 << 1)
#define	FUZZY_S_SH					(1 << 2)
#define	FUZZY_G_K					(1 << 3)
#define	FUZZY_L_N					(1 << 4)
#define	FUZZY_L_R					(1 << 5)
#define	FUZZY_F_H					(1 << 6)
#define	FUZZY_F_HU					(1 << 7)
#define	FUZZY_HUANG_WANG			(1 << 8)
#define	FUZZY_AN_ANG				(1 << 9)
#define	FUZZY_EN_ENG				(1 << 10)
#define	FUZZY_IN_ING				(1 << 11)

#define	FUZZY_REV_Z_ZH				(1 << 12)				//默认为0
#define	FUZZY_REV_C_CH				(1 << 13)				//默认为0
#define	FUZZY_REV_S_SH				(1 << 14)				//默认为0
#define	FUZZY_REV_G_K				(1 << 15)				//默认为0
#define	FUZZY_REV_L_N				(1 << 16)				//默认为0
#define	FUZZY_REV_L_R				(1 << 17)				//默认为0
#define	FUZZY_REV_F_H				(1 << 18)				//默认为0
#define	FUZZY_REV_F_HU				(1 << 19)				//默认为0
#define	FUZZY_REV_HUANG_WANG		(1 << 20)				//默认为0
#define	FUZZY_REV_AN_ANG			(1 << 21)				//默认为0
#define	FUZZY_REV_EN_ENG			(1 << 22)				//默认为0
#define	FUZZY_REV_IN_ING			(1 << 23)				//默认为0

#define	FUZZY_ZCS_IN_CI				(1 << 24)				//默认为0，运行时决定，不允许用户设置
#define	FUZZY_SUPER					(1 << 25)				//超级韵母模糊
#define	FUZZY_CI_SYLLABLE_LENGTH	(1 << 26)				//模糊词长度与音节长度
//************************************GRAM*********************************************//
#define	SJX_FREQ					(200000000)

#define	MAX_BCOUNT					(1.0e+11)

#define	BIGRAM_SIGN					0x20072222
#define	TRIGRAM_SIGN				0x20073333

#define	MAX_WORDS_BIT				18
#define	ONE_COUNT_BIT				14

#define	MAX_SEG_WORD_LENGTH			16						//分词词典中的最大长度
#define	MAX_SEG_WORDS				(1 << MAX_WORDS_BIT)	//词典中最多的词个数

#define	BIGRAM_WORD_LIST_SIZE		(1 << 22)				//词表预留空间
#define	BIGRAM_INDEX0_SIZE			(1 << MAX_WORDS_BIT)	//第一级索引个数
#define	BIGRAM_ITEM_SIZE			(1 << 25)				//Bigram项空间

#define	XM				(0.618)
//#define	XM				(1.0)
#define	RT				(330.0)						//相信因子好像没有用处

//***********************************************SPW****************************************************//
#define MAX_SPW_COUNT   32  //短语文件最多数目
#define SYS_SPW_FILE_NAME "systemspw.ini"   // 短语文件名称
#define SPW_BUFFER_SIZE 0x500000	//短语存储区大小
#define MAX_SPW_LENGTH 2000     //特殊词汇的最大长度（以Char为单位）
#define	SPW_CONTENT_LENGTH  MAX_SPW_LENGTH	//短语内容最大长度
#define	SPW_NAME_LENGTH     16  //短语名字最大长度
#define SPW_HINT_LENGTH     64  //短语提示最大长度

#define	SPW_COMMENT_CHAR    ';'		//短语注释符号
#define	SPW_ASSIGN_CHAR		'='		//短语赋值符号
#define	SPW_HINT_LEFT_CHAR	'['		//短语提示符号_左
#define	SPW_HINT_RIGHT_CHAR	']'		//短语提示符号_右
#define	SPW_HINT_NULL_STR	"[]"	//短语提示空串
#define	SPW_MAX_ITEMS		400000	//短语最多数目


#define	SPW_TYPE_NAME		1   //短语名字
#define	SPW_TYPE_CONTENT	2	//短语内容
#define	SPW_TYPE_COMMENT	4	//短语注释
#define	SPW_TYPE_NONE		8	//出错！非短语相关

#define	SPW_STRING_NORMAL  1    //普通的短语
#define	SPW_STRING_EXEC	   2    //执行程序类型
#define	SPW_STRING_SPECIAL 3    //特殊类型，如I,D,H等
#define	SPW_STRING_BH	   4    //笔划候选
#define	SPW_STIRNG_ENGLISH 5    //英文单词




//*************************************STRUCTURE OF DIFFERENT LIBS*******************************************//

//************************************************SYLLABLE********************************************************//
//音节类型
typedef struct tagSYLLABLE
{
    unsigned short con : 5; //声母标识
    unsigned short vow : 6;	//韵母标识
    unsigned short tone : 5;    //音调0-5，按位设置音调。1-4位分别标识四个音调，5标识轻声
} SYLLABLE;

//音节转换表结构
typedef struct tagSYLLABLEMAP
{
    char pin_yin[8];    //拼音串
    int pin_yin_length : 8; //拼音串长度
    unsigned int con : 5;   //声母
    unsigned int vow : 6;   //韵母
    int fuzzy_flag;	//模糊标志，即在这个模糊标志存在的情况下，才是正确的拼音串
                    //如：cuang，只在c与ch模糊的情况下，才是正确的拼音；再如：len，
                    //在l与r，l与n模糊的情况下，才是正确的。
//    int correct_flag;   //纠错标志，即在这个纠错标志存在的情况下，才是正确的拼音串
    char sp_pin_yin[4]; //双拼拼音串
    char pin_yin_tone1[8];  //带声调的拼音串(1声)
    char pin_yin_tone2[8];  //带声调的拼音串(2声)
    char pin_yin_tone3[8];  //带声调的拼音串(3声)
    char pin_yin_tone4[8];  //带声调的拼音串(4声)
} SYLLABLEMAP;

typedef struct tagSPECIALSYLLABLE
{
    int old_syllable_count;
    SYLLABLE old_syllables[5];
    int new_syllable_count;
    SYLLABLE new_syllables[5];
}SPECIALSYLLABLE;

typedef struct tagNOREVERSESYLLABLE
{
    int count;
    SYLLABLE syllables[2];
}NOREVERSESYLLABLE;

struct SyllableItem
{
    SYLLABLE syllable;
    char pinyin[8];
};

//************************************************HZ********************************************************//
//汉字项类型
typedef struct tagHZITEM
{
    UC  hz; //字(UNICODE)
    unsigned short hz_id;   //汉字条目的ID
    SYLLABLE syllable;  //音节
    int freq;   //字频
    int simplified : 1;     //简体
    int traditional : 1;    //繁体
    int other : 1;  //非繁体/简体的字（一般为日本/韩文汉字）
    int effective : 1;  //有效(支持将来条目变化)
    int show_syllable : 1;  //难字(在候选栏中提示拼音)
    int other_data : 26;    //其他数据(暂时空白)
    int icw_hz : 1; //ICW用(单字词)
}HZITEM;
//************************************************BH********************************************************//
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

//************************************************CI********************************************************//
//词Cache说明，
// B0			：词的长度
// B1 B2 B3		：词的使用度
// 我们....		：词条中文
typedef struct tagCICACHE
{
    int signature;  //词缓冲区标识
    int length;     //cache的使用长度
    int max_used_id;    //词的最大使用次数，到达的时候，需要进行剪除操作
    char cache[CI_CACHE_SIZE + CI_CACHE_EXTRA_LENGTH];		//词cache
}CICACHE;

//************************************************GRAM********************************************************//
//索引项内容
typedef struct tagGRAM_INDEX    //Bigram, Trigram索引
{
    int word_pos;   //在词表中的位置
    int word_freq;  //该词词频
    int item_index; //项所处于的位置
    int start_count;    //该项起头的数目
}GRAM_INDEX;

//项内容
typedef struct tagGRAM_ITEM     //Bigram，Trigram项
{
    unsigned int word_index : MAX_WORDS_BIT;  //词在词表中的位置
    unsigned int count : ONE_COUNT_BIT;     //出现计数，当数据不足以表达的时候，扩大到两个ITEM
}GRAM_ITEM;

//GRAM文件头部
typedef struct tagGRAM_HEADER
{
    int sign;   //gram标识

    long long total_word_freq;  //全部词汇的计数

    long long total_bigram_count;   //全部bigram计数
    long long total_bigram_in_count;    //进入模型的bigram计数总量，用于计算剩余概率

    int end_token_count;    //结束符号计数（用于语句输入结尾的概率计算）

    int word_list_size; //词表大小

    int index0_count;   //第一级索引数目
    int index0_size;    //第一级索引大小

    int index1_count;   //第二级索引数目
    int index1_size;    //第二级索引大小

    int item_count; //数据项数目
    int item_size;  //数据项空间大小

    int word_list_pos;  //词表起始位置
    int index0_data_pos;    //保留用于计算指针的空间
    int index1_data_pos;    //保留用于计算指针的空间
    int item_data_pos;  //保留用户计算指针的空间

    //计算过程，计算指针
    //index0_data = (GRAM_INDEX*)gram_data.index0_data;
    //index1_data = (GRAM_INDEX*)((char*)index0_data + sizeof(GRAM_INDEX) * index0_count);
    //item_data   = (GRAM_ITEM*) ((char*)index1_data + sizeof(GRAM_INDEX) * index2_count);
}GRAM_HEADER;

//gram文件结构
typedef struct tagGRAM_DATA
{
    GRAM_HEADER header;     //Bigram/Trigram头
    char align_data[0x400 - sizeof(GRAM_HEADER)];   //对齐预留空间
}GRAM_DATA;


//************************************************ICW********************************************************//
typedef struct tagNEWICWITEM
{
    int			length;						//候选项的长度
    HZ			*hz;						//汉字
    SYLLABLE	*syllable;					//音节
    int			freq;						//字频或者词频
    double		value;						//估值
    struct tagNEWICWITEM	*next;			//下一项
}NEWICWITEM;

typedef struct tagICWGROUPITEM
{
    int			count;
    NEWICWITEM	item[ICW_MAX_CI_ITEMS];
}ICWGROUPITEM;

typedef struct tagICWITEMSET
{
    int				group_count;
    ICWGROUPITEM	group_item[MAX_ICW_LENGTH];
}ICWITEMSET;


//************************************************MAPFILE********************************************************//
typedef struct tagFILEMAPHANDLE
{
    int h_file;       // 文件句柄
    char* h_map;	// 映射句柄
    long long length;	// 文件的长度
    long long offset;	// 文件当前偏移
    int granularity;	// 映射颗粒度
    char* view;		// 当前的视图
}FILEMAPHANDLE;




//************************************************WORDLIB********************************************************//
//词库头定义
typedef struct tagWORDLIBHEADER
{
    int signature;  //词库的签名
    UTF16 name[WORDLIB_NAME_LENGTH]; //词库的名称
    UTF16 author_name[WORDLIB_AUTHOR_LENGTH]; //词库作者名称
    int word_count; //词汇数目
    int page_count; //已分配的页数
    int can_be_edit; //是否可以编辑
    int	pim_version; //输入法版本号（兼容性考虑）
    int	index[CON_NUMBER][CON_NUMBER]; //索引表
} WORDLIBHEADER;

//页定义
typedef	struct tagPAGE
{
    int page_no; //页号
    int next_page_no; //下一个页号，-1标识结束
    int length_flag; //本页包含的词汇长度的标志
    int data_length; //已经使用的数据长度
    char data[WORDLIB_PAGE_DATA_LENGTH]; //数据开始
} PAGE;

//词库定义
typedef	struct tagWORDLIB
{
    union
    {
        //词库文件头数据
        WORDLIBHEADER header;
        //用于对齐页边界
        PAGE header_data[sizeof(WORDLIBHEADER) / WORDLIB_PAGE_SIZE + 1];
    };
    PAGE pages[1]; //页数据
} WORDLIB;


//************************************************ZI********************************************************//
//汉字信息文件结构(hzpy.dat)
typedef struct tagHZDATAHEADER
{
    unsigned int signature;     //标识
    unsigned int create_date;   //创建日期
    unsigned int modify_date;   //修改日期
    unsigned int check_sum;     //校验和
    int hz_count;   //汉字数目

    HZITEM hz_item[1];  //汉字项
}HZDATAHEADER;

typedef struct tagHZCACHE
{
    int	signature;							//字缓冲区的标志ZICACHE_SIGNATURE（用于版本判断）
    int max_id;								//当前最大的汉字cache标号(如果Cache未满时就是Cache中字的个数))
    int cache[MAX_HZ_IN_PIM];				//ZiCache比较独特，和CiCache不同，文件里并不存储汉字，cache[i]的i是字库里HZITEM的hz_id，而cache[i]的值是该字加入Cache时的顺序号，详见AddHzToCache
    int used_count[MAX_HZ_IN_PIM];			//用户汉字使用度表，和cache成员类似，used_count[i]的i是字库里HZITEM的hz_id，而used_count[i]的值是该字的使用度
}HZCACHE;

//置顶字定义表
typedef struct tagTOPZITABLE
{
    SYLLABLE syllable;      //音节
    HZ top_zi[MAX_TOPZI];   //置顶字个数
}TOPZIITEM;

//简繁对照项
typedef struct tagJFITEM
{
    UC jian;    //简体汉字
    UC fan;     //繁体汉字
}JFITEM;
//*********************************************CANDIDATE*************************************//
//智能组词项类型
typedef struct tagICWCANDIDATE
{
    char length;						//ICW的长度（以汉字为单位）
    SYLLABLE syllable[MAX_ICW_LENGTH];	//ICW的音节数组，用于以词定字的字频更新以及保存词到词库。
    HZ hz[MAX_ICW_LENGTH + 1];		//ICW的汉字内容
}ICWCANDIDATE;

//自定义词项类型
typedef struct tagSPWCANDIDATE
{
    int length;     //长度
    const void* string; //特殊词指针(utf8)
    const void* hint;   //提示信息
    int type;   //短语的类型
    UC hz;      //笔划使用的汉字
}SPWCANDIDATE;

//词项类型
typedef struct tagWORDITEM
{
    unsigned int effective : 1; //有效（没有被删除）
    unsigned int ci_length : 6; //词长度
    unsigned int syllable_length : 6;   //音节长度
    unsigned int freq : 19; //词频（Q: 词频和使用度有什么区别？A: 词频是词库里的，使用度是CiCache里的，词频和使用度的计算方式都是使用1次累加1
                            //(见AddCi和InsertCiToCache)，其实准确地说，在装好输入法开始使用以后，使用度才是使用1次加1，而词库中的次品是不变的，
                            //(用户词库除外)，最初的词库中的词频信息是利用原始语料库中的词频换算得到的(通常不是0，使用度的初始值才是0)，换算公
                            //式见AdjustWordFreqToHZFreq))
    //后面为变长结构，与length相关
    SYLLABLE syllable[1];   //音节, C++要求数组必须有size
}WORDLIBITEM;

//词候选类型
typedef struct tagWORDCANDIDATE
{
    WORDLIBITEM *item;  //词库中词条指针
    int type;       //类型
    int source;     //词汇来源词库标识
    int cache_pos;  //本词条在cache中的位置
    SYLLABLE *syllable; //指向词库词条的指针
    HZ *hz;         //指向词条汉字的指针
    int origin_syllable_length; //原始音节长度(用于双向解析造成的音节长度不符合)
}WORDCANDIDATE;



//字候选类型
typedef struct tagHZCANDIDATE
{
    WORDLIBITEM	*word_item; //词项指针
    HZITEM *item;   //在字表中的汉字项
    int hz_type;    //候选字类型
    int top_pos;    //固顶字标识，0：非固顶字，1-7为固顶字位置。
    int	cache_pos;  //在CACHE中的位置，用于判断最新输入的汉字
    int	is_word;    //是否为词(如：西安)
    SYLLABLE origin_syllable;   //小音节词对应的原始音节，如xi'an的原始音节是xian
}HZCANDIDATE;

//以词定字候选
typedef struct tagZIFROMWORDCANDIDATE
{
    WORDCANDIDATE *word;    //原来的词
    SYLLABLE syllable;  //音节
    UTF16 hz;   //汉字
}ZFWCANDIDATE;

//候选项结构
typedef struct tagCANDIDATE
{
    char type;      //候选类别
    union
    {
        ICWCANDIDATE icw;   //智能组词候选
        SPWCANDIDATE spw;   //自定义词候选
        WORDCANDIDATE word; //词候选
        HZCANDIDATE hz;     //汉字候选
        ZFWCANDIDATE zfw;   //以词定字候选
    };
    char pinyin[MAX_INPUT_LENGTH];
}CANDIDATE;
//************************************************STATE********************************************************//
typedef struct tagEditItem
{
    int			syllable_start_pos;				//起始音节位置
    int			syllable_length;				//占用的音节数目
    CANDIDATE	candidate;						//本项的候选信息
    int			left_or_right;					//是否为以词定字，并标识左侧、右侧
    int         cand_hz_length;
    char        pinyin[MAX_INPUT_LENGTH];
    char        hanzi[MAX_INPUT_LENGTH];
}SELECT_ITEM;

enum EditMode
{
    MODE_NORMAL,    // 正常编辑模式
    MODE_I,     // i模式,阿拉伯数字转换成中文
    MODE_B,     // B模式，笔画输入
    MODE_INVALID,   // 非法输入
};

enum IMMode
{
    IM_MODE_ENGLISH,
    IM_MODE_CHINESE,
};

#endif // UNISPY_STRUCTURE_H
