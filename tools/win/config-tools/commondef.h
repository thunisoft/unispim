#ifndef _CONFIG_TOOLS_COMMONDEF_H
#define _CONFIG_TOOLS_COMMONDEF_H
#include <QString>
#include <QMap>
#include <QVariant>

#ifdef USE_IBUS
#define DEFAULT_CANDIDATE_COUNT 8
#else
#define DEFAULT_CANDIDATE_COUNT 5
#endif

#define MAX_CANDIDATE_COUNT 9

#define INI_POSTFIX ".ini"

typedef QMap<QString, QVariant> UserInfo;

#define	HZ_ADJUST_FREQ_FAST			(1 << 2)		//快速调整字频（默认）
#define	HZ_ADJUST_FREQ_SLOW			(1 << 3)		//慢速调整字频
#define	HZ_ADJUST_FREQ_NONE			(1 << 4)		//不调整字频

//拼音方式
#define	PINYIN_QUANPIN				0				//全拼
#define	PINYIN_SHUANGPIN			1				//双拼

//键盘对配置
#define	KEY_SWITCH_SHIFT			0				//SHIFT切换输入状态
#define	KEY_SWITCH_CONTROL			1				//CONTROL切换输入状态
#define	KEY_SWITCH_NONE				2				//不使用键盘切换输入状态
#define	KEY_2ND_3RD_NONE			0				//不使用第二/第三快速候选键
#define	KEY_2ND_3RD_CONTROL			1				//用CONTROL选择2、3号候选
#define	KEY_2ND_3RD_SHIFT			2				//用SHIFT选择2、3号候选

//候选翻页
#define	KEY_PAIR_1					(1 << 2)		//,.（候选翻页）
#define	KEY_PAIR_2					(1 << 3)		//[]（以词定字）
#define	KEY_PAIR_3					(1 << 4)		//-=
#define	KEY_PAIR_4					(1 << 5)		//()（智能编辑）
#define	KEY_PAIR_5					(1 << 6)		//<>（候选漫游）
#define KEY_PAIR_6					(1 << 7)		//PageUp/PageDown
#define KEY_PAIR_7					(1 << 8)		//Up/Down

//输入汉字的状态
#define	HZ_OUTPUT_SIMPLIFIED		(1 << 0)		//输出简体字（默认）
#define	HZ_OUTPUT_TRADITIONAL		(1 << 1)		//输出繁体字
#define	HZ_OUTPUT_HANZI_ALL 		(1 << 2)		//输出全集
#define	HZ_OUTPUT_ICW_ZI			(1 << 4)		//输出ICW使用的汉字集合
#define	HZ_SYMBOL_CHINESE			(1 << 5)		//中文符号
#define	HZ_SYMBOL_HALFSHAPE			(1 << 6)		//半角符号

//词选项
#define	CI_AUTO_FUZZY				(1 << 0)		//输入词的时候，自动使用z/zh, c/ch, s/sh的模糊（默认）
#define	CI_SORT_CANDIDATES			(1 << 1)		//候选词基于词频进行排序（默认）
#define	CI_ADJUST_FREQ_FAST			(1 << 2)		//快速调整词频（默认）
#define	CI_ADJUST_FREQ_SLOW			(1 << 3)		//慢速调整词频（默认）
#define	CI_ADJUST_FREQ_NONE			(1 << 4)		//不调整词频（默认）
#define	CI_WILDCARD					(1 << 5)		//输入词的时候，使用通配符（默认）
#define	CI_RECENT_FIRST				(1 << 6)		//最新输入的词优先（默认）
#define	CI_USE_FIRST_LETTER			(1 << 7)		//使用首字母输入词（默认）
#define	CI_AUTO_VOW_FUZZY			(1 << 8)		//输入词的时候自动进行韵母首字母模糊匹配

//工具栏按钮配置
#define USE_FAN_JIAN				(1 << 0)
#define USE_ENG_CN					(1 << 1)
#define USE_FULL_HALF_WIDTH			(1 << 2)
#define USE_PUNCTUATION				(1 << 3)
#define USE_SPECIAL_SYMBOL			(1 << 4)
#define USE_SOFT_KEYBOARD			(1 << 5)
#define USE_SETTINGS                (1 << 6)
#define USE_QUAN_SHUANG             (1 << 7)
#define USE_OCR_ADDON               (1 << 8)
#define USE_VOICE_ADDON             (1 << 9)

#define	CI_USE_FIRST_LETTER			(1 << 7)		//使用首字母输入词（默认）

#define HZ_SCOPE_GBK                (1 << 0)
#define HZ_SCOPE_UNICODE            (1 << 1)


struct screenOptions
{
    int minWidth; //最小宽
    int minHeight;//最小高
    int visualWidth;//设计宽度
    int visualHeight;//设计高度
    //小型尺寸缩放系数
    double Small_width_dimension_scale_factor;
    double Small_height_dimension_scale_factor;
    //中型号尺寸缩放系数
    double Middle_width_dimension_scale_factor;
    double Middle_height_dimension_scale_factor;
    double Large_width_dimension_scale_factor;
    //大型尺寸缩放系数
    double Large_height_dimension_scale_factor;
    screenOptions()
    {
        minWidth= 1366;
        minHeight= 768;
        visualWidth= 1920;
        visualHeight= 1080;
        Small_width_dimension_scale_factor=0.563;
        Small_height_dimension_scale_factor=0.703;
        Middle_width_dimension_scale_factor=0.563;
        Middle_height_dimension_scale_factor=0.703;
        Large_width_dimension_scale_factor=0.37;
        Large_height_dimension_scale_factor=0.46;
    }

};


#endif // COMMONDEF_H
