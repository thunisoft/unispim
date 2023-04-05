#include "syllableapi.h"
#include <winnt.h>
#include "share_segment.h"

//声母转换到字符串
static TCHAR con_to_string[][6] =
{
    TEXT(""),						//CON_NULL
    TEXT("b"),						//CON_B
    TEXT("c"),						//CON_C
    TEXT("ch"),						//CON_CH
    TEXT("d"),						//CON_D
    TEXT("f"),						//CON_F
    TEXT("g"),						//CON_G
    TEXT("h"),						//CON_H
    TEXT("j"),						//CON_J
    TEXT("k"),						//CON_K
    TEXT("l"),						//CON_L
    TEXT("m"),						//CON_M
    TEXT("n"),						//CON_N
    TEXT("p"),						//CON_P
    TEXT("q"),						//CON_Q
    TEXT("r"),						//CON_R
    TEXT("s"),						//CON_S
    TEXT("sh"),						//CON_SH
    TEXT("t"),						//CON_T
    TEXT("w"),						//CON_W
    TEXT("x"),						//CON_X
    TEXT("y"),						//CON_Y
    TEXT("z"),						//CON_Z
    TEXT("zh"),						//CON_ZH
    TEXT(""),						//CON_END
    TEXT("*"),						//CON_ANY
};

//韵母转换到字符串
static TCHAR vow_to_string[][8] =
{
    TEXT(""),						//VOW_NULL,
    TEXT("a"),						//VOW_A,
    TEXT("ai"),						//VOW_AI
    TEXT("an"),						//VOW_AN
    TEXT("ang"),					//VOW_ANG
    TEXT("ao"),						//VOW_AO
    TEXT("e"),						//VOW_E
    TEXT("ei"),						//VOW_EI
    TEXT("en"),						//VOW_EN
    TEXT("eng"),					//VOW_ENG
    TEXT("er"),						//VOW_ER
    TEXT("i"),						//VOW_I
    TEXT("ia"),						//VOW_IA
    TEXT("ian"),					//VOW_IAN
    TEXT("iang"),					//VOW_IANG
    TEXT("iao"),					//VOW_IAO
    TEXT("ie"),						//VOW_IE
    TEXT("in"),						//VOW_IN
    TEXT("ing"),					//VOW_ING
    TEXT("iong"),					//VOW_IONG
    TEXT("iu"),						//VOW_IU
    TEXT("o"),						//VOW_O
    TEXT("ong"),					//VOW_ONG
    TEXT("ou"),						//VOW_OU
    TEXT("u"),						//VOW_U
    TEXT("ua"),						//VOW_UA
    TEXT("uai"),					//VOW_UAI
    TEXT("uan"),					//VOW_UAN
    TEXT("uang"),					//VOW_UANG
    TEXT("ue"),						//VOW_UE
    TEXT("ui"),						//VOW_UI
    TEXT("un"),						//VOW_UN
    TEXT("uo"),						//VOW_UO
    TEXT("v"),						//VOW_V
    TEXT(""),						//VOW_END
    TEXT(""),						//VOW_ANY
};

//音调转换成字符串
static TCHAR tone_to_string[][6] =
{
/* 0 */		TEXT(""),		//没有音
/* 1 */		TEXT("1"),		//(1 << 0)	1
/* 2 */		TEXT("2"),		//(1 << 1)	2
/* 3 */		TEXT(""),
/* 4 */		TEXT("3"),		//(1 << 2)	3
/* 5 */		TEXT(""),
/* 6 */		TEXT(""),
/* 7 */		TEXT(""),
/* 8 */		TEXT("4"),		//(1 << 3)	4
};

/*	获得拼音串的音调。
 *	如果拼音串长度为0，则音调为全部包括。
 *	参数：
 *		pin_yin			拼音串
 *		length			串长度
 *	返回：
 *		TONE_ERROR		非法的音调串
 *		其他			音调标识
 */
int GetTone(const TCHAR *pin_yin)
{
    switch(*pin_yin)
    {
    case TONE_CHAR_1:
        return TONE_1;

    case TONE_CHAR_2:
        return TONE_2;

    case TONE_CHAR_3:
        return TONE_3;

    case TONE_CHAR_4:
        return TONE_4;
    }

    return TONE_0;
}

/*	获得拼音的音节标识。当前拼音作为完整的拼音进行检索，找出音节表达。
 *	此拼音串如果其中包含着错误（如：v、u、i开始）则肯定返回失败。
 *	如："wom"将返回错误。"pin4" -> 声母：CON_P，韵母：VOW_IN，音调：4声（10000B）。
 *
 *	由于部分拼音只能在模糊的方式下才是正确的，所以必须将模糊设置传递过来。
 *	如：cuang，在c、ch模糊的情况下是正确的发音。
 *
 *	参数：
 *		pin_yin					拼音串
 *		syllable				音节
 *		*syllable_str_length	音节使用的拼音长度，可以为0
 *		fuzzy_mode				模糊音选项
 *	返回：
 *		1				成功
 *		0				音节解析失败。如：vabcd等。
 */
int GetSyllable(const TCHAR *pin_yin, SYLLABLE *syllable, int *syllable_str_length, int fuzzy_mode/*, int *correct_flag, int *separator_flag*/)
{
    int hi, low, mid, ret;
    int py_length = (int)_tcslen(pin_yin);
    int has_tone = 0, has_separator = 0;	//是否有音调、音节分隔符
    TCHAR py[MAX_PINYIN_LENGTH + 1];		//备份拼音串

    if (!py_length || py_length > MAX_PINYIN_LENGTH)				//没有拼音串，则为非法音节
        return 0;

    //复制一份
    _tcscpy(py, pin_yin);

    //如果末尾为'，则为正确的音节
    if (py[py_length - 1] == SYLLABLE_SEPARATOR_CHAR)
    {
        py[py_length - 1] = 0;
        has_separator	  = 1;
        py_length--;

        if (!py_length)				//没有拼音串，则为非法音节
            return 0;
    }

    //通配符
    if (py_length == 1 && *py == SYLLABLE_ANY_CHAR)
    {
        syllable->con		 = CON_ANY;
        syllable->vow		 = VOW_ANY;
        syllable->tone		 = TONE_0;
        *syllable_str_length = 1;

        return 1;
    }

    //判断尾部是否有音调标志
    if (IS_TONE(py[py_length - 1]))
    {
        has_tone = 1;
        py_length--;

        if (!py_length)				//再次判断是否有拼音串
            return 0;
    }

    //二分法查找拼音
    low = 0;
    hi = share_segment.syllable_map_items - 1;
    while(low <= hi)
    {
        mid = (low + hi) / 2;

        //如果长度不相同，则肯定不能找到，必须进行完整拼音的比较
        //如：如果不进行完整比较men将与meng相同。
        //由于音调为数字，所以比较时不会产生错误。
        if (py_length != share_segment.syllable_map[mid].pin_yin_length)
            ret = _tcscmp(py, share_segment.syllable_map[mid].pin_yin);
        else
            ret = _tcsncmp(py, share_segment.syllable_map[mid].pin_yin, py_length);

        if (!ret)			//找到
            break;

        if (ret < 0)
            hi = mid - 1;
        else
            low = mid + 1;
    }

    if (low > hi)			//没有找到
        return 0;

    //如果该音需要在模糊的情况下才是有效的，则需要判断模糊标志是否设置
    if (share_segment.syllable_map[mid].fuzzy_flag && !(share_segment.syllable_map[mid].fuzzy_flag & fuzzy_mode))
        return 0;

    //如果该音需要在纠正的情况下才是有效的，则需要判断纠正标志是否设置
    //if (share_segment->syllable_map[mid].correct_flag && !(share_segment->syllable_map[mid].correct_flag & pim_config->correct_option))
    //	return 0;

    //存储结果
    syllable->con  = share_segment.syllable_map[mid].con;
    syllable->vow  = share_segment.syllable_map[mid].vow;
    syllable->tone = TONE_0;				//没有音调标识

    if (has_tone)
        syllable->tone = GetTone(py + share_segment.syllable_map[mid].pin_yin_length);

    if (syllable_str_length)
    {
        *syllable_str_length = share_segment.syllable_map[mid].pin_yin_length;

        if (has_tone)
            (*syllable_str_length)++;

        if (has_separator)
            (*syllable_str_length)++;
    }

    return 1;
}

int GetLegalPinYinLength(const TCHAR *pin_yin, int state, int english_state)
{
    int i;

    if (STATE_EDIT == state && ENGLISH_STATE_NONE == english_state)
    {
        for (i = 0; pin_yin[i]; i++)
        {
            if (pin_yin[i] >= 'A' && pin_yin[i] <= 'Z')
                break;
        }
    }
    else
        i = (int)_tcslen(pin_yin);

    return i;
}

/*	判断第一个音节参数所指示的声母集合是否包含第二个音节参数的声母。
 *	参数：
 *		syllable				源音节
 *		checked_syllable		被检查音节
 *		fuzzy_mode				模糊设置
 *	返回：
 *		1			包含
 *		0			不包含
 */
int ContainCon(SYLLABLE syllable, SYLLABLE checked_syllable, int fuzzy_mode)
{
//方便程序编写，建立两个宏。
#define	CHECK_FUZZY(fuzzy_bit, con_value)	((fuzzy_mode & fuzzy_bit) && (checked_syllable.con == con_value))

    if (syllable.con == checked_syllable.con)
        return 1;

    if (syllable.con == CON_ANY)
        return 1;

    //被检查的声母为空则为不包含
    if (checked_syllable.con == CON_NULL)
        return 0;

    switch(syllable.con)
    {
    case CON_Z:
        if (syllable.vow == VOW_NULL &&	CHECK_FUZZY(FUZZY_ZCS_IN_CI, CON_ZH))
            return 1;

        return CHECK_FUZZY(FUZZY_Z_ZH, CON_ZH);

    case CON_ZH:
        return CHECK_FUZZY(FUZZY_REV_Z_ZH, CON_Z);

    case CON_C:
        if (syllable.vow == VOW_NULL &&	CHECK_FUZZY(FUZZY_ZCS_IN_CI, CON_CH))
            return 1;

        return CHECK_FUZZY(FUZZY_C_CH, CON_CH);

    case CON_CH:
        return CHECK_FUZZY(FUZZY_REV_C_CH, CON_C);

    case CON_S:
        if (syllable.vow == VOW_NULL &&	CHECK_FUZZY(FUZZY_ZCS_IN_CI, CON_SH))
            return 1;

        return CHECK_FUZZY(FUZZY_S_SH, CON_SH);

    case CON_SH:
        return CHECK_FUZZY(FUZZY_REV_S_SH, CON_S);

    case CON_G:
        return CHECK_FUZZY(FUZZY_G_K, CON_K);

    case CON_K:
        return CHECK_FUZZY(FUZZY_REV_G_K, CON_G);

    case CON_F:		//由于F与H、HU两个音都可以模糊，所以要特殊处理
        if (CHECK_FUZZY(FUZZY_F_H, CON_H))		//如果F、H模糊，并且被检查声母为H，则为包含
            return 1;

        //模糊F与HU，为了能够使“湖南”可以通过"fn"查到，必须假定f = h，再由韵母进行判断。
        return (fuzzy_mode & FUZZY_F_HU) && IS_START_WITH_HU(checked_syllable);

    case CON_H:		//与CON_F同样的理由需要特殊处理
        if (CHECK_FUZZY(FUZZY_REV_F_H, CON_F))
            return 1;

        //如果被检查声母不是F或者没有设置F与HU的模糊，为不包含
        return ((fuzzy_mode & FUZZY_REV_F_HU) && IS_START_WITH_HU(syllable) && checked_syllable.con == CON_F);

    case CON_L:		//L可以与N、R两个音相互模糊，所以特殊处理
        if (CHECK_FUZZY(FUZZY_L_R, CON_R))		//L与R
            return 1;

        return CHECK_FUZZY(FUZZY_L_N, CON_N);	//L与N

    case CON_N:		//不检查N-L/L-R的映射。
        return CHECK_FUZZY(FUZZY_REV_L_N, CON_L);

    case CON_R:		//不检查R-L/L-N的映射
        return CHECK_FUZZY(FUZZY_REV_L_R, CON_L);
    }

    return 0;

//不再使用这两个宏
#undef	CHECK_FUZZY
}

/*	获得音节的拼音串。
 *	参数：
 *		syllable		音节
 *		str				输出串
 *		length			输入串的长度
 *		correct_flag	纠错标记
 *		tone_char		使用音调(注意不是使用syllable.h中的TONE_CHAR_1等字符，而是将拼音表示为SYLLABLEMAP中的带声调形式，如ā等)
 *	返回值：
 *		字符串长度，0标识没有字符。
 */
int GetSyllableString(SYLLABLE syllable, TCHAR *str, int length, /*int correct_flag, */int tone_char)
{
    TCHAR *con_str, *vow_str;
    TCHAR tones[10] = {0};
    TCHAR py_str[0x10] = {0};
    int count = 0;
    int i, j;

    //CON_ERROR，VOW_ERROR
    if (syllable.con < CON_NULL || syllable.vow < VOW_NULL || syllable.con > CON_ANY || syllable.vow > VOW_ANY)
    {
        *str++ = '?';
        *str++ = 0;
        return 1;
    }

    //非正常的拼音，直接转换，比如*
    if ((CON_NULL == syllable.con || CON_END == syllable.con || CON_ANY == syllable.con) &&
        (VOW_NULL == syllable.vow || VOW_END == syllable.vow || VOW_ANY == syllable.vow))
    {
        con_str = con_to_string[syllable.con];
        vow_str = vow_to_string[syllable.vow];

        while(*con_str && count < length - 1)
            str[count++] = *con_str++;

        while(*vow_str && count < length - 1)
            str[count++] = *vow_str++;
    }
    //正常的拼音，查表获取，主要是为了实现拼音容错
    else
    {
        str[0] = 0;

        for (i = 0; i < share_segment.syllable_map_items; i++)
        {
            if (syllable.con != share_segment.syllable_map[i].con ||
                syllable.vow != share_segment.syllable_map[i].vow/* ||
                correct_flag != share_segment->syllable_map[i].correct_flag*/)
                continue;

            for (j = 0; j <= 3; j++)
            {
                if (!(syllable.tone & (1 << j)))
                    continue;

                if (count++)
                    _tcscat_s(str, length, TEXT(","));

                if (tone_char)
                {
                    switch (j)
                    {
                    case 0:
                        _tcscat_s(str, length, share_segment.syllable_map[i].pin_yin_tone1);
                        break;

                    case 1:
                        _tcscat_s(str, length, share_segment.syllable_map[i].pin_yin_tone2);
                        break;

                    case 2:
                        _tcscat_s(str, length, share_segment.syllable_map[i].pin_yin_tone3);
                        break;

                    case 3:
                        _tcscat_s(str, length, share_segment.syllable_map[i].pin_yin_tone4);
                        break;
                    }
                }
                else
                {
                    _tcscat_s(str, length, share_segment.syllable_map[i].pin_yin);
                    _tcscat_s(str, length, tone_to_string[1 << j]);
                }
            }

            if (!count)
                _tcscpy_s(str, length, share_segment.syllable_map[i].pin_yin);

            count = (int)_tcslen(str);
            break;
        }
    }

    str[count] = 0;

    return count;
}
