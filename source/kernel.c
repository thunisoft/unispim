/*	输入法核心程序。功能包括：
 *	1. 各种候选的处理
 *	2. 各种资源的装载与管理
 *	3. 字频、词频的更新管理
 */
#include <assert.h>
#include <kernel.h>
#include <syllable.h>
#include <config.h>
#include <spw.h>
#include <ci.h>
#include <zi.h>
#include <icw.h>
#include <context.h>
#include <utility.h>
#include <english.h>
#include <time.h>
//#include <url.h>
#include <tchar.h>
#include <editor.h>

#define	MIN_LETTER_WORD_POS		5		//最小的单字母词的位置

int		window_logon = 0;				//系统登录状态

/**	对候选进行排序，主要目的是要让单音节的词排在字后面
 */
void SortCandidates(CANDIDATE *candidate_array, int count, int syllable_count)
{
	int i, j, k;
	int spw_count = 0;
	CANDIDATE cand_saves[MIN_LETTER_WORD_POS];

	//Q: 什么时候单音节的候选里有词？
	//A：我只想到两种情形1)xian，lian等可以拆分的音节，"西安""立案"会作为候选词；
	//2)自定义短语的情况，如fang=□；3)超级简拼音的情形，这其中又分两种子情形：
	//a)如sa，"索爱""涉案"作为候选词(假设超级简拼个数已设为2)；b)如tang，先输入
	//t'a'n'g自己造一个词，如"天阿闹个"，再输入tang(假设超级简拼个数已设为4)。但
	//注意，第1)种情况的候选类型为CAND_TYPE_ZI，因此也算成是字的一种；第2)种情况
	//的候选类型为CAND_TYPE_SPW；第3)种情况a)b)的候选类型均为CAND_TYPE_CI，只是
	//a)的字类型为CI_TYPE_LETTER，本函数的排序显然不针对1)，因为1)可以看成是特殊
	//的字，也不针对2)，因为自定义短语的排序在后续的SortSPWCandidates里会单独处
	//理，只针对3)
	if (syllable_count != 1)
		return;

	//由于是一个音节，因此，所有的词都会是单字母输入的，把字提前即可。
	for (i = 0; i < count; i++)
	{
		//记录下自定义短语的个数，自定义短语不参与本函数的排序
		if (candidate_array[i].type == CAND_TYPE_SPW)
		{
			spw_count++;
		}

		//跳过开头的自定义短语，因为它将在SortSPWCandidates里处理；
		//跳过超级简拼词，因为超级简拼最多只能排到MIN_LETTER_WORD_POS
		//以后的位置(准确的说是spw_count+MIN_LETTER_WORD_POS的位置!)
		if (!((candidate_array[i].type == CAND_TYPE_CI && candidate_array[i].word.type == CI_TYPE_LETTER) ||
			candidate_array[i].type == CAND_TYPE_SPW))
			break;
	}

	//打头的就是字
	if (!i)		
		return;

	//将字候选提前(将情况2)中的词至少排到MIN_LETTER_WORD_POS之后)
	for (j = 0; j < MIN_LETTER_WORD_POS && j + i < count; j++)
		cand_saves[j] = candidate_array[j + i];

	//从后向前复制候选
	for (k = i + j; k > j + spw_count; k--)
		candidate_array[k - 1] = candidate_array[k - 1 - j];

	//填充前面的候选
	for (i = 0; i < j; i++)
		candidate_array[i + spw_count] = cand_saves[i];
}

/**	对候选进行排序，主要目的是要让英文单词排在当前页最后一个
 */
void SortEnglishCandidates(PIMCONTEXT *context, CANDIDATE *candidate_array, int count)
{
	int i, j, index;
	CANDIDATE cand_temp[1];

	for (i = 0; i < count; i++)
	{
		if ((candidate_array[i].type == CAND_TYPE_SPW) && (candidate_array[i].spw.type == SPW_STIRNG_ENGLISH))
		{
			cand_temp[0] = candidate_array[i];
			break;
		}
	}

	//没找到
	if (i == count)
		return;

	index = pim_config->candidates_per_line;
	if (index > count)
		index = count;

	index--;

	//正好在合适的位置，不用移动
	if (i == index)
		return;

	//把前面的往后移
	for (j = i; j < index; j++)
		candidate_array[j] = candidate_array[j + 1];

	//把后面的往前移
	for (j = i; j > index; j--)
		candidate_array[j] = candidate_array[j - 1];

	candidate_array[index] = cand_temp[0];

	return;
}

/**	对候选进行排序，主要目的是要让单个短语候选排在当前页第3个
 */
void SortSPWCandidates(PIMCONTEXT *context, CANDIDATE *candidate_array, int count, int eng_count)
{
	int i, index, maxIndex;
	CANDIDATE cand_temp[1];
	cand_temp[0] = candidate_array[0];

	maxIndex = pim_config->candidates_per_line;
	if (maxIndex > count)
		maxIndex = count;

	maxIndex--;
	if(eng_count == 1)
		maxIndex--;
	index = maxIndex;
	//为兼容以往的配置，spw_position的数值从11~19，使用时直接减10
	if(maxIndex > pim_config->spw_position - 10 - 1)
		index = pim_config->spw_position - 10 - 1;
	if(candidate_array->spw.hint 
		&& _tcslen((TCHAR *)candidate_array->spw.hint)==3 
		&& ((TCHAR *)candidate_array->spw.hint)[1]>TEXT('0') 
		&& ((TCHAR *)candidate_array->spw.hint)[1]<=TEXT('9')){
		index = ((TCHAR *)candidate_array->spw.hint)[1] - TEXT('0') - 1;
		if(index > maxIndex)
			index = maxIndex;
	}
	//正好在合适的位置，不用移动
	if (0 == index)
		return;
	//把后面的往前移
	for (i = 0; i < index; i++)
		candidate_array[i] = candidate_array[i + 1];
	candidate_array[index] = cand_temp[0];
	return;
}

/**	对候选进行排序，主要目的是要让联想词词排在适当的位置
 */
void SortWildcardCandidates(PIMCONTEXT *context, CANDIDATE *candidate_array, int count, int wildcard_count)
{
	int i, j, k, index, last = 0;
	CANDIDATE cand_temp[1];

	index = pim_config->suggest_word_location - 1;

	//如果是自定义短语，则把联想的词位置推后
	for ( i = index; i < count; i++ )
	{
		if ( candidate_array[i].type == CAND_TYPE_SPW )
			index++;
		else
			break;
	}

	for (i = 0; i < wildcard_count; i++)
	{
		for (j = last; j < count; j++)
		{
			if (candidate_array[j].word.type == CI_TYPE_WILDCARD)
			{
				cand_temp[0] = candidate_array[j];
//				cand_temp[0].word.type = CI_TYPE_NORMAL;
				break;
			}
		}

		//没找到
		if (j == count)
			return;

		last = j;

		//正好在合适的位置，不用移动
		if (j == index)
		{
			index++;
			continue;
		}

		//把前面的往后移
		for (k = j; k < index; k++)
			candidate_array[k] = candidate_array[k + 1];

		//把后面的往前移
		for (k = j; k > index; k--)
			candidate_array[k] = candidate_array[k - 1];

		candidate_array[index] = cand_temp[0];

		index++;
	}

	return;
}

int UnifyCandidates(PIMCONTEXT *context, CANDIDATE *candidate_array, int count, int spw_count)
{
	int i, j, k, ret;

	for (i = 0; i < spw_count; i++)
	{
		if (candidate_array[i].type != CAND_TYPE_SPW && candidate_array[i].spw.type != SPW_STRING_NORMAL)
			continue;

		for (j = spw_count; j < count; j++)
		{
			ret = 0;

			switch (candidate_array[j].type)
			{
			case CAND_TYPE_ZI:
				if (candidate_array[j].hz.is_word)
				{
					if (candidate_array[j].hz.word_item->ci_length == candidate_array[i].spw.length)
						ret = !_tcsncmp(candidate_array[i].spw.string,
										GetItemHZPtr(candidate_array[j].hz.word_item),
										candidate_array[i].spw.length);
				}
				else if (1 == candidate_array[i].spw.length)
					ret = !_tcsncmp(candidate_array[i].spw.string,
									&(TCHAR)candidate_array[j].hz.item->hz,
									1);

				break;

			case CAND_TYPE_CI:
				if (candidate_array[j].word.item->ci_length == candidate_array[i].spw.length)
					ret = !_tcsncmp(candidate_array[i].spw.string,
									candidate_array[j].word.hz,
									candidate_array[i].spw.length);

				break;
			}

			if (ret)
			{
				//从后往前挪
				for (k = j; k < count - 1; k++)
					candidate_array[k] = candidate_array[k + 1];

				count--;
				j--;
			}
		}
	}

	return count;
}

//判断短语是不是在候选首位
int IsFirstPosSPW(CANDIDATE *candidate_array)
{
	int index;
	if(!candidate_array)
		return 0;
	if(candidate_array[0].type != CAND_TYPE_SPW)
		return 0;
	if(candidate_array[0].spw.hint 
		&& _tcslen((TCHAR *)candidate_array[0].spw.hint)==3 
		&& ((TCHAR *)candidate_array[0].spw.hint)[1]>TEXT('0') 
		&& ((TCHAR *)candidate_array[0].spw.hint)[1]<=TEXT('9'))
	{
		index = ((TCHAR *)candidate_array[0].spw.hint)[1] - TEXT('0');
		if(index == 1)
			return 1;
	}
	else if(pim_config->spw_position == 11){
		return 1;
	}
	return 0;
}

/**	基于用户当前的输入，获取候选。
 *	参数：
 *		context				输入上下文
 *		input_string		用户输入数据
 *		syllables			音节数组
 *		syllable_count		音节数组长度
 *		candidate_array		候选指针
 *		array_length		候选数组的长度
 *		need_spw			是否需要短语候选
 *	返回：
 *		候选数目。
 *	注意：
 *		获取ICW的时候，如果这段输入已经在词库中，则ICW返回为0。
 *		获取词的时候，先从最长的拼音开始，依次递减直到词的长度等于2
 *	候选优先级：
 *		1. SPW
 *      2. url
 *		3. FULL_CI（完整拼音的词）
 *		4. ICW（在前两个不存在的情况下）
 *		5. PART_CI（减少音节长度的词）
 *		6. ZI
 */
int GetCandidates(PIMCONTEXT *context,
				  const TCHAR *input_string,
				  SYLLABLE *syllables,
				  int syllable_count,
				  CANDIDATE *candidate_array,
				  int array_length,
				  int need_spw)
{
	CANDIDATE *candidate_array_save = candidate_array;
	int count, icw_count, spw_count, /*url_count,*/ ci_count, zi_count, eng_count, wildcard_count;
	int has_star;				//是否包含通配符
	int i, j, syllable_index;
	TCHAR new_input_string[MAX_INPUT_LENGTH + 0x10] = {0};
	SYLLABLE new_syllables[MAX_SYLLABLE_PER_INPUT + 0x10] = {0};
	int new_syllable_count = 0;
	int legal_length;

	//用于正向解析拼音串，FULL_CI和ICW都要进行正向解析
	SYLLABLE other_syllables[MAX_SYLLABLE_PER_INPUT] = {0};
	int other_count = 0;

	//用于将单字的音节拆分(小音节拆分)来构成新词，如xian'shi->xi'an'shi(西安市)，其中
	//small_syllables_arrays用来存放拆分结果，如果拼音串中有k个字可以拆分，则理论上应
	//有2^k种拆分结果；small_arrays_lengths是small_arrays_count中各个拼音串的音节数，
	//small_arrays_count是实际拆分结果的数目(由于拼音串的长度收到限制，某些拆分方案导
	//致拼音串长度过大，应排除，所以实际拆分结果不一定有2^k种)。带有other的变量含义类
	//似，只是针对的是正向解析的结果(因为智能组词也要使用小音节拆分的结果，且正逆向解
	//析都要使用，因此在FULL_CI中需为正逆向小音节拆分配不同的内存，而不能相互覆盖)
	SYLLABLE *small_syllables_arrays = (SYLLABLE*)malloc((2 << MAX_SMALL_SYLLABLES) * MAX_SYLLABLE_PER_INPUT * sizeof(SYLLABLE));
	SYLLABLE *small_other_syllables_arrays = (SYLLABLE*)malloc((2 << MAX_SMALL_SYLLABLES) * MAX_SYLLABLE_PER_INPUT * sizeof(SYLLABLE));
	int *small_arrays_lengths = (int*)malloc((2 << MAX_SMALL_SYLLABLES) * sizeof(int));
	int *small_other_arrays_lengths = (int*)malloc((2 << MAX_SMALL_SYLLABLES) * sizeof(int));
	int small_arrays_count = 0;
	int small_other_arrays_count = 0;

	//增设该变量是为了解决下述问题：
	//先输入liang'ang，造一个词"两昂"。再输入liangangren，此时输入法自动将其分解为
	//lian'gang'ren，按两次left键，光标位于gang前，但此时的候选词却有"两昂"。原因是
	//GetOtherSyllableParse的第2个参数原来为syllables - context->syllables，对于非
	//智能编辑状态，这个值等于context->syllable_pos，即尚未转化为汉字的第一个音节位
	//置，对于上述按left键的情况，并没有任何音节转化为汉字，因此context->syllable_pos
	//为0，所以GetOtherSyllableParse从lian开始进行逆向解析，解析出liang'ang这个部分
	//词。现在使用了start_pos以后，上述情况下start_pos为1，从gang开始解析，比较合理
	int start_pos = 0;

	assert(candidate_array && array_length);
	count = icw_count = spw_count = ci_count = zi_count = eng_count = wildcard_count = 0;
	legal_length = GetLegalPinYinLength(context->input_string, context->state, context->english_state);
	if (context->input_pos >= legal_length)
		return 0;
	has_star = 0;
	for (i = 0; i < syllable_count; i++)
		if (syllables[i].con == CON_ANY && syllables[i].vow == VOW_ANY)
		{
			has_star = 1;
			break;
		}

	//0. 英文输入/候选模式
	//英文输入模式和候选模式的区别：英文输入模式是一直使用英文输入，
	//英文候选模式在输入中文时临时切换过去的，目的是临时输入一个英文
	//单词，选择单词以后下次再输入还是显示中文候选
	context->has_english_candidate = 0;
	if (input_string && context->english_state != ENGLISH_STATE_NONE)
	{
		eng_count = GetEnglishCandidates(input_string, candidate_array, array_length);
		count += eng_count;

		context->has_english_candidate = 1;

		//英文输入法和从步骤1开始的中文输入法是独立的，
		//因此若是英文输入法这里可以返回了
		if (count >= array_length)
			return array_length;
		else
			return count;
	}

	//1. SPW
	if (need_spw && input_string)
	{
		spw_count = GetSpwCandidates(context, input_string, candidate_array, array_length);
		count += spw_count;
		if (count >= array_length)
			return array_length;
	}

	//获取光标之前、尚未转化为汉字部分的音节数
	if (context->state == STATE_IEDIT)
		syllable_index = 0;
	else
		syllable_index = (context->compose_cursor_index) ? GetSyllableIndexByComposeCursor(context, context->compose_cursor_index) : 0;
	for (i = 0; i < context->selected_item_count; i++)
		syllable_index -= context->selected_items[i].syllable_length;

	//不甚明确，只知道syllable_count表示的是尚未转化为汉字的音节总数，syllables则是这些音节。
	//若光标在最后/最前，则new_syllables就是syllables；若光标不在最后/最前，则new_syllables是
	//光标之后的音节(其实可以看到该奇怪的现象，设compose_string为"今天xiawuhui|jiama"，|表示
	//光标，我们可以看到候选词显示的是"价码"等)
	i = (!context->compose_cursor_index || context->compose_cursor_index == context->compose_length) ? 0 : syllable_index;
    start_pos = i; //new_syllables总是正确的候选音节，记录下其偏移值(后面调用GetOtherSyllableParse时会使用)
	for (j = 0; i < syllable_count; i++, j++, new_syllable_count++)
		new_syllables[j] = syllables[i];

	//同上，若光标在最后/最前，new_input_string就是input_string；否则new_input_string是input_string
	//光标之后的部分
	if (input_string)
	{
		i = (!context->cursor_pos || context->cursor_pos == context->input_length) ? 0 : context->cursor_pos;
		for (j = 0; i < context->input_length; i++, j++)
			new_input_string[j] = input_string[i];
	}

	if (!context->selected_item_count && !has_star)
		context->syllable_mode = 1;
	if (spw_count)
		context->syllable_mode = 0;

	//2. FULL_CI：完整的词(例如zhongguogongchandang，候选显示"中国共产党"，通常候选的显示顺序是
	//完整词、部分词、单字)
	if (new_syllable_count /*syllable_count*/ > 0)
	{
		int last_count = count;
		int ci_count_sav;

		//逆向解析结果
		ci_count = ProcessCiCandidate(
			new_syllables, //syllables,
			new_syllable_count, //syllable_count,
			context->selected_item_count ? 0 : new_input_string, //input_string,
			candidate_array + count,
			array_length - count,
			context->selected_item_count ? 1 : 0); //最后一个参数意义不明
		count += ci_count;

		//后4种解析方式可能需要记录原始音节
		ci_count_sav = count;

		//小音节拆分
		small_arrays_count = GetSmallSyllablesParse(new_syllables, new_syllable_count, small_syllables_arrays, small_arrays_lengths);
		for (i = 0; i < small_arrays_count; i++)
		{
			ci_count = ProcessCiCandidate(
				small_syllables_arrays + i * MAX_SYLLABLE_PER_INPUT,
				small_arrays_lengths[i], 
				0,                       //不考虑超级简拼，避免情况过于复杂
				candidate_array + count,
				array_length - count,
				context->selected_item_count || context->state == STATE_IEDIT ? 1 : 0); //最后一个参数意义不明
			count += ci_count;
		}

		//正向解析结果
		//Q：为何ji'er'c(接入层)无法正向解析返回0？
		//A：该拼音串不是完整音节，见GetOtherSyllableParse::IsFullSyllable
		other_count = GetOtherSyllableParse(
			context,
			//syllables - context->syllables不能替换为context->syllable_pos，智能编辑
			//状态时二者取值不同(见MakeCandidate中context->state == STATE_IEDIT的情况)
			start_pos + syllables - context->syllables, 
			new_syllable_count, 
			other_syllables,
			MAX_SYLLABLE_PER_INPUT);
		if (other_count)
		{
			ci_count = ProcessCiCandidate(
				other_syllables,
				other_count,        //这里原代码是new_syllable_count，不明，个人认为是other_count才合理
				0,                  //不考虑超级简拼，之前逆向解析时应该已经处理过了
				candidate_array + count,
				array_length - count,
				context->selected_item_count || context->state == STATE_IEDIT ? 1 : 0); //最后一个参数意义不明
			count += ci_count;

			//小音节拆分
			small_other_arrays_count = GetSmallSyllablesParse(other_syllables, other_count, small_other_syllables_arrays, small_other_arrays_lengths);

			for (i = 0; i < small_other_arrays_count; i++)
			{
				ci_count = ProcessCiCandidate(
					small_other_syllables_arrays + i * MAX_SYLLABLE_PER_INPUT,
					small_other_arrays_lengths[i], 
					0,                       //不考虑超级简拼，避免情况过于复杂
					candidate_array + count,
					array_length - count,
					context->selected_item_count || context->state == STATE_IEDIT ? 1 : 0); //最后一个参数意义不明
				count += ci_count;
			}

			//混合解析问题(只有正逆向解析都不同时才可能存在混合解析)
			//有的音节部分是正向解析的，部分是逆向解析的，比如"反感这个方案的问题"：输入fanganzhegefangan，
			//先选择"反感"，后选择"这个方案"，该词被加入到用户词库，但下次再输入fanganzhegefangan，候选中却
			//没有"反感这个方案"，造成该问题的原因是：fanganzhegefangan正向解析结果为fang'an'zhe'ge'fang'an
			//，逆向解析结果为fan'gan'zhe'ge'fan'gan，它们都和fan'gan'zhe'ge'fang'an(正确的拼音，也是context
			//->result_syllables的值(见PostResult))不同，因此虽然该词连同正确的音节被存入了用户词库，但下次
			//再输入fanganzhegefangan，正逆向解析的结果都无法解析出fan'gan'zhe'ge'fang'an，自然也就无法从词
			//库中查到该词了。解决这个问题最直接的方法就是遍历所有解析音节方式的组合，搜索候选词，但这无疑效
			//率太低。我们采取的方法是以new_syllables、small_syllables_arrays、other_syllables、small_other_syllables_arrays
			//的头两个音节为基础进行词语联想(取头两个音节是为了保证没有混合解析的情况，例如fanganfangan，若
			//取3个音节，就已经涉及到混合解析(fan'gan'fang、fan'gan'fan、fang'an'fang、fang'an'fan)，混合解
			//析不属于上面四种解析结果的任意一种，不能解决问题；若取两个音(fan'gan、fang'an)，分别属于new_syllables
			//和other_syllables)，同时词语联想的范围限于用户词库中4个音节以上的词(至少要fanganfangan这样4个
			//音节的拼音串才可能产生混合解析)，这样就能以较小的代价找出混合解析的候选词
			if (!has_star && !HasSyllableSeparator(context->input_string + context->input_pos))
			{
				int mixed_save_count = count;
				int per_syllable_count;
				TCHAR syllable_string[MAX_SYLLABLE_PER_INPUT * MAX_PINYIN_LENGTH + 1] = {0};

				//获取候选词
				count += GetMixedParseCiCandidate(
					new_syllables, 
					new_syllable_count, 
					candidate_array + count, 
					array_length - count);

				for (i = 0; i < small_arrays_count; i++)
				{
					count += GetMixedParseCiCandidate(
						small_syllables_arrays + i * MAX_SYLLABLE_PER_INPUT,
						small_arrays_lengths[i],
						candidate_array + count,
						array_length - count);
				}

				count += GetMixedParseCiCandidate(
					other_syllables, 
					other_count, 
					candidate_array + count, 
					array_length - count);

				for (i = 0; i < small_other_arrays_count; i++)
				{
					count += GetMixedParseCiCandidate(
						small_other_syllables_arrays + i * MAX_SYLLABLE_PER_INPUT,
						small_other_arrays_lengths[i],
						candidate_array + count,
						array_length - count);
				}

				//验证拼音是否一致
				for (i = mixed_save_count; i < count; i++)
				{	
					per_syllable_count = 0;
					for (j = 0; j < (int)candidate_array[i].word.item->syllable_length; j++)
					{
						//词库里的音调为垃圾值，应忽略掉，否则GetSyllableString可能会出错
						//Q：为什么要使用变量syllable？
						//A：给candidate_array[i].word.item->syllable[j].tone赋值是危险的
						//举动，因为item指向的是词库中的内容，而词库属于共享内存的一部分，
						//如果有多个进程在使用输入法，很可能由于共享内存访问冲突而崩溃！
						SYLLABLE syllable = candidate_array[i].word.item->syllable[j];
						syllable.tone = TONE_0;
						per_syllable_count += GetSyllableString(
							syllable, 
							syllable_string + per_syllable_count, 
							MAX_SYLLABLE_PER_INPUT * MAX_PINYIN_LENGTH + 1 - per_syllable_count, 
							0);
					}

					//这里要求完全相等而不能有任何模糊，例如先输入了fanganzhegefangan，然后设置
					//h->f模糊，再输入hanganzhegefangan，候选词不会出现"反感这个方案"。因为对于
					//还没有划分音节的拼音串，要进行模糊音判断十分困难，暂时没想到好的解决方案。
					//同理也不能有任何简拼，例如fangganzgfangan，候选词也不会出现"反感这个方案"
					if (_tcscmp(syllable_string, context->input_string + context->input_pos))
					{
						candidate_array[i] = candidate_array[count - 1];
						count--;
						i--;
					}
				}

				//去重，我们的算法根据拼音串的头两个音节进行联想，很可能，比如new_syllables和
				//other_syllables的头两个音节是相同的，因此得到的联想词也相同，需要进行去重
				count = last_count + UnifyCiCandidates(candidate_array + last_count, count - last_count);
			}
		}

		//记录原始的音节数(逆向解析的音节数)，貌似主要用于调整光标位置，因为用户在编码窗
		//口中看到的总是逆向解析结果
		for (i = ci_count_sav; i < count; i++)
		{
			candidate_array[i].word.type = CI_TYPE_OTHER;
			candidate_array[i].word.origin_syllable_length = syllable_count;
		}

		//ProcessCiCandidate内部已排序，但还要整体再排一次，例如"xianshi(西安市)"这个词是
		//逆向小音节拆分解析出的，我们输入一次"西安市"后，下次再输入"西安市"，按CiCache原
		//理，本来它应该排在所有拼音为xianshi的词的最前面，但如果只是ProcessCiCandidate内
		//部排序的话，它只会排在拼音为xi'an'shi的词的最前面，而在所有候选中处于较后面的位
		//置，这显得不够合理
		SortCiCandidates(candidate_array + last_count, count - last_count);

		if (count >= array_length)
		{
			//返回前先释放
			free(small_syllables_arrays);
			free(small_other_syllables_arrays);
			free(small_arrays_lengths);
			free(small_other_arrays_lengths);

			return array_length;
		}
	}

	if (!has_star)
	{
		//3. ICW（前提：full ci没有候选，spw如果有候选，而且只有一个）
		//智能组词也要加入正向音节拆分的串，否则组词结果较差，比如：
		//wo'bu'tai'xi'huan'jia'pin'gao'de'shu->wo'bu'tai'xi'huan'jia'ping'ao'de'shu 我不太喜欢贾平凹的书
		//yi'jia'nan'zhuang->yi'jian'an'zhuang 一键安装
		//xian'zai'mei'you'min'ge'le->xian'zai'mei'you'ming'e'le 现在没有名额了
		//tan'lia'nai'de'ren->tan'lian'ai'de'ren 谈恋爱的人
		if((count == 0 || (count == 1 && spw_count == 1)) && (pim_config->use_icw && new_syllable_count >= 2))
		{
			//只有一个候选短语且在候选首位，不使用智能组词
			if(!(count == 1 && IsFirstPosSPW(candidate_array)))
			{ 
				extern int NewGetIcwCandidates(SYLLABLE *, int, CANDIDATE *, double *);

				int has_icw_candidate = 0;
				double current_value, max_value;

				//普通逆向解析结果
				icw_count = NewGetIcwCandidates(new_syllables, new_syllable_count, candidate_array + count, &current_value); //icw_count只能取0或1
				has_icw_candidate = icw_count; //任意一种音节解析下如果存在智能组词候选，此值即为1
				max_value = current_value;
				count += icw_count;

				//逆向小音节拆分
				for (i = 0; i < small_arrays_count; i++)
				{
					icw_count = NewGetIcwCandidates(small_syllables_arrays + i * MAX_SYLLABLE_PER_INPUT,
						small_arrays_lengths[i], candidate_array + count, &current_value);

					//之前是否有候选
					if (!has_icw_candidate)
					{
						if (icw_count)
						{
							has_icw_candidate = icw_count;
							count += icw_count;
						}
					}
					else
					{
						//之前、当前候选都存在的情况
						if (icw_count)
						{
							//取概率较大者
							if (current_value > max_value)
							{
								max_value = current_value;
								candidate_array[count - 1] = candidate_array[count];
							}

							//无论如何最终的candidate_array里只保留一个智能组词结果，
							//由于加入之前候选时count已经加了1，这里不用count--
						}
					}
				}

				//普通正向解析结果
				if (other_count)
				{
					icw_count = NewGetIcwCandidates(other_syllables, other_count, candidate_array + count, &current_value);

					//之前是否有候选
					if (!has_icw_candidate)
					{
						if (icw_count)
						{
							has_icw_candidate = icw_count;
							count += icw_count;
						}
					}
					else
					{
						//之前、当前候选都存在的情况
						if (icw_count)
						{
							//取概率较大者
							if (current_value > max_value)
							{
								max_value = current_value;
								candidate_array[count - 1] = candidate_array[count];
							}

							//无论如何最终的candidate_array里只保留一个智能组词结果，
							//由于加入之前候选时count已经加了1，这里不用count--
						}
					}

					//正向小音节拆分
					for (i = 0; i < small_other_arrays_count; i++)
					{
						icw_count = NewGetIcwCandidates(small_other_syllables_arrays + i * MAX_SYLLABLE_PER_INPUT,
							small_other_arrays_lengths[i], candidate_array + count, &current_value);

						//之前是否有候选
						if (!has_icw_candidate)
						{
							if (icw_count)
							{
								has_icw_candidate = icw_count;
								count += icw_count;
							}
						}
						else
						{
							//之前、当前候选都存在的情况
							if (icw_count)
							{
								//取概率较大者
								if (current_value > max_value)
								{
									max_value = current_value;
									candidate_array[count - 1] = candidate_array[count];
								}

								//无论如何最终的candidate_array里只保留一个智能组词结果，
								//由于加入之前候选时count已经加了1，这里不用count--
							}
						}
					}
				}

				if (count >= array_length)
				{
					//返回前先释放
					free(small_syllables_arrays);
					free(small_other_syllables_arrays);
					free(small_arrays_lengths);
					free(small_other_arrays_lengths);

					return array_length; 
				}
			}
		}

		//无论是否使用了进行智能组词，small_syllables_arrays和small_arrays_lengths
		//后面都不会再用了，应立即释放(注意不要最后再释放，因为中途有很多return语句)
		free(small_syllables_arrays);
		free(small_other_syllables_arrays);
		free(small_arrays_lengths);
		free(small_other_arrays_lengths);

		//4. PART_CI
		for (i = new_syllable_count/*syllable_count*/ - 1; i >= 2; i--)
		{
			int other_count;
			int last_count = count;
			SYLLABLE other_syllables[MAX_SYLLABLE_PER_INPUT];

            //逆向解析部分词
			ci_count = ProcessCiCandidate(
				new_syllables, //syllables,
				i,
				0,
				candidate_array + count,
				array_length - count,
				context->selected_item_count || context->state == STATE_IEDIT ? 1 : 0);

			count += ci_count;

			//正向解析部分词
			//Q：为什么不直接利用上面的other_syllables？
			//A：从理论上来说直接用other_syllables没有什么问题，但在逻辑上不够好，假设
			//有2个逆向解析的音节，其正向解析结果为3个音节(实际中可能不存在这种情况，但
			//如果以后要考虑小音节，则这种设想是合理的)，那么逆向部分词应为2个字，而正
			//向部分词应为3个字(而不是2个字，为了保持某种意义上的对应)
			other_count = GetOtherSyllableParse(
				context, 
				//syllables - context->syllables不能替换为context->syllable_pos，智能编辑
				//状态时二者取值不同(见MakeCandidate中context->state == STATE_IEDIT的情况)
				start_pos + syllables - context->syllables, 
				i, 
				other_syllables,
				MAX_SYLLABLE_PER_INPUT);

			if (other_count)
			{
				int ci_count_sav = count;

				ci_count = ProcessCiCandidate(
					other_syllables,
					other_count,
					0,
					candidate_array + count,
					array_length - count,
					context->selected_item_count || context->state == STATE_IEDIT ? 1 : 0);

				count += ci_count;

				for (j = ci_count_sav; j < count; j++)
				{
					candidate_array[j].word.type = CI_TYPE_OTHER;
					candidate_array[j].word.origin_syllable_length = i;
				}

				if (ci_count)
					SortCiCandidates(candidate_array + last_count, count - last_count);
			}

			if (count >= array_length)
				return array_length;
		}

		//5. ZI
		if (new_syllable_count/*syllable_count*/)
		{
			int zi_count_save = count;
			int small_ci_count = 0;
			int small_ci_count_save = 0;
			int normal_zi_count = 0;   //普通字，如qiangang中的qian和qiang
			int small_zi_count = 0;    //小音节拆分字，如qiangang中的qi
			CANDIDATE candidate_temp;

			//逆向解析(qiangang，候选音节为qian)
			zi_count = ProcessZiCandidates(
							new_syllables[0], //syllables[0],
							candidate_array + count,
							array_length - count,
							context->zi_set_level);
			count += zi_count;
			
			//正向解析(qiangang，候选音节为qiang)
			if (other_count && !SameSyllable(new_syllables[0], other_syllables[0]))
			{
				zi_count = ProcessZiCandidates(
					other_syllables[0], //syllables[0],
					candidate_array + count,
					array_length - count,
					context->zi_set_level);
				count += zi_count;

				//单字和小音节词去重，例如上面的qian和qiang，拆分后都有qi，
				//因此单字有重复。qi'an和qi'ang正常情况下不会产生重复的小音
				//节词，但若设置了an->ang的韵母模糊，仍会产生重复候选，因此
				//也必须去重。下面将小音节词放在前面，单字放在后面，然后对
				//两个区域分别去重
				for (i = zi_count_save; i < count; i++)
				{
					if (candidate_array[i].hz.is_word)
					{
						small_ci_count_save++;
					}
				}

				i = zi_count_save;
				j = count - 1;

				while (1)
				{
					while (i < j && candidate_array[i].hz.is_word)
						i++;
					while(i < j && !candidate_array[j].hz.is_word)
						j--;
					if (i < j)
					{
						candidate_temp = candidate_array[i];
						candidate_array[i] = candidate_array[j];
						candidate_array[j] = candidate_temp;
					}
					else
					{
						break;
					}
				}
				
				small_ci_count = UnifySmallCiCandidates(
					candidate_array + zi_count_save,
					small_ci_count_save);

				//向前移动候选
				memmove(candidate_array + zi_count_save + small_ci_count, 
					candidate_array + zi_count_save + small_ci_count_save, 
					(count - small_ci_count_save) * sizeof(CANDIDATE));
				count -= small_ci_count_save - small_ci_count;

				zi_count = UnifyZiCandidates(
					candidate_array + zi_count_save + small_ci_count, 
					count - zi_count_save - small_ci_count);

				count = zi_count_save + small_ci_count + zi_count;
			}
			
			//将普通字(包括小音节词放在前面)，小音节字放在后面，并且
			//这两个部分内部再进行排序
			for (i = zi_count_save; i < count; i++)
			{
				if (!candidate_array[i].hz.is_word && candidate_array[i].hz.hz_type == ZI_TYPE_OTHER)
				{
					small_zi_count++;
				}
			}

			i = zi_count_save;
			j = count - 1;
			
			while (1)
			{
				while (i < j && (candidate_array[i].hz.is_word || (candidate_array[i].hz.hz_type == ZI_TYPE_NORMAL)))
					i++;
				while(i < j && (!candidate_array[j].hz.is_word && (candidate_array[j].hz.hz_type == ZI_TYPE_OTHER)))
					j--;
				if (i < j)
				{
					candidate_temp = candidate_array[i];
					candidate_array[i] = candidate_array[j];
					candidate_array[j] = candidate_temp;
				}
				else
				{
					break;
				}
			}

			normal_zi_count = count - zi_count_save - small_zi_count; //包括小音节词

			SortZiCandidates(candidate_array + zi_count_save, normal_zi_count);
			SortZiCandidates(candidate_array + zi_count_save + normal_zi_count, small_zi_count);

			if (count >= array_length)
				return array_length;
		}
	}
	else
	{
		//如果不能进行智能组词，那么small_syllables_arrays和small_arrays_lengths
		//可以释放了(注意不要最后再释放，因为中途有很多return语句)
		free(small_syllables_arrays);
		free(small_other_syllables_arrays);
		free(small_arrays_lengths);
		free(small_other_arrays_lengths);
	}


	//6. 英文单词
	//Q：这里的英文单词和第0步中的有何区别？
	//A：这里不是处理英文输入法的，而是针对配置面板中的高级设置/英文输入/中文输入候选词包含英文单词，
	//不过需要注意的是此种模式下至多包含一个英文单词(因为下面的GetEnglishCandidates的最后一个参数为1)，
    //例如输入he'l'l，hell会作为候选词，但hello不会
	if (pim_config->use_english_input && input_string &&  
		(!context->compose_cursor_index || context->compose_cursor_index == context->compose_length))
	{
		eng_count = GetEnglishCandidates(input_string, candidate_array + count, 1);

		//还要进一步筛选，一些太短的词可能被排除，例如输入we，后选中可能不会出现we
		if (context->input_length > 2 && (STATE_EDIT == context->state || STATE_ABC_SELECT == context->state) && eng_count > 0)
			context->has_english_candidate = 1;
		if (0 == count && eng_count > 0 && context->input_length > 1)
			context->has_english_candidate = 1;

		//上面的判断没有通过，则表明eng候选没用上，把eng_count清零
		if(!context->has_english_candidate)
			eng_count = 0;
		else{
			if(pim_config->candidate_include_english)
				count += eng_count;
			else
				eng_count = 0;
		}
		if (count >= array_length)
			return array_length;
	}

	if (!has_star)
	{
		//7. 词语联想
		if (pim_config->use_word_suggestion && pim_config->suggest_word_count > 0 &&
			syllable_count == context->syllable_count && syllable_count >= pim_config->suggest_syllable_location &&
			syllables[syllable_count - 1].con != CON_ANY && syllables[syllable_count - 1].vow != VOW_ANY)
		{
			int i;
			SYLLABLE  new_syllables[MAX_SYLLABLE_PER_INPUT + 1] = {0};
			CANDIDATE new_candidates[MAX_ICW_CANDIDATES] = {0};

			for (i = 0; i < syllable_count; i++)
				new_syllables[i] = syllables[i];

			//在末尾加上通配符
			new_syllables[syllable_count].con  = CON_ANY;
			new_syllables[syllable_count].vow  = VOW_ANY;
			new_syllables[syllable_count].tone = TONE_0;
			wildcard_count = ProcessCiCandidate(new_syllables, syllable_count + 1, 0, new_candidates, MAX_ICW_CANDIDATES, 0);
			for (i = 0; i < pim_config->suggest_word_count && i < wildcard_count; i++)
			{
				candidate_array[count] = new_candidates[i];
				candidate_array[count].word.type = CI_TYPE_WILDCARD;
				count++;
			}
		}
	}

	//前面的ProcessCiCandidate等函数里也有排序步骤，如SortCiCandidates，
	//但这只是针对某种候选类型(如词)内部的排序，下面的排序可能涉及到不同
	//的候选类型

	//8.对候选进行排序
	SortCandidates(candidate_array, count, syllable_count);

	//9.对联想输入进行排序
	if (wildcard_count > 0)
	{
		int nCount;
		if (wildcard_count < pim_config->suggest_word_count)
			nCount = wildcard_count;
		else
			nCount = pim_config->suggest_word_count;
		SortWildcardCandidates(context, candidate_array, count, nCount);
	}

	//10.针对spw候选去重(spw可能恰好定义得与字、词候选相同)
	if (spw_count)
		count = UnifyCandidates(context, candidate_array, count, spw_count);

	//对短语位置进行设定，默认为3(因为pim_config->spw_position默认为13)，
	//暂时不支持修改。适用于有短语候选，但候选中不只是短语的情况，只对单
	//一短语候选有效。为兼容以往的配置，spw_position的数值从11~19，使用时
	//直接减10
	if(spw_count == 1 && spw_count != count && count > 0 && candidate_array[0].spw.type == SPW_STRING_NORMAL){
		SortSPWCandidates(context, candidate_array, count, eng_count);
	}

	//11.对英文候选进行排序
	if (1 == eng_count && (count > 0 || context->state == STATE_IINPUT) && context->has_english_candidate)
	{
		SortEnglishCandidates(context, candidate_array, count);
	}

	//如果只有一个候选，而且是英文的，不显示。因为经常是想输入MCC、GR13这种，而英文候选会使得空格后的输出和想输入的不一致
	//双拼时，E作为直通车的首字母，所以除外
	//全拼时，u除外
	if(1 == eng_count && count == 1 && spw_count == 0 )
		count = 0;

	return count;
}