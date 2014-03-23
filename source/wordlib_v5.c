/* V5词库处理函数
 * 主要目的为从V5词库中导出用户自造词汇，然后合并到用户V6词库中

词库结构说明 

1.头部
	Magic Number:	0x19990604
	Word Index Table(WIT)
	WinTime
	Free Address:	词库的大小

2. 词索引说明（WORD index table)

	+-------------+         +-------------------------+
	|   双字词    |-------> | 以两个声母为Key的hash表 |
	+-------------+         +-------------------------+
	|   三字词    |
	+-------------+
	|   ......    |
	+-------------+
	|   ......    |
	+-------------+
	|   九字词    |
	+-------------+

2.1 hash表说明
	一个18x18的数组，表示从b，到f的声母矩阵。
	注：模糊音被认为是相同的。具体判断，利用词的发音进行。

	+------+     +------------------+
	| b, b |---->| 词列表(wordlist) |
    +------+     +------------------+
	| b, j | 
	+------+
	| .... | 
	+------+
	| .... | 
	+------+
	| f, f | 
	+------+
2.2 词列表(wordlist)说明	

	2.2.1 词项说明(worditem)
		注：每个单位都为16位数字(short)

		特征数：字个数等信息
		发音区(每个16位)：
			1st音节
			2nd音节
			.......
			9th音节
		词区：
		第一个词：
			1st字
			2nd字
			.....
			9th字
		注：字区通过MN_NUMBER进行xor加密，MN_NUMBER:0xfdef
		指针区：
			next WORD address(NWA):		下一个不同词的地址	24位
			next entry address(NEA):	下一个相同词的地址	24位
		第二个词：
			1st字
			2nd字
			.....
			9th字
		指针区：
			next entry address(NEA):	下一个相同词的地址	24位
			词特征数:	8位，最高位表示系统词汇，其他位表示词频。
		......
		......
		......
		......
		最后一个词
			1st字
			2nd字
			.....
			9th字
		指针区：
			next entry address(NEA):	下一个相同词的地址	24位，值为0.
			词特征数:	8位，最高位表示系统词汇，其他位表示词频。


	特征数说明：
		位序：
		0-3	:	词中字数
		4-6	:	reserved
		7	:	多词标志	1:y,0:n
		8-e	:	词汇频率
        f	:	系统词汇
		注：被删除的词汇的频率为0.

	2.2.2 列表说明

		+------------+
		|feature val |		词的特征信息
		+------------+
		| 1st entry  |
		+------------+
   +---<|    NWA     |		24位, next WORD address
   |	+------------+
   | +-<|    NEA     |		24位，next entry address
   | |  +------------+
   | +->| 2nd entry  | >-----+
   | 	+------------+       |		
   | +-<|    NEA     |       |-----> 第二条目的信息			
   | |	+------------+       |		
   | |	|  feature   | >-----+	
   | |	+------------+
   | +->| 3rd entry  | >-----+
   |	+------------+       |		
   |	|    NEA     |       |-----> 第三条目的信息
   |	+------------+       |		
   |	|  feature   | >-----+	
   |	+------------+
   |	|   ......   |
   |	+------------+
   |	|   ......   |
   |	+------------+
   |	| last entry | >-----+
   |	+------------+       |		
   |	|    000     |       |-----> 最后条目的信息
   |	+------------+       |		
   |	|  feature   | >-----+	
   |	+------------+
   +--->|  next WORD |
		+------------+
	
*/

#include <assert.h>
#include <utility.h>
#include <wordlib.h>
#include <kernel.h>
#include <tchar.h>
#include <share_segment.h>

//每次分配的内存数目
#define	WORDS_PER_ALLOC		0x10000
#define	SYLLABLEV5			WORD

//V5词库结构
typedef struct 
{
	WORD feature;								//词频
	WORD length;								//词长度
	int  freq_v6;								//V6词频
	SYLLABLEV5 syllable[MAX_WORD_LENGTH];		//音节数组
	SYLLABLE   syllable_v6[MAX_WORD_LENGTH];	//V6音节数组
	HZ hz[MAX_WORD_LENGTH];						//汉字数组
}ITEMV5;

ITEMV5	*user_word_array = 0;				//用户词汇数组

int max_words_now = 0;
int user_word_count = 0;	
int wl_end;										//WORD library buffer's length

//获得三个字节的值
int Get3Bytes(BYTE *p)
{
	DWORD x, y, z;
	DWORD v;

	assert(p);

	z = *p++;
	y = *p++;
	x = *p++;
	v = (x << 16) | (y << 8) | z;

	return (int)v;
}

/**	装载词库到内存
 *	注意：不使用的时候，需要释放
 *	参数：
 *		wlname		词库名称
 *	返回：
 *		成功：返回词库的指针
 *		失败：0
 */
WORDLIBV5 *LoadWordLibraryV5(const TCHAR *wl_name)
{
	char *buffer;
	int  length;

	assert(wl_name);
	length = GetFileLength(wl_name);
	if (length <= 0)
		return 0;

	buffer = (char*)malloc(length);
	if (!buffer)
		return 0;

	if (length != LoadFromFile(wl_name, buffer, length))
	{
		free(buffer);
		return 0;
	}

	return (WORDLIBV5*) buffer;
}

/**	增加一条用户词汇
 *	参数：
 *		length		词汇长度
 *		feature		词汇特征字
 *		syllable	音节数组
 *		ci			汉字数组
 */
void AddWordToArray(int length, int feature, SYLLABLEV5 *syllable, HZ *ci)
{
	ITEMV5 *new_user_words;
	static count;


	assert(syllable);
	assert(ci);

	count++;
	//if (!(count % 0x10))
	//	printf("%d\r", count);

	//判断用户词缓冲区是否越界
	if (user_word_count >= max_words_now)   //MAX_USER_WORDS)
	{
		new_user_words = (ITEMV5*) malloc((max_words_now + WORDS_PER_ALLOC) * sizeof(ITEMV5));
		if (!new_user_words)		//memory is not enough, so ignore words
			return;

		if (user_word_count)
			memcpy(new_user_words, user_word_array, sizeof(ITEMV5) * max_words_now);

		max_words_now += WORDS_PER_ALLOC;
		free(user_word_array);
		user_word_array = new_user_words;
	}

	//增加用户词条
	user_word_array[user_word_count].length = length;
	user_word_array[user_word_count].feature = (WORD)(feature & 0x7f);	

	memcpy((char *)(user_word_array[user_word_count].syllable), syllable, length * sizeof(SYLLABLE));
	memcpy((char *)(user_word_array[user_word_count].hz), ci, length * sizeof(HZ));
	user_word_array[user_word_count].syllable[length] = 0;
	user_word_array[user_word_count].hz[length] = 0;
	user_word_count++;
}

/**	在词库中寻找用户词条，插入到用户词条数组中
 *	参数：
 *		length			词长度
 *		con1, con2		生母标识
 *		wl_data			词库数据指针
 */
void ExtractUserWordFromLibrary(int length, WORD con1, WORD con2, WORDLIBV5 *wl_data)
{
	int i;
	WORD *p;
	WORD feature;
	DWORD next_word_address, next_entry_address = 0, old_word_address, old_entry_address;
	SYLLABLEV5 syllable[MAX_WORD_LENGTH];
	WORD user_word[MAX_WORD_LENGTH];

	assert(wl_data);

	next_word_address = wl_data->index_table[length - 2][con1][con2];
	if (!next_word_address)		//没有词条
		return;

	//遍历
	while(next_word_address)
	{
		p = (WORD*)((char*)wl_data + next_word_address);		//p指向本组开始的词条

		if (IsBadReadPtr(p, sizeof(WORD)))
			break;
		
		feature = *p++;									//获得第一个词条的特征字
		
		//复制音节与词条汉字
		for (i = 0; i < length; i++)
			syllable[i] = *p++;
		syllable[i] = 0;		

		for (i = 0; i < length; i++)
			user_word[i] = *p++ ^ ENCODERV5;

		user_word[i] = 0;

		//如果为用户所造的词，加入到数组
		if (!(feature & 0x8000) && (feature & 0x7f00))
			AddWordToArray(length, (feature >> 8) & 0x7f, syllable, user_word);

		//获得下一个指针
		old_word_address  = next_word_address;
		next_word_address = Get3Bytes((BYTE*)p);

		if (next_word_address && old_word_address == next_word_address)
			break;

		p = (WORD*)((BYTE*)p + 3);

		old_entry_address  = next_entry_address;
		next_entry_address = Get3Bytes((BYTE*)p);
	
		if (next_entry_address && old_entry_address == next_entry_address)
			break;

		p = (WORD*)((BYTE*)p + 3);

		//遍历
		while(next_entry_address)
		{
			p = (WORD*)((char*)wl_data + next_entry_address);	//指向词条

			if (IsBadReadPtr(p, sizeof(WORD)))
				break;

			for (i = 0; i < length; i++)
				user_word[i] = *p++ ^ ENCODERV5;
			user_word[i] = 0;

			//获得下一条的地址
			old_entry_address = next_entry_address;
			next_entry_address = Get3Bytes((BYTE*)p);
			
			if (next_entry_address && old_entry_address == next_entry_address)
				break;

			p = (WORD*)((BYTE*)p + 3);

			//获得特征字
			feature = *(BYTE*)p;
			if (feature && !(feature & 0x80))
				AddWordToArray(length, feature, syllable, user_word);
		}
	}
}

/**	解出全部用户自造词
 *	参数：
 *		wl_data			词库指针
 *	返回：
 *		
 */
void ExtractAllWordsFromLibrary(WORDLIBV5 *wl_data)
{
	WORD i, j, k;

	assert(wl_data);

	user_word_count = 0;		//设定初值

	for (i = 2; i <= 9; i++)
		for (j = 0; j < 18; j++)
			for (k = 0; k < 18; k++)
				ExtractUserWordFromLibrary(i, j, k, wl_data);
}

/**	获得用户自造词汇
 *	用户自造词，将会保存在user_word_array数组中，词条数目为user_word_count
 *	参数：
 *		wl_name			词库名称
 *	返回：
 *		成功：1
 *		失败：0
 */
int LoadWordsArrayFromLibrary(const TCHAR *wl_name)
{
	WORDLIBV5 *wl_data;

	assert(wl_name);

	wl_data = LoadWordLibraryV5(wl_name);
	if (!wl_data)
		return 0;

	ExtractAllWordsFromLibrary(wl_data);
	
	free(wl_data);
	return 1;
}

/**	将V5的音节变换为V6的音节
 *	参数：
 *		syllable_v5		V5音节数组
 *		syllable_v6		V6音节数组
 *		length			音节长度
 *	返回：
 *		成功：1
 *		失败：0
 */
int SetV6Syllable(SYLLABLEV5 *syllable_v5, SYLLABLE *syllable_v6, int length)
{
	int i;

	if (!share_segment)
		return 0;

	for (i = 0; i < length; i++)
	{
		int index = (int)syllable_v5[i];

		if (index < 0 || index >= share_segment->syllablev5_map_items)
			return 0;

		//V5词库中有没有韵母的情况，应该干掉
		if (share_segment->syllablev5_map[index].vow == VOW_NULL)
			return 0;

		syllable_v6[i].con = share_segment->syllablev5_map[index].con;
		syllable_v6[i].vow = share_segment->syllablev5_map[index].vow;
		syllable_v6[i].tone = TONE_0;
	}

	return 1;
}

/**	调整V5的词频到V6的词频。
 *	注：V5的用户词频最高120，最低100，系统词频则可以更低
 *	调整方式：
 *		v6_freq = (v5_freq - 100) * 20 + 100; (v5_freq >=100)的情况
 *		v6_freq = v5_freq; (v5_freq < 100的情况)
 */
int AdjustV5Freq(int v5_freq)
{
	if (v5_freq < 100)
		return v5_freq * 100;

	v5_freq = (v5_freq - 100) * 200000;			//这样接近50万
	if (v5_freq > WORDLIB_MAX_FREQ)
		v5_freq = WORDLIB_MAX_FREQ;
	return v5_freq;
}

/**	将V5、V3的词库中的用户词条加入到V6中的用户词库中。
 *	注意：由于词库处理方式的不同，V3以及V5中的用户词汇才会升级到新的词库程序中。
 *	参数：
 *		wl_file_name			原始词库文件名称
 *	返回：
 *		成功	1
 *		失败	0
 */
int UpgradeWordLibFromV5ToV6(const TCHAR *wl_file_name)
{
	int  i, err, wl_reloaded;
	int  uwl_id;
	TCHAR uwl_name[MAX_PATH];
	extern HINSTANCE global_instance;
	extern int resource_thread_finished;

	while (!resource_thread_finished)
		Sleep(0);

	ShowWaitingMessage(TEXT("正在导入V3/V5用户词汇......"), global_instance, 500);

	err = 1;
	do
	{
		//获得V3V5词库
		GetFileFullName(TYPE_ALLAPP, WORDLIB_V3V5_FILE_NAME, uwl_name);
		if (!FileExists(uwl_name))
		{
			if (!CreateEmptyWordLibFile(uwl_name, WORDLIB_V3V5_NAME, DEFAULT_USER_WORDLIB_AUTHOR, 1))
				return 0;
		}

		//装载到内存
		uwl_id = LoadWordLibraryWithExtraLength(uwl_name, WORDLIB_CREATE_EXTRA_LENGTH, 1);
		if (uwl_id == -1)
			break;

		//1. 寻找所有的V3词库的用户词条
		if (!LoadWordsArrayFromLibrary(wl_file_name))
			break;

		wl_reloaded = 0;
		//3.插入到词库中
		for (i = 0; i < user_word_count; i++)
		{
			HZ hz_uc[MAX_WORD_LENGTH] = {0};

			//变换音节，注意音节的错误
			if (!SetV6Syllable(user_word_array[i].syllable, user_word_array[i].syllable_v6, user_word_array[i].length))
				continue;

			user_word_array[i].freq_v6 = AdjustV5Freq(user_word_array[i].feature & 0x7f);

			AnsiToUtf16((char*)user_word_array[i].hz, hz_uc, MAX_WORD_LENGTH);

			if (AddCiToWordLibrary(
						uwl_id,
						hz_uc, 
						user_word_array[i].length, 
						user_word_array[i].syllable_v6, 
						user_word_array[i].length,
						user_word_array[i].freq_v6))
				continue;

			//失败，可能词库满
 			SaveWordLibrary(uwl_id);					//保存词库数据
			CloseWordLibrary(uwl_id);					//关闭词库
			uwl_id = LoadWordLibraryWithExtraLength(uwl_name, WORDLIB_CREATE_EXTRA_LENGTH, 0);
			if (uwl_id == -1)							//装载失败
				break;

			wl_reloaded = 1;
			share_segment->can_save_user_wordlib = 0;

			if (AddCiToWordLibrary(
						uwl_id,
						user_word_array[i].hz, 
						user_word_array[i].length, 
						user_word_array[i].syllable_v6, 
						user_word_array[i].length,
						user_word_array[i].freq_v6))
				continue;

			break;
		}
		if (i != user_word_count)
			break;

		//4. 保存词库
		SaveWordLibrary(uwl_id);

		if (wl_reloaded)
			CloseWordLibrary(uwl_id);			//删除共享

		err = 0;

	}while(0);

	ShowWaitingMessage(0, 0, 500);

	if (err)
		return 0;

	return 1;
}

#define	TMP_FILE_NAME	TEXT("upim.tmpfile")

/**	判断是否为V3/V5的词库备份文件
 *	步骤：
 *		1. 解压缩
 *		2. 装载
 *		3. 判断
 *		4. 删除
 *	返回：
 *		1：是，0：否
 */
int IsV3WordLibBK(const TCHAR* wl_name)
{
	WORDLIBHEADER header;
	TCHAR tag_name[MAX_PATH];

	if (!GetFileFullName(TYPE_TEMP, TMP_FILE_NAME, tag_name))
		return 0;

	if (!UncompressFile(wl_name, tag_name, sizeof(header)))
		return 0;

	if ((LoadFromFile(tag_name, (char*)&header, sizeof(header)) < sizeof(header)) ||
		(header.signature != HYPIM_WORDLIB_V5_SIGNATURE))
	{
		_tunlink(tag_name);
		return 0;
	}

	_tunlink(tag_name);
	return 1;
}

/**	从V5的备份文件升级到V6词库
 */
int CheckAndUpgrandWordLibFromV5BKToV6(const TCHAR *wl_name)
{
	WORDLIBHEADER header;
	int ret;
	TCHAR tag_name[MAX_PATH];

	if (!GetFileFullName(TYPE_TEMP, TMP_FILE_NAME, tag_name))
		return 0;

	if (!UncompressFile(wl_name, tag_name, -1))
		return 0;

	if ((LoadFromFile(tag_name, (char*)&header, sizeof(header)) < sizeof(header)) ||
		(header.signature != HYPIM_WORDLIB_V5_SIGNATURE))
	{
		_tunlink(tag_name);
		return 0;
	}

	ret = UpgradeWordLibFromV5ToV6(tag_name);
	_tunlink(tag_name);

	return ret;
}
