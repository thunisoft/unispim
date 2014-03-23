/*	外部归并文本文件排序
 *
 *	对大型文件进行排序。
 *	
 *	1. 分组要求：到达物理内存的80%或者400万行文本
 *	2. 文件分组最大1024组，最大文件：1024*400万=40亿行文本
 *		内存1G，则为1024*1G*80%= 800G大小
 *	3. 合并：采用堆排序进行
 *	4. 以当前目录为临时文件存放目录
 *	5. 从标准输入读取数据，向标准输出写出数据
 */

/*	usage:
 *	sort -v +nn !nn
 *	
 *	-n		以数字方式进行排序
 *	-v		降序
 *	+nn		从第n字节开始比较
 *	最大行长为2048字节, 只用512M内存进行处理，如果不能满足则使用
 *	物理内存的80%进行处理。
 */
#define			_CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <windows.h>

#define	MAX_GROUPS			1024				//最大组数目
#define	MAX_LINE_LENGTH		2048				//行最大长度
#define	MAX_MEMORY_SIZE		(512*1024*1024)		//最大内存使用量
#define	MAX_LINE_PER_GROUP	(1 << 23)			//一次处理最大行数

int  compare_number;							//比较数字
int  ascending;									//排序方向
int  offset;									//比较的偏移字节数

int  memory_size;								//一次处理的内存使用量
char *buffer;									//文本数据
int	 buffer_length;								//缓冲区数据长度

int  line_index[MAX_LINE_PER_GROUP];			//行指针
int  line_count;								//行计数

int  group_count;								//组计数
int  group_order[MAX_GROUPS];					//组排序数序（归并）
char group_line[MAX_GROUPS][MAX_LINE_LENGTH];	//归并时，当前各组的行
int  group_end[MAX_GROUPS];						//组是否结束
int  cur_group_count;							//当前组计数（排序时动态的组数目）
FILE *group_files[MAX_GROUPS];					//组文件句柄

/*	获得组的临时文件名字
 */
char *get_group_files_name(int group_no)
{
	static char group_files_name[0x100];

	sprintf(group_files_name, "%04d-tmp.txt", group_no);
	return group_files_name;
}

/*	程序结束的清除工作
 */
void end()
{
	int i;

	free(buffer);
	for (i = 0; i < group_count; i++)
	{
		if (group_files[i])
			fclose(group_files[i]);
		_unlink(get_group_files_name(i));
	}
}

/*	分配内存初始化。
 */
void memory_init()
{
	MEMORYSTATUS memory_status;

	GlobalMemoryStatus(&memory_status);
	//为避免整数越界，需要先作除法后做乘法
	if (memory_status.dwTotalPhys / 100 * 80 > MAX_MEMORY_SIZE)
		memory_size = MAX_MEMORY_SIZE;
	else
		memory_size = (int) (memory_status.dwTotalPhys / 100 * 80);

	buffer = (char*)malloc(memory_size);
	if (!buffer)
	{
		fprintf(stderr, "内存不足分配");
		end();
		exit(-1);
	}

	memset(group_files, 0, sizeof(group_files));
}

/*	从文件中读取一行数据。
 *	如果没有数据，返回0，有则返回1
 */
int read_line(char *line, int length, FILE *fr)
{
	int str_len;

	fgets(line, length, fr);
	str_len = (int)strlen(line);
	if (line[str_len - 1] == '\n')
	{
		line[str_len - 1] = 0;
		str_len--;
	}

	if (feof(fr))
		return 0;

	return str_len;
}

/*	向buffer中读取数据，直到buffer满或者行数到达最大值。
 */
void read_lines()
{
	int len;

	buffer_length = 0;
	line_count = 0;

	do
	{
		len = read_line(buffer + buffer_length, MAX_LINE_LENGTH, stdin);
		if (feof(stdin))
			return; 				//文件全部读取完毕

		line_index[line_count++] = buffer_length;
		buffer_length += len + 1;
		if (memory_size - buffer_length < MAX_LINE_LENGTH)
			break;				//不足一行数据
		if (line_count >= MAX_LINE_PER_GROUP)
			break;
	}while(1);
}

int compare_line(const int *index1, const int *index2)
{
	int i;
	char *p1, *p2;

	p1 = &buffer[*index1];
	p2 = &buffer[*index2];

	//跳过offset
	for (i = 0; i < offset; i++, p1++, p2++)
	{
		if (!*p1)
		{
			if (!*p2)				//两个长度都不足
				return 0;
			return -1;
		}

		if (!*p2)
			return 1;
	}

	for (; *p1 && *p2; p1++, p2++)
	{
		if (*p1 != *p2)
			return *p1 - *p2;
	}

	if (!*p1)
	{
		if (!*p2)
			return 0;
		return -1;
	}

	return 1;
}

/* 对文本进行排序
 */
void sort_buffer()
{
	if (line_count < 2)
		return;

	qsort(line_index, line_count, sizeof(line_index[0]), compare_line);
}

void output_group(int group_no)
{
	FILE *fw;
	int  i;

	fw = fopen(get_group_files_name(group_no), "wt");
	if (!fw)
	{
		fprintf(stderr, "临时文件无法创建\n");
		end();
		exit(-1);
	}

	for (i = 0; i < line_count; i++)
	{
		fputs(buffer + line_index[ascending ? i : line_count - 1 - i], fw);
		fputs("\n", fw);
	}

	fclose(fw);
}

/*	以组的方式进行处理
 */
void process_group()
{
	group_count = 0;

	while(!feof(stdin) && group_count < MAX_GROUPS)
	{
		read_lines();

		sort_buffer();			//排序
		output_group(group_count);
		group_count++;
	};
}

/*	处理归并排序
 */
void process_combine()
{
	int i, j;
	int min_idx;
	int out_group;

	for (i = 0; i < group_count; i++)
	{
		group_files[i] = fopen(get_group_files_name(i), "rt");
		if (!group_files[i])
		{
			fprintf(stderr, "临时文件打开失败。%s\n", get_group_files_name(i));
			exit(-1);
		}
		group_end[i] = 0;
	}

	cur_group_count = 0;
	for (i = 0; i < group_count; i++)
	{
		if (group_end[i])
			continue;
		read_line(group_line[i], MAX_LINE_LENGTH, group_files[i]);
		group_order[cur_group_count++] = i;
	}
	
	//初次排序
	for (i = 0; i < cur_group_count; i++)
	{
		min_idx = i;
		for (j = i + 1; j < cur_group_count; j++)
			if (strcmp(group_line[group_order[min_idx]], group_line[group_order[j]]) > 0)
				min_idx = j;
		group_order[min_idx] = i;
		group_order[i] = min_idx;
	}

	do
	{
		printf("%s\n", group_line[group_order[0]]);
		out_group = group_order[0];
		read_line(group_line[out_group], MAX_LINE_LENGTH, group_files[out_group]);
		if (feof(group_files[out_group]))
		{
			group_end[out_group] = 1;
			cur_group_count--;
			for (i = 0; i < cur_group_count; i++)				//排序组向前移动
				group_order[i] = group_order[i + 1];
			continue;
		}

		for (i = 1; i < cur_group_count; i++)
		{
			if (strcmp(group_line[out_group], group_line[group_order[i]]) <= 0)		//新的小
				break;
			group_order[i - 1] = group_order[i];
		}

		group_order[i - 1] = out_group;
	}while(cur_group_count > 0);

	for (i = 0; i < group_count; i++)
		if (group_files[i])
			fclose(group_files[i]);
}

void usage()
{
	static char *usage_msg =
		"usage:\n"
		"sort -n -v +nn\n"
		"\n"
		"-n		以数字方式进行排序\n"
		"-v		降序\n"
		"+nn		从第n字节开始比较\n"
		"最大行长为2048字节, 只用512M内存进行处理，如果不能满足则使用\n"
		"物理内存的80%进行处理。\n";

	fprintf(stderr, "%s", usage_msg);
}


int main(int argc, char **argv)
{
	int i;
	char *str;

	compare_number = 0;
	ascending = 1;
	offset = 0;

	for (i = 1; i < argc; i++)
	{
		str = argv[i];
		if (!strcmp(str, "-n"))
		{
			compare_number = 1;
			continue;
		}

		if (!strcmp(str, "-v"))
		{
			ascending = 0;
			continue;
		}

		if (str[0] == '+')		//offset
			if (1 == sscanf(str, "%d", &offset))
				continue;

		usage();
		exit(-1);
	}

	memory_init();
	process_group();
	process_combine();
	end();

	return 0;
}

