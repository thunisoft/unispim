/**	汉语语料切分程序
 *	采用正向
 *	对于带有二义性的切分语句，输出到ami.txt文件中，用于
 *	后期检查改进。
 */
#include <iostream>
#include <fstream>
#include <string.h>
#include "dictionary.h"

using namespace std;

//#define	OUTPUT_AMBI

#define	DEFAULT_DICT_NAME		"dict-seg.txt"
#define	MAX_MID_RESULTS			0x40

struct MID_RESULT					//分词中间结果
{
	int length0;					//第一块长度
	int length1;					//第二块长度
	int length2;					//第三块长度
	int count;						//块数目
	int total_length;				//全部长度
	int avg_length;					//平均长度
	int avg_diff2;					//均方差
	int free_morphemic;				//自由语素度
};

class mmseg
{
	public:
		const char	*ambi_name;
		dictionary	*dict;

		mmseg(const char *dict_name, const char *ambi_name);
		~mmseg();
		int get_word_length(const char *str, int length);
		int get_word_length(const char *str);
		int get_mid_result(const char *str, int length, MID_RESULT result[]);
		int new_get_mid_result(const char *str, int length, MID_RESULT result[]);
		void output_result(MID_RESULT *result, int index, const char *str);
		int segment(const char *str, int length);
		int segment(const char *str);
		void ambi_out(const char *str, MID_RESULT *result);
};

