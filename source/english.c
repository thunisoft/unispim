/**	英文模块
 *	英文单词的读取以及检索功能。
 */

#include <assert.h>
#include <config.h>
#include <utility.h>
#include <english.h>
#include <tchar.h>
#include <share_segment.h>

static ENGLISHWORDLIB *eng_wordlib   = 0;
static ENGLISHTRANSLIB *eng_translib = 0;

static TCHAR *english_share_name  = TEXT("HYPIM_ENGLISH_SHARED_NAME");
static TCHAR *engtrans_share_name = TEXT("HYPIM_ENGTRANS_SHARED_NAME");

//#pragma data_seg(HYPIM_SHARED_SEGMENT)
//static int english_loaded  = 0;		//英文词典是否已经在内存中
//static int engtrans_loaded = 0;		//英文翻译是否已经在内存中
//#pragma	data_seg()


/**	加载英文词典到内存。
 *	参数：
 *		file_name			英文词典文件全路径
 *	返回：
 *		成功：1
 *		失败：0
 */
int LoadEnglishData(const TCHAR *file_name)
{
	int file_length;

	assert(file_name);

	if (share_segment->english_loaded)
		return 1;

	file_length = GetFileLength(file_name);
	if (file_length <= 0)
		return 0;

	eng_wordlib = AllocateSharedMemory(english_share_name, file_length);
	if (!eng_wordlib)
		return 0;

	if ((file_length = LoadFromFile(file_name, eng_wordlib, file_length)) == -1)
	{
		FreeSharedMemory(english_share_name, eng_wordlib);
		Log(LOG_ID, L"英文词典文件打开失败。name=%s", file_name);
		return 0;
	}

	if (!file_length)
		return 0;

	share_segment->english_loaded = 1;

	return 1;
}

/**	释放英文词典文件
 */
int FreeEnglishData()
{
	share_segment->english_loaded = 0;

	if (eng_wordlib)
	{
		FreeSharedMemory(english_share_name, eng_wordlib);
		eng_wordlib = 0;
	}

	return 1;
}

/**	检索英文词典，获得候选，放入候选数组中。
 *	参数：
 *		prefix				英文单词前缀名称
 *		candidate_array		候选缓冲区
 *		array_length		候选数组长度
 *	返回：
 *		检索到的短语候选数目
 */
int GetEnglishCandidates(const TCHAR *prefix, CANDIDATE *candidate_array, int array_length)
{
	int i, m, n, prefix_len;
	int count = 0, has_star = 0, is_true;
	char first_letter;
	char prefix_char[ENGLISH_WORD_MAX_SIZE] = {0};
	char *english_str;
	extern int LoadEnglishResource();

	assert(prefix && candidate_array);

	if (!pim_config->use_english_input)
		return 0;

	if(!share_segment->english_loaded)
		LoadEnglishResource();

	if (!eng_wordlib)
	{
		eng_wordlib = GetReadOnlySharedMemory(english_share_name);

		//可能存在其他进程已经装载了，但是退出后共享内存被释放的问题
		if (!eng_wordlib && share_segment->english_loaded)
		{
			share_segment->english_loaded = 0;
			LoadEnglishResource();
		}
	}

	if (!eng_wordlib)
		return 0;

	prefix_len = (int)_tcslen(prefix);
	if (!prefix_len)
		return 0;

	for (i = 0; i < (int)_tcslen(prefix); i++)
	{
		if (prefix[i] == '*')
		{
			has_star = 1;
			break;
		}
	}

	Utf16ToAnsi((wchar_t*)prefix, prefix_char, sizeof(prefix_char));

	first_letter = (char)prefix[0];
	if (!isalpha(first_letter))
		return 0;

	if (isupper(first_letter))
		first_letter = first_letter - 'A' + 'a';

	m = eng_wordlib->letter_index[first_letter - 'a'];
	if ('z' == first_letter)
		n = eng_wordlib->count;
	else
		n = eng_wordlib->letter_index[first_letter - 'a' + 1];

	for (i = m, count = 0; count < array_length && i < n; i++)
	{
		english_str = eng_wordlib->data + eng_wordlib->index[i];

		is_true = 0;
		if (has_star)
			is_true = strMatch(english_str, prefix_char);
		else
			is_true = !_strnicmp(prefix_char, english_str, prefix_len);

		if (is_true)
		//if (!_strnicmp(prefix_char, english_str, prefix_len) || strMatch(english_str, prefix_char))
		{
			//将内容加入到候选数组中
			candidate_array[count].type		  = CAND_TYPE_SPW;
			candidate_array[count].spw.type	  = SPW_STIRNG_ENGLISH;
			candidate_array[count].spw.string = eng_wordlib->data + eng_wordlib->index[i];
			candidate_array[count].spw.length = (int)strlen(eng_wordlib->data + eng_wordlib->index[i]);

			count++;
		}
	}

	return count;
}

/**	加载英文翻译到内存。
 *	参数：
 *		file_name			英文翻译数据文件全路径
 *	返回：
 *		成功：1
 *		失败：0
 */
int LoadEnglishTransData(const TCHAR *file_name)
{
	int file_length;

	assert(file_name);

	if (share_segment->engtrans_loaded)
		return 1;

	file_length = GetFileLength(file_name);
	if (file_length <= 0)
		return 0;

	eng_translib = AllocateSharedMemory(engtrans_share_name, file_length);
	if (!eng_translib)
		return 0;

	if ((file_length = LoadFromFile(file_name, eng_translib, file_length)) == -1)
	{
		FreeSharedMemory(engtrans_share_name, eng_translib);
		Log(LOG_ID, L"英文翻译数据文件打开失败。name=%s", file_name);
		return 0;
	}

	if (!file_length)
		return 0;

	share_segment->engtrans_loaded = 1;

	return 1;
}

/**	释放英文翻译数据文件
 */
int FreeEnglishTransData()
{
	share_segment->engtrans_loaded = 0;

	if (eng_translib)
	{
		FreeSharedMemory(engtrans_share_name, eng_translib);
		eng_translib = 0;
	}

	return 1;
}

/**	检索英文词典，获得候选。
 *	参数：
 *		english_word		英文单词
 *	返回：
 *		检索到的英文翻译
 */
TCHAR* GetEnglishTranslation(const TCHAR *english_word)
{
	int i, m, n, length;
	TCHAR letter1, letter2;
	extern int LoadEnglishTransResource();

	if (!pim_config->use_english_input/* || !pim_config->english_candidate_vertical || !pim_config->use_english_translate*/)
		return 0;

	if(!share_segment->engtrans_loaded)
		LoadEnglishTransResource();

	if (!eng_translib)
	{
		eng_translib = GetReadOnlySharedMemory(engtrans_share_name);

		//可能存在其他进程已经装载了，但是退出后共享内存被释放的问题
		if (!eng_translib && share_segment->engtrans_loaded)
		{
			share_segment->engtrans_loaded = 0;
			LoadEnglishTransResource();
		}
	}

	if (!eng_translib)
		return 0;

	length = (int)_tcslen(english_word);
	if (length < 2)
		return 0;

	letter1 = english_word[0];
	letter2 = english_word[1];

	if (!isalpha(letter1) || !isalpha(letter2))
		return 0;

	if (isupper(letter1))
		letter1 = letter1 - 'A' + 'a';

	if (isupper(letter2))
		letter2 = letter2 - 'A' + 'a';

	m = eng_translib->letter_index[letter1 - 'a'][letter2 - 'a'];
	if ('z' == letter1 && 'z' == letter2)
		n = eng_translib->count;
	else if ('z' == letter2)
		n = eng_translib->letter_index[letter1 - 'a' + 1][0];
	else
		n = eng_translib->letter_index[letter1 - 'a'][letter2 - 'a' + 1];

	for (i = m; i < n; i++)
	{
		if (length != _tcslen(eng_translib->data + eng_translib->EngIndex[i]))
			continue;

		if (!_tcsicmp(english_word, eng_translib->data + eng_translib->EngIndex[i]))
			return eng_translib->data + eng_translib->TransIndex[i];
	}

	return 0;
}