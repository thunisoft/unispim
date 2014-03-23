/**	分词程序Dll版本
 */
#include <iostream>
#include <fstream>

using namespace std;

#include <stdio.h>
#include <time.h>
#include <string.h>

#include "mmseg.h"
#include <windows.h>

#define	AMBI_FILE_NAME		"ambi.txt"

extern "C" __declspec(dllexport) int Segment(const char *dict_name, const char *str, char *result);

/**	分词过程
 *	参数：
 *		dict_name			分词词典名称，如果已经调用过，则可以为0
 *		str					等待分词的字符串，当结束的时候，以0调用，则释放分配的内存
 *		result				分词结果
 *	返回：
 *		1		成功
 *		0		失败
 */
int Segment(const char *dict_name, const char *str, char *result)
{
	static mmseg *seg = 0;

	if (result)
		*result = 0;

	if (!str)
	{
		if (seg)
			delete seg;
		seg = 0;
	}

	if (dict_name)
	{
		if (seg)
			delete seg;
		seg = new mmseg(dict_name, AMBI_FILE_NAME);
	}

	if (!seg || !str || !result)
		return 0;

	int index = 0;
	int str_len = (int)strlen(str);
	int len = seg->segment(str, str_len);
	while(len)
	{
		strncat_s(result, 1024, str + index, len);
		strcat_s(result, 1024, " ");
		index += len;
		len = seg->segment(str + index, str_len - index);
	}
	
	return 1;
}