/*	智能组词头文件
 */

#ifndef	_ICW_H_
#define	_ICW_H_

#include <kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

#define	BIGRAM_FILE_NAME		TEXT("unispim6\\wordlib\\bigram.dat")

#define	ICW_MAX_ITEMS			1024					//每一个ICW项的最大候选数目
#define	ICW_MAX_CI_ITEMS		256						//每项最大的词数目
#define	ICW_MAX_PART_SYLLABLES	5						//最多5个非全音节

extern int GetIcwCandidates(SYLLABLE *syllable, int syllable_count, CANDIDATE *candidate);
extern int LoadBigramData(const TCHAR *name);
extern int FreeBigramData();
extern int MakeBigramFaster();

#ifdef __cplusplus
}
#endif

#endif