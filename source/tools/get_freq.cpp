/**	获得熟语料的字频、词频信息
 *
 *	熟语料是指已经完成汉字分词处理的语料。
 *	文字中词与词之间用空格分割，句子用△分割的语料
 */

#include <fstream>
#include <iostream>

using namespace std;

#include <stdio.h>
#include <time.h>
#include <hash_table.h>

#define	MAX_FREQ		0x7fffffff

const char *usage =
	"get_freq in_file out_file\n"
	"\n"
	"in_file           熟语料文件名称\n"
	"out_file          频率文件名称\n"
	"\n";

char buffer[0x800000];
int  buffer_index;
int  buffer_length;
long long total_count;

FILE *in_file;

/**	向缓冲区添加数据
 */
int get_data()
{
	buffer_length = fread(buffer, 1, sizeof(buffer), in_file);
	buffer_index = 0;
	total_count += buffer_length;
	return buffer_length;
}

/**	从缓冲区中获得一个字符
 */
char get_char()
{
	if (buffer_index >= buffer_length)
	{
		if (!get_data())
			return 0;
	}

	return buffer[buffer_index++];
}

/**	获得一个词汇
 */
int get_word_item(char *word_item)
{
	char ch;
	char *word_item_save = word_item;

	//寻找非空白字符
	while((ch = get_char()) && (unsigned char)ch < 0x40)
		;

	if (!ch)			//结尾
		return 0;

	*word_item++ = ch;
	while((ch = get_char()) && (unsigned char)ch > 0x40)
		*word_item++ = ch;

	if (!ch)			//结束标志
		return 0;

	*word_item = 0;
	return word_item - word_item_save;
}

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		cout << usage;
		return -1;
	}

	ofstream out_file;

	fopen_s(&in_file, argv[1], "r");
	if (!in_file)
	{
		cout << "文件<" << argv[1] << ">无法打开!\n";
		return -1;
	}

	out_file.open(argv[2]);
	if (!out_file)
	{
		cout << "文件<" << argv[2] << ">无法创建!\n";
		return -1;
	}

	cout << "构造hash表...\n";
	hash_table table;

	cout << "处理字词频率...\n";

	long long c_count = 0;
	int  st = clock();
	int  c = 0;
	char item[0x100];
	
	while(get_word_item(item))
	{
		c++;
		if (!(c & 0x7ffff))
			cout << total_count / 0x100000 << "M, " << (clock() - st) / 1000 << "s\r";

		table.increase_count(item);
	}

	fclose(in_file);

	cout << "词频生成用时:" << (clock() - st) / 1000 << "秒\n";
	cout << "输出频率结果\n";
	table.output(out_file);
	out_file.close();

	return 0;
}