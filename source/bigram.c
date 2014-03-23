/**	处理bigram函数组
 */
#include <icw.h>
#include <zi.h>
#include <ci.h>
#include <config.h>
#include <wordlib.h>
#include <assert.h>
#include <utility.h>
#include <malloc.h>
#include <math.h>

#include <string.h>
#include <gram.h>

static int GetCiIndex(GRAM_DATA *bigram, const char *ci)
{
	int ci_len;
	int start = 0;
	int end = bigram->header.index0_count - 1;
	int mid, pos, ret;
	GRAM_INDEX *index0 = GetGramIndex(bigram);
	char *word_list = GetGramWordList(bigram);

	ci_len = (int)strlen(ci);
	if (ci_len > MAX_SEG_WORD_LENGTH || ci_len < 2)		//长度错误
		return 0;

	//二分法查找
	while(start <= end)
	{
		mid = (start + end) / 2;
		pos = index0[mid].word_pos;
		ret = strcmp(ci, &word_list[pos]);

		if (!ret)
			return mid;

		if (ret < 0)
			end = mid - 1;
		else
			start = mid + 1;
	}

	return -1;		//没有找到
}

/**	获得bigram计数
 */
int GetBigramCount(GRAM_DATA *bigram_data, const char *ci1, const char *ci2)
{
	GRAM_INDEX *index0;
	GRAM_ITEM  *item;
	int ci1_index, ci2_index;
	int start, end, mid, ret, count;
	int start_sav, end_sav;

	//寻找词索引
	if ((ci1_index = GetCiIndex(bigram_data, ci1)) < 0)
		return 0;

	if ((ci2_index = GetCiIndex(bigram_data, ci2)) < 0)
		return 0;

	index0 = GetGramIndex(bigram_data);
	item   = GetGramItem(bigram_data);

	if (index0[ci1_index].item_index < 0)
		return 0;

	//在item区域进行搜索
	start = index0[ci1_index].item_index;
	if (ci1_index == bigram_data->header.index0_count - 1)		//最后一个
		end = bigram_data->header.item_count - 1;
	else
		end = index0[ci1_index + 1].item_index - 1;

	end_sav = end, start_sav = start;

	//再次使用二分法进行查找
	while (start <= end)
	{
		mid = (start + end) / 2;
		ret = item[mid].word_index - ci2_index;
		if (!ret)
			break;

		if (ret > 0)
			end = mid - 1;
		else
			start = mid + 1;
	}

	if (start > end)
		return 0;

	if (mid > start_sav && item[mid - 1].word_index == ci2_index)
		count = (item[mid].count << ONE_COUNT_BIT) | item[mid - 1].count;
	else if (mid < end_sav && item[mid + 1].word_index == ci2_index)
		count = (item[mid + 1].count << ONE_COUNT_BIT) | item[mid].count;
	else
		count = item[mid].count;

	return count;
}

/**	获得回退的概率估计
 */
double newGetBackOffProbability(GRAM_DATA *bigram_data, int index1, int index2)
{
	GRAM_INDEX *index0;
	GRAM_ITEM  *item;
	char *word_list;
	int freq1, freq2;
	int in_freq1;
	double value;
	double factor = 1.0;
	double back_off_weight = 1.0 - 1.0 * bigram_data->header.total_bigram_in_count / bigram_data->header.total_bigram_count;

	back_off_weight *= 2.0;

	word_list = GetGramWordList(bigram_data);
	index0	  = GetGramIndex(bigram_data);
	item	  = GetGramItem(bigram_data);

	freq1	 = index0[index1].word_freq;
	in_freq1 = index0[index1].start_count;
	freq2	 = index0[index2].word_freq;

	//在△之前如果需要回退并且是词频较高，说明不好作为语句的结尾。
	if (freq1 >= 4000)
	{
		if (freq2 > SJX_FREQ)					//△
			factor = 0.3;
	}

	value = 1.0 * freq2 / bigram_data->header.total_word_freq;// * (1.0 - 1.0 * in_freq1 / freq1);

	if (freq1 < SJX_FREQ && freq2 < SJX_FREQ &&	!word_list[index0[index2].word_pos + 2])
		value *= XM;

	return value * factor * back_off_weight;
}

/**	获得回退的概率估计
 */
double GetBackOffProbability(GRAM_DATA *bigram_data, int index1, int index2)
{
	GRAM_INDEX *index0;
	GRAM_ITEM  *item;
	char *word_list;
	int freq1, freq2;
	int in_freq1;
	double value;
	double factor = 1.0;
//	double back_off_weight = 0.65;
	double back_off_weight = 0.70;

	word_list = GetGramWordList(bigram_data);
	index0	  = GetGramIndex(bigram_data);
	item	  = GetGramItem(bigram_data);

	freq1	 = index0[index1].word_freq;
	in_freq1 = index0[index1].start_count;
	freq2	 = index0[index2].word_freq;

	//在△之前如果需要回退并且是词频较高，说明不好作为语句的结尾。
	if (freq1 >= 4000)
	{
		if (freq2 > SJX_FREQ)					//△
			factor = 0.08;
	}

	value = 1.0 * freq2 / bigram_data->header.total_word_freq * (1.0 - 1.0 * in_freq1 / freq1);
	factor *= pow(freq1, 1.0 / 32.0);

	if (freq1 < SJX_FREQ && freq2 < SJX_FREQ &&	!word_list[index0[index2].word_pos + 2])
//		value *= XM * 0.6;
		value *= XM * 0.5;

	return value * factor * back_off_weight;
}

/**	获得bigram概率
 */
double GetBigramValue(GRAM_DATA *bigram_data, const char *ci1, const char *ci2)
{
	char *word_list;
	GRAM_INDEX *index0;
	GRAM_ITEM  *item;
	double value;
	int ci1_index, ci2_index;
	int ci1_freq, ci2_freq;
	int start, end, mid, ret, count;
	int start_sav, end_sav;

	if (!bigram_data || !ci1 || !ci2)				//合法性检查
		return 1 / MAX_BCOUNT;

	index0	  = GetGramIndex(bigram_data);
	item	  = GetGramItem(bigram_data);
	word_list = GetGramWordList(bigram_data);

	//寻找词索引
	ci1_index = GetCiIndex(bigram_data, ci1);
	ci2_index = GetCiIndex(bigram_data, ci2);

	ci1_freq = ci2_freq = 0;
	if (ci1_index >= 0)
		ci1_freq = index0[ci1_index].word_freq;

	if (ci2_index >= 0)
		ci2_freq = index0[ci2_index].word_freq;

	if (ci1_index < 0 && ci2_index < 0)
		return 1.0 / MAX_BCOUNT;

	if (ci1_index < 0)					//第一个词没有找到
	{
		//直接返回ci2的估值
		value = 1.0	* ci2_freq / bigram_data->header.total_word_freq;

		if (!ci2[2] && *(short*)ci1 != *(short*)"△")
			value *= XM;

		return value;
	}

	if (ci2_index < 0)		//进行回退
	{
		//进行回退，跟下面代码中使用GetBackOffProbability进行回退的公式很相似，但
		//由于这里ci2不在wordlist中，所以不存在词频，因此将其词频设为1进行计算
		value = 1.0	/ bigram_data->header.total_word_freq * (ci1_freq - index0[ci1_index].start_count) / ci1_freq;

		if (!ci2[2] && *(short*)ci1 != *(short*)"△")
			value *= XM;

		return value;
	}

	//判断词频是否很低
	if (index0[ci2_index].word_freq < 500)
		return GetBackOffProbability(bigram_data, ci1_index, ci2_index);

	//在item区域进行搜索
	start = index0[ci1_index].item_index;

	if (ci1_index == bigram_data->header.index0_count - 1)		//最后一个
		end = bigram_data->header.item_count - 1;
	else
		end = index0[ci1_index + 1].item_index - 1;

	end_sav = end, start_sav = start;

	//再次使用二分法进行查找
	while (start <= end)
	{
		mid = (start + end) / 2;
		ret = item[mid].word_index - ci2_index;
		if (!ret)
			break;

		if (ret > 0)
			end = mid - 1;
		else
			start = mid + 1;
	}

	//此时没有找到，则需要根据词频进行回退。
	if (start > end)
		return GetBackOffProbability(bigram_data, ci1_index, ci2_index);

	//找到
	if (mid > start_sav && item[mid - 1].word_index == ci2_index)
		count = (item[mid].count << ONE_COUNT_BIT) | item[mid - 1].count;
	else if (mid < end_sav && item[mid + 1].word_index == ci2_index)
		count = (item[mid + 1].count << ONE_COUNT_BIT) | item[mid].count;
	else
		count = item[mid].count;

	value = 1.0 * count / ci1_freq;

	if (!ci1[2] && !ci2[2] && *(short*)ci1 != *(short*)"△" && *(short*)ci2 != *(short*)"△")				//两个都是单字，将频率大幅降低
		value *= XM;

	return value;
}

/**	获得bigram概率
 */
double newGetBigramValue(GRAM_DATA *bigram_data, const char *ci1, const char *ci2)
{
	GRAM_INDEX *index0;
	GRAM_ITEM  *item;
	double value;
	int ci1_index, ci2_index;
	int ci1_freq, ci2_freq;
	int start, end, mid, ret, count;
	int start_sav, end_sav;
	double back_off_weight = 1.0 - 1.0 * bigram_data->header.total_bigram_in_count / bigram_data->header.total_bigram_count;

	back_off_weight /= 1e6;

	if (!bigram_data || !ci1 || !ci2)				//合法性检查
		return 1 / MAX_BCOUNT;

	index0 = GetGramIndex(bigram_data);
	item   = GetGramItem(bigram_data);

	//寻找词索引
	ci1_index = GetCiIndex(bigram_data, ci1);
	ci2_index = GetCiIndex(bigram_data, ci2);

	ci1_freq = ci2_freq = 0;
	if (ci1_index >= 0)
		ci1_freq = index0[ci1_index].word_freq;

	if (ci2_index >= 0)
		ci2_freq = index0[ci2_index].word_freq;

	if (ci1_index < 0 && ci2_index < 0)
		return 1.0 / MAX_BCOUNT;

	if (ci1_index < 0)					//第一个词没有找到
	{
		//直接返回ci2的估值
		value = 1.0	* ci2_freq / bigram_data->header.total_word_freq;

		return value * back_off_weight;
	}

	if (ci2_index < 0)		//进行回退
	{
		value = 1.0 / bigram_data->header.total_word_freq;
		return value * back_off_weight;
	}

	//在item区域进行搜索
	start = index0[ci1_index].item_index;

	if (ci1_index == bigram_data->header.index0_count - 1)		//最后一个
		end = bigram_data->header.item_count - 1;
	else
		end = index0[ci1_index + 1].item_index - 1;

	end_sav = end, start_sav = start;

	//再次使用二分法进行查找
	while (start <= end)
	{
		mid = (start + end) / 2;
		ret = item[mid].word_index - ci2_index;
		if (!ret)
			break;

		if (ret > 0)
			end = mid - 1;
		else
			start = mid + 1;
	}

	//此时没有找到，则需要根据词频进行回退。
	if (start > end)
		return GetBackOffProbability(bigram_data, ci1_index, ci2_index);

	//找到
	if (mid > start_sav && item[mid - 1].word_index == ci2_index)
		count = (item[mid].count << ONE_COUNT_BIT) | item[mid - 1].count;
	else if (mid < end_sav && item[mid + 1].word_index == ci2_index)
		count = (item[mid + 1].count << ONE_COUNT_BIT) | item[mid].count;
	else
		count = item[mid].count;

	if (!ci1[2] && !ci2[2] && *(short*)ci1 != *(short*)"△" && *(short*)ci2 != *(short*)"△")				//两个都是单字，将频率大幅降低
		return 1.0 * count / ci1_freq * XM;

	return 1.0 * count / index0[ci1_index].start_count;//ci1_freq;
}

/**	对词表进行加密
 *	加密方法，循环右移7位，异或aa55
 */
void encode_word_list(char *word_list, int word_list_pos)
{
	int index = 0;
	unsigned short x;

	return;

	while(index < word_list_pos)
	{
		if (!word_list[index])
		{
			index++;
			continue;
		}

		x = *(unsigned short*)&word_list[index];
		x = ENCODE(x);
		*(unsigned short*)&word_list[index] = x;
		index += 2;
	}
}

void decode_word_list(char *word_list, int word_list_pos)
{
	int index = 0;
	unsigned short x;

	return;

	while(index < word_list_pos)
	{
		if (!word_list[index])
		{
			index++;
			continue;
		}

		x = *(unsigned short*)&word_list[index];
		x = DECODE(x);
		*(unsigned short*)&word_list[index] = x;
		index += 2;
	}
}