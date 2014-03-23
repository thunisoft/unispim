/* file iterator.h */

#ifndef	_FI_H_
#define	_FI_H_

#include <io.h>
#include <stdlib.h>
#include <direct.h>

#ifdef	unix
#define	CMB_CHAR			'/'
#else
#define	CMB_CHAR			'\\'
#endif

/* const */
#define	FI_ALLFILE			"*.*"

/* flag */
#define	FI_HIDEN			1
#define	FI_DIR				2
#define	FI_NORMAL			3

/* error return */
#define	FIE_NOFILE				0
#define	FIE_NOMEMORY			1
#define	FIE_NODIR				2
#define	FIE_TIMEOUT				3
#define	FIE_EXCEED_CONCURRENTS	4
#define	FIE_CHDIR_FAILED		5
#define	FIE_HANDLE_ILLEGAL		6
#define	FIE_FINDFIRST_FAILED	7
#define	FIE_FINDNEXT_FAILED		8

/* fi used constants */
#define	FI_MAX_CONCURRENTS	5		//最多5个并发
#define	FI_MAX_DEPTH		100		//最多100级目录
#define	FI_MAX_TRIES		1000	//不能得到互斥锁的最大尝试数目
#define	FI_MUTEX			"FI_MUTEX"

struct fi_data_t
{
	unsigned attrib; 		//File attribute. 
	
	time_t time_create; 	//Time of file creation 
							//( –1L for FAT file systems). 
	time_t time_access; 	//Time of last file access 
							//(–1L for FAT file systems). 
	time_t time_write; 		//Time of last write to file. 
	_fsize_t size;			//Length of file in bytes. 

	char *name;				//file name
	char *dir;				//full dir name
};

/* struct for using fi routines */
int fi_first(const char *dir, struct fi_data_t *);
int fi_next(int handle, struct fi_data_t *);
void fi_close(int handle);
 
#endif
