#ifndef	_DICT_H_
#define	_DICT_H_

#include <iostream>
#include <fstream>
#include "hash_table.h"

using namespace std;

#define	MAX_WORD_LENGTH	16
#define	DICT_HASH_SIZE	0x200000

class dictionary
{
	public:
		int			hz_freq[0x10000];			//ºº×Ö
		hash_table	*table;						//hash±í
		int			ci_count;					//´Ê»ã¼ÆÊý

		dictionary(const char *dict_name);
		~dictionary();
		void make_item(const char *dict_name);
		int is_word(const char *str, int length);
		int max_match_length(const char *str, int length);
		int max_match_length(const char *str);
		int get_free_morphemic(const char *str);
		int get_words_length(const char *str, int length, int *word_length);
};

#endif
