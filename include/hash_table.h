#ifndef	_HASH_H_
#define	_HASH_H_

#include <fstream>

using namespace std;

#include <stdlib.h>
#include <string.h>

#define	HASH_SIZE	0x400000

/**	hash表节点
 */
class hash_node
{
	public:
		char 			str[24];				//本项的字符串
		int				value;					//本项的值
		int				close_word_length;		//最接近的下一个词汇的长度
};

/**	字符串hash表
 */
class hash_table
{
	public:
		unsigned int ascii_keys[0x100][0x40];		//字符散列key

		hash_node	*table;							//hash表项

		~hash_table();
		hash_table();
		void add(const char *str, int value);
		void add(const char *str);
		void pure_add(const char *str, int value);
		void increase_count(const char *str);
		hash_node *get_node(const char *str, int length);
		hash_node *get_node(const char *str);
		unsigned int get_rand_byte();
		void init_ascii_keys();
		int get_value(const char *str);
		int get_value(const char *str, int length);
		static int is_same_string(const char *s1, int length, const char *s2);
		void output(ofstream &out_file);
		void output_conflics();
		static int is_same_string(const char *s1, const char *s2);
		unsigned int get_key(const char *str);
		unsigned int get_key(const char *str, int length);
};

#endif
