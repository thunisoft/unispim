#ifndef	_HASH_H_
#define	_HASH_H_

#include <stdlib.h>

/**	hash表节点
 */
class hash_node
{
	public:
		const char 		*str;					//本项的字符串
		int				value;					//本项的值
		hash_node		*next;					//同一个Slot中的下一项

		hash_node(const char *str, int value)
		{

			this->str = str;
			this->value = value;
			this->next = 0;
		}

		hash_node(const char *str)
		{
			hash_node(str, 0);
		}

};

/**	字符串hash表
 */
class hash_table
{
	public:
		unsigned int ascii_keys[0x100][0x10];			//字符散列key

		hash_node	**table;						//hash表项
		int			size;							//表中hash项数目

		~hash_table()
		{
			for (int i = 0; i < size; i++)
				if (table[i])
					delete table[i];

			delete table;
		}

		hash_table(int size)		
		{
			init_ascii_keys();					//初始化字符key表

			this->size = size;
			this->table = new hash_node* [size];	//初始化hash表项
		}

		//增加散列项
		void add(const char *str, int value)
		{
			hash_node *node = new hash_node(str, value);
			unsigned int key = get_key(str);
			int index = key % size;

			node->next = table[index];
			table[index] = node;
		}

		//获得散列项
		hash_node *get_node(const char *str, int length)
		{
			unsigned int key = get_key(str, length);
			int index = key % size;
			hash_node *node = table[index];

			while(node)
			{
				if (is_same_string(str, length, node->str))
					return node;
				node = node->next;
			}
			return 0;
		}

		hash_node *get_node(const char *str)
		{
			unsigned int key = get_key(str);
			int index = key % size;
			hash_node *node = table[index];

			while(node)
			{
				if (is_same_string(str, node->str))
					return node;
				node = node->next;
			}
			return 0;
		}

		//初始化字符key表
		void init_ascii_keys()
		{
			srand(12345);
			for (int i = 0; i < 0x10; i++)
				for (int j = 0; j < 0x100; j++)
					ascii_keys[j][i] = (unsigned int)
						((rand() & 0xff) | ((rand() & 0xff) << 8) | ((rand() & 0xff) << 16) | ((rand() % 0xff) << 24));
			if (!ascii_keys[0][0])
				ascii_keys[0][0] = 1;
		}

		//找到字符串对应的值
		int get_value(const char *str)
		{
			hash_node *node;

			node = get_node(str);
			if (!node)
				return 0;

			return node->value;
		}

		int get_value(const char *str, int length)
		{
			hash_node *node;

			node = get_node(str, length);
			if (!node)
				return 0;

			return node->value;
		}

		//判断是否为相同的字符串，比较串带有长度
		static int is_same_string(const char *s1, int length, const char *s2)
		{
			int i;

			for (i = 0; i < length && s1[i] && s2[i]; i++)
				if (s1[i] != s2[i])
					return 0;

			if (i != length || !s2[i])
				return 0;

			return 1;
		}

		static int is_same_string(const char *s1, const char *s2)
		{
			int i;

			for (i = 0; s1[i] && s2[i]; i++)
				if (s1[i] != s2[i])
					return 0;

			if (s1[i] || s2[i])
				return 0;
			return 1;
		}

		//获得字符串的key
		unsigned int get_key(const char *str)
		{
			unsigned int key = 0;
			for (int i = 0; str[i]; i++)
				key |= ascii_keys[str[i]][i % 0x10];

			return key;
		}

		unsigned int get_key(const char *str, int length)
		{
			unsigned int key = 0;
			for (int i = 0; i < length && str[i]; i++)
				key |= ascii_keys[str[i]][i % 0x10];
			return key;
		}
};

#endif
