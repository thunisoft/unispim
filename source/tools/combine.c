/*	进行两个词频文件的合并
 *	前提：
 *		两个词频文件必须都是已经排序完成的
 *	归并排序
 *		
 */
#define		_CRT_SECURE_NO_DEPRECATE
#include <time.h>
#include <stdio.h>
#include <string.h>

typedef	unsigned short	HZ;

#define	MAX_FILES		0x20			//最多文件数目
#define	MAX_FREQ		0x7fffffff		//最大词频数
#define	MIN_FREQ		3				//最小词频数
#define	MAX_LINE_LENGTH	0x200			//最大行长度

const char *usage = 
    "combine file1 file2 [file3].... out_file\n"
	"\n"
	"进行文件的合并工作，用于词频、字频、二元以及三元频率的合并\n"
	"\n"
	"参数中，最后一个是结果输出文件，其他为进行合并的文件,\n"
	"最多可以同时合并32个文件。\n"
	"文件中，只有最后一项应该为数字，其他都应该为中文或英文符号\n"
	"\n"
	"file1/file2/file....     两个准备进行合并的词频文件\n"
	"out_file	              输出文件名字\n";

char in_file_buffer[0x20][0x100000];
char out_file_buffer[0x100000];

char *out_file_name;
char *in_file_name[MAX_FILES];
int  in_file_count;
FILE *in_file[MAX_FILES];
FILE *out_file;

void output(FILE *f, const char *string, int count)
{
	fprintf(f, "%s\t%u\n", string, count);
}

/**	在文件中读取一行 
 */
int get_line(FILE *f, char *token, int *count)
{
	int index = 0;
	int break_pos;
	int no;

	*token = 0;
	*count = 0;

	if (!fgets(token, MAX_LINE_LENGTH, f))
		return 0;				//文件结尾

	index = 0;
	while(token[index] && (token[index] < '0' || token[index] > '9'))
		index++;

	break_pos = index - 1;

	no = 0;
	while(token[index] && token[index] >= '0' && token[index] <= '9')
	{
		no = no * 10 + (token[index] - '0');
		index++;
	}

	token[break_pos] = 0;
	*count = no;
	return 1;
}

/**	初始化，打开各个文件
 */
int open_all()
{
	int i, j;

	out_file = fopen(out_file_name, "w");
	if (!out_file)
	{
		printf( "文件<%s>创建失败\n", out_file_name);
		return 0;
	}
	setvbuf(out_file, out_file_buffer, _IOFBF, sizeof(out_file_buffer));

	for (i = 0; i < in_file_count; i++)
	{
		in_file[i] = fopen(in_file_name[i], "r");
		if (!in_file[i])
		{
			printf("打开文件<%s>失败!\n", in_file_name[i]);
			for (j = 0; j < i; j++)
				fclose(in_file[j]);
			fclose(out_file);
			return 0;
		}
		
		setvbuf(in_file[i], in_file_buffer[i], _IOFBF, sizeof(in_file_buffer[0]));
	}

	return 1;
}

/**	关闭打开的文件
 */
void close_all()
{
	int i;

	fclose(out_file);
	for (i =0; i < in_file_count; i++)
		fclose(in_file[i]);
}

typedef struct tagLINE
{
	char *str;
	long long count;
	FILE *from_file;
	struct tagLINE *next;
}LINE;

char line[MAX_FILES][MAX_LINE_LENGTH];

LINE line_heap[MAX_FILES];
int  line_count;

/**	插入到堆中。如果发现有重复的串，则合并计数，并返回0
 */
int insert_to_heap(char *str, int count, FILE *from_file)
{
	int i, j, ret;

	for (i = 0; i < line_count; i++)
	{
		ret = strcmp(str, line_heap[i].str);
		if (!ret)
		{
			line_heap[i].count += count;
			return 0;
		}

		if (ret > 0)
			break;
	}

	//当前位置为插入位置
	for (j = line_count; j > i; j--)
		line_heap[j] = line_heap[j - 1];

	line_heap[i].str = str;
	line_heap[i].from_file = from_file;
	line_heap[i].count = count;

	line_count++;
	return 1;
}

/**	初始化行堆
 */
void init_line_heap()
{
	int  count;
	int  i;

	line_count = 0;
	
	//确定所有的文件
	for (i = 0; i < in_file_count; i++)
	{
		while (get_line(in_file[i], line[i], &count))
		{
			if (insert_to_heap(line[i], count, in_file[i]))
				break;
		}
	}
}

/**	处理
 */
int process()
{
	int count;
	int c, st;

	c = 0;
	st = clock();

	init_line_heap();

	while(line_count)
	{
		LINE *out_line = &line_heap[line_count - 1];
		line_count--;

		c++;
		if (!(c & 0xffff))
			printf("line:%dM, time:%dS\r", c / 0x100000, (clock() - st) / 1000);
		output(out_file, out_line->str, out_line->count > MAX_FREQ ? MAX_FREQ : (int)out_line->count);

		while (get_line(out_line->from_file, out_line->str, &count))
			if (insert_to_heap(out_line->str, count, out_line->from_file))
				break;
	};

	return 1;
}

int main(int argc, char **argv)
{
	int i;

	if (argc < 4)
	{
		printf(usage);
		return -1;
	}

	in_file_count = argc - 2;
	if (in_file_count > MAX_FILES)
	{
		printf("文件太多，最多32个!\n");
		return -1;
	}

	for (i = 0; i < in_file_count; i++)
		in_file_name[i] = argv[1 + i];

	out_file_name = argv[argc - 1];

	if (!open_all())
		return -1;

	process();

	printf("\n");

	close_all();
	return 0;
}
   
