/**	通过在语料中计算出的词汇的词频，对系统词库、人名词库等
 *	进行调整，确定最为准确的词频。
 *
 *	使用本程序的要求：
 *		两个输入文件必须按照汉字（Short）的方式进行排序。
 *		(由于本人的疏忽，采用了这种方式。实际上，应该用string
 *		的排序方式)
 *
 *	命令行参数：
 *		set_freq ci_file freq_file out_file
 *
 *	ci_file格式：
 *		我们		wo'men		1
 *	
 *	可以省略词频，系统将省略的词频设置为1
 *
 *	词频转换方式：
 *		对于词，采用19位词频；字，采用23位字频。
 *	词频首先进行右移两位的操作，如果词频数目大于500000的话，将
 *	高于500000的部分映射到2000以内。
 *	字频首先右移一位，如果大于8000000则，将高出部分映射到300000
 *	以内。
 *		
 */
#define		_CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <string.h>

typedef	unsigned short	HZ;

//词
#define	FREQ_SHIFT		2
#define	HIGHER_DIVIDER	2000
#define	MAX_FREQ		((1 << 19) - 1)
#define	MAX_BASE_FREQ	500000

//字
//#define	FREQ_SHIFT		1
//#define	HIGHER_DIVIDER	2000
//#define	MAX_FREQ		((1 << 23) - 1)
//#define	MAX_BASE_FREQ	8000000

char *ci_file_name;
char *freq_file_name;
char *out_file_name;

/**	获得入库的词频
 */
int get_adjusted_freq(unsigned int freq)
{
	int new_freq;

	new_freq = (int)(freq >> 2);
	if (new_freq > MAX_BASE_FREQ)
		new_freq = MAX_BASE_FREQ + new_freq / HIGHER_DIVIDER;

	if (new_freq > MAX_FREQ)
		new_freq = MAX_FREQ;

	return new_freq;
}

/**	输出词条
 */
void output(FILE *f, const char *hz, const char *py, unsigned int freq)
{
	freq = get_adjusted_freq(freq);
	fprintf(f, "%s\t%s\t%u\n", hz, py, freq);
}

/**	获取词条文件中的一条
 */
int get_ci_line(FILE *f, char *hz, char *py, int *freq)
{
	static char buffer[0x400];
	char   *p;
	int    ret;

	do
	{
		p = fgets(buffer, sizeof(buffer), f);
		if (!p)
			return 0;
	}while(*p == '#');

	ret = sscanf(p, "%s %s %d", hz, py, freq);
	if (ret == 3)
		return 1;
	if (ret == 2)
	{
		*freq = 0;
		return 1;
	}

	return 0;
}

/**	获得语料频率中的一条
 */
int get_freq_line(FILE *f, char *hz, int *freq)
{
	static char buffer[0x400];
	char   *p;
	int    ret;
	static int line;

	line++;
	if (!(line & 0xffff))
		printf("%d\r", line);

	do
	{
		p = fgets(buffer, sizeof(buffer), f);
		if (!p)
			return 0;
	}while(*p == '#');

	ret = sscanf(p, "%s %d", hz, freq);
	return ret == 2;
}

/**	比较汉字
 *	改为ansi排序
 */
int compare_hz(HZ *hz1, HZ *hz2)
{
	unsigned char *p, *q;
	p = (unsigned char*)hz1, q = (unsigned char*)hz2;
	while(*p && *q)
	{
		/*
		if (*(HZ*)p != *(HZ*)q)
			return *(HZ*)p - *(HZ*)q;
		p += sizeof(HZ);
		q += sizeof(HZ);
		*/
		if (*p != *q)
			return *p - *q;
		p++;
		q++;
	}
	if (*p)
		return 1;
	if (*q)
		return -1;
	return 0;
}

/**	文件缓冲区
 */
char file_buffer0[0x100000];
char file_buffer1[0x100000];
char file_buffer2[0x100000];

/**	处理文件
 */
int process()
{
	char ci_hz[0x100], freq_hz[0x100], py[0x100];
	int  old_freq, new_freq;
	FILE *f_ci, *f_freq, *fw;
	int  ret, freq_over, ci_over;

	fw = fopen(out_file_name, "w");
	if (!fw)
	{
		printf("<%s>无法创建\n", out_file_name);
		return -1;
	}

	//设置缓冲区大小，加快程序处理速度
	if (setvbuf(fw, file_buffer0, _IOFBF, sizeof(file_buffer0)))
		printf("设置缓冲区出错\n");

	f_ci = fopen(ci_file_name, "r");
	if (!f_ci)
	{
		printf("<%s>无法打开\n", ci_file_name);
		return -1;
	}

	//设置缓冲区大小，加快程序处理速度
	if (setvbuf(f_ci, file_buffer1, _IOFBF, sizeof(file_buffer1)))
		printf("设置缓冲区出错\n");

	f_freq = fopen(freq_file_name, "r");
	if (!f_freq)
	{
		printf("<%s>无法打开\n", freq_file_name);
		return -1;
	}

	//设置缓冲区大小，加快程序处理速度
	if (setvbuf(f_freq, file_buffer2, _IOFBF, sizeof(file_buffer2)))
		printf("设置缓冲区出错\n");

	if (!get_freq_line(f_freq, freq_hz, &new_freq) || !get_ci_line(f_ci, ci_hz, py, &old_freq))
	{
		printf("文件读取失败\n");
		fclose(f_ci);
		fclose(f_freq);
		fclose(fw);
		return -1;
	}

	freq_over = 0;
	ci_over = 0;
	//遍历词条文件
	while(!freq_over && !ci_over)
	{
		ret = compare_hz((HZ*)ci_hz, (HZ*)freq_hz);
		if (!ret)		//相同
		{
			output(fw, ci_hz, py, new_freq);
			//由于可能在词库中有相同的词，所以要继续读取词条
			if (!get_ci_line(f_ci, ci_hz, py, &old_freq))
				ci_over = 1;
			continue;
		} 

		if (ret < 0)		//词条文件小于词频文件
		{
			output(fw, ci_hz, py, 0);		//此时必须输出0
			if (!get_ci_line(f_ci, ci_hz, py, &old_freq))
				ci_over = 1;
			continue;
		}

		//词条文件大于词频文件
		if (!get_freq_line(f_freq, freq_hz, &new_freq))
			freq_over = 1;
	};

	//如果freq文件结束，则输出剩余的词条数目
	if (freq_over)
	{
		output(fw, ci_hz, py, 0);
		while(get_ci_line(f_ci, ci_hz, py, &old_freq))
			output(fw, ci_hz, py, old_freq);
	}

	fclose(fw);
	fclose(f_ci);
	fclose(f_freq);

	return 0;
}

const char *usage = 
	"set_freq ci_file freq_file out_file\n"
	"\n"
	"ci_file     准备进行词频设置的文件\n"
	"freq_file   基于语料生成的词频文件\n"
	"out_file    输出文件\n";

int main(int argc, char **argv)
{
	if (argc != 4)
	{
		printf(usage);
		return -1;
	}

	ci_file_name = argv[1];
	freq_file_name = argv[2];
	out_file_name = argv[3];

	return process();
}
   
