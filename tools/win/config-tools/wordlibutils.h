#ifndef WORDLIBUTILS_H
#define WORDLIBUTILS_H
#include <Windows.h>
#include <tchar.h>
#include <fileapi.h>
#include "wordlibdef.h"
#include "syllableapi.h"


int LoadFromFile(const TCHAR *file_name, void *buffer, int buffer_length);
int SaveToFile(const TCHAR *file_name, void *buffer, int buffer_length);

int GetLineFromFile(FILE *fr, TCHAR *line, int length);
void MakeStringFitLength(TCHAR *string, int length);
int ParsePinYinStringReverse(const TCHAR *pin_yin, SYLLABLE *syllables, int array_length, int fuzzy_mode, int pinyin_mode);
WORDLIB *GetWordLibrary(int wordlib_id);
int GetWordLibSize(WORDLIB *wordlib);
int FileExists(const TCHAR *file_name);
int GetItemLength(int hz_length, int syllable_length);
int NewWordLibPage(WORDLIB* wordlib);
HZ *GetItemHZPtr(WORDLIBITEM *item);
int GetCiInPageWild(PAGE *page,	SYLLABLE *syllable_array, int syllable_count, CANDIDATE *candidate_array, int candidate_length,	int fuzzy_mode);

int GetCiInPage(PAGE *page,	SYLLABLE *syllable_array, int syllable_count, CANDIDATE *candidate_array, int candidate_length,	int fuzzy_mode);

WORDLIBITEM *GetNextCiItem(WORDLIBITEM *item);

int GetFileLength(const TCHAR *file_name);

#endif // WORDLIBUTILS_H
