/**	
 *	词条入库调整程序
 *	将文件中的词条与系统词库中的词汇进行比较，如果存在，则不输出。
 *	如果该条目没有任何候选，但ICW能够组成这个条目，不输出。
 *	其他情况，输出本条目，作为候选条目
 *
 *	文件格式
 *		我们			wo'men
 *		...				...
 */

#define	MAX_CANDIDATE		0x100
#define	MAX_SYLLABLE		0x10

#include <kernel.h>
#include <syllable.h>
#include <config.h>
#include <spw.h>
#include <ci.h>
#include <zi.h>
#include <icw.h>
#include <context.h>
#include <utility.h>
#include <pim_resource.h>

/**	检查词条，如果核心能够处理（有这个词条，或在没有其他候选的情况下能够ICW出来）
 *	返回1，否则返回0
 */
int HaveItem(SYLLABLE *syllables, HZ *ci, int ci_length)
{
	int i, count;
	CANDIDATE candidate[MAX_CANDIDATES];

	if (ci_length < 2)
		return 0;

	count = ProcessCiCandidate(syllables, ci_length, 0, candidate, MAX_CANDIDATES, 0);
	if (count)		//有候选
	{
		for (i = 0; i < count; i++)
		{
			if (ci_length == candidate[i].word.item->ci_length &&
				!strncmp((char*)ci, (char*)candidate[i].word.hz, sizeof(HZ) * ci_length))
				return 1;		//找到该词
		}
		return 0;				//有其他候选，不需要判断ICW
	}

	count = GetIcwCandidates(syllables, ci_length, candidate);
	if (!count)
		return 0;

	if (!strncmp((char*)ci, (char*)candidate[0].icw.hz, sizeof(HZ) * ci_length))
		return 1;
	return 0;
}

void Process(FILE *f)
{
	char ci[0x100], py[0x100];
	int  line_no;
	int  ci_length, py_length;
	SYLLABLE syllable[MAX_SYLLABLE];

	line_no = 0;
	while(2 == fscanf_s(f, "%s %s", ci, sizeof(ci), py, sizeof(py)))
	{
		line_no++;
		ci_length = (int)strlen(ci);
		if (ci_length & 1)
			continue;
		ci_length /= 2;
		
		py_length = ParsePinYinStringReverse(py, syllable, MAX_SYLLABLE, 0, PINYIN_QUANPIN);
		if (!py_length || py_length != ci_length)
			continue;

		if (!strcmp(ci, "阿尔卑斯山脉"))
			ci_length = ci_length;
		if (HaveItem(syllable, (HZ*)ci, ci_length))
			continue;

		printf("%s %s\n", ci, py);
	}	
}

int main(int argc, char **argv)
{
	FILE *fr;

	if (argc == 1)
	{
		printf("check_ci txt_file\n");
		return -1;
	}
	
	fr = fopen(argv[1], "rt");
	if (!fr)
	{
		fprintf(stderr, "can't open <%s>.\n", argv[1]);
		return -2;
	}

	MaintainConfig();
	PIM_LoadResources();
	Process(fr);

	fclose(fr);
	return 0;
}

