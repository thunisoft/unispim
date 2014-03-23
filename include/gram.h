/**	处理马可夫链式结构的头文件
 */

#ifndef	_GRAM_H_
#define	_GRAM_H_

#define	SJX_FREQ					(200000000)

#define	MAX_BCOUNT					(1.0e+11)

#define	BIGRAM_SIGN					0x20072222
#define	TRIGRAM_SIGN				0x20073333

#define	MAX_WORDS_BIT				18
#define	ONE_COUNT_BIT				14

#define	MAX_SEG_WORD_LENGTH			16						//分词词典中的最大长度
#define	MAX_SEG_WORDS				(1 << MAX_WORDS_BIT)	//词典中最多的词个数

#define	BIGRAM_WORD_LIST_SIZE		(1 << 22)				//词表预留空间
#define	BIGRAM_INDEX0_SIZE			(1 << MAX_WORDS_BIT)	//第一级索引个数
#define	BIGRAM_ITEM_SIZE			(1 << 25)				//Bigram项空间

#define	ENCODE(x)		((((x) >> 7) | ((x) << 9)) ^ 0xaa55)
#define	DECODE(x)		((((x) ^ 0xaa55) << 7) | (((x) ^ 0xaa55) << 7))

#define	XM				(0.618)
//#define	XM				(1.0)
#define	RT				(330.0)						//相信因子好像没有用处

typedef unsigned short HZ;
typedef unsigned short WORD;
typedef unsigned char  BYTE;

#pragma pack(1)

//索引项内容
typedef struct tagGRAM_INDEX			//Bigram, Trigram索引
{
	int		word_pos;					//在词表中的位置
	int		word_freq;					//该词词频
	int		item_index;					//项所处于的位置
	int		start_count;				//该项起头的数目
}GRAM_INDEX;

//项内容
typedef struct tagGRAM_ITEM				//Bigram，Trigram项
{
	unsigned int	word_index : MAX_WORDS_BIT;		//词在词表中的位置
	unsigned int	count : ONE_COUNT_BIT;			//出现计数，当数据不足以表达的时候，扩大到两个ITEM
}GRAM_ITEM;

//GRAM文件头部
typedef struct tagGRAM_HEADER
{
	int			sign;								//gram标识

	long long	total_word_freq;					//全部词汇的计数

	long long	total_bigram_count;					//全部bigram计数
	long long	total_bigram_in_count;				//进入模型的bigram计数总量，用于计算剩余概率

	int			end_token_count;					//结束符号计数（用于语句输入结尾的概率计算）

	int			word_list_size;						//词表大小

	int			index0_count;						//第一级索引数目
	int			index0_size;						//第一级索引大小

	int			index1_count;						//第二级索引数目
	int			index1_size;						//第二级索引大小

	int			item_count;							//数据项数目
	int			item_size;							//数据项空间大小

	int			word_list_pos;						//词表起始位置
	int			index0_data_pos;					//保留用于计算指针的空间
	int			index1_data_pos;					//保留用于计算指针的空间
	int			item_data_pos;						//保留用户计算指针的空间

	//计算过程，计算指针
	//index0_data = (GRAM_INDEX*)gram_data.index0_data;
	//index1_data = (GRAM_INDEX*)((char*)index0_data + sizeof(GRAM_INDEX) * index0_count);
	//item_data   = (GRAM_ITEM*) ((char*)index1_data + sizeof(GRAM_INDEX) * index2_count);

}GRAM_HEADER;

//gram文件结构
typedef struct tagGRAM_DATA
{
	GRAM_HEADER		header;										//Bigram/Trigram头	
	char			align_data[0x400 - sizeof(GRAM_HEADER)];	//对齐预留空间

//	char			data[];									//数据开始位置

//	GRAM_WORD_INDEX	word_index[GRAM_WORD_SIZE];					//词表项数目
//	char			words[BIGRAM_WORD_LIST_SIZE];				//词表内容	

//	GRAM_INDEX		index0[BIGRAM_INDEX0_SIZE];					//index0的存储区
//	GRAM_INDEX		index1[TRIGRAM_INDEX1_SIZE];				//index1的存储区
//	GRAM_ITEM		items[BIGRAM_ITEM_SIZE];					//Bigram数据，包括索引以及项数据
}GRAM_DATA;

#pragma pack()

#ifdef	__cplusplus
extern "C" 
{
#endif

int GetBigramCount(GRAM_DATA *bigram_data, const char *ci1, const char *ci2);
double GetBigramValue(GRAM_DATA *bigram_data, const char *ci1, const char *ci2);
double GetBackOffProbability(GRAM_DATA *bigram_data, int index1, int index2);

#define	GetGramWordList(bigram)	((char*)bigram + bigram->header.word_list_pos)
#define	GetGramIndex(bigram)	((GRAM_INDEX*) ((char*)bigram + bigram->header.index0_data_pos))
#define	GetGramItem(bigram)		((GRAM_ITEM*) ((char*)bigram + bigram->header.item_data_pos))

void encode_word_list(char *word_list, int word_list_pos);
void decode_word_list(char *word_list, int word_list_pos);

#ifdef	__cplusplus
}
#endif

#endif
