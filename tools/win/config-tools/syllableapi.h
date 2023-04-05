#ifndef SYLLABLEAPI_H
#define SYLLABLEAPI_H
#include <Windows.h>
#include <tchar.h>
#include "wordlibdef.h"



int GetTone(const TCHAR *pin_yin);

int GetSyllable(const TCHAR *pin_yin, SYLLABLE *syllable, int *syllable_str_length, int fuzzy_mode/*, int *correct_flag, int *separator_flag*/);

int GetLegalPinYinLength(const TCHAR *pin_yin, int state, int english_state);

int ContainCon(SYLLABLE syllable, SYLLABLE checked_syllable, int fuzzy_mode);

int GetSyllableString(SYLLABLE syllable, TCHAR *str, int length, /*int correct_flag, */int tone_char);


#endif // SYLLABLEAPI_H
