#include "wordlibutils.h"
#include "share_segment.h"
#include "config.h"


#include <iostream>
#include <winnt.h>
#include <fstream>
#include <tchar.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <minwindef.h>
#include <ShlObj.h>
#include <math.h>


int sp_fuzzy_map_items;
SPFUZZYMAPITEM sp_fuzzy_map[MAX_SYLLABLE_PER_INPUT];
/*	将文件装载到缓冲区。
 *	请注意：缓冲区的大小必须满足文件读取的要求，不能小于文件的长度。
 *	参数：
 *		file_name			文件全路径名称
 *		buffer				缓冲区
 *		buffer_length		缓冲区长度
 *	返回：
 *		成功：返回读取的长度，失败：-1
 */

int LoadFromFile(const TCHAR *file_name, void *buffer, int buffer_length)
{
    FILE *fd;
    int length;

    fd = _tfopen(file_name, TEXT("rb"));
    if (!fd)
    {
        return 0;
    }

    length = (int)fread(buffer, 1, buffer_length, fd);
    fclose(fd);

    if (length < 0)
    {
        return 0;
    }

    return length;
}

/*	将内存保存到文件。如果目标存在，则覆盖。
 *	参数：
 *		file_name			文件全路径名称
 *		buffer				缓冲区指针
 *		buffer_length		文件长度
 *	返回：
 *		成功：1，失败：0
 */
int SaveToFile(const TCHAR *file_name, void *buffer, int buffer_length)
{
    FILE *fd;
    int length;

    fd = _tfopen(file_name, TEXT("wb"));
    if (!fd)
    {
        TCHAR dir_name[MAX_PATH];
        int  i, index, ret;

        //可能需要创建目录
        //1. 寻找当前文件的目录
        index = 0;
        _tcscpy(dir_name, file_name);
        for (i = 0; dir_name[i]; i++)
            if (dir_name[i] == '\\')
                index = i;

        if (!index)
            return 0;

        dir_name[index] = 0;		//dir_name中包含有目录名字
        ret = SHCreateDirectoryEx(0, dir_name, 0);
        if (ret != ERROR_SUCCESS)
            return 0;

        //创建目录成功，再次打开
        fd = _tfopen(file_name, TEXT("wb"));
        if (!fd)
            return 0;
    }

    length = (int)fwrite(buffer, 1, buffer_length, fd);
    fclose(fd);

    if (length != buffer_length)
    {
        return 0;
    }
    return length;
}

/**	获得一行文本，并且除掉尾回车
 */
int GetLineFromFile(FILE *fr, TCHAR *line, int length)
{
    TCHAR *p = line, ch;

    ch = _fgettc(fr);
    if (WEOF == ch)
        return 0;

    while (length-- && ch != WEOF)
    {
        *p = ch;
        if (0xa == ch)
            break;

        ch = _fgettc(fr);

        if (0xd == *p && 0xa == ch)
            break;

        p++;
    }

    *p = 0;

    return 1;
}

/**	使字符串适合于缓冲区的大小，如果过大则截取在汉字的边界上
 */
void MakeStringFitLength(TCHAR *string, int length)
{
    int index, last_index;

    index = last_index = 0;
    while(string[index] && index < length)
    {
        index++;

        if (index >= length)
            break;

        last_index = index;
    }

    string[last_index] = 0;
}


/*	获得双拼的音节
 */
int GetSPSyllable(const TCHAR *pin_yin, SYLLABLE *syllable, int *syllable_str_length, int fuzzy_mode)
{
    int  i, max_length, index;
    int  syllable_length;
    int  py_length = (int)_tcslen(pin_yin);
    int  has_tone = 0, has_separator = 0;		//是否有音调、音节分隔符
    TCHAR py[MAX_PINYIN_LENGTH + 1];			//备份拼音串

    if (!py_length || py_length > 3)			//没有拼音串，则为非法音节
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
    int useHzTone = 1;
    ConfigItemStruct configInfo;
    if(Config::Instance()->GetConfigItemByJson("use_hz_tone",configInfo))
    {
        useHzTone = configInfo.itemCurrentIntValue;
    }

    if (useHzTone && IS_TONE(py[py_length - 1]))
    {
        has_tone = 1;
        py[py_length - 1] = 0;
        py_length--;

        if (!py_length)				//再次判断是否有拼音串
            return 0;
    }

    //优先匹配pin_yin_length最大的，但对于有多个匹配项它们的pin_yin_length
    //最大相同的情况，将其加入到sp_fuzzy_syllables(目前同一个双拼组合最多支
    //持4个音节模糊的情况，如蓝天双拼方案中的双拼组合dk，可以解析为den或dia
    //，即2个音节模糊)

    //查找拼音
    max_length = 0;
    index = -1;

    //找到第一个最大匹配项的位置
    for (i = 0; i < share_segment.syllable_map_items; i++)
    {
        if (!_tcscmp(py, share_segment.syllable_map[i].sp_pin_yin) &&
            max_length < share_segment.syllable_map[i].pin_yin_length)
        {
            max_length = share_segment.syllable_map[i].pin_yin_length;
            index = i;
        }
    }

    //没有找到
    if (index == -1)
        return 0;

    //理论上来说必然会进入if分支
    if (sp_fuzzy_map_items < MAX_SYLLABLE_PER_INPUT)
    {
        //把所有和上面找出的最大匹配项长度相等的匹配项(包括其自身)加入到sp_fuzzy_map
        for (i = 0; i < share_segment.syllable_map_items; i++)
        {
            //首次进入下面的if时必有i==index
            if (!_tcscmp(py, share_segment.syllable_map[i].sp_pin_yin) &&
                max_length == share_segment.syllable_map[i].pin_yin_length)
            {
                int count = sp_fuzzy_map[sp_fuzzy_map_items].sp_fuzzy_syllable_count;
                if (count < MAX_SP_FUZZY_SYLLABLE_COUNT)
                {
                    sp_fuzzy_map[sp_fuzzy_map_items].sp_fuzzy_syllables[count].con = share_segment.syllable_map[i].con;
                    sp_fuzzy_map[sp_fuzzy_map_items].sp_fuzzy_syllables[count].vow = share_segment.syllable_map[i].vow;
                    sp_fuzzy_map[sp_fuzzy_map_items].sp_fuzzy_syllables[count].tone = TONE_0;
                    sp_fuzzy_map[sp_fuzzy_map_items].syllable_map_index[count] = i;
                    sp_fuzzy_map[sp_fuzzy_map_items].sp_fuzzy_syllable_count++;
                }
            }
        }

        for (i = 0; i < sp_fuzzy_map[sp_fuzzy_map_items].sp_fuzzy_syllable_count; i++)
        {
            index = sp_fuzzy_map[sp_fuzzy_map_items].syllable_map_index[i];

            //如果该音需要在模糊的情况下才是有效的，则需要判断模糊标志是否设置，若未设置将其从sp_fuzzy_syllables中删除
            if (share_segment.syllable_map[index].fuzzy_flag && !(share_segment.syllable_map[index].fuzzy_flag & fuzzy_mode))
            {
                if (i < sp_fuzzy_map[sp_fuzzy_map_items].sp_fuzzy_syllable_count - 1)
                {
                    memmove(sp_fuzzy_map[sp_fuzzy_map_items].sp_fuzzy_syllables + i,
                        sp_fuzzy_map[sp_fuzzy_map_items].sp_fuzzy_syllables + i + 1,
                        sp_fuzzy_map[sp_fuzzy_map_items].sp_fuzzy_syllable_count - i - 1);
                    memmove(sp_fuzzy_map[sp_fuzzy_map_items].syllable_map_index + i,
                        sp_fuzzy_map[sp_fuzzy_map_items].syllable_map_index + i + 1,
                        sp_fuzzy_map[sp_fuzzy_map_items].sp_fuzzy_syllable_count - i - 1);
                }

                sp_fuzzy_map[sp_fuzzy_map_items].sp_fuzzy_syllable_count--;
                i--;
            }
        }

        if (!sp_fuzzy_map[sp_fuzzy_map_items].sp_fuzzy_syllable_count)
            return 0;

        //如果存在多个合理的匹配项，设置解析出的首个为解析结果，其余作为模糊音
        i = sp_fuzzy_map[sp_fuzzy_map_items].syllable_map_index[0];

        sp_fuzzy_map_items++;
    }
    else
    {
        //如果该音需要在模糊的情况下才是有效的，则需要判断模糊标志是否设置
        if (share_segment.syllable_map[index].fuzzy_flag && !(share_segment.syllable_map[index].fuzzy_flag & fuzzy_mode))
            return 0;

        i = index;
    }


    //存储结果
    syllable->con  = share_segment.syllable_map[i].con;
    syllable->vow  = share_segment.syllable_map[i].vow;
    syllable->tone = TONE_0;			//没有音调标识

    syllable_length = (int)_tcslen(share_segment.syllable_map[i].sp_pin_yin);
    if (has_tone)
        syllable->tone = GetTone(pin_yin + syllable_length);

    if (syllable_str_length)
    {
        *syllable_str_length = syllable_length;

        if (has_tone)
            (*syllable_str_length)++;

        if (has_separator)
            (*syllable_str_length)++;
    }

    return 1;
}

/*	带有长度的复制字符串（为了进行移植的需要）。
 *	参数：
 *		target				目标地址
 *		source				源地址
 *	返回：无
*/
void CopyPartString(TCHAR *target, const TCHAR *source, int length)
{
    int i;

    for (i = 0; i < length && source[i]; i++)
        target[i] = source[i];

    target[i] = 0;
}

/*	对双拼串进行解析的递归过程
 */
static int ProcessParseSPInternal(const TCHAR *pin_yin, SYLLABLE *syllables, int array_length, int fuzzy_mode)
{
    int i, ret;
    int py_length;									//拼音串长度
    int syllable_string_length;						//音节字符串长度
    TCHAR cur_py[MAX_PINYIN_LENGTH + 1];			//当前处理的临时拼音串

    assert(pin_yin && syllables);

    //跳过音节切分符号
    if (*pin_yin == SYLLABLE_SEPARATOR_CHAR)
        pin_yin++;

    //减少递归次数直接发现错误。
    if (IS_TONE(*pin_yin))
        return 0;

    py_length = (int)_tcslen(pin_yin);

    for (i = fmin(3, py_length); i > 0; i--)
    {
        //复制第一个可能的音节
        CopyPartString(cur_py, pin_yin, i);

        //查找合法的拼音
        if (!GetSPSyllable(cur_py, syllables, &syllable_string_length, fuzzy_mode))
            continue;			//没有找到，减少一个音符，再次检索

        //没有需要检索的输入拼音了，则结束递归 || 没有空余的音节数组空间
        if (py_length == i || array_length == 1)
            return 1;

        //找到第一个音节，查找剩余音节
        ret = ProcessParseSPInternal(pin_yin + syllable_string_length, syllables + 1, array_length - 1, fuzzy_mode);
        if (!ret)
            continue;			//没有合法的音节，如：li，如果第一个为l则永远没有合法的音节

        //现在获得了整个解
        return ret + 1;
    }

    //没有找到解
    return 0;
}

/*	对双拼串进行解析
 *  由于sp_fuzzy_map的清0针对的是整个拼音串，而不是单个音节，
 *  因此不能放在递归函数里，需要把ProcessParseSP里的递归部分
 *  单独构成一个函数ProcessParseSPInternal
 */
static int ProcessParseSP(const TCHAR *pin_yin, SYLLABLE *syllables, int array_length, int fuzzy_mode)
{
    //在解析双拼拼音之前，把必要的数据结构清0
    memset(sp_fuzzy_map, 0, sizeof(sp_fuzzy_map));
    sp_fuzzy_map_items = 0;

    return ProcessParseSPInternal(pin_yin, syllables, array_length, fuzzy_mode);
}

/*	用递归的方式正向解析拼音串。
 *	参数：
 *		pin_yin				拼音串
 *		*syllables			音节存储数组
 *		array_length		音节数组长度
 *		fuzzy_mode			模糊设置
 *	返回值：
 *		0：没有解析出正确的结果
 *		> 0：解析出的音节数目
 */
static int ProcessParse(const TCHAR *pin_yin, SYLLABLE *syllables, int array_length, int fuzzy_mode/*, int *correct_flag, int *separator_flag*/)
{
    int i, ret;
    int py_length;									//拼音串长度
    int syllable_string_length;						//音节字符串长度
    TCHAR cur_py[MAX_PINYIN_LENGTH + 1];			//当前处理的临时拼音串
    TCHAR new_pin_yin[MAX_INPUT_LENGTH + 0x10];

    assert(pin_yin && syllables);

    //跳过音节切分符号
    if (*pin_yin == SYLLABLE_SEPARATOR_CHAR)
        pin_yin++;

    //减少递归次数直接发现错误。
    if (*pin_yin == 'i' || *pin_yin == 'u' || *pin_yin == 'v')
        return 0;

    if (IS_TONE(*pin_yin))
        return 0;

    //py_length = (int)_tcslen(pin_yin);
    py_length = GetLegalPinYinLength(pin_yin, STATE_EDIT, ENGLISH_STATE_NONE);
    //如果拼音字符长度大于最大长度限制，则退出
    if ( py_length > _SizeOf(new_pin_yin) )
        return 0;
    _tcsncpy_s(new_pin_yin, _SizeOf(new_pin_yin), pin_yin, py_length);

    for (i = min(MAX_PINYIN_LENGTH, py_length); i > 0; i--)
    {
        //复制第一个可能的音节
        CopyPartString(cur_py, new_pin_yin, i);

        //查找合法的拼音
        if (!GetSyllable(cur_py, syllables, &syllable_string_length, fuzzy_mode/*, correct_flag, separator_flag*/))
            continue;			//没有找到，减少一个音符，再次检索

        //没有需要检索的输入拼音了，则结束递归 || 没有空余的音节数组空间
        if (py_length == i || array_length == 1)
            return 1;

        //找到第一个音节，查找剩余音节
        ret = ProcessParse(new_pin_yin + syllable_string_length,
                           syllables + 1,
                           array_length - 1,
                           fuzzy_mode//,
                           /*correct_flag ? correct_flag + 1 : 0,*/
                           /*separator_flag ? separator_flag + 1 : 0*/);

        if (!ret)
            continue;			//没有合法的音节，如：li，如果第一个为l则永远没有合法的音节

        //现在获得了整个解
        return ret + 1;
    }

    //没有找到解
    return 0;
}

/*	用递归的方式反向解析拼音串。
 *	参数：
 *		pin_yin				拼音串
 *		*syllables			音节存储数组
 *		array_length		音节数组长度
 *		fuzzy_mode			模糊设置
 *	返回值：
 *		0：没有解析出正确的结果
 *		> 0：解析出的音节数目
 */
static int ProcessParseReverse(const TCHAR *pin_yin, SYLLABLE *syllables, int array_length, int fuzzy_mode/* ,int *correct_flag, int *separator_flag*/)
{
    int i, ret;
    int py_length;													//拼音串长度
    int syllable_string_length;										//音节字符串长度
    TCHAR cur_py[MAX_PINYIN_LENGTH * MAX_SYLLABLE_PER_INPUT + 1];	//当前处理的临时拼音串
    TCHAR new_pin_yin[MAX_INPUT_LENGTH + 0x10];

    assert(pin_yin && syllables);

    //跳过音节切分符号
    if (*pin_yin == SYLLABLE_SEPARATOR_CHAR)
        pin_yin++;

    //减少递归次数直接发现错误。
    if (*pin_yin == 'i' || *pin_yin == 'u' || *pin_yin == 'v')
        return 0;

    if (IS_TONE(*pin_yin))
        return 0;

    //py_length = (int)_tcslen(pin_yin);

    py_length = GetLegalPinYinLength(pin_yin, STATE_EDIT, ENGLISH_STATE_NONE);
    //如果拼音字符长度大于最大长度限制，则退出
    if ( py_length > _SizeOf(new_pin_yin) )
        return 0;
    _tcsncpy_s(new_pin_yin, _SizeOf(new_pin_yin), pin_yin, py_length);

    for (i = min(MAX_PINYIN_LENGTH, py_length); i > 0; i--)
    {
        //查找合法的拼音
        if (!GetSyllable(new_pin_yin + py_length - i, syllables, &syllable_string_length, fuzzy_mode/*, correct_flag, separator_flag*/))
            continue;			//没有找到，减少一个音符，再次检索

        //没有需要检索的输入拼音了，则结束递归 || 没有空余的音节数组空间
        if (py_length == i || array_length == 1)
            return 1;

        //复制第一个可能的音节
        CopyPartString(cur_py, new_pin_yin, py_length - i);

        //找到第一个音节，查找剩余音节
        ret = ProcessParseReverse(cur_py,
                                  syllables + 1,
                                  array_length - 1,
                                  fuzzy_mode//,
                                  /*correct_flag ? correct_flag + 1 : 0, */
                                  /*separator_flag ? separator_flag + 1 : 0*/);

        if (!ret)
            continue;			//没有合法的音节，如：li，如果第一个为l则永远没有合法的音节

        //现在获得了整个解
        return ret + 1;
    }

    //没有找到解
    return 0;
}

/*	判断拼音串是否为合法的。
 *	参数：
 *		pin_yin				拼音串
 *	返回值：
 *		1：合法
 *		0：非法
 */
static int LegalPinYin(const TCHAR *pin_yin)
{
    int i, nlen = GetLegalPinYinLength(pin_yin, STATE_EDIT, ENGLISH_STATE_NONE);

    if (!pin_yin)
        return 0;

    //进行比较粗浅的合法性检查，主要集中与输入串中是否包含不合法的
    //字符以及uu、vv、ii、uv、vi、vu。
    //该检查的目的在于避免递归分析数据的时间过长。
    //v的合法前缀：jlnqxy
    //u的非法前缀：{NUL}aeuv{TONE}
    //i的非法前缀：{NUL}fgikovw{TONE}		h需要特殊处理，shi,zhi,chi合法。
    //TONE的非法前缀：{NUL}{TONE}{SEPARATOR}
    //SEPARATOR的非法前缀：{NUL}{SEPARATOR}

    //打头的应该是a-z，并且不是uvi
    if (*pin_yin < 'a' || *pin_yin > 'z' ||	*pin_yin == 'u' || *pin_yin == 'v' || *pin_yin == 'i')
        return 0;

    for (i = 1; pin_yin[i] && i < nlen; i++)
    {
        //拼音串中出现非法的字符
        if (!((pin_yin[i] >= 'a' && pin_yin[i] <= 'z') || (pin_yin[i] == SYLLABLE_SEPARATOR_CHAR) ||
              (pin_yin[i] == SYLLABLE_ANY_CHAR) || IS_TONE(pin_yin[i])))
            return 0;
        ConfigItemStruct configInfo;
        int ci_option = 0;
        int use_hz_tone = 0;
        switch(pin_yin[i])
        {
        case SYLLABLE_ANY_CHAR:


            if(Config::Instance()->GetConfigItemByJson("ci_option",configInfo))
            {
                ci_option = configInfo.itemCurrentIntValue;
            }
            if (!(ci_option & CI_WILDCARD))
                return 0;

            if (pin_yin[i - 1] == SYLLABLE_ANY_CHAR)
                return 0;

            break;

        case SYLLABLE_SEPARATOR_CHAR:
            if (pin_yin[i - 1] == SYLLABLE_SEPARATOR_CHAR)
                return 0;

            break;

        case 'v':
            {
                int wrong = 1;
                switch(pin_yin[i - 1])
                {
                case  'j':	case  'l':	case  'n':	case  'q':	case  'x':	case  'y':
                    wrong = 0;
                }

                if (wrong)
                    return 0;
            }
            break;

        case 'u':
            switch(pin_yin[i - 1])
            {
            case 'a':	case 'e':	case 'v':	case 'u':
                return 0;
            }

            break;

        case 'i':
            switch(pin_yin[i - 1])
            {
            case 'f':
                if (pin_yin[i + 1] == 'a' && pin_yin[i + 2] == 'o')
                    break;

                return 0;

            case 'g':	case 'i':	case 'o':	case 'v':	case 'w':
                return 0;

            case 'h':
                if (i < 2 || (pin_yin[i - 2] != 's' && pin_yin[i - 2] != 'c' &&	pin_yin[i - 2] != 'z'))
                    return 0;

                break;

            default:
                if (IS_TONE(pin_yin[i - 1]))
                    return 0;
            }

            break;

        case TONE_CHAR_1:
        case TONE_CHAR_2:
        case TONE_CHAR_3:
        case TONE_CHAR_4:

            if(Config::Instance()->GetConfigItemByJson("use_hz_tone",configInfo))
            {
                use_hz_tone = configInfo.itemCurrentIntValue;
            }
            if (!use_hz_tone)
                return 0;

            if (IS_TONE(pin_yin[i - 1]))
                return 0;

            if (pin_yin[i - 1] == SYLLABLE_SEPARATOR_CHAR || pin_yin[i - 1] == SYLLABLE_ANY_CHAR)
                return 0;

            break;
        }
    }

    return 1;
}

/*	解析拼音串到音节数组。采用反向最大匹配的方式进行。
 *	参数：
 *		pin_yin				拼音串（可能包含音节分隔符）
 *		*syllable_array		解析后的音节数组，以0为标识
 *		syllable_length		音节数组长度
 *		fuzzy_mode			模糊音设置
 *		pinyin_mode			全拼、双拼
 *	返回：
 *		音节的数目，等于0的时候一般为错误输入
 */
int ParsePinYinStringReverse(const TCHAR *pin_yin, SYLLABLE *syllables, int array_length, int fuzzy_mode, int pinyin_mode/* ,int *correct_flag,int *separator_flag*/)
{
    int i;
    int count, other_count;		//返回音节数目
    SYLLABLE tmp_syllable;		//交换临时音节
    SYLLABLE tmp_syllables[MAX_SYLLABLE_PER_INPUT];

    if (pinyin_mode == PINYIN_SHUANGPIN)
        return ProcessParseSP(pin_yin, syllables, array_length, fuzzy_mode);

    //全拼
    if (!LegalPinYin(pin_yin))
        return 0;

    //解析，采用两种解析的方法进行，哪一种的音节数目少使用哪一个，如果相同，使用反向的。
    count = ProcessParseReverse(pin_yin, syllables, array_length, fuzzy_mode/*, correct_flag, separator_flag*/);
    other_count = ProcessParse(pin_yin, tmp_syllables, array_length, fuzzy_mode/*, tmp_correct_flags, tmp_separator_flags*/);

    if ((other_count < count)) //IsSpecialPinYinStr(pin_yin))		//正向的结果好
    {
        for (i = 0; i < other_count; i++)
        {
            syllables[i] = tmp_syllables[i];
        }

        return other_count;
    }

    //反向的结果好，将解析音节数组倒序
    for (i = 0; i < count / 2; i++)
    {
        tmp_syllable			 = syllables[i];
        syllables[i]			 = syllables[count - 1 - i];
        syllables[count - 1 - i] = tmp_syllable;
    }

    return count;
}

/*	依据词库标识，获得内存中词库的指针。
 *	参数：
 *		wordlib_id			词库标识
 *	返回：
 *		找到：词库指针
 *		未找到：0
 */
WORDLIB *GetWordLibrary(int wordlib_id)
{
    return wordlib_buffer[wordlib_id];
}

/*	获得词库的内存大小（用于保存词库数据）
 *	参数：
 *		wordlib				词库指针
 *	返回：
 *		词库的大小（头加上页长度）
 */
int GetWordLibSize(WORDLIB *wordlib)
{
    assert(wordlib);

    return sizeof(wordlib->header_data) + wordlib->header.page_count * WORDLIB_PAGE_SIZE;
}

/*	判断文件是否存在。
 */
int FileExists(const TCHAR *file_name)
{
    if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(file_name))
        return 0;

    return 1;
}

/**	基于词长以及音节长度确定词项的长度
 */
int GetItemLength(int hz_length, int syllable_length)
{
    return sizeof(HZ) * hz_length + sizeof(SYLLABLE) * syllable_length + WORDLIB_FEATURE_LENGTH;
}

/**	在词库中建新页。
 *	参数：
 *		wordlib_id				词库句柄
 *	返回：
 *		成功创建：页号
 *		失败：-1
 */
int NewWordLibPage(WORDLIB* wordlib)
{
    //int	length = share_segment->wordlib_length[wordlib_id];		//词库的总长度
    int length = 10485760;//词库总长度没确定，暂时定为10485760=10M
    int	new_length, new_page_no;								//新的词库长度、新页号

    if (!wordlib)			//没有这个词库
        return -1;

    //计算当前词库的Size是否已经到达词库的边界
    new_length = sizeof(wordlib->header_data) +							//词库头
                 wordlib->header.page_count * WORDLIB_PAGE_SIZE + 		//页数据长度
                 WORDLIB_PAGE_SIZE;										//新页数据长度

    if (new_length > length)			//超出内存边界，无法分配
        return -1;

    //对页初始化
    new_page_no = wordlib->header.page_count;

    wordlib->pages[new_page_no].data_length  = 0;
    wordlib->pages[new_page_no].next_page_no = PAGE_END;
    wordlib->pages[new_page_no].page_no		 = new_page_no;
    wordlib->pages[new_page_no].length_flag  = 0;

    wordlib->header.page_count++;

    return new_page_no;
}

/**	获得词汇的汉字指针
 */
HZ *GetItemHZPtr(WORDLIBITEM *item)
{
    return (HZ*)((char*)item + sizeof(int) + sizeof(SYLLABLE) * item->syllable_length);
}

/*	获得下一条词的指针。
 *	参数：
 *		item		当前词指针
 *	返回：
 *		下一条词的指针
 */
WORDLIBITEM *GetNextCiItem(WORDLIBITEM *item)
{
    //feature + syllable + hz
    return (WORDLIBITEM*)((char*)item +	sizeof(int) + sizeof(SYLLABLE) * item->syllable_length + sizeof(HZ) * item->ci_length);
}

/*	比较一组音节，进行通配符的比较，不进行音调比较。
 *	检查音节可能含有通配符，但被检查音节中不包括这些。
 *	不应有连续两个以上的通配符。
 *	参数：
 *		syllables					比较的音节
 *		length						比较音节长度
 *		checked_syllables			被比较音节
 *		checked_length				被比较音节的长度
 *		fuzzy_mode					模糊方式
 *	返回：
 *		相符：1
 *		不相符：0
 */
int WildCompareSyllables(SYLLABLE *syllables, int length, SYLLABLE *checked_syllables, int checked_length, int fuzzy_mode)
{
    int i, j;

    //被比较音节长度大于等于比较音节
    if (length > checked_length)
        return 0;

    for (i = 0; i < length; i++)
    {
        if (syllables[i].con != CON_ANY)		//不是通配符
        {
//            if (!ContainSyllable(syllables[i], checked_syllables[i], fuzzy_mode))
//                return 0;		//不匹配
            continue;
        }

        //通配符
        if (i == length - 1)		//最后一个是通配符，肯定匹配
            return 1;

        //需要使用递归来处理
        for (j = i + 1; j < checked_length; j++)
        {
            if (WildCompareSyllables(
                syllables + i + 1,				//跳过通配符
                length - i - 1,					//长度减少
                checked_syllables + j,			//跳过通配符
                checked_length - j,				//长度减少
                fuzzy_mode))
                return 1;
        }
        break;
    }

    //最后的比较必须加上比较程度等于被比较长度，否则会
    //出现部分匹配，如：zg*j  与 zgrmjcy相互匹配。
    return i == length && length == checked_length;
}



/*	获得页中的词候选（使用通配符）。
 *	参数：
 *		page					页指针
 *		syllable_array			音节序列
 *		syllable_count			音节长度
 *		candidate_array			候选数组
 *		candidate_length		候选数组长度
 *		fuzzy_mode				模糊音选项
 *	返回：
 *		候选词汇数目
 */
int GetCiInPageWild(PAGE *page,	SYLLABLE *syllable_array, int syllable_count, CANDIDATE *candidate_array, int candidate_length,	int fuzzy_mode)
{
    WORDLIBITEM *item;				//词项
    int count = 0;//候选计数
    ConfigItemStruct configInfo;
    int ci_option = 0;
    if(Config::Instance()->GetConfigItemByJson("ci_option",configInfo))
    {
        ci_option = configInfo.itemCurrentIntValue;
    }
    if (!(ci_option & CI_WILDCARD))
        return 0;

    if (!candidate_length)
        return 0;

    //如果页中没有包含这个长度的词，则直接返回（为提高检索效率）
    if ((unsigned int)page->length_flag < (unsigned int)(1 << syllable_count))
    {
        if (page->length_flag != 1)		//1 << 32 = 1, 所以必须加这个判断
            return 0;
    }

    //遍历页表
    for (item = (WORDLIBITEM*) page->data; (char*)item < (char*) &page->data + page->data_length; item = GetNextCiItem(item))
    {
        if (!(fuzzy_mode & FUZZY_CI_SYLLABLE_LENGTH))
            if (item->ci_length != item->syllable_length)
                continue;

        //判断长度是否符合
        if ((int)item->syllable_length < syllable_count)
            continue;

        //判断音节是否符合
        if (!WildCompareSyllables(syllable_array, syllable_count, item->syllable, item->syllable_length, fuzzy_mode))
            continue;

        //找到!
        candidate_array[count].word.type	  = CI_TYPE_NORMAL;
        candidate_array[count].word.item	  = item;
        candidate_array[count].word.cache_pos = 0;
        candidate_array[count].word.syllable  = (SYLLABLE*)((char*)item + sizeof(int));
        candidate_array[count].word.hz		  = GetItemHZPtr(item);

        count++;
        if (count >= candidate_length)
            break;
    }

    return count;
}

/*	比较一组音节，不进行通配符的比较，不进行音调比较。
 *	注：不能有两个以上的通配符。
 *	参数：
 *		syllables					比较的音节
 *		checked_syllables			被比较音节
 *		length						比较音节长度
 *		fuzzy_mode					模糊方式
 *	返回：
 *		相符：1
 *		不相符：0
 */
int CompareSyllables(SYLLABLE *syllables, SYLLABLE *checked_syllables, int length, int fuzzy_mode)
{
    Q_UNUSED(fuzzy_mode);
    Q_UNUSED(checked_syllables);
    Q_UNUSED(syllables);
    Q_UNUSED(length);

    return 1;
}

/*	获得页中的词候选（没有通配符）。
 *	参数：
 *		page					页指针
 *		syllable_array			音节序列
 *		syllable_count			音节长度
 *		candidate_array			候选数组
 *		candidate_length		候选数组长度
 *		fuzzy_mode				模糊音选项
 *	返回：
 *		候选词汇数目
 */
int GetCiInPage(PAGE *page,	SYLLABLE *syllable_array, int syllable_count, CANDIDATE *candidate_array, int candidate_length,	int fuzzy_mode)
{
    WORDLIBITEM *item;				//词项
    int count = 0;					//候选计数

    if (!candidate_length)
        return 0;

    //如果页中没有包含这个长度的词，则直接返回（为提高检索效率）
    if (!(page->length_flag & (1 << syllable_count)))
        return 0;

    //遍历页表
    for (item = (WORDLIBITEM*) page->data; (char*)item < (char*) &page->data + page->data_length; item = GetNextCiItem(item))
    {
        if (!(fuzzy_mode & FUZZY_CI_SYLLABLE_LENGTH))
            if (item->ci_length != item->syllable_length)
                continue;

        //判断长度是否符合
        if (item->syllable_length != syllable_count)
            continue;

        //判断音节是否符合
        if (!CompareSyllables(syllable_array, item->syllable, syllable_count, fuzzy_mode))
            continue;

        //找到!
        candidate_array[count].word.type	  = CI_TYPE_NORMAL;
        candidate_array[count].word.item	  = item;
        candidate_array[count].word.cache_pos = 0;
        candidate_array[count].word.syllable  = (SYLLABLE*)((char*)item + sizeof(int));
        candidate_array[count].word.hz		  = GetItemHZPtr(item);

        count++;
        if (count >= candidate_length)
            break;
    }

    return count;
}
/*	获得文件长度。
 *	参数：
 *		file_name			文件名称
 *	返回：
 *		文件长度，-1标识出错。
 */
int GetFileLength(const TCHAR *file_name)
{
    struct _stat f_data;

    if (_tstat(file_name, &f_data))
        return -1;

    return (int) f_data.st_size;
}

