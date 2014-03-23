#ifndef	_ENGLISH_H_
#define	_ENGLISH_H_

#include <kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

#define	ENGLISH_LIB_FILE_NAME		TEXT("unispim6\\english\\english.dat")
#define ENGLISH_TRANS_FILE_NAME		TEXT("unispim6\\english\\english_trans.dat")

#define ENGLISH_MAX_ITEMS			80000
#define ENGLISH_LETTER_COUNT		26
#define ENGLISH_WORD_MAX_SIZE		0x200
#define ENGLISH_TRANS_BUFFER_SIZE	50000

//英文词库头定义
typedef struct tagENGLISHWORDLIB
{
	int			signature;													//词库的签名
	int			count;														//词汇数目
	int			letter_index[ENGLISH_LETTER_COUNT];							//首字母索引表
	int			index[ENGLISH_MAX_ITEMS];									//索引表
	char		data[1];													//数据
} ENGLISHWORDLIB;

//英文翻译头定义
typedef struct tagENGLISHTRANSLIB
{
	int			signature;													//签名
	int			count;														//条目数
	int			letter_index[ENGLISH_LETTER_COUNT][ENGLISH_LETTER_COUNT];	//两字母索引
	int			EngIndex[ENGLISH_TRANS_BUFFER_SIZE];						//英文单词索引
	int			TransIndex[ENGLISH_TRANS_BUFFER_SIZE];						//翻译索引
	TCHAR		data[1];													//数据
} ENGLISHTRANSLIB;

extern int LoadEnglishData(const TCHAR *file_name);
extern int FreeEnglishData();
extern int GetEnglishCandidates(const TCHAR *prefix, CANDIDATE *candidate_array, int array_length);

extern int LoadEnglishTransData(const TCHAR *file_name);
extern int FreeEnglishTransData();
extern TCHAR* GetEnglishTranslation(const TCHAR *english_word);

#ifdef __cplusplus
}
#endif

#endif