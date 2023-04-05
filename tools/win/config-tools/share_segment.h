#ifndef	_CONFIG_SHARE_SEGMENT_
#define	_CONFIG_SHARE_SEGMENT_
#include <Windows.h>
#include <tchar.h>
#include <winnt.h>


#ifdef __cplusplus
extern "C" {
#endif

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

typedef struct tagSHARE_SEGMENT
{
	int	spw_count;													//短语数目
	int	spw_length;													//缓冲区数据长度
	int	spw_loaded;													//短语是否已经在内存中

	SYLLABLEMAP syllable_map[460];									//拼音－音节转换表
	int syllable_map_items;
	SYLLABLEV5MAP syllablev5_map[451];								//拼音－音节转换表(V5)
	int syllablev5_map_items;

	TCHAR con_sp_string_save[26][8];								//双拼声母表
	TCHAR con_sp_string[26][8];
	TCHAR vow_sp_string_save[46][8];								//双拼韵母表
	TCHAR vow_sp_string[46][8];
	TCHAR vow_sp_string_single_save[94][8];							//双拼音节表
	TCHAR vow_sp_string_single[94][8];
}SHARE_SEGMENT;

extern SHARE_SEGMENT share_segment;

#ifdef __cplusplus
}
#endif

#endif
