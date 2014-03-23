/**	将QQ拼音输入法的系统词库解出
 *
 *	
 *	QQ词库词条的编制方法：
 *
 *	{
 *		short	ci_count;				//形同拼音的词条数目 
 *		char	pinyin[];				//拼音asciiz串
 *		{
 *			int		freq;				//词频
 *			char	ci_length;			//词条汉字数目
 *			short	ci[];				//第奇数个汉字低位用长度异或
 *		}
 *		{
 *			...							//如果还有词条
 *		}
 *
 *	
 *	拼音串的标识方法：
 *		letter		: a-z
 *					;
 *
 *		syllable	: letter
 *					| syllable letter
 *					;
 *		
 *		pinyin		: syllable ' syllable
 *					| pinyin ' syllable
 *					;
 *
 *
 */
#define	_CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <string.h>

#define	FILE_NAME		"qq.dat"

char buffer[0xa00000];					//10M的数据
char *buffer_end;						//缓冲区结尾
int  file_length;						//词库文件长度

int load_data()
{
	FILE *fr;

	fr = fopen(FILE_NAME, "rb");
	if (!fr)
		return 0;

	file_length = fread(buffer, sizeof(buffer[0]), sizeof(buffer), fr);
	buffer_end = buffer + file_length;
	fclose(fr);

	return 1;
}

char *get_next_pinyin(char *p)
{
	int  has_sep;
	char *q;

	//判断是否到结尾
	for (; p < buffer_end; p++)
	{
		//找到第一个英文字母
		if (*p < 'a' || *p > 'z')
			continue;

		if (*(p - 1) != 0)			//必须以0开头
			continue;

		//寻找拼音串
		has_sep = 0;
		for (q = p; q < buffer_end; q++)
		{
			if (*q == '\'')
			{
				if (*(q - 1) == '\'')
					break;
				has_sep = 1;
				continue;
			}

			if (*q >= 'a' && *q <= 'z')
				continue;
			break;
		}

		if (!has_sep || *q != 0)		//必须以0结尾
		{
			p = q;
			continue;
		}
		return p;			//找到一个拼音
	}

	return 0;
}

void output_ci(char *p, int length)
{
	int i;
	char ch;

	for (i = 0; i < length; i++)
	{
		ch = p[i];
		if (!(i % 4))
			ch ^= (char)length;
		printf("%c", ch);
	}
}

void process()
{
	char *p = buffer, *q;
	char ci_count;
	char ci_length;
	int  i;

	if (!load_data())
		return;

	p += 0xcef50;		//跳过前面不需要的数据

	while(p = get_next_pinyin(p))
	{
		//判断拼音前面是否为候选的数目，注意：a'a前面为0
		ci_count = *(p - 2);
		if (ci_count <= 0 || ci_count > 0x20)		//判断词计数的合法性
			continue;

		q = p + strlen(p) + 1;		//指向拼音结尾
		for (i = 0; i < ci_count; i++)
		{
			q += sizeof(int);				//跳过词频
			ci_length = *q++;
			output_ci(q, ci_length);
			q += ci_length;
			printf("\t");
			//输出拼音
			printf("%s\n", p);
		}
		p = q;
	}
}

void main()
{
	process();
}
