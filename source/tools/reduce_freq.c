/*	进行词频文件的减少词条操作
 *	依据当前词频是否超过最小词频，决定是否
 *	保留该词条
 *	
 *	Usgae:
 *		reduce in_file out_file min_freq 
 */
#define		_CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <string.h>

typedef	unsigned short	HZ;

#define	MAX_FREQ		0x7fffffff		//最大词频数
#define	MIN_FREQ		3				//最小词频数
#define	MAX_LENGTH		8				//最大词长度

char *file_in_name;
char *file_out_name;
int	 min_freq = 2;

void output(FILE *f, const char *string, unsigned int count)
{
	fprintf(f, "%s\t%u\n", string, count);
}

void get_line(FILE *f, char *token, int *count)
{
	static int line;

	line++;
	if (!(line & 0x3ff))
		printf("%d\r", line);
	if (2 != fscanf(f, "%s %d", token, count))
		*count = -1;
}

char file_buffer0[0x100000];
char file_buffer1[0x100000];

int process()
{
	char token[0x40];
	int  count;
	FILE *fr, *fw;

	fw = fopen(file_out_name, "w");
	if (!fw)
	{
		printf("<%s>无法创建\n", file_out_name);
		return -1;
	}

	//设置缓冲区大小，加快程序处理速度
	if (setvbuf(fw, file_buffer0, _IOFBF, sizeof(file_buffer0)))
		printf("设置缓冲区出错\n");

	fr = fopen(file_in_name, "r");
	if (!fr)
	{
		printf("<%s>无法打开\n", file_in_name);
		return -1;
	}

	//设置缓冲区大小，加快程序处理速度
	if (setvbuf(fr, file_buffer1, _IOFBF, sizeof(file_buffer1)))
		printf("设置缓冲区出错\n");

	get_line(fr, token, &count);

	while(count >= 0)
	{
		if (count >= min_freq)
			output(fw, token, count);
		get_line(fr, token, &count);
	};

	fclose(fw);
	fclose(fr);

	return 0;
}

const char *usage = 
	"reduce_freq in_file out_file min_freq\n";

int main(int argc, char **argv)
{
	if (argc != 4)
	{
		printf(usage);
		return -1;
	}

	file_in_name = argv[1];
	file_out_name = argv[2];
	min_freq = atoi(argv[3]);

	return process();
}
   
