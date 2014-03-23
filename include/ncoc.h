/* tcoc_lib.h
   三元重现概率库构建头文件
*/

#ifndef	_NCOC_LIB_H_
#define	_NCOC_LIB_H_

typedef unsigned short HZ;
typedef unsigned short WORD;
typedef unsigned char  BYTE;

#define	BCOC_SIGN		0x2c0c1999
#define	BCOC_NAME		"bcoc.dat"
#define	TCOC_SIGN		0x3c0c1999
#define	TCOC_NAME		"tcoc.dat"
#define	BCOC_SIZE		0x400000
#define	TCOC_SIZE		0x2000000
#define	END				0
#define IsGB(hz) 		(((hz & 0xff) >= 0xb0) && ((hz & 0xff) <= 0xf7) && ((hz >> 8) >= 0xa1) && ((hz >> 8) <= 0xfe))
#define	HZ				unsigned short
#define	TERMINAL		(*(HZ*)"△")

#pragma pack(1)

//索引项内容
typedef struct 
{
	HZ		hz;
	int		item_pos;
}INDEXITEM;

//NCOC项内容
typedef struct
{
	HZ		hz;									//汉字
	BYTE	count;								//数值，用字节来表达（压缩方法）
}NCOCITEM;

//NCOC文件头部
typedef struct tagTCOCHEADER
{
	int			sign;								//tcoc标识
	int			index0_count;						//第一级索引数目
	int			index1_count;						//第二级索引数目
	int			item_count;							//bcoc/tcoc)项数目
	int			index0_data_pos;					//保留用于计算指针的空间
	int			index1_data_pos;					//保留用于计算指针的空间
	int			item_data_pos;						//保留用户计算指针的空间

	//计算过程
	//index0_data = (INDEXITEM*)tcoc_data.index0_data;
	//index1_data = (INDEXITEM*)((char*)index0_data + sizeof(INDEXITEM) * index0_count);
	//item_data   = (NCOCITEM*)((char*)index1_data + sizeof(INDEXITEM) * index1_count);

}TCOCHEADER;

//BCOC文件头部
typedef struct tagBCOCHEADER
{
	int			sign;								//bcoc标识
	int			index0_count;						//第一级索引数目
	int			item_count;							//bcoc/tcoc)项数目
	int			index0_data_pos;					//保留用于计算指针的空间
	int			item_data_pos;						//保留用户计算指针的空间

	//计算过程
	//index0_data = (INDEXITEM*)bcoc_data.index0_data;
	//item_data   = (BCOCITEM*)((char*)index1_data + sizeof(INDEXITEM) * index0_count);

}BCOCHEADER;

//TCOC文件结构
typedef struct tag_TCOCDATA
{
	//保留1K的数据区域
	union
	{
		TCOCHEADER header;
		char align_data[0x400];					//1K对齐数据
	};
	char	data[0x10];							//TCOC数据开始位置
}TCOCDATA;

//BCOC文件结构
typedef struct tag_BCOCDATA
{
	//保留1K的数据区域
	union
	{
		BCOCHEADER header;
		char align_data[0x400];					//1K对齐数据
	};
	char	data[0x10];							//BCOC数据开始位置
}BCOCDATA;

#pragma pack()

int get_bcoc(const HZ *hz);
int get_tcoc(const HZ *hz);

int open_bcoc(const char *name);
int open_tcoc(const char *name);

#endif
