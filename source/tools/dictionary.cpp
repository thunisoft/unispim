#include <iostream>
#include <fstream>
#include <dictionary.h>

using namespace std;

dictionary::~dictionary()
{
	delete table;
}

dictionary::dictionary(const char *dict_name)
{
	ci_count = 1024;
	table = new hash_table();
	memset(hz_freq, 0, sizeof(hz_freq));
	make_item(dict_name);
}

void dictionary::make_item(const char *dict_name)
{
	ifstream in_file;
	char ci_item[0x100];
	int  freq;

	in_file.open(dict_name);

	do
	{
		in_file >> ci_item;
		in_file >> freq;

		if (ci_item[0] == '#')			//注释行
			continue;

		if (!in_file.good())
			break;
		//检查是否为单个汉字，如果是，则对汉字的频率进行赋值
		int ci_len = (int)strlen(ci_item);

		if (ci_len == 2)		//单个汉字
			hz_freq[*(unsigned short*)ci_item] = freq;
		else if (ci_len > 16)
			continue;
		table->add(ci_item, ci_count++);
	}while(1);

	in_file.close();
}

/**	字符串是否为词汇
 */
int dictionary::is_word(const char *str, int length)
{
	if (length == 2)
	{
		if (str[0] & 0x80)
			return 1;
		return 0;
	}

	if (table->get_value(str, length))
		return 1;
	return 0;
}

//进行最大的匹配
int dictionary::max_match_length(const char *str, int length)
{
	//使用偶数长度进行查找即为汉字模式
	int len, last_len;

	last_len = 0;
	for (len = 2; len <= length && len < MAX_WORD_LENGTH; len += 2)
	{
		if (table->get_value(str, len))
			last_len = len;
	}

	return last_len;
}

int dictionary::max_match_length(const char *str)
{
	//使用偶数长度进行查找即为汉字模式
	int len, last_len;

	if (!str[0])
		return 0;

	last_len = 0;
	for (len = 2; str[len] && len < MAX_WORD_LENGTH; len += 2)
	{
		if (table->get_value(str))
			last_len = len;
	}

	return last_len;
}

/**	获得汉字的自由语素度
 */
int dictionary::get_free_morphemic(const char *str)
{
	return hz_freq[*(unsigned short*)str];
}

/**	获得从当前开始的词汇长度，最多可能8个结果（都是词汇）
 */
int dictionary::get_words_length(const char *str, int length, int *word_length)
{
	//此处要求词首的汉字必须在词表中，否则就不会进入模型
	//造成类似：汹涌澎湃不能分解正确的错误。
	if (!hz_freq[*(unsigned short*)str])
		return 0;

	length &= 0xfffffffe;
	length = MAX_WORD_LENGTH < length ? MAX_WORD_LENGTH : length;

	word_length[0] = 2;					//第一个预先设置
	int count = 1;
	hash_node *node;

	for (int i = 4; i <= length; i += 2)
	{
		node = table->get_node(str, i);
		if (node)
		{
			word_length[count++] = i;
			if (!node->close_word_length)
				break;
			i = node->close_word_length - 2;
		}
	}

//	if (count == 1 && !hz_freq[*(unsigned short*)str])
//		return 0;

	return count;
}

