/*	成生ncoc库文件.
 *	从已经处理完毕的文件中读取汉字组以及ncoc值, 生成ncoc数据文件
 *	数据文件内容：
 *	△我		100
 *	我们		200000000
 *	...
 *	
 *	处理过程：
 *	一个字节放置一个BCOC、TCOC的自然对数。
 *	TCOC处理方法(x)：
 *		if (x < 4096)
 *			return;
 *		x /= 4096
 *		x = ln(x);
 *		x *= 32;
 *		x = int(x, 255);
 *
 *	还原(x)
 *		x /= 32;
 *		x = exp(x);
 *		x *= 4096;
 *	
 *		
 *	1. 依据数据缩减方法对频率进行缩小工作，对于小于2的数据将放弃
 *	2. 依据参数，确定生成bcoc还是tcoc
 *	3. 放置，如果count小于256，则以一个字节来表达，如果不小于256，则
 *		在下一个位置，放置同样的汉字，并把高位放上
 *
 *	使用参数：
 *		make_ncoc -b|-t src_txt reduce_bit min_freq
 *	如：make_ncoc -b f2.txt 0 4				->现有语料, 2.8M左右
 *		make_ncoc -t f3.txt 0 4				->现有语料, 13.0M左右
 *
 *		实际的BCOC参数：10位移动，BCOC数字在2以上入库
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncoc.h>
#include <math.h>

#define	TEST_FILE	"test.txt"

#define	MAX_INDEX0_ITEMS	(1 << 13)
#define	MAX_INDEX1_ITEMS	(1 << 22)
#define	MAX_ITEMS			(1 << 24)

int 	index0_count = 0;							//第一级索引计数
int		index1_count = 0;							//第二级索引计数
int		item_count = 0;								//项计数

int		min_freq = 1;								//最小词频数目
int		multi_factor = 1;							//对数的放大倍数

INDEXITEM	index0[MAX_INDEX0_ITEMS];				//第一级索引表
INDEXITEM	index1[MAX_INDEX1_ITEMS];				//第二级索引表
NCOCITEM	item[MAX_ITEMS];						//数据

/**	自动增加测试用例, 以512分之一的概率选择测试
 *	数据, 当然第一个与最后一个必须是用例.
 *	注:当str == 0时表示结束
 *	由于norton的缘故，每次向外写出文件会造成系统变慢，
 *	因此，需要最后关闭。
 */
void add_test(const char *str, int ncoc)
{
	static FILE *f;
	static first = 1;
	static char last_str[0x20];
	static int last_ncoc;

	if (first)
	{
		_unlink(TEST_FILE);
		f = fopen(TEST_FILE, "a");
	}

	/* 以接近512分之一的概率进行选择测试用例 */
	/* 第一个与最后一个必须加入到测试中. */
	if (str && !first && 0)//rand() % 512 != 0)
	{
		strcpy(last_str, str);
		last_ncoc = ncoc;
		return;
	}

	first = 0;

	if (!f)
		return;

	if (str)
	{
		fprintf(f, "%s %d\n", str, ncoc);
		strcpy(last_str, str);
		last_ncoc = ncoc;
	}
	else		/* 最后一个 */
	{
		fprintf(f, "%s %d\n", last_str, last_ncoc);
		fclose(f);
	}
}

/**	输出BCOC文件
 */
int output_bcoc()
{
	char buffer[0x400];				//1Kheader
	FILE *fw;
	BCOCHEADER *header = (BCOCHEADER*)buffer;

	fw = fopen(BCOC_NAME, "wb");
	if (!fw)
	{
		fprintf(stderr, "<%s>文件写打开错误!\n", BCOC_NAME);
		return -1;
	}

	memset(buffer, 0, sizeof(buffer));
	header->sign = BCOC_SIGN;
	header->index0_count = index0_count;
	header->item_count = item_count;
	header->index0_data_pos = 0;
	header->item_data_pos = header->index0_data_pos + index0_count * sizeof(index0[0]);

	if (sizeof(buffer) != fwrite(buffer, 1, sizeof(buffer), fw) ||
		index0_count != (int)fwrite(index0, sizeof(index0[0]), index0_count, fw) ||
		item_count != (int)fwrite(item, sizeof(item[0]), item_count, fw))
	{
		printf("文件输出错误!\n");
		fclose(fw);
		exit(-1);
	}

	fclose(fw);
	return 0;
}

/**	输出TCOC文件
 */
int output_tcoc()
{
	char buffer[0x400];
	TCOCHEADER	*header = (TCOCHEADER*)buffer;
	FILE *fw;

	fw = fopen(TCOC_NAME, "wb");
	if (!fw)
	{
		fprintf(stderr, "<%s>文件写打开错误!\n", TCOC_NAME);
		return -1;
	}

	memset(buffer, 0, sizeof(buffer));

	header->sign = TCOC_SIGN;
	header->index0_count = index0_count;
	header->index1_count = index1_count;
	header->item_count = item_count;
	header->index0_data_pos = 0;
	header->index1_data_pos = header->index0_data_pos + index0_count * sizeof(index0[0]);
	header->item_data_pos = header->index1_data_pos + index1_count * sizeof(index1[0]);

	if (sizeof(buffer) != fwrite(buffer, 1, sizeof(buffer), fw) ||
		index0_count != (int)fwrite(index0, sizeof(index0[0]), index0_count, fw) ||
		index1_count != (int)fwrite(index1, sizeof(index1[0]), index1_count, fw) ||
		item_count != (int)fwrite(item, sizeof(item[0]), item_count, fw))
	{
		printf("文件输出错误!\n");
		fclose(fw);
		exit(-1);
	}

	fclose(fw);
	return 0;
}

/**	添加BCOC项
 */
void add_bcoc_item(const char *str, int count)
{
	static HZ last_hz0, last_hz1;
	HZ  hz0, hz1;
	double x;
	static int c;

	c++;
	if (!(c & 0x3ff))
		printf("%d\r", c);

	if (!str)		//结束处理, 输出最后一项
	{
		index0[index0_count].hz = 0;
		index0[index0_count++].item_pos = item_count;
		item_count++;
		add_test(0, 0);
		return;
	}

	//校验汉字串的长度	
	if (strlen(str) != 2 * sizeof(HZ))			//无效
		return;

	count /= min_freq;
	if (!count)			//没有必要
		return;

	x = 1.0 * count;
	x = log(x);
	x *= multi_factor;

	count = (int)x;
	if (count > 255)
		count = 255;

	hz0 = *(HZ*)str;
	hz1 = *(HZ*)(str + sizeof(HZ));

	if (last_hz0 != hz0)				//第一个就不相等
	{
		index0[index0_count].hz = hz0;
		index0[index0_count].item_pos = item_count;
		index0_count++;
	}

	item[item_count].hz= hz1;
	item[item_count].count = count & 0xff;
	item_count++;
	if (count >= (1 << 8))
	{
		item[item_count].hz = hz1;
		item[item_count].count = (count & 0xff00) >> 8;
		item_count++;
	}

	if (index0_count >= sizeof(index0) / sizeof(index1[0]) ||
		item_count >= sizeof(item) / sizeof(item[0]))
	{
		fprintf(stderr, "索引表溢出. index0:%d, item:%d\n", index0_count, item_count);
		exit(-1);
		return;
	}

	add_test(str, count);
	last_hz0 = hz0;
	last_hz1 = hz1;

	return;
}

/**	添加TCOC项
 */
void add_tcoc_item(const char *str, int count)
{
	static HZ last_hz0, last_hz1, last_hz2;
	HZ hz0, hz1, hz2;
	double x;
	static int c;

	c++;
	if (!(c & 0xfff))
		printf("%d\r", c);

	if (!str)		/* 结束处理, 输出最后一项 */
	{
		index0[index0_count].hz = 0;
		index0[index0_count++].item_pos = index1_count;
		index1[index1_count].hz = 0;
		index1[index1_count++].item_pos = item_count;
		item_count++;
		add_test(0, 0);
		return;
	}

	count /= min_freq;
	if (!count)				//过小，没有必要
		return;

	x = 1.0 * count;
	x = log(x);
	x *= multi_factor;

	count = (int)x;
	if (count > 255)		//放入一个字节以内
		count = 255;

	//检查汉字串长度
	if (strlen(str) != 3 * sizeof(HZ))
		return;

	hz0 = *(HZ*)str;
	hz1 = *(HZ*)(str + sizeof(HZ));
	hz2 = *(HZ*)(str + 2 * sizeof(HZ));

	if (last_hz0 != hz0)		//第一个不相等
	{
		index0[index0_count].hz = hz0;
		index0[index0_count].item_pos = index1_count;
		index0_count++;
	}

	if (last_hz0 != hz0 || last_hz1 != hz1)		//第二个不相等
	{
		index1[index1_count].hz = hz1;
		index1[index1_count].item_pos = item_count;
		index1_count++;
	}

	//第三个肯定要增加
	item[item_count].hz= hz2;
	item[item_count].count = count;
	item_count++;

	if (index0_count >= sizeof(index0) / sizeof(index0[0]) ||
		index1_count >= sizeof(index1) / sizeof(index1[0]) ||
		item_count >= sizeof(item) / sizeof(item[0]))
	{
		fprintf(stderr, "索引表溢出. index0:%d, index1:%d, item:%d\n", index0_count, index1_count, item_count);
		exit(-1);
		return;
	}

	add_test(str, count);

	last_hz0 = hz0;
	last_hz1 = hz1;
	last_hz2 = hz2;
	return;
}

/**	测试BCOC。
 *	程序从测试文件中读取测试数据。
 *	一行一个测试, 以汉字打头, ncoc为第二个数字.
 *	如: 美丽 18
 */
void test_bcoc()
{
	char str[0x20];
	int  bcoc, value;
	FILE *fr;

	if (!open_bcoc(BCOC_NAME))
	{
		printf("BCOC文件<%s>打开失败\n", BCOC_NAME);
		exit(-1);
	}

	fr = fopen(TEST_FILE, "rt");
	if (!fr)
	{
		printf("测试文件<%s>打开失败\n", TEST_FILE);
		exit(-1);
	}

	while(2 == fscanf(fr, "%s %d", str, &bcoc))
	{
		value = get_bcoc((HZ*)str);
		if (value != bcoc)
		{
			fprintf(stderr, "%s的BCOC值应为%d，实际为%d\n", str, bcoc, value);
			exit(-1);
		}
	}
	printf("测试通过\n");
	fclose(fr);
	return;
}

/**	测试TCOC。
 *	程序从测试文件中读取测试数据。
 *	一行一个测试, 以汉字打头, ncoc为第二个数字.
 *	如: 美丽 18
 */
void test_tcoc()
{
	char str[0x20];
	int  tcoc, value;
	FILE *fr;

	if (!open_tcoc(TCOC_NAME))
	{
		printf("TCOC文件<%s>打开失败\n", TCOC_NAME);
		exit(-1);
	}

	fr = fopen(TEST_FILE, "rt");
	if (!fr)
	{
		printf("测试文件<%s>打开失败\n", TEST_FILE);
		exit(-1);
	}

	while(2 == fscanf(fr, "%s %d", str, &tcoc))
	{
		value = get_tcoc((HZ*)str);
		if (value != tcoc)
		{
			fprintf(stderr, "%s的TCOC值应为%d，实际为%d\n", str, tcoc, value);
			exit(-1);
		}
	}
	printf("测试通过\n");
	fclose(fr);
	return;
}

void usage(const char *p_name)
{
	fprintf(stdout, "usage:\n"
					"%s -t|-b src_file 最小词频 对数放大倍数\n"
					"-t           创建TCOC数据文件\n"
					"-b           创建BCOC数据文件\n"
					"src_file     包含词频信息的文件名称\n"
					"最小词频     低于本数值的词将不放入，TCOC一般为4096，BCOC一般为1024\n"
					"对数放大倍数 求对数之后进行的放大系数，TCOC一般为32，BCOC一般为...\n",
					p_name);
}

int main(int argc, char **argv)
{
	char str[0x20];
	int  freq;
	FILE *fr;

	if (argc != 5)
	{
		usage(argv[0]);
		return -1;
	}

	fr = fopen(argv[2], "rt");
	if (!fr)
	{
		printf("无法打开<%s>文件\n", argv[2]);
		return -1;
	}

	min_freq = atoi(argv[3]);
	multi_factor = atoi(argv[4]);

	if (!strcmp(argv[1], "-b"))
	{
		while(2 == fscanf(fr, "%s %d", str, &freq))
			add_bcoc_item(str, freq);
		add_bcoc_item(0, 0);
		output_bcoc();
		test_bcoc();
	}
	else if (!strcmp(argv[1], "-t"))
	{
		while(2 == fscanf(fr, "%s %d", str, &freq))
			add_tcoc_item(str, freq);
		add_tcoc_item(0, 0);
		output_tcoc();
		test_tcoc();
	}
	else
	{
		usage(argv[0]);
		return -1;
	}

	fclose(fr);

	printf("完成，共%d条\n", item_count);
	return 0;
}
