/*	词汇处理模块
 */
#include <assert.h>
#include <tchar.h>
#include <kernel.h>
#include <syllable.h>
#include <wordlib.h>
#include <config.h>
#include <utility.h>
#include <zi.h>
#include <ci.h>
#include <share_segment.h>

//#pragma	data_seg(HYPIM_SHARED_SEGMENT)
//CICACHE	ci_cache = {CI_CACHE_V66_SIGNATURE, 0, 0, 0};
//int		ci_cache_loaded   = 0;				//是否已经装入
//int		ci_cache_modified = 0;				//是否改变
//
//NEWCI	new_ci = {0, {0},};
//int		new_ci_loaded	= 0;				//是否装入
//int		new_ci_modified = 0;				//是否已经修改
//#pragma data_seg()

/*	保存词Cache数据。
 *	参数：
 *		ci_cache_file_name			词Cache文件名称
 *	返回：
 *		成功：1
 *		失败：0
 */
int SaveCiCacheData(const TCHAR *ci_cache_file_name)
{
	if (!share_segment->ci_cache_modified)
		return 1;

	share_segment->ci_cache_modified = 0;
	if (!SaveToFile(ci_cache_file_name, &share_segment->ci_cache, sizeof(share_segment->ci_cache)))
	{
		Log(LOG_ID, L"保存词Cache数据失败，file:%s", ci_cache_file_name);
		return 0;
	}

	return 1;
}

void CheckAndUpgradeCiCache()
{
	if (0 == share_segment->ci_cache.length || CI_CACHE_V66_SIGNATURE == share_segment->ci_cache.signature)
		return;

	if (CI_CACHE_SIGNATURE == share_segment->ci_cache.signature)
	{
		int pos = 0;
		int item_length = (char)share_segment->ci_cache.cache[pos] * sizeof(HZ);

		share_segment->ci_cache.signature = CI_CACHE_V66_SIGNATURE;

		//遍历词的cache
		while(pos + item_length <= share_segment->ci_cache.length)		//需要判定最后一个词条不越界
		{
			char ci_ansi[MAX_WORD_LENGTH * sizeof(HZ)] = {0};
			TCHAR ci_uc[MAX_WORD_LENGTH] = {0};

			memcpy_s(ci_ansi, MAX_WORD_LENGTH * sizeof(HZ), &share_segment->ci_cache.cache[pos + WORDLIB_FEATURE_LENGTH], item_length);

			AnsiToUtf16(ci_ansi, ci_uc, MAX_WORD_LENGTH);

			memcpy_s(&share_segment->ci_cache.cache[pos + WORDLIB_FEATURE_LENGTH], item_length, ci_uc, item_length);

			//下一个词
			pos += WORDLIB_FEATURE_LENGTH + share_segment->ci_cache.cache[pos] * sizeof(HZ);
			item_length = (char)share_segment->ci_cache.cache[pos] * sizeof(HZ);
		}
	}
	else
	{
		share_segment->ci_cache.signature	 = CI_CACHE_V66_SIGNATURE;
		share_segment->ci_cache.length		 = 0;
		share_segment->ci_cache.max_used_id  = 0;
	}

	return;
}

/*	装载词Cache数据。
 *	参数：
 *		ci_cache_file_name			词Cache文件名字
 *	返回：
 *		成功：1
 *		失败：0
 */
int LoadCiCacheData(const TCHAR *ci_cache_file_name)
{
	int length;

	assert(ci_cache_file_name);

	//初始化数据，避免文件装载失败造成程序崩溃
	share_segment->ci_cache.length = share_segment->ci_cache.max_used_id = 0;

	length = LoadFromFile(ci_cache_file_name, (char*)&share_segment->ci_cache, sizeof(share_segment->ci_cache));
	if (length < 0)
	{
		share_segment->ci_cache.length = share_segment->ci_cache.max_used_id = 0;
		Log(LOG_ID, L"读取词Cache文件出错，file:%s, length=%d", ci_cache_file_name, length);
		return 0;
	}

	CheckAndUpgradeCiCache();

	share_segment->ci_cache_loaded = 1;
	return 1;
}

/**	释放词Cache数据
 */
int FreeCiCacheData()
{
	share_segment->ci_cache_loaded = 0;
	return 1;
}

/*	在词Cache中找出词的位置。
 *	参数：
 *		hz			词
 *		length		长度
 *	返回：
 *		-1：没有找到
 *		>0 该词在Cache中的位置（越小越新）
 */
int GetCiCacheInfo(HZ *hz, int length)
{
	int pos = 0;
	int item_length = WORDLIB_FEATURE_LENGTH + sizeof(HZ) * length;

	//遍历词的cache
	while(pos + item_length <= share_segment->ci_cache.length)		//需要判定最后一个词条不越界
	{
		if ((char)share_segment->ci_cache.cache[pos] == (char)length &&										//长度相同
			!memcmp(&share_segment->ci_cache.cache[pos + WORDLIB_FEATURE_LENGTH], hz, length * sizeof(HZ)))	//汉字相同
			return pos;

		//下一个词
		pos += WORDLIB_FEATURE_LENGTH + share_segment->ci_cache.cache[pos] * sizeof(HZ);
	}

	//没有找到
	return -1;
}

/*	词Cache的使用度到达临界值，需要将所有的词的使用度减少。
 *	参数：无
 *	返回：无
 */
void ReduceCiCacheUsedCount()
{
	int pos = 0;

	//遍历词条
	while(pos < share_segment->ci_cache.length)
	{
		int used_count;
		int length = share_segment->ci_cache.cache[pos];

		if ((int)(pos + WORDLIB_FEATURE_LENGTH + sizeof(HZ) * length) >= share_segment->ci_cache.length)	//越界
			break;

		used_count = *(int*)&share_segment->ci_cache.cache[pos] >> 8;
		used_count -= CI_MAX_USED_COUNT / 2;		//减少一半的值
		*(int*)&share_segment->ci_cache.cache[pos] = (used_count << 8) + length;

		pos += WORDLIB_FEATURE_LENGTH + sizeof(HZ) * length;
	}
}

/*	向词的cache中插入新的词。
 *	参数：
 *		hz			词
 *		length		长度
 *	返回：无
 */
void InsertCiToCache(HZ *hz, int length, int syllable_length, int set_fix_top)
{
	int pos;						//Cache中的位置
	int item_length;				//本词条的长度
	int used_count;					//词汇使用度
	int i;
	char item_save[WORDLIB_FEATURE_LENGTH + sizeof(HZ) * MAX_WORD_LENGTH];		//临时保存区

	//如果以当前的位置确定词的顺序，则不能向词频中增加内容
	if (pim_config->ci_option & CI_ADJUST_FREQ_NONE)
		return;

	if (length > MAX_WORD_LENGTH)
		return;

	//UCS4的词不加入Cache
	if (length != syllable_length)
		return;

	item_length = length * sizeof(HZ) + WORDLIB_FEATURE_LENGTH;

	//以下这个词是否出现在Cache中?
	pos = GetCiCacheInfo(hz, length);

	if (pos != -1)		//找到
	{
		used_count = *(int*)&share_segment->ci_cache.cache[pos] >> 8;		//找出词的使用度
		used_count++;
	}
	else		//没有找到的话，当作插入到cache末尾处
	{
		used_count = 1;
		pos = share_segment->ci_cache.length;
		share_segment->ci_cache.length	   = pos + item_length;
		share_segment->ci_cache.cache[pos] = (char)length;

		for (i = 0; i < (int)(length * sizeof(HZ)); i++)
			share_segment->ci_cache.cache[pos + i + WORDLIB_FEATURE_LENGTH] = *((char*)hz + i);	//将汉字拷贝到Cache
	}

	if (set_fix_top)
		used_count = CI_TOP_USED_COUNT;
	else if (CI_TOP_USED_COUNT == used_count)
		used_count++;

	*(int*)&share_segment->ci_cache.cache[pos] = (used_count << 8) + share_segment->ci_cache.cache[pos];	//更新使用度

	//调整顺序
	//(0)(1)....(pos-1)(pos)(pos+1)...(cache_length-1)  ====> (pos)(0)(1)...(pos-1)(pos+1)...(cache_length-1)
	//保存当前词条
	for (i = 0; i < item_length; i++)
		item_save[i] = share_segment->ci_cache.cache[pos + i];

	for (i = pos - 1; i >= 0; i--)
		share_segment->ci_cache.cache[i + item_length] = share_segment->ci_cache.cache[i];

	for (i = 0; i < item_length; i++)
		share_segment->ci_cache.cache[i] = item_save[i];

	//将越界的词Cache长度减少回来
	if (share_segment->ci_cache.length > CI_CACHE_SIZE)
		share_segment->ci_cache.length = CI_CACHE_SIZE;

	//如果使用度越界，则需要将所有词项的使用度减少
	if (used_count > CI_MAX_USED_COUNT)
		ReduceCiCacheUsedCount();

	share_segment->ci_cache_modified = 1;
	return;
}

/**	返回词的使用度
 */
int GetCiUsedCount(int cache_pos)
{
	if (cache_pos < 0)
		return 0;

	return *(int*)&share_segment->ci_cache.cache[cache_pos] >> 8;
}

/*	比较词中的汉字以及词频，用于处理词的候选顺序。
 *	比较过程：
 *		1. 比较两个词在Cache中的位置
 *		2. 比较两个词的词频
 *	参数：
 *		cand1, cand2		用于比较的两个候选指针
 *	返回：
 *		第一个比第二个小 <0 表示cand1排在cand2前
 *		第一个等于第二个 =0
 *		第一个大于第二个 >0 表示cand2排在cand1前
 */
static int CompareCi(const CANDIDATE *cand1, const CANDIDATE *cand2)
{
	int cache_pos1, cache_pos2;
	int used_count1, used_count2;

	//排序方式：置顶词排在非置顶词前；若都是置顶词或者都不是置顶词，看是否
	//设置了快速调整词频，若设置，在Cache中位置靠前的词排在前面(新近使用并
	//加入Cache的词)，位置靠后的词排在后面，不在Cache中的词排在最后；若未设
	//置，看是否设置了慢速词频调整，若设置，使用度高的词排在前面，使用度低
	//的词排在后面，不在Cache中的词排在最后；若未设置，词频高的排在前面，词
	//频低的排在后面

	//特别注意CiCache和ZiCache结构区别很大，其cache_pos意义也不同，请
	//认真阅读其定义

	cache_pos1  = cand1->word.cache_pos;
	cache_pos2  = cand2->word.cache_pos;
	used_count1 = GetCiUsedCount(cache_pos1);
	used_count2 = GetCiUsedCount(cache_pos2);

	if (CI_TOP_USED_COUNT == used_count1 && CI_TOP_USED_COUNT != used_count2)
		return -1;
	else if (CI_TOP_USED_COUNT != used_count1 && CI_TOP_USED_COUNT == used_count2)
		return 1;

	//快速调整词频
	if (pim_config->ci_option & CI_ADJUST_FREQ_FAST)
	{
		//不是同一个词
		if (cache_pos1 != cache_pos2)
		{
			//cand1不在Cache中(cand2不可能同时不在Cache中，
			//否则不满足上面的if条件)，cand2在Cache中，cand1
			//排在后面
			if (cache_pos1 == -1)
				return 1;

			//cand1在Cache中，cand2不在Cache中，cand1排在前面
			//(返回-1)
			if (cache_pos2 == -1)
				return -1;

			//在Cache中位置靠前的排在前面
			return cache_pos1 - cache_pos2;
		}
	}

	//慢速词频调整
	if (pim_config->ci_option & CI_ADJUST_FREQ_SLOW)
	{
		if (cache_pos1 != cache_pos2)
		{
			if (cache_pos1 == -1)
				return 1;

			if (cache_pos2 == -1)
				return -1;

			if (used_count1 != used_count2)
				return used_count2 - used_count1;
		}
	}

	//比较词频
	return cand2->word.item->freq - cand1->word.item->freq;
}

/*	比较词中的汉字以及词频，用于处理词的排重
 *	参数：
 *		cand1, cand2		用于比较的两个候选指针
 *	返回：
 *		第一个比第二个小 <0
 *		第一个等于第二个 =0
 *		第一个大于第二个 >0
 */
static int CompareCiCodeFreq(const CANDIDATE *cand1, const CANDIDATE *cand2)
{
	int ret;

	assert(cand1 && cand2);

	//长度不同
	if (cand1->word.item->ci_length != cand2->word.item->ci_length)
		return cand2->word.item->ci_length - cand1->word.item->ci_length;

	//如果词的字不同则返回两个词的内码差
	ret = memcmp(cand2->word.hz, cand1->word.hz, sizeof(HZ) * cand1->word.item->ci_length);
	if (ret)
		return ret;

	//如果两个词中有一个是被删除的，则排在前面
	if (cand1->word.item->effective && !cand2->word.item->effective)
		return 1;

	if (cand2->word.item->effective && !cand1->word.item->effective)
		return -1;

	//比较字频
	return cand2->word.item->freq - cand1->word.item->freq;
}

/*	对词进行排序。
 *	参数：
 *		candidates		词候选
 *		count			候选个数
 *	返回：无
 */
void SortCiCandidates(CANDIDATE *candidates, int count)
{
	if (count < 2)
		return;

	qsort(candidates, count, sizeof(CANDIDATE), CompareCi);
}

/*	词候选排重。
 *	参数：
 *		candidates			词候选数组
 *		count				数组长度
 *	返回：
 *		排重后的候选数目
 */
int UnifyCiCandidates(CANDIDATE *candidates, int count)
{
	int new_count, i;

	if (!count)		//两个以下词不用排重
		return count;

	//以hz内码顺序进行排序，两个相同的词汇将处于一起的位置
	qsort(candidates, count, sizeof(CANDIDATE), CompareCiCodeFreq);

	//排重
	for (new_count = 1, i = 1; i < count; i++)
	{
		if (candidates[i].word.item->ci_length == candidates[i - 1].word.item->ci_length &&
			!memcmp(candidates[i].word.hz, candidates[i - 1].word.hz, sizeof(HZ) * candidates[i].word.item->ci_length))
			continue;

		candidates[new_count] = candidates[i];
		new_count++;
	}

	count = new_count;

	//删除已经删除的词汇
	for (new_count = 0, i = 0; i < count; i++)
	{
		if (!candidates[i].word.item->effective)	//找到被删除的词条
			continue;

		candidates[new_count++] = candidates[i];
	}

	return new_count;
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

/*	V6B1词库，获得下一条词的指针。
 *	参数：
 *		item		当前词指针
 *	返回：
 *		下一条词的指针
 */
WORDLIBITEMV6B1 *GetNextCiItemV6B1(WORDLIBITEMV6B1 *item)
{
	//feature + syllable + hz
	return (WORDLIBITEMV6B1*)((char*)item +	sizeof(int) + sizeof(SYLLABLE) * item->length +	sizeof(HZ) * item->length);
}

/**	获得词汇的汉字指针
 */
HZ *GetItemHZPtr(WORDLIBITEM *item)
{
	return (HZ*)((char*)item + sizeof(int) + sizeof(SYLLABLE) * item->syllable_length);
}

/**	获得词汇的音节指针
 */
SYLLABLE *GetItemSyllablePtr(WORDLIBITEM *item)
{
	return (SYLLABLE*)((char*)item + sizeof(int));
}

/**	基于词长以及音节长度确定词项的长度
 */
int GetItemLength(int hz_length, int syllable_length)
{
	return sizeof(HZ) * hz_length + sizeof(SYLLABLE) * syllable_length + WORDLIB_FEATURE_LENGTH;
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
	int count = 0;					//候选计数

	if (!(pim_config->ci_option & CI_WILDCARD))
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

/*	使用首字母，获得页中的词候选（没有通配符）。
 *	参数：
 *		page					页指针
 *		letters					音节序列
 *		letter_count			音节长度
 *		candidate_array			候选数组
 *		candidate_length		候选数组长度
 *		fuzzy_mode				模糊音选项
 *	返回：
 *		候选词汇数目
 */
int GetCiInPageByLetter(PAGE *page, const TCHAR *letters, int letter_count, CANDIDATE *candidate_array, int candidate_length)
{
	WORDLIBITEM *item;				//词项
	int count = 0;					//候选计数

	if (!candidate_length || !letters)
		return 0;

	//如果页中没有包含这个长度的词，则直接返回（为提高检索效率）
	if (!(page->length_flag & (1 << letter_count)))
		return 0;

	//遍历页表
	for (item = (WORDLIBITEM*) page->data; (char*)item < (char*) &page->data + page->data_length; item = GetNextCiItem(item))
	{
		//判断长度是否符合
		if (letter_count != item->syllable_length)
			continue;

		//判断音节是否符合
		if (!CompareSyllablesAndLetters(letters, item->syllable, letter_count))
			continue;

		//找到!
		candidate_array[count].word.type	  = CI_TYPE_LETTER;
		candidate_array[count].word.item	  = item;
		candidate_array[count].word.cache_pos = 0;
		candidate_array[count].word.syllable  = GetItemSyllablePtr(item);
		candidate_array[count].word.hz		  = GetItemHZPtr(item);

		count++;
		if (count >= candidate_length)
			break;
	}

	return count;
}

/*	使用首字母，获得页中的词候选（使用通配符）。
 *	参数：
 *		page					页指针
 *		letters					音节序列
 *		letter_count			音节长度
 *		candidate_array			候选数组
 *		candidate_length		候选数组长度
 *	返回：
 *		候选词汇数目
 */
int GetCiInPageByWildLetter(PAGE *page, const TCHAR *letters, int letter_count,	CANDIDATE *candidate_array,	int candidate_length)
{
	WORDLIBITEM *item;				//词项
	int count = 0;					//候选计数

	if (!candidate_length || !letters)
		return 0;

	//如果页中没有大于等于这首字母个数的词，则直接返回（为提高检索效率）
	if ((unsigned int)page->length_flag < (unsigned int)(1 << letter_count))
	{
		if (page->length_flag != 1)
			return 0;
	}

	//遍历页表
	for (item = (WORDLIBITEM*) page->data; (char*)item < (char*) &page->data + page->data_length; item = GetNextCiItem(item))
	{
		//判断长度是否符合
		if (letter_count < pim_config->first_letter_input_min_hz)
		{
			if (item->syllable_length != letter_count)
				continue;
		}
		else if ((int)item->syllable_length < pim_config->first_letter_input_min_hz)
			continue;

		//判断音节是否符合
		if (!WildCompareSyllablesAndLetters(letters, (int)_tcslen(letters), item->syllable, item->syllable_length))
			continue;

		//找到!
		candidate_array[count].word.type	  = CI_TYPE_LETTER;
		candidate_array[count].word.item	  = item;
		candidate_array[count].word.cache_pos = 0;
		candidate_array[count].word.syllable  = GetItemSyllablePtr(item);
		candidate_array[count].word.hz		  = GetItemHZPtr(item);

		count++;
		if (count >= candidate_length)
			break;
	}

	return count;
}

/*	获得词汇候选。
 *	参数：
 *		wordlib_id				词库标识
 *		syllable_array			音节序列
 *		syllable_count			音节长度
 *		candidate_array			候选数组
 *		candidate_length		候选数组长度
 *		fuzzy_mode				模糊音选项
 *	返回：
 *		候选词汇数目
 */
int GetCiCandidates(int wordlib_id, SYLLABLE *syllable_array, int syllable_count, CANDIDATE *candidate_array, int candidate_length, int fuzzy_mode)
{
	WORDLIB *wordlib;			//词库指针
	int page_no;				//页号
	int count = 0;				//候选计数
	int i, j;
	SYLLABLE syllable_tmp1, syllable_tmp2;
	int wild_compare = 0;								//是否使用通配比较

	if (!candidate_length || wordlib_id < 0)
		return 0;

	//音节过少或者过大
	if (syllable_count < 2 || syllable_count > MAX_WORD_LENGTH)
		return 0;				

	wordlib = GetWordLibrary(wordlib_id);
	if (!wordlib)				//没有这个词库
		return 0;

	//查看是否需要进行通配比较
	for (i = 0; i < syllable_count; i++)
		if (syllable_array[i].con == CON_ANY)		//带有通配符
			break;

	wild_compare = i != syllable_count;

	//用于比较的临时音节初始化
	syllable_tmp1.con  = syllable_tmp2.con  = CON_NULL;
	syllable_tmp1.vow  = syllable_tmp2.vow  = VOW_NULL;
	syllable_tmp1.tone = syllable_tmp2.tone = TONE_0;

	//由于有模糊音的存在，所以必须进行声母的遍历才能找出全部正确的声母。
	for (i = CON_NULL; i < CON_END; i++)
	{
		syllable_tmp1.con = i;
		if (!ContainCon(syllable_array[0], syllable_tmp1, fuzzy_mode))
			continue;

		//找到第一个相同的音节
		for (j = CON_NULL; j < CON_END; j++)
		{
			syllable_tmp2.con = j;
			if (!ContainCon(syllable_array[1], syllable_tmp2, fuzzy_mode))
				continue;

			//音节序列的词库页索引
			page_no = wordlib->header.index[i][j];

			//遍历页表找出词候选
			while(page_no != PAGE_END)
			{
				count += wild_compare
					?
						GetCiInPageWild(			//使用通配符
							&wordlib->pages[page_no],
							syllable_array,
							syllable_count,
							candidate_array + count,
							candidate_length - count,
							fuzzy_mode)
					:
						GetCiInPage(				//不使用通配符
							&wordlib->pages[page_no],
							syllable_array,
							syllable_count,
							candidate_array + count,
							candidate_length - count,
							fuzzy_mode);

				if (count >= candidate_length)
					break;

				page_no = wordlib->pages[page_no].next_page_no;
			}

			if (count >= candidate_length)
				break;
		}
	}

	//设置词库标识
	for (i = 0; i < count; i++)
	{
		candidate_array[i].word.source = wordlib_id;
		candidate_array[i].type		   = CAND_TYPE_CI;
	}

	return count;
}

/*	获得词汇候选。
 *	参数：
 *		wordlib_id				词库标识
 *		letters					首字母串
 *		candidate_array			候选数组
 *		candidate_length		候选数组长度
 *	返回：
 *		候选词汇数目
 */
int GetCiCandidatesByLetter(int wordlib_id, const TCHAR *letters, CANDIDATE *candidate_array, int candidate_length)
{
	WORDLIB *wordlib;							//词库指针
	SYLLABLE syllable_tmp1, syllable_tmp2;		//临时音节
	int page_no;								//页号
	int count = 0;								//候选计数
	int length = 0;								//首字母数目
	int wild_compare = 0;						//是否使用通配比较
	int i, j, k, l;

	assert(letters);

	//检查首字母合法性
	while(letters[length])
	{
		if (letters[length] == SYLLABLE_ANY_CHAR)
			wild_compare = 1;
		else if (letters[length] < 'a' || letters[length] > 'z')
			return 0;
		else if (letters[length] == 'i' || letters[length] == 'v' || letters[length] == 'u')
			return 0;

		length++;
	};

	//音节过小、过大
	if (length < 2 || length > MAX_WORD_LENGTH)
		return 0;

	wordlib = GetWordLibrary(wordlib_id);
	if (!wordlib)				//没有这个词库
		return 0;

	//进行声母的遍历，找出全部正确的声母。
	for (i = CON_NULL; i < CON_END; i++)
	{
		for (j = VOW_NULL; j < VOW_END; j++)
		{
			syllable_tmp1.con = i;
			syllable_tmp1.vow = j;
			if (!SyllableStartWithLetter(letters[0], syllable_tmp1))
				continue;

			j = VOW_END;			//一旦找到，就不进行内层循环

			//找到第一个相同的音节
			for (k = CON_NULL; k < CON_END; k++)
			{
				for (l = VOW_NULL; l < VOW_END; l++)
				{
					syllable_tmp2.con = k;
					syllable_tmp2.vow = l;

					if (!SyllableStartWithLetter(letters[1], syllable_tmp2))
						continue;

					l = VOW_END;		//一旦找到，就不进行内层循环

					//音节序列的词库页索引
					page_no = wordlib->header.index[i][k];

					//遍历页表找出词候选
					while(page_no != PAGE_END)
					{
						count += wild_compare
							?	GetCiInPageByWildLetter(			//使用通配符
									&wordlib->pages[page_no],
									letters,
									length,
									candidate_array + count,
									candidate_length - count)
							:	GetCiInPageByLetter(				//不使用通配符
									&wordlib->pages[page_no],
									letters,
									length,
									candidate_array + count,
									candidate_length - count);

						page_no = wordlib->pages[page_no].next_page_no;
					}
				}
			}
		}
	}

	//设置词库标识
	for (i = 0; i < count; i++)
	{
		candidate_array[i].word.source = wordlib_id;
		candidate_array[i].type		   = CAND_TYPE_CI;
	}

	return count;
}

/*	处理词汇被用户选中后的操作：插入到Cache中，增加用户使用度。
 *	参数：
 *		ci_cand			词候选
 *	返回：无
 */
void ProcessCiSelected(SYLLABLE *syllable, int syllable_length, HZ *hz, int hz_length)
{
/*
	//加入到用户词库中，并且更新词条信息
	if (pim_config->insert_used_word_to_user_wl)
		AddCi(syllable, syllable_length, hz, hz_length);
*/
	//插入到词cache中
	InsertCiToCache(hz, hz_length, syllable_length, 0);
}

/*	基于音调删除不正确的词。
 *	一般词库中的词都不附带音调，需要进行字的反查。
 *	如：z4f 能够捡出的词包括：征服，但“征”字并没有四声，因此必须除掉
 *
 *	参数：
 *		syllable_array			音节数组
 *		syllable_count			音节计数
 *		candidate_array			候选数组
 *		candidate_count			候选数目
 *	返回：
 *		剩余候选数目
 */
int DeleteCiCandidateByTone(SYLLABLE *syllable_array, int syllable_count, CANDIDATE *candidate_array, int candidate_count)
{
	int i, j;
	int new_count = 0;
	int has_tone = 0, has_wild = 0;

	//检查音节是否带有音调
	for (i = 0; i < syllable_count; i++)
	{
		if (syllable_array[i].tone != TONE_0)
			has_tone = 1;

		if (syllable_array[i].con == CON_ANY)
			has_wild = 1;
	}

	if (!has_tone)		//没有带有音调的音或者包含通配符（已经排除），直接返回
		return candidate_count;

	if (has_wild)
	{
		for (i = 0; i < candidate_count; i++)
		{
			if (!WildCompareSyllablesWithCi(syllable_array, syllable_count, candidate_array[i].word.syllable, candidate_array[i].word.hz, candidate_array[i].word.item->syllable_length, 0))
			{
				candidate_array[i] = candidate_array[candidate_count - 1];
				candidate_count--;
				i--;
				continue;
			}
		}

		return candidate_count;
	}

	for (i = 0; i < candidate_count; i++)
	{
		for (j = 0; j < syllable_count; j++)
		{
			if (syllable_array[j].tone != TONE_0 && !ZiContainTone(candidate_array[i].word.hz[j], candidate_array[i].word.syllable[j], syllable_array[j].tone))
				break;
		}

		if (j != syllable_count)			//不符合音调要求
		{
			candidate_array[i] = candidate_array[candidate_count - 1];
			candidate_count--;
			i--;

			continue;
		}
	}

	return candidate_count;
}

/*	基于音节处理词的候选。
 *		1. 获得候选（音节候选以及单字母候选）
 *		2. 排重
 *		3. 排序
 *	参数：
 *		syllable_array			音节数组
 *		syllable_count			音节长度
 *		letters					用户输入串，为0时为不操作
 *		candidate_array			候选数组
 *		candidate_length		候选数组长度
 *		same_ci_syllable_length	是否需要词与音节的长度相同
 *	返回：
 *		候选数目
 */
int ProcessCiCandidate(SYLLABLE *syllable_array, int syllable_count, const TCHAR *letters, CANDIDATE *candidate_array, int candidate_length, int same_ci_syllable_length)
{
	int count = 0;					//候选数目
	int fuzzy_mode = pim_config->use_fuzzy ? pim_config->fuzzy_mode : 0;			//模糊方式
	int wordlib_id;					//词库标识
	int i;

	assert(syllable_array && candidate_array && syllable_count >= 1);

	//设置中包含词输入自动模糊
	if (pim_config->ci_option & CI_AUTO_FUZZY)
		fuzzy_mode |= FUZZY_ZCS_IN_CI;

	if (pim_config->pinyin_mode == PINYIN_QUANPIN && (pim_config->ci_option & CI_AUTO_VOW_FUZZY))
		fuzzy_mode |= FUZZY_SUPER;

	if (!same_ci_syllable_length)
		fuzzy_mode |= FUZZY_CI_SYLLABLE_LENGTH;

	//在内存中的全部词库中查找候选
	wordlib_id = GetNextWordLibId(-1);
	while (wordlib_id != -1)
	{
		count += GetCiCandidates(wordlib_id, syllable_array, syllable_count, candidate_array + count, candidate_length - count, fuzzy_mode);
		wordlib_id = GetNextWordLibId(wordlib_id);
	};

	//如果有音调的话，需要将不包含音调的词除去
	count = DeleteCiCandidateByTone(syllable_array, syllable_count, candidate_array, count);

	//进行首字母词输入，需要加入判断是否为1个音节，如果为一个音节，
	//则不能进行单字母输入（解决"中华儿女"问题）
	//可能以后，采用必须在第五个候选之后的方法进行。
	if (letters && (pim_config->ci_option & CI_USE_FIRST_LETTER) &&
		(int)_tcslen(letters) >= pim_config->first_letter_input_min_hz)
	{
		//在内存中的全部词库中查找候选
		wordlib_id = GetNextWordLibId(-1);
		while (wordlib_id != -1)
		{
			count += GetCiCandidatesByLetter(wordlib_id, letters, candidate_array + count, candidate_length - count);
			wordlib_id = GetNextWordLibId(wordlib_id);
		};
	}

	//排重
	count = UnifyCiCandidates(candidate_array, count);

	for (i = 0; i < count; i++)
		candidate_array[i].word.cache_pos = GetCiCacheInfo(candidate_array[i].word.hz, candidate_array[i].word.item->ci_length);

	//排序
	SortCiCandidates(candidate_array, count);

	return count;
}

/*	获取可能存在混合解析的候选词
 *	参数：
 *		syllable_array			音节数组
 *		syllable_count			音节长度
 *		candidate_array			候选数组
 *		candidate_length		候选数组长度
 *	返回：
 *		候选数目
 */
int GetMixedParseCiCandidate(SYLLABLE *syllable_array, int syllable_count, CANDIDATE *candidate_array, int candidate_length)
{
	SYLLABLE  base_syllables[3];    //用于联想的首音节
	int count = 0;					//候选数目
	int wordlib_id;					//词库标识
	int i;

	assert(syllable_array && candidate_array && syllable_count >= 1 && candidate_length > 0);

	//混合解析至少要4个音节
	if (syllable_count < 4)
		return 0;

	//产生联想音节
	for (i = 0; i < 2; i++)
		base_syllables[i] = syllable_array[i];
	base_syllables[2].con = CON_ANY;
	base_syllables[2].vow = VOW_ANY;
	base_syllables[2].tone = TONE_0;

	//Q：为何不使用任何模糊？
	//A：见kernel.c本函数引用处附近注释

	//在内存中的全部词库中查找候选
	wordlib_id = GetUserWordLibId();
	if (wordlib_id != -1)
		count += GetCiCandidates(wordlib_id, base_syllables, 3, candidate_array + count, candidate_length - count, 0);

	//如果base_syllables中有音调的话，需要将candidate_array中音调不匹配的词除去
	count = DeleteCiCandidateByTone(base_syllables, 3, candidate_array, count);

	//混合解析至少要4个音节
	for (i = 0; i < count; i++)
	{
		if (candidate_array->word.item->syllable_length < 4)
		{
			candidate_array[i] = candidate_array[count - 1];
			count--;
			i--;
		}
	}

	//取得词在Cache中的位置
	for (i = 0; i < count; i++)
		candidate_array[i].word.cache_pos = GetCiCacheInfo(candidate_array[i].word.hz, candidate_array[i].word.item->ci_length);

	//排序
	SortCiCandidates(candidate_array, count);

	return count;
}

/*	获得用户词库中的词汇候选，用于ICW的用户词读取。
 *	参数：
 *		syllable_array			音节序列
 *		syllable_count			音节长度
 *		candidate_array			候选数组
 *		candidate_length		候选数组长度
 *		fuzzy_mode				模糊音选项
 *	返回：
 *		候选词汇数目
 */
int GetUserCiCandidates(SYLLABLE *syllable_array, int syllable_count, CANDIDATE *candidate_array, int candidate_length, int fuzzy_mode)
{
	return GetCiCandidates(GetUserWordLibId(), syllable_array, syllable_count, candidate_array, candidate_length, fuzzy_mode);
}

/*	向用户词库增加词条。
 *	参数：
 *		syllable		音节
 *		ci				词汉字
 *		length			词长度
 *	返回：
 *		成功：1
 *		失败：0
 */
int AddCi(SYLLABLE *syllable, int syllable_length, HZ *hz, int hz_length)
{
	int user_wl_id = GetUserWordLibId();

	//UCS4的词不加入用户词库
	if (syllable_length != hz_length)
		return 0;

	if (!AddCiToWordLibrary(user_wl_id, hz, hz_length, syllable, syllable_length, USER_BASE_FREQ))
		return 0;		//失败

	return 1;
}

/**	查找新词
 *	返回：
 *		查到返回在new_ci数组中的下标
 *		没有查到，则返回-1
 */
//int GetNewWordPosition(HZ *ci_str, int ci_length, SYLLABLE *syllable, int syllable_length)
//{
//	int index = 0;
//
//	while(index < share_segment->new_ci.length)
//	{
//		if (share_segment->new_ci.data[index] == (char)ci_length && share_segment->new_ci.data[index +1] == (char)syllable_length &&
//			!memcmp(&share_segment->new_ci.data[index + 2], ci_str, ci_length * sizeof(HZ)) &&
//			!memcmp(&share_segment->new_ci.data[index + 2 + ci_length * sizeof(HZ)], syllable, syllable_length * sizeof(SYLLABLE)))
//			return index;
//
//		index += 2 + share_segment->new_ci.data[index] * sizeof(HZ) + share_segment->new_ci.data[index + 1] * sizeof(SYLLABLE);
//	}
//
//	return -1;
//}

/**	添加新词
 */
//int InsertNewWord(HZ *ci_str, int ci_length, SYLLABLE *syllable, int syllable_length)
//{
//	int size;
//	int i;
//
//	if (ci_length < 2 || ci_length > MAX_WORD_LENGTH ||
//		syllable_length < 2 || syllable_length > MAX_WORD_LENGTH)
//		return 0;
//
//	//将音调除掉
//	for (i = 0; i < syllable_length; i++)
//		syllable[i].tone = TONE_0;
//
//	//是否已经在表中
//	if (GetNewWordPosition(ci_str, ci_length, syllable, syllable_length) >= 0)
//		return 1;			//已经存在
//
//	//在前面加入
//	size = ci_length * sizeof(HZ) + syllable_length * sizeof(SYLLABLE) + 2;
//	for (i = share_segment->new_ci.length - 1; i >= 0; i--)
//		share_segment->new_ci.data[i + size] = share_segment->new_ci.data[i];
//
//	share_segment->new_ci.data[0] = (char)ci_length;
//	share_segment->new_ci.data[1] = (char)syllable_length;
//
//	memcpy(&share_segment->new_ci.data[2], ci_str, ci_length * sizeof(HZ));
//	memcpy(&share_segment->new_ci.data[2 + ci_length * sizeof(HZ)], syllable, syllable_length * sizeof(SYLLABLE));
//
//	share_segment->new_ci.length += size;
//	if (share_segment->new_ci.length > sizeof(share_segment->new_ci.data))
//		share_segment->new_ci.length = sizeof(share_segment->new_ci.data);
//
//	share_segment->new_ci_modified = 1;
//	return 1;
//}

/**	删除新词
 */
//void DeleteNewWord(HZ *ci_str, int ci_length, SYLLABLE *syllable, int syllable_length)
//{
//	int index, i, size;
//
//	if (ci_length < 0 || ci_length > MAX_WORD_LENGTH ||
//		syllable_length < 0 || syllable_length > MAX_WORD_LENGTH)
//		return;
//
//	index = GetNewWordPosition(ci_str, ci_length, syllable, syllable_length);
//	if (index < 0)
//		return;
//
//	size = 2 + ci_length * sizeof(HZ) + syllable_length * sizeof(SYLLABLE);
//
//	//依次向前移动一个词条的位置
//	for (i = index; i < share_segment->new_ci.length - size; i++)
//		share_segment->new_ci.data[i] = share_segment->new_ci.data[i + size];
//
//	share_segment->new_ci.length -= size;
//	if (share_segment->new_ci.length < 0)
//		share_segment->new_ci.length = 0;
//
//	share_segment->new_ci_modified = 1;
//}

/**	生成词条的拼音串
 *	返回：拼音串长度
 *		缓冲区不足，返回-1
 */
//int __stdcall GenerateNewWordString(int index, char *buffer, int buffer_length)
//{
//	int size, length;
//	int ci_length, syllable_length;
//	char tmp[0x100];
//
//	ci_length		= share_segment->new_ci.data[index];
//	syllable_length = share_segment->new_ci.data[index + 1];
//	size			= ci_length * sizeof(HZ) + syllable_length * sizeof(SYLLABLE) + 2;
//
//	if (buffer_length - 1 < size || sizeof(tmp) - 1 < size)
//		return -1;
//
//	memcpy(tmp, &(share_segment->new_ci.data[index]), size);
//	tmp[size] = 0;
//
//	//存储词条长度以及音节长度
//	length = ArrayToHexString(tmp, size, buffer, buffer_length);
//	if (!length)
//		return -1;
//
//	return length;
//}

/**	获得新的词条
 *	返回：
 *		字符串的长度
 */
//int __stdcall GetNewestUserWords(char *buffer, int buffer_length)
//{
//	int index, buffer_index;
//	short sign;				//字符串签名
//
//	buffer_length -= sizeof(sign) + 1;		//预留签名的数值
//	index = buffer_index = 0;
//	while(index < share_segment->new_ci.length)
//	{
//		if (share_segment->new_ci.data[index + 2] != 0)		//不是被删除的
//		{
//			int length = GenerateNewWordString(index, buffer + buffer_index, buffer_length - buffer_index);
//
//			if (length < 0)		//缓冲区满
//				break;
//
//			buffer_index += length;
//		}
//
//		index += 2 + share_segment->new_ci.data[index] * sizeof(HZ) + share_segment->new_ci.data[index + 1] * sizeof(SYLLABLE);
//	}
//
//	if (buffer_index)
//	{
//		//只取short的签名信息
//		sign = (short)GetSign(buffer, buffer_index);
//		ArrayToHexString((const char*)&sign, sizeof(sign), buffer + buffer_index, sizeof(sign) * 2 + 1);
//		buffer_index += sizeof(sign) * 2;
//	}
//
//	return buffer_index;
//}

/**	删除一组新词（UPLOAD完毕）
 */
//void __stdcall DelNewestUserWords(const char *buffer)
//{
//	int length;
//	int index;
//	int ci_length, syllable_length;
//	const char *p_ci, *p_syllable;
//	char tmp[0x1000];			//转换后的字符串
//	int  tmp_len;
//	short  sign;					//签名数据
//
//	if (!(length = (int)strlen(buffer)))
//		return;
//
//	//过长
//	if (!(tmp_len = HexStringToArray(buffer, tmp, sizeof(tmp))) || tmp_len < 4)
//		return;
//
//	//判断签名是否正确
//	sign = *(short*)(tmp + tmp_len - sizeof(short));
//	if (sign != (short)(GetSign(buffer, length - sizeof(sign) * 2)))
//		return;		//签名不对
//
//	//逐条分析词条
//	//B0：词长度，B1：音节长度
//	index = 0;
//	while(index < tmp_len)
//	{
//		ci_length = tmp[index];
//		syllable_length = tmp[index + 1];
//
//		if (ci_length < 0 || syllable_length < 0 ||
//			ci_length > MAX_WORD_LENGTH || syllable_length > MAX_WORD_LENGTH)
//			return;
//
//		p_ci = &tmp[index + 2];
//		p_syllable = p_ci + ci_length * sizeof(HZ);
//		DeleteNewWord((HZ*)p_ci, ci_length, (SYLLABLE*)p_syllable, syllable_length);
//		index += 2 + ci_length * sizeof(HZ) + syllable_length * sizeof(SYLLABLE);
//	}
//}

/**	检查是否在用户词库中存在这个词，如果不存在，则加入到新词表中
 */
//void CheckNewUserWord(SYLLABLE *syllables, int syllable_length, HZ *ci_string, int ci_length)
//{
//	WORDLIBITEM *item;
//	int user_wl_id = GetUserWordLibId();
//
//	if (user_wl_id < 0)
//		return;
//
//	//UCS4的词不会存在于用户词库
//	if (syllable_length != ci_length)
//		return;
//
//	if ((item = GetCiInWordLibrary(user_wl_id, ci_string, ci_length, syllables, syllable_length)) != 0)
//		return;		//存在，返回
//
//	InsertNewWord(ci_string, ci_length, syllables, syllable_length);
//}

/**	装载新词表文件
 */
//int LoadNewWordTable()
//{
//	int length;
//	TCHAR name[MAX_PATH];
//
//	if (share_segment->new_ci_loaded)
//		return 1;
//
//	GetFileFullName(TYPE_ALLAPP, NEWCI_FILE_NAME, name);
//
//	//初始化数据，避免文件装载失败造成程序崩溃
//	share_segment->new_ci.length = 0;
//
//	length = LoadFromFile(name, (char*)&share_segment->new_ci, sizeof(share_segment->new_ci));
//	if (length < 0)
//	{
//		Log(LOG_ID, L"读取词新词表文件出错，file:%s, length=%d", name, length);
//		share_segment->new_ci.length = 0;
//		return 0;
//	}
//
//	share_segment->new_ci_loaded = 1;
//	return 1;
//}

/**	保存新词表文件
 */
//int SaveNewWordTable()
//{
//	TCHAR name[MAX_PATH];
//
//	if (!share_segment->new_ci_modified)
//		return 1;
//
//	GetFileFullName(TYPE_ALLAPP, NEWCI_FILE_NAME, name);
//	if (!SaveToFile(name, (char*)&share_segment->new_ci, sizeof(share_segment->new_ci)))
//	{
//		Log(LOG_ID, L"写入新词表文件失败,file:%s", name);
//		return 0;
//	}
//
//	share_segment->new_ci_modified = 0;
//	return 1;
//}

/**	释放新词表
 */
//void FreeNewWordTable()
//{
//	SaveNewWordTable();
//	share_segment->new_ci_loaded = 0;
//}