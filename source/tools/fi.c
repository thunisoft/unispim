/*	file iterator
 *	文件系统遍历器. 带有子目录的搜索, 返回
 *	每一个文件, 并附带有属性(与findfirst基本
 *	相同).
 *	注: 目前不是thread safe版本.
 *	WangChuan, 2004-04-12.
 *	使用:
 *		1.定义fi_data_t 结构变量.
 *		2.调用fi_first(dir, fi_data_t*), 返回值为本次操作
 *		  的handle.
 *		3.调用fi_next(handle, fi_data_t*);
 *		4.全部结束后, 调用fi_close(handle);
 */
#define		_CRT_SECURE_NO_DEPRECATE
#include "fi.h"
#include <string.h>

#define	FI_STAGE_START		0	//起始状态
#define	FI_STAGE_ENTER		1	//进入目录
#define	FI_STAGE_SEEK_BEGIN	2	//查找文件, 需要初始化
#define	FI_STAGE_SEEKING	3	//查找文件, 无需初始化
#define	FI_STAGE_SEEK_END	5	//退出目录
#define	FI_STAGE_JUDGE_FILE	4	//匹配文件
#define	FI_STAGE_RETURN		6	//返回结果
#define	FI_STAGE_END		7	//结束状态

static int fi_error;

struct fi_data
{
	int stage;					//处理阶段
	int fd_handle[FI_MAX_DEPTH];	//findfirst/findnext使用的句柄/
	int depth;					//目录深度
	char *cur_dir;				//当前目录
	char *next_dir;				//下一个目录
	char *name;					//当前文件名称
};

/* construct inner fi data, check flags, do first find 
 *	input:
 *		dir		-> starting directory, 0->current dir
 *		p_fd	-> a fi_data_t struct, used for returning
 *					file information.
 *	output:
 *		n		-> handle of inner data structure
 *		0		-> failed.
 */
int fi_first(const char *dir, struct fi_data_t *p_fd)
{
	char cwd[_MAX_PATH];
	struct fi_data *p_fi;

	p_fi = (struct fi_data*)malloc(sizeof(struct fi_data));
	if (!p_fi)
	{
		fi_error = FIE_NOMEMORY;
		return 0;
	}
	memset(p_fi, 0, sizeof(*p_fi));

	if (!dir)			//current dir
		dir = _getcwd(cwd, sizeof(cwd));
	p_fi->depth = 0;
	p_fi->stage = FI_STAGE_START;
	p_fi->cur_dir = _strdup(dir);
	p_fi->next_dir = _strdup(dir);

	//如果内存不够，或者当前目录出错，本次操作出错返回。
	if (!p_fi->cur_dir || !p_fi->next_dir)
	{
		fi_error = FIE_NOMEMORY;
		fi_close((int)p_fi);
		return 0;
	}

	if (-1 == fi_next((int)p_fi, p_fd))
	{	//fi_error 在fi_next中被赋予
		fi_close((int)p_fi);
		return 0;
	}

	return (int)p_fi;
}

/* 把临时变量中的数据复制到用户传递过来的参数中,并把当前结构p_fi中
 * 的file更新.
 *	Input:
 *		p_fi	->系统数据结构
 *		p_fd	->用户返回数据
 *		p_file	->临时数据区
 */
void fi_make_file(struct fi_data *p_fi, struct fi_data_t *p_fd, struct _finddata_t *p_file)
{
	int len;

	free(p_fi->name);
	p_fi->name        = _strdup(p_file->name);
	p_fd->attrib      = p_file->attrib;
	p_fd->time_create = p_file->time_create;
	p_fd->time_access = p_file->time_access;
	p_fd->time_write  = p_file->time_write;
	p_fd->size        = p_file->size;
	p_fd->dir         = p_fi->cur_dir;
	p_fd->name        = p_fi->name;

	//最后一个是\则删除
	if (len = strlen(p_fd->dir))
		if (p_fd->dir[len - 1] == '\\')
			p_fd->dir[len - 1] = 0;
}

/* 进入到一级子目录
 *	input:
 *		p_fi	->系统使用的数据结构
 *	output:
 *		-1		->失败
 *		0		->成功
 */
int fi_into_dir(struct fi_data *p_fi)
{
	if (p_fi->depth >= FI_MAX_DEPTH) //超出最大深度后, 不进入新的目录
		return -1;

	if (!_chdir(p_fi->next_dir))	 	//目录进入,深度+1
	{
		char cwd[_MAX_PATH];
		p_fi->depth++;
		free(p_fi->cur_dir);
		p_fi->cur_dir = _strdup(_getcwd(cwd, sizeof(cwd)));
		free(p_fi->next_dir);
		p_fi->next_dir = 0;
		return 0;
	}
	//没能够进入目录, 返回
	return -1;
}
	
/* search next file
 *	input:
 *		handle	-> fi handle which returned by fi_first
 *		p_ifi		-> fi_data_t struct point
 *	output:
 *		-1		-> error occurs, or end of search
 *		0		-> ok.
 */
int fi_next(int handle, struct fi_data_t *p_fd)
{
	char cwd[_MAX_PATH];
	struct fi_data *p_fi;
	struct _finddata_t c_file;

	p_fi = (struct fi_data *)handle;
	if (!(p_fi && p_fi->cur_dir && p_fd))
	{
		fi_error = FIE_HANDLE_ILLEGAL;
		return -1;
	}

	//第二次进入,需要设定当前目录
	if (p_fi->stage != FI_STAGE_START && -1 == _chdir(p_fi->cur_dir))
	{	//系统目录无法进入,肯定出问题了.
		fi_error = FIE_CHDIR_FAILED;
		return -1;
	}

	while(1)
	{
		switch(p_fi->stage)
		{
		case FI_STAGE_START:		//进行第一次的初始化
			if (-1 == _chdir(p_fi->cur_dir))
			{	//系统目录无法进入,肯定出问题了.
				fi_error = FIE_CHDIR_FAILED;
				return -1;
			}
			free(p_fi->cur_dir);
			p_fi->cur_dir = _strdup(_getcwd(cwd, sizeof(cwd)));
			if (!p_fi->cur_dir)
			{
				fi_error = FIE_NOMEMORY;
				return -1;
			}
			p_fi->stage = FI_STAGE_SEEK_BEGIN;
			break;

		case FI_STAGE_ENTER:
			//进入目录
			if (!fi_into_dir(p_fi))
				p_fi->stage = FI_STAGE_SEEK_BEGIN;
			else
				p_fi->stage = FI_STAGE_SEEKING;
			break;

		case FI_STAGE_SEEK_BEGIN:
			//本目录还没有进行过Seek, findfirst需要被调用
			p_fi->fd_handle[p_fi->depth] = _findfirst(FI_ALLFILE, &c_file);
			if (-1 == p_fi->fd_handle[p_fi->depth])
			{
				fi_error = FIE_NOMEMORY;
				return -1;
			}
			p_fi->stage = FI_STAGE_JUDGE_FILE;
			break;

		case FI_STAGE_SEEKING:		//初始化已经完毕可以进行下一次的检索
			//没有文件了,退回一级目录
			if (-1 == _findnext(p_fi->fd_handle[p_fi->depth], &c_file)) 
				p_fi->stage = FI_STAGE_SEEK_END;
			else
				p_fi->stage = FI_STAGE_JUDGE_FILE;
			break;

		case FI_STAGE_JUDGE_FILE:
			if(!strcmp(".", c_file.name) || !strcmp("..", c_file.name))
			{	//ignore . and ..
				p_fi->stage = FI_STAGE_SEEKING;
				break;
			}
			if (c_file.attrib & _A_SUBDIR)
			{	//找到一个子目录, 设定进入条件
				free(p_fi->next_dir);
				p_fi->next_dir = _strdup(c_file.name);
				p_fi->stage = FI_STAGE_ENTER;
			}
			else 	//普通文件则继续检索
				p_fi->stage = FI_STAGE_SEEKING;

			fi_make_file(p_fi, p_fd, &c_file);
			return 0;

		case FI_STAGE_SEEK_END:
			//退回一级目录，释放filefind内存
			_findclose(p_fi->fd_handle[p_fi->depth]);
			if (!p_fi->depth)		//全部检索完毕.
			{
				fi_error = FIE_NOFILE;
				return -1;
			}
			if (-1 == _chdir(".."))
			{
				fi_error = FIE_CHDIR_FAILED;
				return -1;
			}
			free(p_fi->cur_dir);
			p_fi->cur_dir = _strdup(_getcwd(cwd, sizeof(cwd)));
			if (!p_fi->cur_dir)
			{
				p_fi->stage = FI_STAGE_END;
				fi_error = FIE_NOMEMORY;
				return -1;
			}
			p_fi->depth--;
			p_fi->stage = FI_STAGE_SEEKING;
			break;
		case FI_STAGE_END:
			return -1;
		}
	}
}

/* close file iterator, release memory.
 *	input:
 *		handle	-> file iterator handle
 *	output:
 *		none
 */
void fi_close(int handle)
{
	struct fi_data *pfi = (struct fi_data*) handle;
	free(pfi->name);
	free(pfi->cur_dir);
	free(pfi);
}

