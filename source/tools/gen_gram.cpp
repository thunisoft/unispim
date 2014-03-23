/**	获得熟语料的bigram信息。用于整句输入
 *
 *	熟语料是指已经完成汉字分词处理的语料。
 *	文字中词与词之间用空格分割，句子用△分割的语料
 *
 *	该程序必须运行在至少1.5G的内存的机器上。
 */

#include <fstream>
#include <iostream>

using namespace std;

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <map_file.h>

#define	MAX_FREQ		0x7fffffff
#define	MIN_FREQ		3

const char *usage =
	"gen_gram in_file out_file [min_freq]\n"
	"\n"
	"in_file           熟语料文件名称\n"
	"out_file          gram文件名称，以0-9的数字作为换出文件的后缀\n"
	"min_freq          最小的词频，默认为3\n"
	"\n"
	"注意！本程序使用内存很大，建议在2G以上的机器上运行\n"
	"\n";

char *buffer;
int  buffer_index;
int  buffer_length;
int  buffer_size = 0x800000;
long long total_count;

char file_buffer[0x100000];				//文件输出缓冲区

FILEMAPHANDLE in_file;					//读入文件句柄

int min_freq = MIN_FREQ;

//BIGRAM链表项
struct B_ITEM
{
	int index0, index1;			//词条编号
	int count;					//计数
	struct B_ITEM *next;		//同一个index0开始的下一项位置
};

//TRIGRAM链表项
struct T_ITEM
{
	int index0, index1, index2;	//词条编号
	int count;					//计数
	struct T_ITEM *next;		//下一项位置
};

//散列表项
struct W_NODE
{
	char	str[28];			//本项的字符串
	int		is_gb_word;			//是否为GB单词
};

#define	W_HASH_SIZE			0x100000			//词条hash表大小
#define	B_HASH_SIZE			0x1000000			//B项hash表大小
#define	B_ITEM_SIZE			0x2000000			//二元条目数目
#define	T_HASH_SIZE			0x2000000			//T项hash表大小
#define	T_ITEM_SIZE			0x2000000			//三元条目数目

W_NODE		*w_hash_table;						//词hash表

B_ITEM		**b_hash_table;						//二元hash表
B_ITEM		*b_item_table;						
int			b_item_count;

T_ITEM		**t_hash_table;						//三元hash表
T_ITEM		*t_item_table;
int			t_item_count;

int	process_bigram = 1;						//是否处理bigram
int process_trigram = 0;					//是否处理trigram

unsigned int ascii_keys[0x100][0x20];		//字符散列key

__inline void swap(int *x, int *y)
{
	int t;
	t = *x;
	*x = *y;
	*y = t;
}

/**	向缓冲区添加数据
 */
int get_data()
{
	buffer_length = FileMapGetBuffer(in_file, &buffer, buffer_size);
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

/**	初始化数据
 */
int init()
{
	cout << "申请hash表内存...\n";
	w_hash_table = new W_NODE[W_HASH_SIZE];
	if (!w_hash_table)
	{
		cout << "申请失败\n";
		return 0;
	}

	memset(w_hash_table, 0, W_HASH_SIZE * sizeof(W_NODE));
	b_item_count = t_item_count = 0;

	if (process_bigram)
	{
		cout << "申请二元数据内存...\n";
		b_hash_table = new B_ITEM * [B_HASH_SIZE];
		if (!b_hash_table)
		{
			cout << "申请b_hash_table失败\n";
			return 0;
		}
		b_item_table = new B_ITEM[B_ITEM_SIZE];
		if (!b_item_table)
		{
			cout << "申请b_item_table失败\n";
			return 0;
		}

		cout << "内存清零\n";
		memset(b_hash_table, 0, B_HASH_SIZE * sizeof(B_ITEM*));
		memset(b_item_table, 0, B_ITEM_SIZE * sizeof(B_ITEM));
	}

	if (process_trigram)
	{
		cout << "申请三元数据内存...\n";
		t_hash_table = new T_ITEM*[T_HASH_SIZE];
		if (!t_hash_table)
		{
			cout << "申请t_hash_table失败\n";
			return 0;
		}
		t_item_table = new T_ITEM[T_ITEM_SIZE];
		if (!t_item_table)
		{
			cout << "申请t_item_table失败\n";
			return 0;
		}

		cout << "内存清零\n";
		memset(t_hash_table, 0, T_HASH_SIZE * sizeof(T_ITEM*));
		memset(t_item_table, 0, T_ITEM_SIZE * sizeof(T_ITEM));
	}

#define	RAND_BYTE	(((unsigned int) rand()) & 0xff)

	cout << "构造hash键表\n";
	srand(12345);
	for (int i = 0; i < sizeof(ascii_keys) / sizeof(ascii_keys[0]); i++)
		for (int j = 0; j < sizeof(ascii_keys[0]) / sizeof(ascii_keys[0][0]); j++)
			ascii_keys[i][j] = 
				(RAND_BYTE) |
				(RAND_BYTE << 8) |
				(RAND_BYTE << 16) |
				(RAND_BYTE << 24);

	return 1;
}

/**	释放全部内存
 */
void free_all()
{
	delete [] w_hash_table;
	if (process_bigram)
	{
		delete [] b_hash_table;
		delete [] b_item_table;
	}

	if (process_trigram)
	{
		delete [] t_hash_table;
		delete [] t_item_table;
	}
}

/**	获得字符串的散列键
 */
unsigned int get_string_key(const char *str)
{
	unsigned int key = 0;
	
	while(*str)
	{
		key ^= ascii_keys[(unsigned char)*str][(unsigned char)*str];
		str++;
	}

	return key;
}

/**	判断是否为GB2312字（包含比较常用的GBK字）
 */
__inline int is_gb(unsigned short zi)
{
	int  i;
	static const char special_zi[] = "△碁傢俬焗嚇瞭槃啫镕";

	if ((zi & 0xff) >= 0xb0 && ((zi & 0xff) <= 0xf7) && (zi >> 8) >= 0xa1 && (zi >> 8) <= 0xfe)
		return 1;		//GB2312汉字

	for (i = 0; i < sizeof(special_zi); i += 2)
		if (zi == *(unsigned short*)(special_zi + i))
			return 1;

	return 0;
}

/**	判断是否为gb单词
 */
int is_all_gb(const char *str)
{
	while(*str)
	{
		if (!is_gb(*(unsigned short*)str))
			return 0;
		str += 2;
	}
	return 1;
}

/**	获得词散列项，如果不存在，则创建一个
 *	并获得字符串的散列键值。
 */
int get_word_index(const char *str, unsigned int *ret_key)
{
	unsigned int key = get_string_key(str);
	int index = key % W_HASH_SIZE;

	while(*(int*)w_hash_table[index].str)
	{
		if (!strcmp(str, w_hash_table[index].str))
		{
			*ret_key = key;
			return index;
		}

		index = (index + 1) & (W_HASH_SIZE - 1);
	}

	//没有找到，增加一个
	strcpy(w_hash_table[index].str, str);
	w_hash_table[index].is_gb_word = is_all_gb(str);
	*ret_key = key;
	return index;
}

/**	增加二元数据项。
 *	返回：
 *		1：成功
 *		0：失败，应为存储区域满
 */
int inc_binary_item(int index0, unsigned int key0, int index1, unsigned int key1)
{
	unsigned int key = key0 ^ ((key1 << 5) | (key1 >> 27));
	int index = key % B_HASH_SIZE;
	B_ITEM *p, *last_p;

	p = b_hash_table[index];
	last_p = 0;
	while(p)
	{
		if (p->index0 == index0 && p->index1 == index1)			//相同的项
		{
			p->count++;
			//为了加快处理速度，对高频率的项向前移动。
			if (last_p && p->count > last_p->count)// + 0x10)
			{
				swap(&p->count, &last_p->count);
				swap(&p->index0, &last_p->index0);
				swap(&p->index1, &last_p->index1);
			}
			return 1;
		}
		last_p = p;
		p = p->next;
	}

	//没有找到，创建一个
	b_item_table[b_item_count].index0 = index0;
	b_item_table[b_item_count].index1 = index1;
	b_item_table[b_item_count].count = 1;

	//组合链表
	b_item_table[b_item_count].next = b_hash_table[index];
	b_hash_table[index] = &b_item_table[b_item_count];
	b_item_count++;
	
	//判断是否越界
	if (b_item_count >= B_ITEM_SIZE)
		return 0;
	return 1;
}

/**	增加三元数据项。
 *	返回：
 *		1：成功
 *		0：失败，应为存储区域满
 */
int inc_triple_item(int index0, unsigned int key0, int index1, unsigned int key1, int index2, unsigned int key2)
{
	unsigned int key = key0 ^ key1 ^ key2;
	int index = key % T_HASH_SIZE;
	T_ITEM *p;

	p = t_hash_table[index];
	while(p)
	{
		if (p->index0 == index0 && p->index1 == index1 && p->index2 == index2)			//相同的项
		{
			p->count++;
			return 1;
		}
		p = p->next;
	}

	//没有找到，创建一个
	t_item_table[t_item_count].index0 = index0;
	t_item_table[t_item_count].index1 = index1;
	t_item_table[t_item_count].index2 = index2;
	t_item_table[t_item_count].count = 1;

	//组合链表
	t_item_table[t_item_count].next = t_hash_table[index];
	t_hash_table[index] = &t_item_table[t_item_count];
	t_item_count++;
	
	//判断是否越界
	if (t_item_count >= T_ITEM_SIZE)
		return 0;
	return 1;
}

/**	比较Binary项的大小
 */
int compare_bnode(const void *vp1, const void *vp2)
{
	int ret;
	const B_ITEM *p1;
	const B_ITEM *p2;

	p1 = (const B_ITEM*) vp1, p2 = (const B_ITEM*) vp2;

	ret = strcmp(w_hash_table[p1->index0].str, w_hash_table[p2->index0].str);
	if (ret)
		return ret;

	return strcmp(w_hash_table[p1->index1].str, w_hash_table[p2->index1].str);
}

/**	输出结果 
 */
int output_result(const char *out_file_name)
{
	static int count;
	char name[0x200];
	int  break_pos;

	//寻找最后一个.，用于扩展名前面增加序号
	strcpy(name, out_file_name);
	int len = (int)strlen(out_file_name);
	for (int i = len - 1; i >= 0; i--)
		if (name[i] == '.')
		{
			break_pos = i;
			break;
		}

	count++;
	strncpy(name, out_file_name, break_pos);
	sprintf(name + break_pos, "-%02d", count);
	strcat(name, out_file_name + break_pos);

	cout << "\n向<" << name << ">文件输出中间数据\n";

	int st = clock();
	FILE *fw;
	fw = fopen(name, "w");
	if (!fw)
	{
		cout << "无法打开文件<" << name << ">进行写入\n";
		return 0;
	}
	setvbuf(fw, file_buffer, _IOFBF, sizeof(file_buffer));

	cout << "对item进行排序...\n";
	qsort(b_item_table, b_item_count, sizeof(B_ITEM), compare_bnode);
	cout << "排序用时:" << (clock() - st) / 1000 << "s\n";

	for (int i = 0; i < b_item_count; i++)
	{
		B_ITEM *p = &b_item_table[i];
		char *str;

		if (p->count < min_freq)
			continue;

		if (!w_hash_table[p->index0].is_gb_word ||
			!w_hash_table[p->index1].is_gb_word)
			continue;

		str = w_hash_table[p->index0].str;
		while(*str)
			_fputc_nolock(*str++, fw);
		_fputc_nolock('\t', fw);

		str = w_hash_table[p->index1].str;
		while(*str)
			_fputc_nolock(*str++, fw);
		_fputc_nolock('\t', fw);

		char no[0x20];
		_ltoa(p->count, no, 10);

		str = no;
		while(*str)
			_fputc_nolock(*str++, fw);
		_fputc_nolock('\n', fw);

//		out_file << w_hash_table[p->index0].str << "\t";
//		out_file << w_hash_table[p->index1].str << "\t";
//		out_file << p->count << endl;
	}

	//清理hash表以及item表
	memset(w_hash_table, 0, W_HASH_SIZE * sizeof(W_NODE));
	memset(b_hash_table, 0, B_HASH_SIZE * sizeof(B_ITEM*));
	b_item_count = 0;

	cout << "输出用时:" << (clock() - st) / 1000 << "s\n";

	fclose(fw);
	return 1;
}

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		cout << usage;
		return -1;
	}

	if (argc >= 4)
		min_freq = atoi(argv[3]);

	ofstream out_file;

	in_file = FileMapOpen(argv[1]);
	if (!in_file)
	{
		cout << "文件<" << argv[1] << ">无法打开!\n";
		return -1;
	}

	//删除即将生成的文件
	_unlink(argv[2]);

	cout << "初始化...\n";
	init();

	cout << "处理bigram...\n";

	long long c_count = 0;
	int  st = clock();
	int  c = 0;
	char item0[0x100], item1[0x100];
	int  index0, index1;
	unsigned int key0, key1;
	
	get_word_item(item0);
	index0 = get_word_index(item0, &key0);

	while(get_word_item(item1))
	{
		c++;
		if (!(c & 0xffff))
		{
			cout << "data:" << (total_count - buffer_length + buffer_index) / 0x100000 << "M, ";
			cout << "item:" << int((float)b_item_count / B_ITEM_SIZE * 100) << "%, ";
			cout << "time:" << (clock() - st) / 1000 << "s\r";
		}

		index1 = get_word_index(item1, &key1);

		//只计算GB的词汇
		if (w_hash_table[index0].is_gb_word && w_hash_table[index1].is_gb_word)
			if (!inc_binary_item(index0, key0, index1, key1))			//缓冲区已满，需要清理
				output_result(argv[2]);

		index0 = index1;
		key0 = key1;
	}

	FileMapClose(in_file);

	output_result(argv[2]);

	cout << "词频生成用时:" << (clock() - st) / 1000 << "秒\n";
	cout << "输出频率结果\n";

	return 0;
}