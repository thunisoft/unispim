#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ncoc.h>

TCOCDATA *tcoc_data;
BCOCDATA *bcoc_data;

/**	获得BCOC
 */
int get_bcoc(const HZ *hz)
{
	int start, end, mid, start_sav, end_sav;
	HZ hz0, hz1;
	INDEXITEM *index0;
	NCOCITEM  *item;

	hz0 = hz[0], hz1 = hz[1];
	index0 = (INDEXITEM*)((char*)bcoc_data->data + bcoc_data->header.index0_data_pos);
	item = (NCOCITEM*)((char*)bcoc_data->data + bcoc_data->header.item_data_pos);

	//对第一级索引进行检索，最后一项为0
	start = 0, end = bcoc_data->header.index0_count - 2;
	while(start <= end)
	{
		mid = (start + end) / 2;
		if (index0[mid].hz == hz0)
			break;
		if (hz0 > index0[mid].hz)
			start = mid + 1;
		else
			end = mid - 1;
	}
	if (start > end)		//没有该项
		return 1;

	start_sav = start = index0[mid].item_pos;			//第二级索引开始/结束位置
	end_sav = end = index0[mid + 1].item_pos - 1;		//需要将两个区间保留, 用于重复项判断

	//对数据项进行检索(带有重复项的二分法检索)
	while(start <= end)
	{
		mid = (start + end) / 2;
		if (item[mid].hz == hz1)
			break;
		if (hz1 > item[mid].hz)
			start = mid + 1;
		else
			end = mid - 1;
	}
	if (start > end)			/* 没有该项 */
		return 1;

	//由于可能出现重复项, 所以要对两边的数据进行判断
	if (mid > start_sav && item[mid - 1].hz == hz1)	//前面与当前项相同
		return item[mid].count * 0x100 + item[mid - 1].count;
	if (mid < end_sav && item[mid + 1].hz == hz1)		//后面与当前项相同
		return item[mid + 1].count * 0x100 + item[mid].count;
	return item[mid].count;
}

/**	获得汉字串的TCOC
 */
int get_tcoc(const HZ *hz)
{
	int start, end, mid, start_sav, end_sav;
	HZ hz0, hz1, hz2;
	INDEXITEM *index0, *index1;
	NCOCITEM  *item;

	hz0 = hz[0], hz1 = hz[1], hz2 = hz[2];
	item = (NCOCITEM*)((char*)tcoc_data->data + tcoc_data->header.item_data_pos);
	index0 = (INDEXITEM*)((char*)tcoc_data->data + tcoc_data->header.index0_data_pos);
	index1 = (INDEXITEM*)((char*)tcoc_data->data + tcoc_data->header.index1_data_pos);

	//对第一级索引进行检索
	start = 0, end = tcoc_data->header.index0_count - 2;
	while(start <= end)
	{
		mid = (start + end) / 2;
		if (index0[mid].hz == hz0)
			break;
		if (index0[mid].hz < hz0)
			start = mid + 1;
		else
			end = mid - 1;
	}
	if (start > end)		//没有该项
		return 1;

	start = index0[mid].item_pos;		
	end   = index0[mid + 1].item_pos - 1;	//第二级索引开始/结束位置

	//对第二级索引进行检索
	while(start <= end)
	{
		mid = (start + end) / 2;
		if (index1[mid].hz == hz1)
			break;
		if (index1[mid].hz < hz1)
			start = mid + 1;
		else
			end = mid - 1;
	}
	if (start > end)		//没有该项
		return 1;

	start_sav = start = index1[mid].item_pos;			//数据项开始/结束位置
	end_sav = end = index1[mid + 1].item_pos - 1;

	//对数据项进行检索(带有重复项的二分法检索)
	while(start <= end)
	{
		mid = (start + end) / 2;
		if (item[mid].hz == hz2)
			break;
		if (item[mid].hz < hz2)
			start = mid + 1;
		else
			end = mid - 1;
	}
	if (start > end)			//没有该项
		return 1;

	//由于可能出现重复项, 所以要对两边的数据进行判断
	if (mid > start_sav && item[mid - 1].hz == hz2)	//前面与当前项相同
		return item[mid].count * 0x100 + item[mid - 1].count;
	if (mid < end_sav && item[mid + 1].hz == hz2)	//后面与当前项相同
		return item[mid + 1].count * 0x100 + item[mid].count;
	return item[mid].count;
}

/**	打开BCOC文件
 */
int open_bcoc(const char *name)
{
	FILE *f;

	bcoc_data = (BCOCDATA*)malloc(BCOC_SIZE);
	if (!bcoc_data)
	{
		fprintf(stderr, "内存不足.\n");
		return 0;
	}

	f = fopen(name, "rb");
	if (!f)
	{
		fprintf(stdout, "文件(%s)打开出错.\n");
		return 0;
	}
	fread(bcoc_data, 1, BCOC_SIZE, f);
	fclose(f);

	return 1;
}

/**	打开TCOC文件
 */
int open_tcoc(const char *name)
{
	FILE *f;

	tcoc_data = (TCOCDATA*)malloc(TCOC_SIZE);
	if (!tcoc_data)
	{
		fprintf(stderr, "内存不足.\n");
		return 0;
	}

	f = fopen(name, "rb");
	if (!f)
	{
		fprintf(stdout, "文件(%s)打开出错.\n", name);
		return 0;
	}
	fread(tcoc_data, 1, TCOC_SIZE, f);
	fclose(f);

	return 1;
}

