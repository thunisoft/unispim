#ifndef UNISPYDEF_H
#define UNISPYDEF_H
typedef unsigned short UTF16;
//汉字类型
typedef unsigned int UC;    //Unicode编码
typedef unsigned short HZ;
typedef unsigned char byte;

#define MAX_I_MODE_INPUT_LENGTH 36  // i模式下最大的输入长度
#define MAX_B_MODE_INPUT_LENGTH 80  // B模式下最大的输入长度

#define PY_SEPARATOR '\''   // 音节分隔符

#define MAX_PATH 1024
#define MAX_FILE_NAME_LENGTH 256

#define MAX_INPUT_LENGTH 64 // 最大用户输入字符长度
#define MAX_SYLLABLE_PER_INPUT  32  // 一次能够输入的音节数目
#define MAX_ICW_LENGTH  16  //智能组词的最大汉字数目
#define MAX_COMPOSE_LENGTH  128 //最大写作串长度
#define	MAX_WORD_LENGTH     32  //词的最大汉字数目
#define MAX_SMALL_SYLLABLES 4   //一个拼音串最多可解析的小音节(由字的音节拆分成的音节，如xian->xi'an)数
                                //其最大值不应超过MAX_SYLLABLE_PER_INPUT/2(否则拆分后的音节数溢出)
#define MAX_CANDIDATES  3000    //最大的候选项个数
#define MAX_ICW_CANDIDATES 10   //最大的智能组词个数

//不同类别的候选的长度定义（默认以汉字为单位）
#define MAX_HZ_IN_PIM   65536   //本输入法中的汉字数目最大值
#define MAX_TOPZI   8   //最多置顶字个数


//候选类型定义
#define CAND_TYPE_ICW   1   //智能组词候选
#define	CAND_TYPE_SPW   2   //自定义词候选
#define	CAND_TYPE_CI    3   //词候选
#define	CAND_TYPE_ZI    4   //字候选
#define	CAND_TYPE_ZFW   5   //以词定字候选

#define CI_TYPE_NORMAL  1   //正常的词汇输入(逆向解析输入)
#define CI_TYPE_LETTER  2   //单字母输入(超级简拼)
#define CI_TYPE_OTHER   3   //正向解析输入
#define CI_TYPE_WILDCARD 4   //词语联想输入

#define ZI_TYPE_NORMAL 1    //正常的字输入(非小音节拆分)
#define ZI_TYPE_OTHER  2    //小音节拆分的字，如xian中的xi

#define HZ_ALL_USED	 (HZ_MOST_USED | HZ_MORE_USED | HZ_LESS_USED)	//全集汉字
#define HZ_MOST_USED (1 << 0)   //常用字
#define HZ_MORE_USED (1 << 1)   //次常用字
#define HZ_LESS_USED (1 << 2)   //非常用字

#define MAX_PINYIN_LENGTH   8   //最长的拼音串长度
#define MAX_OTHER_PARSE_SYLLABLE    32  //最大正向解析音节数(SyllableHandler::GetOtherSyllableParse)


#define HZ_CACHE_MAX_ID 0x70000000  //最大的汉字cache标志。为避免出现符号错误，不设定为7fffffff

#define	MAX_RESULT_LENGTH	2048	//最大结果串长度

#define ERR_FILE_NAME "error.log"
#define	DEFAULT_FREQ        0

#define	HYPIM_VERSION   6 //输入法大版本号
#define HYPIM_WORDLIB_V66_SIGNATURE     0x14091994  //输入法词库V66标志
#define		WORDLIB_V66					66			//词库为V6.6词库

#define	WORDLIB_CREATE_EXTRA_LENGTH		0x100000					//创建新词库时，增加额外的1M数据。

#define		STATE_EDIT			1				//编辑状态
#define		ENGLISH_STATE_NONE	0				//非英文模式
//拼音方式
#define	PINYIN_QUANPIN				0				//全拼
#define	PINYIN_SHUANGPIN			1				//双拼

//音阶相关定义

//音调字符
#define	TONE_CHAR_1			'!'				//一声
#define	TONE_CHAR_2			'@'				//二声
#define	TONE_CHAR_3			'#'				//三声
#define	TONE_CHAR_4			'$'				//四声
#define	TONE_CHAR_CHANGE	'~'				//变换音调

#define	IS_TONE(ch)						((ch) == TONE_CHAR_1 || (ch) == TONE_CHAR_2 || (ch) == TONE_CHAR_3 || (ch) == TONE_CHAR_4)

#define _CanInLibrary(x)	((x >= 0x00080 && x <= 0x0D7FF) || (x >= 0x0E000 && x <= 0x0FFEF) || \
                             (x >= 0x20000 && x <= 0x2A6D6) || (x >= 0x2A700 && x <= 0x2B734) || \
                             (x >= 0x2F800 && x <= 0x2FA1D))

#endif // UNISPYDEF_H
