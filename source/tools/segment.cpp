/**	本汉字分词算法采用mmseg算法。
 *	算法参考：http://technology.chtsai.org/mmseg
 */

#include <iostream>
#include <fstream>

using namespace std;

#include <stdio.h>
#include <time.h>

#include "mmseg.h"

#define	DEFAULT_IN_FILE		"in.txt"
#define	DEFAULT_OUT_FILE	"out.txt"
#define	DEFAULT_DICT_FILE	"dict.txt"
#define	DEFAULT_AMBI_FILE	"ambi.txt"

const char *usage =
	"segment [in_file] [out_file] [dictionary] [ambi_file]\n"
	"\n"
	"in_file                 被处理的文本数据，默认为in.txt\n"
	"out_file                输出分词结果文件，默认为out.txt\n"
	"dictionary              词典文件（单字需要附带字频，词频需要给0），默认为dict.txt\n"
	"ambi_file               混淆输出文件，默认为ambi.txt\n"
	"\n";

char buffer[0x100000];
char out_buffer[0x100000];
int  out_index = 0;

void output(FILE *fw, const char *str, int length = 0)
{
	static int line_count = 0;

	if (!str)			//输出
	{
		fwrite(out_buffer, 1, out_index, fw);
		return;
	}

	if (!length)
	{
		while(*str)
		{
			out_buffer[out_index++] = *str;
			str++;
			line_count++;
		}
	}
	else
	{
		for (int i = 0; i < length; i++)
			out_buffer[i + out_index] = str[i];
		out_index += length;
		line_count += length;
	}

	if (line_count > 80)
	{
		out_buffer[out_index++] = '\n';
		line_count = 0;
	}

	if (out_index > sizeof(out_buffer) * 3 / 4)
	{
		fwrite(out_buffer, 1, out_index, fw);
		out_index = 0;
	}
}

int main(int argc, char **argv)
{
	const char *in_file_name = DEFAULT_IN_FILE;
	const char *out_file_name = DEFAULT_OUT_FILE;
	const char *dict_file_name = DEFAULT_DICT_FILE;
	const char *ambi_file_name = DEFAULT_AMBI_FILE;

	mmseg *seg;

	//处理命令行
	if (argc > 1)
		in_file_name = argv[1];
	if (argc > 2)
		out_file_name = argv[2];
	if (argc > 3)
		dict_file_name = argv[3];
	if (argc > 4)
		ambi_file_name = argv[4];

	if (!strcmp(in_file_name, "/?") ||
		!strcmp(in_file_name, "/help"))
	{
		cout << usage;
		return -1;
	}

	cout << "正在读入分词词典...\n";
	seg = new mmseg(dict_file_name, ambi_file_name);

	FILE *in_file, *out_file;

	cout << "开始处理原始语料...\n";
	fopen_s(&in_file, in_file_name, "rb");
	if (!in_file)
	{
		cout << "文件<" << in_file_name << ">无法打开\n";
		return -1;
	}

	fopen_s(&out_file, out_file_name, "w");
	if (!out_file)
	{
		cout << "文件<" << out_file_name << ">无法创建\n";
		return -1;
	}

	output(out_file, "△");
	int last_is_terminator = 1;
	long long char_count = 0;
	int index = sizeof(buffer);
	int str_len = sizeof(buffer);
	int remain_chars = 0;
	int length;

	int st = clock();

	while(1)
	{
		int len;

		length = fread(buffer + remain_chars, 1, sizeof(buffer) - remain_chars, in_file);
		if (length <= 0 && remain_chars < 0x100)
			break;

		str_len = length + remain_chars;
		char_count += length;
		index = 0;

		cout << char_count / 0x100000 << "M, " << (clock() - st) / 1000 << "s\r";

		index = 0;
		do
		{
			len = seg->segment(buffer + index, str_len - index);
			if (len)
			{
				output(out_file, " ");
				output(out_file, buffer + index, len);
				last_is_terminator = 0;
			}
			else
			{
				if (!last_is_terminator)
					output(out_file, " △");

				if (buffer[index] & 0x80)
					len = 2;
				else 
					len = 1;
				last_is_terminator = 1;
			}
			index += len;
		}while(index < str_len && sizeof(buffer) - index > 0x200);

		remain_chars = str_len - index;
		//将剩余字节复制到缓冲区前端
		for (int i = 0; i < remain_chars; i++)
			buffer[i] = buffer[i + index];
	}

	output(out_file, 0);
	fclose(in_file);
	fclose(out_file);
	delete seg;

	int et = clock();
	cout << "分词处理结束\n";
	cout << "时间耗费:" << et - st << endl;

	return 0;
}
