/* 进行词汇的的词频减少。
   将已经是词的字进行分解成为二、三、、、词，减去该词的词频。
   如: 
   	我们的		1234

   拆分后：
   我们		-1234
   门的		-1234

   命令行参数：
   reduce [词长度]		如果没有参数，则为将所有的拆分都输出
   如：reduce 2			将减少2字词词频
*/
#define		_CRT_SECURE_NO_DEPRECATE
#include <stdio.h>

typedef	unsigned short	HZ;

#define	MAX_COUNT		999999
#define	MAX_LENGTH		8				//最大词长度
#define	MIN_FREQ		3				//最小词频
#define	LINE_LENGTH		0x400			//最大行长度

#define	MIN_TOKEN_LENGTH	2
#define	MAX_TOKEN_LENGTH	8

int min_length;
int max_length;

void output_token(HZ *hz, int count, int freq)
{
	int i;
	char tmp[0x100];

	strncpy(tmp, (char*)hz, count * 2);
	printf("%-20s  %6d\n", tmp, -freq);
}

void process()
{
	char line[LINE_LENGTH];
	HZ token[LINE_LENGTH / 2];
	int freq;
	int i, j, len;

	do
	{
		fgets(line, LINE_LENGTH, stdin);
		if (feof(stdin))
			break;
		sscanf(line, "%s%d", (char*)token, &freq);

		len = (int)strlen((char*)token) / 2;

		for (i = min_length; i <= max_length; i++)
			for (j = 0; j <= len - i; j++)
				output_token(token + j, i, freq);

	}while(1);
}

int main(int argc, char **argv)
{
	int tmp;

	min_length = MIN_TOKEN_LENGTH;
	max_length = MAX_TOKEN_LENGTH;

	if (argc == 2)
	{
		tmp = atoi(argv[1]);
		if (tmp >= MIN_TOKEN_LENGTH && tmp <= MAX_TOKEN_LENGTH)
			min_length = max_length = tmp;
	}

	if (argc == 3)
	{
		tmp = atoi(argv[1]);
		if (tmp >= MIN_TOKEN_LENGTH && tmp <= MAX_TOKEN_LENGTH)
			min_length  = tmp;
		tmp = atoi(argv[2]);
		if (tmp >= MIN_TOKEN_LENGTH && tmp <= MAX_TOKEN_LENGTH)
			max_length  = tmp;
	}

	process();
	return 0;
}
   
