#ifndef	_MAP_FILE_H_
#define	_MAP_FILE_H_

#ifdef	__cplusplus
extern "C" {
#endif

#include <windows.h>

typedef struct tagFILEMAPDATA
{
	HANDLE		h_file;			//文件句柄
	HANDLE		h_map;			//映射句柄
	long long   length;			//文件的长度
	long long	offset;			//文件当前偏移
	int			granularity;	//映射颗粒度
	char		*view;			//当前的视图
}FILEMAPDATA, *FILEMAPHANDLE;

FILEMAPHANDLE FileMapOpen(const TCHAR *file_name);
int FileMapGetBuffer(FILEMAPHANDLE handle, char **buffer, int length);
int FileMapSetOffset(FILEMAPHANDLE handle, long long offset);
int FileMapClose(FILEMAPHANDLE handle);

#ifdef	__cplusplus
}
#endif

#endif