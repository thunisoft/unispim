/*	智能组词模块
 *
 *	智能组词的改进
 *	1. 估值函数中包含词频信息，现在去掉
 *	2. 用户自造词的词频加入到估值函数中
 *	3. 将公式修改为更简洁的方式
 *	4. 用动态规划法进行编写算法
 *	5. 组成ICW的词条越短越好，用除法进行操作
 *
 *
 *	TCOC: 最小词频：4096，对数放大倍数32
 */
#include <icw.h>
#include <gram.h>
#include <zi.h>
#include <ci.h>
#include <config.h>
#include <utility.h>
#include <map_file.h>

int	show_icw_info = 0;
FILEMAPHANDLE	bigram_handle;
GRAM_DATA		*bigram_data;
int				bigram_data_length = 0;

typedef struct tagNEWICWITEM
{
	int			length;						//候选项的长度
	HZ			*hz;						//汉字
	SYLLABLE	*syllable;					//音节
	int			freq;						//字频或者词频
	double		value;						//估值
	struct tagNEWICWITEM	*next;			//下一项
}NEWICWITEM;

typedef struct tagICWLIFE
{
	double	value;
	const NEWICWITEM *item[MAX_ICW_LENGTH];
}ICWLIFE;

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


static int show_detail_bigram = 0;

void OutputLife(ICWITEMSET *icw_items, ICWLIFE *life)
{
	int i = 0, j;
	void EvaluateLifeByBigram(const ICWITEMSET*, ICWLIFE*);

	printf("%10g\t", life->value);

	while(i < icw_items->group_count)
	{
		for (j = 0; j < life->item[i]->length; j++)
			OutputHz(life->item[i]->hz[j]);

		printf(".");
		i += life->item[i]->length;
	}

	printf("\t");
	show_detail_bigram = 1;
	EvaluateLifeByBigram(icw_items, life);
	show_detail_bigram = 0;

	printf("\n");
}

int ci_option_save = 0;

void SaveCiOption()
{
	ci_option_save = pim_config->ci_option;
	pim_config->ci_option &= ~CI_AUTO_VOW_FUZZY;
}

void RestoreCiOption()
{
	pim_config->ci_option = ci_option_save;
}

/**	装载bigram数据
 */
int LoadBigramData(const TCHAR *name)
{
	if (bigram_data)			//已经装载
		return 1;

	bigram_handle = FileMapOpen(name);
	if (!bigram_handle)
		return 0;

	if ((bigram_data_length = FileMapGetBuffer(bigram_handle, (char**)&bigram_data, 0)) < 0)
		return 0;

	decode_word_list(GetGramWordList(bigram_data), bigram_data->header.word_list_size);

	return 1;
}

/**	加快bigram装载速度
 */
int MakeBigramFaster()
{
	int i, x;
	int *data;

	data = (int*)bigram_data;
	if (!data)
		return 0;

	for (x = 0, i = 0; i < bigram_data_length / 4; i++)
		x += data[i];

	return x;
}

/**	释放bigram数据
 */
int FreeBigramData()
{
	FileMapClose(bigram_handle);
	bigram_data = 0;
	bigram_handle = 0;
	return 1;
}

/**	产生ICW的项
 */
int GenerateICWItems(ICWITEMSET *icw_items, SYLLABLE *syllable, int syllable_count)
{
	//CANDIDATE	candidates[ICW_MAX_ITEMS];
	CANDIDATE	*candidates;	//改为在堆上分配空间，避免堆栈溢出
	int			count;
	int			i, j;

	candidates = malloc(sizeof(CANDIDATE) * ICW_MAX_ITEMS);

	//音节数目
	icw_items->group_count = syllable_count;

	//提取每一个音节的候选
	for (i = 0; i < syllable_count; i++)
	{
		count = 0;
		for (j = 2; j <= min(8, syllable_count - i); j++)
			count += ProcessCiCandidate(syllable + i, j, 0, candidates + count, ICW_MAX_ITEMS - count, 1);

		count = min(ICW_MAX_CI_ITEMS, count);			//多于词的除掉

		//获得汉字
		count += GetZiCandidates(syllable[i], candidates + count, ICW_MAX_ITEMS - count, pim_config->use_fuzzy ? pim_config->fuzzy_mode : 0, HZ_MORE_USED, HZ_OUTPUT_ICW_ZI);
		if (!count)		//在没有任何项的情况下，才找更大集合的汉字
			count += GetZiCandidates(syllable[i], candidates + count, ICW_MAX_ITEMS - count, pim_config->use_fuzzy ? pim_config->fuzzy_mode : 0, HZ_MORE_USED, HZ_OUTPUT_SIMPLIFIED);

		count = min(count, ICW_MAX_CI_ITEMS);		//多余的项除掉

		if (!count)		//没有找到候选(可能是拼音错误，如:chua)，或无法分配内存
		{
			free(candidates);
			return 0;
		}

		//设置汉字数组以及长度
		for (j = 0; j < count; j++)
		{
			if (candidates[j].type == CAND_TYPE_ZI)
			{
				icw_items->group_item[i].item[j].length		= 1;
				icw_items->group_item[i].item[j].hz			= (HZ*)&candidates[j].hz.item->hz;
				icw_items->group_item[i].item[j].syllable	= &candidates[j].hz.item->syllable;
				icw_items->group_item[i].item[j].freq		= ConvertToRealHZFreq((int)(candidates[j].hz.item->freq));
			}
			else if (candidates[j].type == CAND_TYPE_CI &&
				candidates[j].word.item->ci_length == candidates[j].word.item->syllable_length)
			{
				icw_items->group_item[i].item[j].length		= candidates[j].word.item->ci_length;
				icw_items->group_item[i].item[j].hz			= candidates[j].word.hz;
				icw_items->group_item[i].item[j].syllable	= candidates[j].word.syllable;
				icw_items->group_item[i].item[j].freq		= ConvertToRealCIFreq((int)candidates[j].word.item->freq);
			}
			else
			{
				count--; //count需要先减1，否则candidates[count]实际上是越界的
				candidates[j] = candidates[count];
				j--;
			}
		}

		//本音节的候选数目
		icw_items->group_item[i].count = count;
	}

	free(candidates);

	return 1;
}

/**	使用Bigram进行估值
 */
void EvaluateLifeByBigram(const ICWITEMSET *icw_items, ICWLIFE *life)
{
	int i;
	const NEWICWITEM *item0, *item1;
	double b_count;
	double value = 1.0;
	char ci1[0x40];
	char ci0[0x40];
	static c;

	c++;
	i = 0;
	while(i < icw_items->group_count)
	{
		item0 = life->item[i];
		strncpy_s(ci0, sizeof(ci0), (char*)item0->hz, item0->length * 2);

		if (i == 0)
		{
			b_count = GetBigramValue(bigram_data, "△", ci0);
			if (show_detail_bigram)
				printf("(%4.4g)\t", b_count);
			value *= b_count;
		}

		if (i + item0->length < icw_items->group_count)
		{
			item1 = life->item[i + item0->length];
			strncpy_s(ci1, sizeof(ci1), (char*)item1->hz, item1->length * 2);
			b_count = GetBigramValue(bigram_data, ci0, ci1);

			if (show_detail_bigram)
				printf("(%4.4g)\t", b_count);
			value *= b_count;
		}
		else
		{
			b_count = GetBigramValue(bigram_data, ci0, "△");
			if (show_detail_bigram)
				printf("(%4.4g)\t", b_count);
			value *= b_count;
		}
		i += item0->length;
	}

	life->value = value;

//	printf("%8d\t", c);
//	OutputLife((ICWITEMSET*)icw_items, life);
}

/**	估计组价值
 */
void NewEvaluateGroup(ICWITEMSET *icw_items, int group_no)
{
	NEWICWITEM *items, *next_items;
	int i, j, index, next_group_no;
	int start, end;
	int count, next_count;
	char ci0[0x20], ci1[0x20];
	double value, max_value;

	items = icw_items->group_item[group_no].item;
	count = icw_items->group_item[group_no].count;
	if (group_no == icw_items->group_count - 1)			//最后一个
		end = 1;

	if (!group_no)										//第一个
		start = 1;

	for (i = 0; i < count; i++)
	{
		char c0[0x20] = {0};

		memcpy(c0, (char*)items[i].hz, items[i].length * 2);
		//strncpy_s(ci0, sizeof(ci0), (char*)items[i].hz, items[i].length * 2);
		Utf16ToAnsi((TCHAR*)c0, ci0, sizeof(ci0));

		next_group_no = group_no + items[i].length;
		if (next_group_no == icw_items->group_count)
		{
			items[i].value = GetBigramValue(bigram_data, ci0, "△");
			items[i].next = 0;
			continue;
		}

		//在后续的组中找出最佳值
		next_items = icw_items->group_item[next_group_no].item;
		next_count = icw_items->group_item[next_group_no].count;
		max_value = -1.0;
		index = 0;
		for (j = 0; j < next_count; j++)
		{
			char c1[0x20] = {0};

			memcpy(c1, (char*)next_items[j].hz, next_items[j].length * 2);
			//strncpy_s(ci1, sizeof(ci1), (char*)next_items[j].hz, next_items[j].length * 2);
			Utf16ToAnsi((TCHAR*)c1, ci1, sizeof(ci1));

			value = GetBigramValue(bigram_data, ci0, ci1);
			value *= next_items[j].value;
			if (value > max_value)
			{
				index = j;
				max_value = value;
			}
		}
		items[i].value = max_value;
		items[i].next = &next_items[index];

		if (!group_no)				//开始位置，需要计算开始的结果
			items[i].value *= GetBigramValue(bigram_data, "△", ci0);
	}
}

/**	获得ICW候选，动态规划方法
 */
int NewGetIcwCandidates(SYLLABLE *syllable, int syllable_count, CANDIDATE *candidate, double *max_value)
{
	int i, index, part_syllable_count;
//	ICWITEMSET icw_items;
	ICWITEMSET *icw_items;					//为了避免堆栈越界的错误，必须采用在堆中分配的方式 2008-03-06.

	NEWICWITEM	*icw_item;
	HZ *icw_hz;
	SYLLABLE *icw_syllable;

	//先赋初值，避免函数返回0时*max_value没有初始化
	*max_value = -1.0;

	if (!bigram_data || syllable_count < 2 || syllable_count > MAX_ICW_LENGTH)
		return 0;

	//检查音节是否为全音节，如果非全音节数目超过5，则不计算
	for (i = 0, part_syllable_count = 0; i < syllable_count; i++)
		if (syllable[i].vow == VOW_NULL)
			part_syllable_count++;

	if (part_syllable_count >= ICW_MAX_PART_SYLLABLES)
		return 0;

	//由于icw组词不需要超级模糊，所以要在pim_config中清除该位
	SaveCiOption();

	icw_items = malloc(sizeof(ICWITEMSET));
	if (!icw_items)
		return 0;

	//1. 产生每一个候选
	if (!GenerateICWItems(icw_items, syllable, syllable_count))
	{
		RestoreCiOption();
		free(icw_items);
		return 0;
	}

	RestoreCiOption();

	//2. 倒序估计每一个组的价值
	for (i = icw_items->group_count - 1; i >= 0; i--)
		NewEvaluateGroup(icw_items, i);

	//3. 查找最大值
	index = 0;
	for (i = 0; i < icw_items->group_item[0].count; i++)
	{
		if (icw_items->group_item[0].item[i].value > *max_value)
		{
			*max_value = icw_items->group_item[0].item[i].value;
			index = i;
		}
	}

	//4. 填充candidate数据
	icw_item = &icw_items->group_item[0].item[index];
	i = 0;
	icw_hz = candidate->icw.hz;
	icw_syllable = candidate->icw.syllable;
	while(i < icw_items->group_count && icw_item)
	{
		for (i = 0; i < icw_item->length; i++)
		{
			*icw_hz++ = icw_item->hz[i];
			if (show_icw_info)
				OutputHz(icw_item->hz[i]);

			*icw_syllable++ = icw_item->syllable[i];
		}

		if (show_icw_info)
			printf(".");

		icw_item = icw_item->next;
	}

	if (show_icw_info)
		printf("\n");

	candidate->icw.length = icw_items->group_count;
	candidate->type = CAND_TYPE_ICW;

	free(icw_items);
	return 1;
}

/**	输出计算的过程
 */
void OutputBigramProcess(int ci_count, char **ci)
{
	int i;
	double b_count;
	double value = 1.0;
	char *ci0, *ci1;

	ci0 = ci[0];
	b_count = GetBigramValue(bigram_data, "△", ci0);
	printf("%s\t%s\t%6.8g\n", "△", ci0, b_count);
	value *= b_count;

	for (i = 0; i < ci_count - 1; i++)
	{
		ci0 = ci[i];
		ci1 = ci[i + 1];

		b_count = GetBigramValue(bigram_data, ci0, ci1);
		printf("%s\t%s\t%6.8g\n", ci0, ci1, b_count);
		value *= b_count;
	}

	ci0 = ci[ci_count - 1];
	b_count = GetBigramValue(bigram_data, ci0, "△");
	printf("%s\t%s\t%6.8g\n", ci0, "△", b_count);
	value *= b_count;

	printf("value:%6.8g\n", value);
}