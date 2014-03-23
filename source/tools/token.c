/*	将语料拆分成为二字/三字/四字.../N字的Token，用于进行
 *	二元Token出现次数的统计。
 *	
 *	程序从标准输入中读取数据，向标准输出输出数据
 *
 *	Usage:
 *	token [max] | [min max]
 *	min		最小token长度，默认为2，最小值为2
 *	max		最大token长度，默认为8，最大值为8
 *	
 */	
#include <stdio.h>
#include <stdlib.h>

#define		MIN_LENGTH		2
#define		MAX_LENGTH		8

#define IsGB(hz) 		(((hz & 0xff) >= 0xb0) && ((hz & 0xff) <= 0xf7) && ((hz >> 8) >= 0xa1) && ((hz >> 8) <= 0xfe))
#define	TERMINAL		(*(HZ*)"△")
#define	END				0

typedef unsigned short	HZ;

int min_length, max_length;

HZ get_hz()
{
	unsigned char char1, char2;
	HZ hz;

	if (1 != fscanf_s(stdin, "%c", &char1, sizeof(char)))
		return END;

	if (char1 < 0xa0)
		return TERMINAL;

	if (1 != fscanf_s(stdin, "%c", &char2, sizeof(char)))
		return END;

	hz = char2 * 256 + char1;

	if (IsGB(hz))
		return hz;

	return TERMINAL;
}

int is_correct_token(HZ *hz, int count)
{
	int i;

	for (i = 0; i < count; i++)
		if (hz[i] == TERMINAL || hz[i] == END)
			return 0;
	return 1;
}

void output_token(HZ *hz, int count)
{
	int i;

	for (i = 0; i < count; i++)
		printf("%c%c", hz[i] % 0x100, hz[i] / 0x100);

	printf("\n");
}

void output_correct_token(HZ *hz, int count)
{
	if (is_correct_token(hz, count))
		output_token(hz, count);
}

int process_token()
{
	HZ hz_set[MAX_LENGTH];
	int i;

	for (i = 0; i < max_length; i++)
		hz_set[i] = TERMINAL;

	do
	{
		for (i = min_length; i <= max_length; i++)
			output_correct_token(hz_set, i);

		for (i = 0; i < max_length - 1; i++)
			hz_set[i] = hz_set[i + 1];

		hz_set[i] = get_hz();
	}while(hz_set[0] != END);

	return 1;
}

void usage()
{
	static const char *usage_string = 
		"Usage:\n"
		"token [max] | [min max]\n"
		"min		最小token长度，默认为2，最小值为2\n"
		"max		最大token长度，默认为8，最大值为8\n"
		"例：\n"
		"token 5\n"
		"      最大token长度为5\n"
		"token 3 7\n"
		"      最小token长度为3，最大长度为7\n";

	printf("%s\n", usage_string);
}

int main(int argc, char **argv)
{
	min_length = MIN_LENGTH;				//默认的token长度
	max_length = MAX_LENGTH;

	if (argc == 2)
		max_length = atoi(argv[1]);
	else if (argc == 3)
	{
		min_length = atoi(argv[1]);
		max_length = atoi(argv[2]);
	}

	if (min_length < MIN_LENGTH || max_length > MAX_LENGTH || min_length > max_length)
	{
		usage();
		exit(-1);
	}

	process_token();

	return 0;
}
