/**	汉语语料切分程序
*	采用正向
*	对于带有二义性的切分语句，输出到ami.txt文件中，用于
*	后期检查改进。
*/
#include <iostream>
#include <fstream>
#include <string.h>
#include <mmseg.h>

using namespace std;

mmseg::mmseg(const char *dict_name, const char *ambi_name)
{
	dict = new dictionary(dict_name);
#ifdef	OUTPUT_AMBI
	ambi_file.open(ambi_name);
#endif
}

mmseg::~mmseg()
{
#ifdef	OUTPUT_AMBI
	ambi_file.close();
#endif
	delete dict;
}

//查找第一个词的长度
int mmseg::get_word_length(const char *str, int length)
{
	return dict->max_match_length(str, length);
}

int mmseg::get_word_length(const char *str)
{
	return dict->max_match_length(str, (int)strlen(str));
}

/**	寻找分词的中间结果（三个连续词汇）
 */
int mmseg::get_mid_result(const char *str, int length, MID_RESULT result[])
{
	int i, j, k;
	int count;

	length = length / 2 * 2;
	memset(result, 0, MAX_MID_RESULTS * sizeof(MID_RESULT));

	count = 0;
	for (i = 2; i <= MAX_WORD_LENGTH && i <= length; i += 2)
	{
		if (!dict->is_word(str, i))
			continue;
		result[count].length0 = i;
		result[count].length1 = result[count].length2 = 0;
		result[count].count = 1;
		result[count].total_length = i;

		count++;
		if (count >= MAX_MID_RESULTS)
			return count;
		result[count] = result[count - 1];

		for (j = 2; j <= MAX_WORD_LENGTH && i + j <= length; j += 2)
		{
			if (!dict->is_word(str + i, j))
				continue;
			result[count].length1 = j;
			result[count].length2 = 0;
			result[count].count = 2;
			result[count].total_length = i + j;

			count++;
			if (count >= MAX_MID_RESULTS)
				return count;
			result[count] = result[count - 1];

//					for (k = min(MAX_WORD_LENGTH, length - i - j); k >= 2; k -= 2)
			for (k = 2; k <= MAX_WORD_LENGTH && i + j + k <= length; k +=2 )
			{
				if (!dict->is_word(str + i + j, k))
					continue;
				result[count].length2 = k;
				result[count].count = 3;
				result[count].total_length = i + j + k;

				count++;
				if (count >= MAX_MID_RESULTS)
					return count;

				result[count] = result[count - 1];

			}
		}
	}
	return count;
}

/**	寻找分词的中间结果（三个连续词汇）
 */
int mmseg::new_get_mid_result(const char *str, int length, MID_RESULT result[])
{
	int i, j;
	int count;
	int length0[0x10], count0;
	int length1[0x10], count1;
	int length2[0x10], count2;

	length = length / 2 * 2;
	memset(result, 0, MAX_MID_RESULTS * sizeof(MID_RESULT));

	int max_length = 0;
	count = 0;
	count0 = dict->get_words_length(str, length, length0);
	for (i = count0 - 1; i >= 0; i--)
	{
		count1 = dict->get_words_length(
			str + length0[i], 
			length - length0[i], 
			length1);

		if (count1)
		{
			for (j = count1 - 1; j >= 0; j--)
			{
				count2 = dict->get_words_length(
					str + length0[i] + length1[j], 
					length - length0[i] - length1[j],
					length2);

				if (count2)
				{
					if (length0[i] + length1[j] + length2[count2 - 1] >= max_length)
					{
						result[count].length0 = length0[i];
						result[count].length1 = length1[j];
						result[count].length2 = length2[count2 - 1];
						result[count].count = 3;
						result[count].total_length = result[count].length0 + result[count].length1 + result[count].length2;
						max_length = length0[i] + length1[j] + length2[count2 - 1];
						count++;
					}
				}
				else
				{
					if (length0[i] + length1[j] >= max_length)
					{
						result[count].length0 = length0[i];
						result[count].length1 = length1[j];
						result[count].length2 = 0;
						result[count].count = 2;
						result[count].total_length = result[count].length0 + result[count].length1;
						max_length = length0[i] + length1[j];
						count++;
					}
				}
			}

		}
		else
		{
			if (length0[i] >= max_length)
			{
				result[count].length0 = length0[i];
				result[count].length1 = result[count].length2 = 0;
				result[count].total_length = length0[i];
				result[count].count = 1;
				max_length = length0[i];
				count++;
			}
		}
	}
	return count;
}

void mmseg::output_result(MID_RESULT *result, int index, const char *str)
{
	cout << index << "\t";

	int i;
	for (i = 0; i < result[index].length0; i++)
		cout << str[i];
	cout << " ";

	for (i = 0; i < result[index].length1; i++)
		cout << str[result[index].length0 + i];
	cout << " ";

	for (i = 0; i < result[index].length2; i++)
		cout << str[result[index].length0 + result[index].length1 + i];
	cout << "\n";
}

/**	进行mm分词，返回第一个词的长度
 */
int mmseg::segment(const char *str, int length)
{
	MID_RESULT result[MAX_MID_RESULTS];
	int remains0[MAX_MID_RESULTS];
	int remains1[MAX_MID_RESULTS];
	int count, count_tmp, index;
	int i, max_avg_length, min_avg_diff2;
	double max_degree;

//	if (!strncmp(str, "汹涌澎湃", 8))
//		i = 5;

	if (!(str[0] & 0x80))	//非汉字
		return 0;

	//0. 获得中间结果
//			count = get_mid_result(str, length, result);
	count = new_get_mid_result(str, length, result);
//			for (i = 0; i < count; i++)
//				output_result(result, i, str);
	if (!count)
		return 0;

	if (count == 1)
		return result[0].length0;

	//1. 最长的结果判断（已经在获取过程中处理过了）
	//for (max_length = count_tmp = i = 0; i < count; i++)
	//{
	//	if (result[i].total_length > max_length)
	//	{
	//		max_length = result[i].total_length;
	//		count_tmp = 0;
	//	}
	//	if (result[i].total_length == max_length)
	//		remains0[count_tmp++] = i;
	//}

	//2. 平均长度最大值判断
	for (max_avg_length = count_tmp = i = 0; i < count; i++)
	{
		result[i].avg_length = result[i].total_length * 6 / result[i].count;
		if (result[i].avg_length > max_avg_length)
		{
			count_tmp = 0;
			max_avg_length = result[i].avg_length;
		}
		if (result[i].avg_length == max_avg_length)
			remains1[count_tmp++] = i;
	}

	count = count_tmp;
	if (count == 1)
		return result[remains1[0]].length0;

	//3. 长度均方差比较
	for (min_avg_diff2 = 65536, count_tmp = i = 0; i < count; i++)
	{
		index = remains1[i];
		result[index].avg_diff2 =
			(result[index].length0 * 6 - result[index].avg_length) * (result[index].length0 * 6 - result[index].avg_length);
		if (result[index].count > 1)
			result[index].avg_diff2 +=
				(result[index].length1 * 6 - result[index].avg_length) * (result[index].length1 * 6 - result[index].avg_length);
		if (result[index].count > 2)
			result[index].avg_diff2 +=
				(result[index].length2 * 6 - result[index].avg_length) * (result[index].length2 * 6 - result[index].avg_length);
		result[index].avg_diff2 /= result[index].count;

		if (result[index].avg_diff2 < min_avg_diff2)
		{
			min_avg_diff2 = result[index].avg_diff2;
			count_tmp = 0;
		}

		if (result[index].avg_diff2 == min_avg_diff2)
			remains0[count_tmp++] = index;
	}

	count = count_tmp;
	if (count == 1)
		return result[remains0[0]].length0;

	//4. 判断自由语素度
	for (max_degree = 1.0, count_tmp = i = 0; i < count; i++)
	{
		index = remains0[i];
		int length = 0;
		double degree = 1.0;

		if (result[index].length0 == 2)
			degree *= dict->get_free_morphemic(str);
		length += result[index].length0;

		if (result[index].length1 == 2)
			degree *= dict->get_free_morphemic(str + length);
		length += result[index].length1;

		if (result[index].length2 == 2)
			degree *= dict->get_free_morphemic(str + length);

		if (degree > max_degree)
		{
			max_degree = degree;
			count_tmp = 0;
		}

		if (degree == max_degree)
			remains1[count_tmp++] = index;
	}

	count = count_tmp;
	for (i = 0; i < count; i++)
		ambi_out(str, &result[remains1[0]]);

	return result[remains1[0]].length0;
}

int mmseg::segment(const char *str)
{
	return segment(str, (int)strlen(str));
}

/**	输出混淆数据
 */
void mmseg::ambi_out(const char *str, MID_RESULT *result)
{
#ifdef	OUTPUT_AMBI
	int i = 0;

	if (!output_ambi)
		return;

	for (i = 0; i < result->length0; i++)
		ambi_file.put(*str++);
	ambi_file.put(' ');

	for (i = 0; i < result->length1; i++)
		ambi_file.put(*str++);
	ambi_file.put(' ');

	for (i = 0; i < result->length2; i++)
		ambi_file.put(*str++);
	ambi_file << "\n";
#endif
}
