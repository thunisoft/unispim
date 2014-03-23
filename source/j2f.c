/*	简体到繁体的转换模块
 */
#include <zi.h>
#include <wordlib.h>
#include <utility.h>
#include <tchar.h>
#include <share_segment.h>

#define	JF_MAXSTRINGLEN			40		/* 词最大长度 */
#define	JF_SLOTMAXITEMS			10		/* 一个SLOT中最多10个冲突 */
#define	JF_HASHMULT				41		/* 散列乘数 */
#define	JF_HASHSLOTS			16384	/* 散列项数 */
#define	JF_BASEADDRESS			(sizeof(int) * JF_HASHSLOTS)	/* 跳过index的起始地址 */

#define	JF_OK					0		/* 成功 */
#define	JF_OPENSRCFILEFAIL		1		/* 源文件不能打开 */
#define	JF_OPENDSTFILEFAIL		2		/* 目标文件不能打开 */
#define	JF_WRITEDSTFILEFAIL		3		/* 目标文件写错误 */
#define	JF_SRCFILEFORMATWRONG	4		/* 源文件读错误 */
#define	JF_NOMEMORY				5		/* 内存不足 */

static JFITEM *jf_info		= 0;
static TCHAR *jf_share_name = TEXT("HYPIM_JF_SHARED_NAME");
static jf_count  = 0;
static jf_loaded = 0;

//#pragma data_seg(HYPIM_SHARED_SEGMENT)
//static int	jf_loaded	= 0;			//简繁对照表是否已经在内存中
//static int	jf_count	= 0;			//简繁对照项数目
//#pragma	data_seg()

int LoadJFResource()
{
	TCHAR file_name[MAX_PATH];
	int file_length;

	if (share_segment && share_segment->jf_loaded)
		return 1;

	GetFileFullName(TYPE_ALLAPP, HZJF_FILE_NAME, file_name);

	file_length = GetFileLength(file_name);
	if (file_length <= 0)
		return 0;

	jf_info = AllocateSharedMemory(jf_share_name, file_length);
	if (!jf_info)
		return 0;

	if ((file_length = LoadFromFile(file_name, jf_info, file_length)) == -1)
	{
		FreeSharedMemory(jf_share_name, jf_info);
		Log(LOG_ID, L"简繁对照表文件打开失败。name=%s", file_name);
		return 0;
	}

	if (!file_length)
		return 0;

	if (share_segment)
	{
		share_segment->jf_count  = file_length / sizeof(JFITEM);
		share_segment->jf_loaded = 1;
	}
	else
	{
		jf_count  = file_length / sizeof(JFITEM);
		jf_loaded = 1;
	}

	return 1;
}

/**	释放j2f文件
 */
int FreeJFResource()
{
	if (share_segment)
		share_segment->jf_loaded = 0;
	else
		jf_loaded = 0;

	if (jf_info)
	{
		FreeSharedMemory(jf_share_name, jf_info);
		jf_info = 0;
	}

	return 1;
}

//reload j2f resource
int ReloadJFResource()
{
	LoadJFResource();
	FreeJFResource();
	return 1;
}

/**	单字简繁转换
 */
static UC ZiJ2F(UC zi)
{
	int i;

	if (share_segment)
	{
		if (!share_segment->jf_loaded)
			LoadJFResource();
	}
	else
	{
		if (!jf_loaded)
			LoadJFResource();
	}

	if (!jf_info)
	{
		jf_info = GetReadOnlySharedMemory(jf_share_name);

		//可能存在其他进程已经装载了，但是退出后共享内存被释放的问题
		if (!jf_info && 
			((share_segment && share_segment->jf_loaded) || (!share_segment && jf_loaded)))
		{
			if (share_segment)
				share_segment->jf_loaded = 0;
			else
				jf_loaded = 0;

			LoadJFResource();
		}
	}

	if (!jf_info)
		return zi;

	if (share_segment)
		jf_count = share_segment->jf_count;

	for (i = 0; i < jf_count; i++)
	{
		JFITEM *p = (JFITEM*)((INT_PTR)jf_info + i * sizeof(JFITEM));

		if (p->JianTi == zi)
			return p->FanTi;
	}

	return zi;
}

/*	汉字数组转换
 */
void StringJ2F(TCHAR *zi_string)
{
	int i, len = (int)_tcslen(zi_string);

	for (i = 0; i < len; i++)
		zi_string[i] = ZiJ2F(zi_string[i]);
}


//获得汉字词串的哈希散列Key. 注意: 在程序中不判断字符串的长度
static int GetHashKey(const TCHAR *str)
{
	unsigned int key = 0;

	while (*str)
		key = key * JF_HASHMULT + (TCHAR) *str++;

	return (int)(key % JF_HASHSLOTS);
}

//词简繁转换函数
static int ProcessWordJ2F(TCHAR *zi_word)
{
	FILE *fr;
	TCHAR filename[MAX_PATH];
	int  key, offset[2] = {0, 0}, len, ls;
	TCHAR buffer[0x201];		// 513字节
	TCHAR *p, *pb = buffer;

	GetFileFullName(TYPE_ALLAPP, J2F_FILE_NAME, filename);

	fr = _tfopen(filename, TEXT("rb"));
    if (!fr)
		return FALSE;

	//哈希值
	key = GetHashKey(zi_word);
	fseek(fr, key * sizeof(int), SEEK_SET);

	if (sizeof(offset) != fread(offset, 1, sizeof(offset), fr))
	{
		int err = GetLastError();
		fclose(fr);
		return 0;
	}

	len = sizeof(buffer) - 1;

	//len是当前slot的大小(哈希值相同的词在同一个slot中)
	if ((key != JF_HASHSLOTS) && (len > offset[1] - offset[0]))
		len = offset[1] - offset[0];

	//buffer里的数据具有"标准,標凖,朝着,朝著..."这样的排列形式
	fseek(fr, JF_BASEADDRESS + offset[0], SEEK_SET);
	len = (int)fread(buffer, 1, len, fr);
	fclose(fr);

	if (-1 == len)		// 出错
		return 0;

	buffer[len] = '\0';
	ls = (int)_tcslen(zi_word);

	while(p = _tcsstr(pb, zi_word))
	{
		// 查到的词是的完整性判断, 避免映射到同一Slot中的词中包含这个词.
		if ((p != pb && *(p - 1) != (UC)'.') || *(p + ls) != (UC)',')
		{
			pb = p + ls;
			continue;
		}

		p += ls + 1;

		while (*zi_word)		// 复制
			*zi_word++ = *p++;

		return 1;
	}

	return 0;
}

/*	处理词从简体到繁体的转换
 */
void WordJ2F(TCHAR *zi_word)
{
	if (!ProcessWordJ2F(zi_word))
		StringJ2F(zi_word);
}