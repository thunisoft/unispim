/*	系统的资源管理。
 *		华宇拼音输入法安装目录设计
 *
 *
 *		输入法安装目录一共涉及四个目录：
 *		SYSTEM			-> windows\system32
 *		PROGRAM			-> program files\unispim6
 *		USERAPP			-> documents and setting\{user}\application data\unispim6
 *		ALLUSERAPP		-> documents and setting\all user\applicateion data\unispim6
 *
 *		输入法资源的目录以相对目录存放。
 *		如：c:\program files\unispim6\wordlib\sys.uwl存放为
 *		wordlib\sys.uwl
 *
 *		项目							目录					说明
 *		1.	主题						ALLUSERAPP\theme\		每一个主题一个目录
 *		2.	系统词库					ALLUSERAPP\wordlib\		系统词库：sys.uwl
 *		3.	外挂词库					ALLUSERAPP\wordlib\		每一个词库一个文件
 *		4.	用户词库					USERAPP\wordlib\		用户自造词库只有一个文件
 *		5.	双拼定义文件				USERAPP\ini\			双拼.ini
 *		6.	短语(符号)定义文件			USERAPP\ini\			短语.ini
 *		7.	置顶字定义文件				USERAPP\ini\			置顶字.ini
 *		8.	BCOC数据文件				ALLUSERAPP\wordlib\		bcoc.dat
 *		9.	汉字Cache数据文件			USERAPP\wordlib\		zi_cache.dat
 *		10.	词汇Cache数据文件			USERAPP\wordlib\		ci_cache.dat
 *		11.	可执行程序					PROGRAM\				设置程序、词库维护程序等
 *
 *		如上所述，每一种资源都在相应的目录中进行检索。
 *		特例：
 *		5/6/7三个INI文件，如果USERAPP中不存在，则在ALLUSERAPP中检索，
 *		这是考虑到用户可能并没有变更系统所带的默认设置。
 *
 *		如：
 *		main_bk.png定义为：				theme\{themename}\main_bk.png
 *		外挂词库“地名.uwl”定义为：	wordlib\地名.uwl
 *		双拼定义文件：					ini\双拼.ini
 */

#include <zi.h>
#include <ci.h>
#include <icw.h>
#include <spw.h>
#include <config.h>
#include <utility.h>
#include <wordlib.h>
#include <symbol.h>
#include <english.h>
//#include <url.h>
#include <fontcheck.h>
#include <gbk_map.h>
#include <share_segment.h>

//#pragma	data_seg(HYPIM_SHARED_SEGMENT)
//int		resource_loaded = 0;				//资源尚未装载
//#pragma data_seg()

/**	装载全部词库文件。
 *	其中，用户词库与系统词库为默认加载；外挂词库基于config中的设定进行加载
 *	返回：
 *		0：失败，一般为系统词库或者用户词库设置错误
 *		1：成功，但可能存在某些外挂词库错误的情况
 */
int LoadWordLibraryResource()
{
	return LoadAllWordLibraries();
}

/**	释放词库的资源
 */
int FreeWordLibraryResource()
{
	//保存用户词库文件
	SaveWordLibrary(GetUserWordLibId());

	//关闭所有词库文件
	CloseAllWordLibrary();

	return 1;
}

/**	装载用户自定义短语文件（短语.ini）
 *	首先在USERAPP目录中寻找，如果没有再在ALLUSERAPP目录中寻找
 */
int LoadSpwResource()
{
	if (!pim_config->use_special_word)
		return 0;

	return LoadAllSpwData();
}

int FreeSpwResource()
{
	return FreeSpwData();
}

//int WINAPI PIM_ReloadSPWResource()
//{
//	FreeSpwResource();
//	LoadSpwResource();
//
//	return 1;
//}

/**	装载置顶字定义文件
 */
int LoadTopZiResource()
{
	TCHAR name[MAX_PATH];

	if (!(pim_config->hz_option & HZ_USE_FIX_TOP))
		return 0;

	GetFileFullName(TYPE_USERAPP, TOPZI_FILE_NAME, name);
	if (LoadTopZiData(name))
		return 1;

	GetFileFullName(TYPE_ALLAPP, TOPZI_FILE_NAME, name);
	return LoadTopZiData(name);
}

/**	释放置顶字资源
 */
int FreeTopZiResource()
{
	return FreeTopZiData();
}

/**	装载汉字Cache数据
 */
int LoadZiCacheResource()
{
	TCHAR name[MAX_PATH];

	GetFileFullName(TYPE_USERAPP, ZICACHE_FILE_NAME, name);

	return LoadZiCacheData(name);
}

/*	释放汉字Cache数据
 */
int FreeZiCacheResource()
{
	TCHAR name[MAX_PATH];

	GetFileFullName(TYPE_USERAPP, ZICACHE_FILE_NAME, name);

	return FreeZiCacheData(name);
}

int LoadHZDataResource()
{
	TCHAR name[MAX_PATH];

	GetFileFullName(TYPE_USERAPP, HZDATA_FILE_NAME, name);
	if (LoadHZData(name))
		return 1;

	GetFileFullName(TYPE_ALLAPP, HZDATA_FILE_NAME, name);
	return LoadHZData(name);
}

int FreeHZDataResource()
{
	return FreeHZData();
}

/**	装载汉字Cache数据
 */
int SaveZiCacheResource()
{
	TCHAR name[MAX_PATH];

	GetFileFullName(TYPE_USERAPP, ZICACHE_FILE_NAME, name);

	return SaveZiCacheData(name);
}

/**	装载词Cache数据
 */
int LoadCiCacheResource()
{
	TCHAR name[MAX_PATH];

	GetFileFullName(TYPE_USERAPP, CICACHE_FILE_NAME, name);

	return LoadCiCacheData(name);
}

/**	保存词Cache数据
 */
int SaveCiCacheResource()
{
	TCHAR name[MAX_PATH];

	GetFileFullName(TYPE_USERAPP, CICACHE_FILE_NAME, name);

	return SaveCiCacheData(name);
}

/**	释放词Cache数据
 */
int FreeCiCacheResource()
{
	SaveCiCacheResource();
	return FreeCiCacheData();
}

/**	装载bigram资源
 */
int LoadBigramResource()
{
	TCHAR name[MAX_PATH];

	GetFileFullName(TYPE_ALLAPP, BIGRAM_FILE_NAME, name);

	return LoadBigramData(name);
}

/**	释放bigram资源
 */
int FreeBigramResource()
{
	return FreeBigramData();
}

/*	加载双拼资源
 */
int LoadSPResource()
{
	TCHAR name[MAX_PATH];

	GetFileFullName(TYPE_USERAPP, SP_INI_FILE_NAME, name);
	if (LoadSPIniData(name))
		return 1;

	GetFileFullName(TYPE_ALLAPP, SP_INI_FILE_NAME, name);
	return LoadSPIniData(name);
}

int FreeSPResource()
{
	FreeSPIniData();
	return 1;
}

int FreeSymbolResource()
{
	FreeSymbolData();
	return 1;
}

/**	装载中文符号INI文件
 */
int LoadSymbolResource()
{
	TCHAR name[MAX_PATH];

	GetFileFullName(TYPE_USERAPP, SYMBOL_INI_FILE_NAME, name);
	if (LoadSymbolData(name))
		return 1;

	GetFileFullName(TYPE_ALLAPP, SYMBOL_INI_FILE_NAME, name);
	return LoadSymbolData(name);
}

/**	装载url data文件
 */
/*int LoadURLResource()
{
	TCHAR name[MAX_PATH];

	if (!pim_config->use_url_hint)
		return 0;

	GetFileFullName(TYPE_USERAPP, URL_FILE_NAME, name);
	if(LoadURLData(name))
		return 1;

	GetFileFullName(TYPE_ALLAPP, URL_FILE_NAME, name);
	return LoadURLData(name);
}

int FreeURLResource()
{
	FreeURLData();
	return 1;
}*/

/**	装载font map data文件
 */
int LoadFontMapResource()
{
	TCHAR name[MAX_PATH];

	GetFileFullName(TYPE_USERAPP, FONTMAP_FILE_NAME, name);

	return LoadFontMapData(name);
}

int FreeFontMapResource()
{
	FreeFontMapData();
	return 1;
}

/**	装载gbk map data文件
 */
int GenGBKMapResource()
{
	return GenGBKMapData();
}

int FreeGBKMapResource()
{
	FreeGBKMapData();
	return 1;
}

/**	装载英文词典文件
 */
int LoadEnglishResource()
{
	TCHAR name[MAX_PATH];

	if (!pim_config->use_english_input)
		return 0;

	GetFileFullName(TYPE_ALLAPP, ENGLISH_LIB_FILE_NAME, name);

	return LoadEnglishData(name);
}

int FreeEnglishResource()
{
	FreeEnglishData();

	return 1;
}

int PIM_ReloadEnglishResource()
{
	FreeEnglishResource();
	LoadEnglishResource();

	return 1;
}

//装载英文翻译数据文件
int LoadEnglishTransResource()
{
	TCHAR name[MAX_PATH];

	if (!pim_config->use_english_input || !pim_config->use_english_translate)
		return 0;

	GetFileFullName(TYPE_ALLAPP, ENGLISH_TRANS_FILE_NAME, name);

	return LoadEnglishTransData(name);
}

int FreeEnglishTransResource()
{
	FreeEnglishTransData();

	return 1;
}

int PIM_ReloadEnglishTransResource()
{
	FreeEnglishTransResource();
	LoadEnglishTransResource();

	return 1;
}

int PIM_ReloadEnglishAllResource()
{
	FreeEnglishResource();
	LoadEnglishResource();

	FreeEnglishTransResource();
	LoadEnglishTransResource();

	return 1;
}

/** 装载笔划数据文件
*/
int LoadBHResource()
{
	TCHAR name[MAX_PATH];

	GetFileFullName(TYPE_USERAPP, HZBH_FILE_NAME, name);
	if(LoadBHData(name))
		return 1;

	GetFileFullName(TYPE_ALLAPP, HZBH_FILE_NAME, name);
	return LoadBHData(name);
}

int FreeBHResource()
{
	FreeBHData();
	return 1;
}

int WINAPI PIM_ReloadBHResource()
{
	FreeBHResource();
	LoadBHResource();
	return 1;
}

/**	释放输入法资源
 */
int PIM_FreeResources()
{
	if (!share_segment->resource_loaded)
		return 1;

	share_segment->resource_loaded = 0;

	FreeWordLibraryResource();
	FreeSpwResource();
	FreeTopZiResource();
	FreeZiCacheResource();
	FreeCiCacheResource();
	FreeBigramResource();
	FreeSymbolResource();
	//FreeNewWordTable();
	FreeEnglishResource();
	FreeEnglishTransResource();
//	FreeURLResource();
	FreeBHResource();

	return 1;
}

/**	保存当前的数据资源
 */
int PIM_SaveResources()
{
	SaveWordLibrary(GetUserWordLibId());
	SaveZiCacheResource();
	SaveCiCacheResource();
	//SaveNewWordTable();

	return 1;
}

/**	装载输入法使用的资源，如果失败，则输入法不起作用
 *	返回：
 *		成功：1，失败：0
 */
int PIM_LoadResources()
{
	LoadWordLibraryResource();			//词库
	LoadBigramResource();				//Bigram数据

	if (share_segment->resource_loaded)
		return 1;

	share_segment->resource_loaded = 1;

	LoadSpwResource();					//短语
	LoadTopZiResource();				//置顶字
	LoadZiCacheResource();				//汉字Cache
	LoadHZDataResource();				//装载汉字数据
	LoadCiCacheResource();				//词Cache
	//LoadNewWordTable();					//新词表
	LoadSPResource();					//装载双拼资源
	LoadSymbolResource();				//装载中文符号资源
	LoadFontMapResource();              //load font map
	GenGBKMapResource();                //load gbk map
//	LoadEnglishResource();				//装载英文词典数据
//	LoadEnglishTransResource();			//装载英文翻译数据
//	LoadURLResource();				    //装载URL数据
//	LoadBHResource();					//装载笔划数据文件

	return 1;
}

int PIM_ReloadResources()
{
	PIM_FreeResources();
	PIM_LoadResources();

	return 1;
}

/**	重新装载INI相关数据，包括：置顶字、双拼、符号、短语
 */
int PIM_ReloadINIResource()
{
	FreeSpwResource();
	FreeTopZiResource();
	FreeSymbolResource();
	FreeSPResource();

	LoadSpwResource();		//短语定制
	LoadSPResource();		//双拼资源
	LoadTopZiResource();	//置顶字
	LoadSymbolResource();	//中文符号

	return 1;
}

/**	Reload Wordlib resource
 */
int PIM_ReloadWordlibResource()
{
	LoadWordLibraryResource();			//词库

	return 1;
}

int PIM_ForceReloadWordlibResource()
{
	CloseAllWordLibrary();
	LoadWordLibraryResource();			//词库

	return 1;
}

/**	Reload Zi resource
 */
int WINAPI PIM_ReloadZiResource()
{
	FreeHZDataResource();
	LoadHZDataResource();

	return 1;
}

/*int WINAPI PIM_ReloadURLResource()
{
	FreeURLResource();
	LoadURLResource();

	return 1;
}*/

int WINAPI PIM_ReloadFontMapResource()
{
	FreeFontMapResource();
	LoadFontMapResource();

	return 1;
}
