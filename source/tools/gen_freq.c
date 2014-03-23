/**	在语料中计算特定长度的词汇的词频。
 *	语料庞大，内存过小，必须采用比较谨慎的开发方法
 *
 *	本算法采用Hash表的方式进行，
 *	考虑Hash散列的冲突可能性，每一项中包含
 *	相同Key的下一项数据。
 *
 *	命令行参数：
 *		输入文件名字可以为带有通配符的文件名称
 */
#define		_CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <windows.h>

#define	MIN_ITEM_LENGTH			2						//最小词长度
#define	MAX_ITEM_LENGTH			16						//最大词长度

#define	MAX_FREQ				0x7fffffff				//最大词频数目
#define	LOW_FREQ				2						//低频词的最大词频
#define	MIN_SWAP_DIFF			0						//决定交换的词频差异

#define	DEFAULT_FILE_NAME		"freq.txt"				//输出文件默认名称
#define	DEFAULT_HASH_SIZE		(32 * 0x100000)			//默认Hash表项数目
#define	DEFAULT_ITEM_SIZE		(640 * 0x100000)		//默认词条数目

#define	END						0
#define	TERMINAL				(*(HZ*)"△")

#define	ERR_NO_MEMORY			-1
#define	ERR_ITEM_FULL			-2
#define	ERR_FILE_OPEN			-3

typedef	unsigned short HZ;

typedef struct tagITEM
{
	struct tagITEM	*next;						//下一项冲突内容
	unsigned int	count;						//词频
	HZ				ci[2];						//词汇
}ITEM;

int		verbose = 1;							//是否显示提示信息

int		need_terminator = 1;					//是否需要分隔符“△”
char	*output_file = DEFAULT_FILE_NAME;		//输出文件名称

unsigned int keys[0x100][0x10];					//以ASCII编码为单位的散列key表（出现在不同位置的key不相同）
int		max_item_length = MAX_ITEM_LENGTH;		//最大项长度
int		min_item_length = MIN_ITEM_LENGTH;		//最小项长度

char	*item_table;							//Item缓冲区
int		item_size = DEFAULT_ITEM_SIZE;			//Item的数目
char*	item_pointer;							//当前item的指针

ITEM 	**hash_table;							//散列用的hash表，每一项指到Item缓冲区
const int hash_size = DEFAULT_HASH_SIZE;		//散列表项的数目

int		file_size;								//处理中文件的长度（用于计算处理百分比）
int		last_err;								//错误号
int		percent = 0;							//处理进度百分比
long long char_processed = 0;					//处理过的字符数目
long long total_chars = 0;						//全部字符数目

/**	获得词项的长度
 */
__inline int get_item_size()
{
	return sizeof(int) + sizeof(int) + sizeof(HZ) * max_item_length;
}

/**	判断是否词项表已满
 */
__inline int is_item_table_full()
{
	return item_pointer + get_item_size() >= item_table + item_size;
}

/**	初始化程序
 */
int init()
{
	int i, j;

	if (verbose)
		printf("系统初始化...\n");
	//初始化hash键值表
	srand(12345);
	for (i = 0; i < 0x10; i++)
		for (j = 0; j < 0x100; j++)
			keys[j][i] = (unsigned int)
				((rand() & 0xff) | ((rand() & 0xff) << 8) | ((rand() & 0xff) << 16) | ((rand() & 0xff) << 24));

	//初始化hash表
	hash_table = (ITEM**) malloc(hash_size * sizeof(ITEM*));
	if (!hash_table)
	{
		last_err = ERR_NO_MEMORY;
		return 0;
	}

	//初始化词项表
	item_table = (char*) malloc(item_size);
	item_pointer = item_table;
	if (!item_table)
	{
		last_err = ERR_NO_MEMORY;
		return 0;
	}

	memset(item_table, 0, item_size);
	memset(hash_table, 0, hash_size * sizeof(ITEM*));
	return 1;
}

/**	释放内存
 */
void free_all()
{
	free(hash_table);
	free(item_table);
}

/**	输出词项
 *	为加快输出速度，不能够使用printf函数（一般慢十倍）。
 */
void output_item(FILE *f, ITEM *item)
{
	int i;
	char *ci = (char*)item->ci;
	char buffer[0x100];
	char *p = buffer;

	if (!ci[0])
		return;

	for (i = 0; ci[i] && i < (int)(max_item_length * sizeof(HZ)); i++)
		*p++ = ci[i];

	*p++ = 9;	//TAB

	_ltoa(item->count, p, 10);

	p = buffer;
	while(*p)
		_fputc_nolock(*p++, f);
	_fputc_nolock(0xa, f);
}

/**	将内存中的数据输出
 */
int flush_memory()
{
	FILE *fw;
	ITEM *item;
	int  item_size;
	static int flush_count = -1;
	char file_name[0x400];

	flush_count++;

	if (verbose)
		printf("输出词频结果...\n");

	sprintf(file_name, "%d-%s", flush_count, output_file);
	fw = fopen(file_name, "w");
	if (!fw)
	{
		last_err = ERR_FILE_OPEN;
		return 0;
	}

	item_size = get_item_size();
	for (item = (ITEM*)item_table; (char*)item < item_pointer; item = (ITEM*)((char*)item + item_size))
		output_item(fw, item);

	fclose(fw);
	return 1;
}

/**	获得词条的hash键
 */
__inline unsigned int get_hash_key(const HZ *ci, int length)
{
	unsigned int key, x;
	int i;

	key = 0;
	for (i = 0; i < length; i++)
	{
		x = ci[i];
		key ^= keys[x & 0xff][i * 2];
		key ^= keys[x >> 8][i * 2 + 1];
	}
	return key;
}

/**	获得词条的散列指针
 */
ITEM **get_hash_item(const HZ *ci, int length)
{
	unsigned int key;

	key = get_hash_key(ci, length);
	key &= (hash_size - 1);

	return &hash_table[key];
}

/**	Item中是否包含本词
 */
int ci_in_item(ITEM *item, const HZ *ci, int length)
{
	int i;

	//前面是否完全相同
	for (i = 0; i < length; i++)
		if (item->ci[i] != ci[i])
			return 0;

	//如果不是最大长度，并且item中还有其他汉字，则不相同
	if (length != max_item_length)
		if (item->ci[i])
			return 0;

	//完全相同
	return 1;
}

/**	向词项表中增加一项
 */
ITEM *insert_item(const HZ *ci, int length)
{
	ITEM *item;
	int  i;

	if (is_item_table_full())
	{	//词项表已满
		last_err = ERR_ITEM_FULL;
		return 0;
	}

	//添加新词
	item = (ITEM*)item_pointer;
	item->count = 1;
	item->next = 0;
	for (i = 0; i < length; i++)
		item->ci[i] = ci[i];
	for (; i < max_item_length; i++)
		item->ci[i] = 0;
	item_pointer += get_item_size();
	return item;
}

/**	交换词项
 */
void swap_item(ITEM *item1, ITEM *item2)
{
	int i, count;
	HZ hz;

	count = item1->count;
	item1->count = item2->count;
	item2->count = count;

	for (i = 0; i < max_item_length; i++)
	{
		hz = item1->ci[i];
		item1->ci[i] = item2->ci[i];
		item2->ci[i] = hz;
	}
}

/**	处理一项
 */
int process_item(const HZ *ci, int length)
{
	ITEM **hash_item;
	ITEM *item, *last_item;
	int  i;

	if (length == 1)
	{
		if (ci[0] == TERMINAL)
			return 1;
	}
	else if (length == 2)
	{
		if (*(int*)ci == *(int*)"△△")
			return 1;
	}
	else if (length == 3)
	{
		if (ci[1] == TERMINAL)
			return 1;
		if (ci[0] == TERMINAL && ci[2] == TERMINAL)
			return 1;
	}
	else if (length == 4)
	{
		if (ci[0] == TERMINAL || ci[1] == TERMINAL ||
			ci[2] == TERMINAL || ci[3] == TERMINAL)
			return 1;
	}
	else
	{
		for (i = 0; i < length; i++)
			if (ci[i] == TERMINAL)
				return 1;
	}

	hash_item = get_hash_item(ci, length);

	if (!*hash_item)					//没有存储词项
	{
		//该项尚未加入，加入到Item数组中
		item = insert_item(ci, length);
		if (!item)
			return 0;					//失败

		*hash_item = item;
		return 1;
	}


	//在同一个Slot中寻找相同的项
	item = *hash_item;
	last_item = 0;
	while(item)
	{
		if (ci_in_item(item, ci, length))
			break;
		last_item = item;
		item = item->next;
	}

	if (item)					//找到
	{
		if (item->count < MAX_FREQ)
			item->count++;
		//如果上一个比这一个小一些，则进行交换
		//小一些的原因是为了避免震荡
		if (last_item)
			if (last_item->count + MIN_SWAP_DIFF < item->count)		//交换两项
				swap_item(last_item, item);
		return 1;
	}
	//没有找到，需要在后面追加

	item = insert_item(ci, length);
	if (!item)					//增加项失败
		return 0;

	item->next = *hash_item;	//插入到冲突链表的前部
	*hash_item = item;			//将hash表中的指针指向最后插入的项
	return 1;
}

/**	判断是否为GB2312字（包含比较常用的GBK字）
 */
__inline int is_gb(HZ zi)
{
	int  i;
	char special_zi[] = "碁傢俬焗嚇瞭槃啫镕";

	if ((zi & 0xff) >= 0xb0 && ((zi & 0xff) <= 0xf7) && (zi >> 8) >= 0xa1 && (zi >> 8) <= 0xfe)
		return 1;		//GB2312汉字

	for (i = 0; i < sizeof(special_zi); i += 2)
		if (zi == *(HZ*)(special_zi + i))
			return 1;

	return 0;
}

/**	从文件中读入汉字
 */
__inline HZ get_hz(FILE *f)
{
	unsigned char char1, char2;
	int ch;
	HZ hz;

	ch = _fgetc_nolock(f);
	if (ch == EOF)
		return END;

	char1 = (unsigned char)ch;
	char_processed++;

	if (char1 < 0x80)
		return TERMINAL;

	ch = _fgetc_nolock(f);
	if (ch == EOF)
		return END;

	char2 = (unsigned char)ch;
	char_processed++;

	hz = char2 * 256 + char1;

	if (!(char_processed & 0xff) && (char_processed * 100 / file_size) > percent)
	{
		percent = (int)(char_processed * 100 / file_size);
		printf("%%%d\r", percent);
	}

	if (is_gb(hz))
		return hz;

	return TERMINAL;
}

/**	比较词项
 */
int compare_item(const ITEM *item1, const ITEM *item2)
{
	int i;

	//在进行BCOC、TCOC或者词频设置时，不能依据词频进行排序
	if (0 && item1->count != item2->count)
		return item2->count - item1->count;
	for (i = 0; i < max_item_length; i++)
		if (item1->ci[i] != item2->ci[i])
			return item1->ci[i] - item2->ci[i];
	return 0;
//	return memcmp(item1->ci, item2->ci, max_item_length * sizeof(HZ));
}

/**	词项排序
 */
void sort_items()
{
	if (verbose)
		printf("对词项进行排序...\n");
	qsort(item_table, (item_pointer - item_table) / get_item_size(), get_item_size(), compare_item);
	return;
}

/**	获得下一个词条
 */
__inline ITEM *get_next_item(ITEM *item)
{
	return (ITEM*)((char*)item + get_item_size());
}

/**	删除词频较低的词条。词频为1、2的为词频较低。
 *	采用方式：
 *		1. 检索全部词条，将1、2词频的词条删除，并且维护原有的链接关系
 *		2. 计算出全部空洞的数目
 *		3. 再次检索hash表，当hash表项在空洞数目之外时，进行移动
 *		4. 全部完成后，可以确定能够进行分配的空间。
 *	返回：
 *		如果清理不成功，返回0，成功：1.
 */
int reduce_item()
{
	int i;
	int hole_count;					//空洞计数
	int moved_count;				//移动过的词项数目
	ITEM *item;						//词项
	ITEM *hole_item;				//空洞项
	ITEM *last_item;				//合并空洞后，最后一个词项地址
	ITEM **last_ptr;				//上一级
	int  st, et;

	st = clock();
	//1. 检索词条是否为低频词条，如果是，则删除该条
	for (i = 0, hole_count = 0; i < hash_size; i++)
	{
		item = hash_table[i];
		last_ptr = &hash_table[i];
		while(item)						//遍历词项链表
		{
			if (item->count <= LOW_FREQ)
			{
				item->ci[0] = 0;
				*last_ptr = item->next;
				item = item->next;
				hole_count++;
				continue;
			}

			last_ptr = &item->next;
			item = item->next;
		}
	}

	//如果清理的词条空间低于128M，则返回不能清理
	//不然清扫垃圾的频度过大
	if (hole_count * get_item_size() < 0x8000000)
		return 0;

	//2. 再次进行检查，发现处于后部的词条，向前移动
	last_item = (ITEM*)(item_pointer - hole_count * get_item_size());
	for (i = moved_count = 0, hole_item = (ITEM*)item_table; 
			i < hash_size && moved_count < hole_count; 
			i++)
	{
		item = hash_table[i];
		last_ptr = &hash_table[i];
		while(item)						//遍历词项链表
		{
			if (item >= last_item)		//在后部
			{
				while(hole_item->count > LOW_FREQ)
					hole_item = get_next_item(hole_item);
				//复制词项
				memcpy(hole_item, item, get_item_size());
				*last_ptr = hole_item;
				item = hole_item;
				moved_count++;
			}

			last_ptr = &item->next;
			item = item->next;
		}
	}

	//3. 清理完毕，将item_pointer设置到正确的位置
	item_pointer = (char*)last_item;

	et = clock();

	if (verbose)
		printf("回收空间:%dM，用时%d\n", hole_count * get_item_size() / 0x100000, et - st);

	return 1;
}

/**	处理标准输入形成项
 */
int process_all(FILE *f)
{
	HZ  hz_tokens[MAX_ITEM_LENGTH];
	int i;
	int ret;

	for (i = 0; i < max_item_length; i++)
		hz_tokens[i] = TERMINAL;

	do
	{
		//依次向左移动一个汉字
		for (i = 0; i < max_item_length - 1; i++)
			hz_tokens[i] = hz_tokens[i + 1];

		hz_tokens[i] = get_hz(f);
		if (hz_tokens[i] == END)
			break;

		for (i = min_item_length; i <= max_item_length; i++)
		{
			ret = process_item(hz_tokens, i);
			if (!ret)		//出错
			{
				if (last_err != ERR_ITEM_FULL)
					return 0;
				last_err = 0;
				//词条满
				if (!reduce_item())
				{
					sort_items();
					flush_memory();
					free_all();
					if (!init())
						return 0;
				}
				i--;
			}
		}
	}while(1);

	return 1;
}

/**	获得链表长度信息（判断是否进行排序）
 */
void check_chain_length()
{
	int i, length;
	int length_count[0x10];
	ITEM *item;

	if (!verbose)
		return;

	memset(length_count, 0, sizeof(length_count));
	for (i = 0; i < hash_size; i++)
	{
		item = hash_table[i];
		length = 0;
		while(item)
		{
			length++;
			item = item->next;
		}
		if (length >= 0x10)
			length = 0x10 - 1;
		length_count[length]++;
	}

	printf("链条长度与数目\n");
	for (i = 0; i < sizeof(length_count) / sizeof(length_count[0]); i++)
		printf("%02d:%d\n", i, length_count[i]);
}

const char *usage =
	"gen_freq input_files min_length max_length output_file [item_size]\n"
	"  input_files: 输入文件名字，可以附带通配符\n"
	"  min_length:  最小词长度（最小2）\n"
	"  max_length:  最大词长度（最大16）\n"
	"  output_file: 输出文件名称\n"
	"  item_size:   处理数据的内存数量，以M为单位\n";

static char file_buffer[0x100000];

int main(int argc, char **argv)
{
	HANDLE f_handle;
	WIN32_FIND_DATA f_data;
	char *in_file;
	FILE *fr;
	int  st0, st, et;

	if (argc < 5)
	{
		printf("%s", usage);
		return -1;
	}

	st0 = st = clock();
	min_item_length = atoi(argv[2]);
	max_item_length = atoi(argv[3]);
	output_file = argv[4];

	if (argc == 6)	//有item_size
		item_size = atoi(argv[5]) * 0x100000;
	if (item_size < 0x200000)
	{
		printf("内存太小，无法工作，至少2M\n");
		return -1;
	}

	_unlink(output_file);				//首先将原始记录删除

	//初始化
	if (!init())
		return 0;
	et = clock();
	if (verbose)
		printf("初始化用时:%d\n", et - st);

	f_handle = FindFirstFile(argv[1], &f_data);
	if (!f_handle)
	{
		printf("err:%d\n", ERR_FILE_OPEN);
		return -1;
	}

	do
	{
		int st1, et1;

		st1 = clock();
		in_file = f_data.cFileName;
		printf("处理<%s>文件\n", in_file);
		file_size = f_data.nFileSizeLow;			//不处理大于2G的文件
		char_processed = percent = 0;
		fr = fopen(in_file, "rb");
		if (!fr)
			printf("<%s>文件无法打开，跳过\n");

		//设置缓冲区大小，加快程序处理速度
		if (setvbuf(fr, file_buffer, _IOFBF, sizeof(file_buffer)))
			printf("设置缓冲区出错\n");

		if (!process_all(fr))
		{
			printf("err:%d\n", last_err);
			return -1;
		}
		fclose(fr);
		total_chars += char_processed;

		et1 = clock();
		if (verbose)
			printf("处理用时:%d, %4.2fM/s\n", 
					et1 - st1, 
					1000.0 * char_processed / 1024.0 / 1024.0 / (et1 - st1));
	}while(FindNextFile(f_handle, &f_data));

	FindClose(f_handle);
	et = clock();
	if (verbose)
		printf("处理用时:%d, %4.2fM/s\n", 
				et - st, 
				1000.0 * total_chars / 1024.0 / 1024.0 / (et - st));

	//检查最长的链表
	st = et;
	check_chain_length();
	et = clock();
	if (verbose)
		printf("处理最长链表总用时:%d\n", et - st);

	//减少词频过低词条
	reduce_item();
	st = et;
	et = clock();
	if (verbose)
		printf("垃圾收集用时:%d\n", et - st);

	//排序
	sort_items();
	st = et;
	et = clock();
	if (verbose)
		printf("排序用时:%d\n", et - st);

	//输出
	flush_memory();
	st = et;
	et = clock();
	if (verbose)
		printf("写出文件用时:%d\n", et - st);

	//释放
	free_all();

	et = clock();
	if (verbose)
		printf("处理总用时:%d\n", et - st0);

	return 0;
}
