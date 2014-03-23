/*	音节定义头文件
 */

#ifndef	_SYLLABLE_H_
#define	_SYLLABLE_H_

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

//extern int sp_used_v;			//双拼是否使用了v
//extern int sp_used_i;
//extern int sp_used_u;

#define	SP_INI_FILE_NAME	TEXT("unispim6\\ini\\双拼.ini")

#define	MAX_SYLLABLES		512			//最多的音节数目

//判断音节是否以HU开头
#define	IS_START_WITH_HU(syllable)		(syllable.con == CON_H && syllable.vow >= VOW_U && syllable.vow <= VOW_UO)
//判断音节是否完全相同
#define	IS_SAME_SYLLABLE(s1, s2)		(*(short*)(&(s1)) == *(short*)(&(s2)))

#define	IS_TONE(ch)						((ch) == TONE_CHAR_1 || (ch) == TONE_CHAR_2 || (ch) == TONE_CHAR_3 || (ch) == TONE_CHAR_4)

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

//切分音符号
#define	SYLLABLE_SEPARATOR_CHAR		'\''			//音节分隔符号（用户输入）
#define	SYLLABLE_SEPARAOTR_CHAR		9				//音节分隔符号（系统添加）

//声母定义
#define	CON_NUMBER			24			//合法声母的数目

#define	CON_ERROR			-1			//错误的声母
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

//音节类型
typedef struct tagSYLLABLE
{
	unsigned short	con : 5;				//声母标识
	unsigned short	vow : 6;				//韵母标识
	unsigned short	tone : 5;				//音调0-5，按位设置音调。1-4位分别标识四个音调，5标识轻声
} SYLLABLE;

//音节转换表结构
typedef struct tagSYLLABLEMAP
{
	TCHAR	pin_yin[8];				//拼音串
	int		pin_yin_length : 8;		//拼音串长度
	unsigned int con : 5;			//声母
	unsigned int vow : 6;			//韵母
	int		fuzzy_flag;				//模糊标志，即在这个模糊标志存在的情况下，才是正确的拼音串
									//如：cuang，只在c与ch模糊的情况下，才是正确的拼音；再如：len，
									//在l与r，l与n模糊的情况下，才是正确的。
	//int     correct_flag;           //纠错标志，即在这个纠错标志存在的情况下，才是正确的拼音串
	TCHAR	sp_pin_yin[4];			//双拼拼音串
	TCHAR	pin_yin_tone1[8];		//带声调的拼音串(1声)
	TCHAR	pin_yin_tone2[8];		//带声调的拼音串(2声)
	TCHAR	pin_yin_tone3[8];		//带声调的拼音串(3声)
	TCHAR	pin_yin_tone4[8];		//带声调的拼音串(4声)
} SYLLABLEMAP;

typedef struct tagSYLLABLEV5MAP
{
	int con : 5;
	int vow : 6;
}SYLLABLEV5MAP;

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

//拼音纠错选项
//#define CORRECT_GN_NG               (1 << 0)
//#define CORRECT_MG_NG               (1 << 1)
//#define CORRECT_IOU_IU              (1 << 2)
//#define CORRECT_UEI_UI              (1 << 3)
//#define CORRECT_UEN_UN              (1 << 4)
//#define CORRECT_ON_ONG              (1 << 5)

//拼音－音节转换表。按升序排列，用于二分检索
//extern SYLLABLEMAP syllable_map[];
//extern SYLLABLEV5MAP syllablev5_map[];

struct tagPIMCONTEXT;

//拼音－音节转换表长度
//extern const int syllable_map_items;
//extern const int syllablev5_map_items;

//不使用音调的比较，判断第一个音节参数所指示的音节集合是否包含第二个音节参数。
extern int ContainSyllable(SYLLABLE syllable, SYLLABLE checked_syllable, int fuzzy_mode);

//使用音调的比较，判断第一个音节参数所指示的音节集合是否包含第二个音节参数。
extern int ContainSyllableWithTone(SYLLABLE syllable, SYLLABLE checked_syllable, int fuzzy_mode);

//获得拼音的音节标识。
extern int GetSyllable(const TCHAR *pin_yin, SYLLABLE *syllable, int *syllable_str_length, int fuzzy_mode/*, int *correct_flag, int *separator_flag*/);

//获得单个音节拆分成的两个小音节
extern int GetSmallSyllables(SYLLABLE syllable, SYLLABLE *small_syllables);

//解析拼音串到音节数组。
extern int ParsePinYinString(const TCHAR *pin_yin, SYLLABLE *syllables, int array_length, int fuzzy_mode/*, int *correct_flag, int *separator_flag*/);

//通过拆分字的音节来获取另外的拼音解析方式
extern int GetSmallSyllablesParse(SYLLABLE *syllables, int syllable_count, SYLLABLE *other_arrays, int *other_lengths);

//反向解析拼音串到音节数组。
extern int ParsePinYinStringReverse(const TCHAR *pin_yin, SYLLABLE *syllables, int array_length, int fuzzy_mode, int pinyin_mode/*, int *correct_flag, int *separator_flag*/);
//获取另外的拼音解析方式
extern int GetOtherSyllableParse(struct tagPIMCONTEXT *context, int syllable_start, int syllable_count, SYLLABLE *other_syllables, int other_length);

//判断音节首字母是否与参数字母相同。用于单键取词。
extern int SyllableStartWithLetter(TCHAR ch, SYLLABLE syllable);

//判断第一个音节参数所指示的声母集合是否包含第二个音节参数的声母。
extern int ContainCon(SYLLABLE syllable, SYLLABLE checked_syllable, int fuzzy_mode);

//判断第一个音节参数所指示的韵母集合是否包含第二个音节参数的韵母。
extern int ContainVow(SYLLABLE syllable, SYLLABLE checked_syllable, int fuzzy_mode);

//比较一组音节，不进行通配符的比较，不进行音调比较。
extern int CompareSyllables(SYLLABLE *syllables, SYLLABLE *checked_syllables, int length, int fuzzy_mode);

//比较一组音节，进行通配符的比较，不进行音调比较。
extern int WildCompareSyllables(SYLLABLE *syllables, int length, SYLLABLE *checked_syllables, int checked_length, int fuzzy_mode);
extern int WildCompareSyllablesWithCi(SYLLABLE *syllables, int length, SYLLABLE *checked_syllabe, HZ *ci, int checked_length, int fuzzy_mode);

//比较一组音节的首字母是否与目标相符，不进行通配符的比较，不进行音调比较。
extern int CompareSyllablesAndLetters(const TCHAR *letters, SYLLABLE *checked_syllables, int length);
//比较一组音节的首字母是否与目标相符，进行通配符的比较。
extern int WildCompareSyllablesAndLetters(const TCHAR *letters, int letter_length, SYLLABLE *checked_syllables, int checked_length);

//判断是否为音调的字符
extern int IsToneChar(char ch);
//判断是否为完整的音节串
extern int IsFullSyllable(SYLLABLE *syllable_array, int syllable_count);
//判断是否包含音节分隔符
extern int HasSyllableSeparator(const TCHAR *str);

//判断连个音节是否完全相同
extern int SameSyllable(SYLLABLE s1, SYLLABLE s2);

//获得音节的拼音串
extern int GetSyllableString(SYLLABLE syllable, TCHAR *str, int length,/* int correct_flag,*/ int tone_char);
//得到全拼的字符串（拼音全串）
//extern void GetAllSyllableStringQP(struct tagPIMCONTEXT *context, TCHAR *pinyin, unsigned int pinyin_len);
extern int GetSyllableStringSP(SYLLABLE syllable, TCHAR *str, int length);
extern void GetSPHintString(struct tagPIMCONTEXT *context, TCHAR *hint_buffer, int buffer_length);

//双拼资源函数
extern int LoadSPIniData(const TCHAR *ini_name);
extern int FreeSPIniData();

extern int GetLegalPinYinLength(const TCHAR *pin_yin, int state, int english_state);

#ifdef __cplusplus
}
#endif

#endif