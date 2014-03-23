#include <iostream>
#include <fstream>
#include <hash_table.h>

using namespace std;

#include <stdlib.h>
#include <string.h>

hash_table::~hash_table()
{
	delete [] table;
}

hash_table::hash_table()		
{
	init_ascii_keys();					//初始化字符key表

	this->table = new hash_node[HASH_SIZE];	//初始化hash表项
	memset(this->table, 0, sizeof(hash_node) * HASH_SIZE);
}

//增加散列项
void hash_table::add(const char *str, int value)
{
	hash_node *node;
	unsigned int key = get_key(str);
	int index = key % HASH_SIZE;

	while(table[index].str[0])
		index++;

	node = &table[index];
	strcpy(node->str, str);
	node->value = 0;

	//检查是否为其他词的前缀
	int length = (int)strlen(str);
	for (int i = length - 2; i >= 4; i -= 2)
	{
		hash_node *prefix_node = get_node(str, i);
		if (prefix_node)
		{
			if (!prefix_node->close_word_length ||
				length < prefix_node->close_word_length)
				prefix_node->close_word_length = length;
		}
	}
}

void hash_table::add(const char *str)
{
	add(str, 0);
}

//仅仅增加散列项，不做前缀判断
void hash_table::pure_add(const char *str, int value)
{
	unsigned int key = get_key(str);
	int index = key % HASH_SIZE;
	hash_node *node;

	while (table[index].str[0])				//寻找空的Slot
	{
		index++;
		if (index == HASH_SIZE)
			index = 0;
	}

	//找到！
	node = &table[index];
	strcpy(node->str, str);
	node->value = value;
}

//仅仅增加散列项，不做前缀判断
void hash_table::increase_count(const char *str)
{
	unsigned int key = get_key(str);
	int index = key % HASH_SIZE;

	while (table[index].str[0])				//寻找空的Slot
	{
		if (is_same_string(str, table[index].str))
		{
			table[index].value++;
			return;
		}
		index++;
		if (index == HASH_SIZE)
			index = 0;
	}

	//没有找到，增加一项
	strcpy(table[index].str, str);
	table[index].value = 1;
}

//获得散列项
hash_node *hash_table::get_node(const char *str, int length)
{
	unsigned int key = get_key(str, length);
	int index = key % HASH_SIZE;
	hash_node *node = &table[index];

	while(node->str[0])
	{
		if (is_same_string(str, length, node->str))
			return node;
		node++;
	}
	return 0;
}

hash_node *hash_table::get_node(const char *str)
{
	unsigned int key = get_key(str);
	int index = key % HASH_SIZE;
	hash_node *node = &table[index];

	while(node->str[0])
	{
		if (is_same_string(str, node->str))
			return node;
		node++;
	}
	return 0;
}

unsigned int hash_table::get_rand_byte()
{
	return ((unsigned int) rand()) & 0xff;
}

//初始化字符key表
void hash_table::init_ascii_keys()
{
	srand(12345);
	for (int i = 0; i < 0x40; i++)
		for (int j = 0; j < 0x100; j++)
			ascii_keys[j][i] = 
				(get_rand_byte()) |
				(get_rand_byte() << 8) |
				(get_rand_byte() << 16) |
				(get_rand_byte() << 24);

	if (!ascii_keys[0][0])
		ascii_keys[0][0] = 1;
}

//找到字符串对应的值
int hash_table::get_value(const char *str)
{
	hash_node *node;

	node = get_node(str);
	if (!node)
		return 0;

	return node->value;
}

int hash_table::get_value(const char *str, int length)
{
	hash_node *node;

	node = get_node(str, length);
	if (!node)
		return 0;

	return node->value;
}

//判断是否为相同的字符串，比较串带有长度
int hash_table::is_same_string(const char *s1, int length, const char *s2)
{
	int i;

	for (i = 0; i < length && s1[i] && s2[i]; i += 2)
		if (*(short*)&s1[i] != *(short*)&s2[i])
			return 0;

	if (i != length || s2[i])
		return 0;

	return 1;
}

void hash_table::output(ofstream &out_file)
{
	for (int i = 0; i < HASH_SIZE; i++)
		if (table[i].str[0])
			out_file << table[i].str << "\t" << table[i].value << endl;
}

int hash_table::is_same_string(const char *s1, const char *s2)
{
	int i;

//	return !strcmp(s1, s2);

	for (i = 0; s1[i] && s2[i]; i += 2)
		if (*(short*)&s1[i] != *(short*)&s2[i])
			return 0;

	if (s1[i] || s2[i])
		return 0;
	return 1;
}

/*
unsigned int hash_table::get_key(const char *str)
{
	unsigned int seed1 = 0x7FED7FED, seed2 = 0xEEEEEEEE;

	for (int i = 0; str[i]; i++)
	{
		seed1 = ascii_keys[0][(unsigned char)str[i]] ^ (seed1 + seed2);
		seed2 = (unsigned char)str[i] + seed1 + seed2 + (seed2 << 5) + 3;
	}

	return seed1;
}

unsigned int hash_table::get_key(const char *str, int length)
{
	unsigned int seed1 = 0x7FED7FED, seed2 = 0xEEEEEEEE;

	for (int i = 0; i < length; i++)
	{
		seed1 = ascii_keys[0][(unsigned char)str[i]] ^ (seed1 + seed2);
		seed2 = (unsigned char)str[i] + seed1 + seed2 + (seed2 << 5) + 3;
	}
	return seed1;
}
*/

unsigned int hash_table::get_key(const char *str)
{
	unsigned int key = 0;
	for (int i = 0; str[i]; i++)
		key ^= ascii_keys[(unsigned char)str[i]][i];

	return key;
}

unsigned int hash_table::get_key(const char *str, int length)
{
	unsigned int key = 0;
	for (int i = 0; i < length; i++)
		key ^= ascii_keys[(unsigned char)str[i]][i];
	return key;
}
