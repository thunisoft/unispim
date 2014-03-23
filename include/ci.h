/*	词处理头文件
 */

#ifndef	_CI_H_
#define	_CI_H_

#ifdef __cplusplus
extern "C"
{
#endif

//词Cache定义，一般一个词占用5-7个字节
//256K空间，可以放置256000/5 ~= 51200条词汇，应该可以用了。
#define	CI_CACHE_SIZE				0x80000				//词cache的size
#define	CI_MOVE_SIZE				(CI_CACHE_SIZE / 4)	//Cache满后的移动幅度
#define	CI_MAX_USED_COUNT			0x700000			//词的最大使用度
#define CI_TOP_USED_COUNT			0x5FFFFF			//置顶词的使用度
#define	CI_CACHE_EXTRA_LENGTH		0x100				//词Cache的临界缓冲区长度
#define CI_CACHE_V66_SIGNATURE		0x20000919			//词缓冲区的标识（用于版本判断）
#define	CI_CACHE_SIGNATURE			0x20000918			//词缓冲区的标识（用于版本判断）
#define	CI_NEW_BUFFER_LENGTH		0x8000				//32K的新词缓冲区
#define	CI_NEW_EXTRA_LENGTH			0x100				//新词的缓冲区

//词Cache说明，
// B0			：词的长度
// B1 B2 B3		：词的使用度
// 我们....		：词条中文
typedef struct tagCICACHE
{
	int		signature;				//词缓冲区标识
	int		length;					//cache的使用长度
	int		max_used_id;			//词的最大使用次数，到达的时候，需要进行剪除操作
	char	cache[CI_CACHE_SIZE + CI_CACHE_EXTRA_LENGTH];		//词cache
}CICACHE;

//新词记录结构
//B0词长度，B1音节长度
//B2 B3...  词
//.....		音节
//typedef struct tagNEWCI
//{
//	int		length;
//	char	data[CI_NEW_BUFFER_LENGTH + CI_NEW_EXTRA_LENGTH];
//}NEWCI;

//#define	NEWCI_FILE_NAME				TEXT("unispim6\\wordlib\\new_ci.dat")
#define	CICACHE_FILE_NAME			TEXT("unispim6\\wordlib\\ci_cache.dat")

//获得用户词库的词候选
int GetUserCiCandidates(SYLLABLE *syllable_array, int syllable_count, CANDIDATE *candidate_array, int candidate_length, int fuzzy_mode);

//通过音节获得词候选
int GetCiCandidates(int wordlib_id, SYLLABLE *syllable_array, int syllable_count, CANDIDATE *candidate_array, int candidate_length, int fuzzy_mode);

//通过首字母获得词汇候选。
int GetCiCandidatesByLetters(int wordlib_id, const char *letters, int letters_count, CANDIDATE *candidate_array, int candidate_length);

//处理词汇被用户选中后操作。插入到Cache中，增加用户使用度。
extern void ProcessCiSelected(SYLLABLE *syllable, int syllable_length, HZ *hz, int hz_length);

//词候选排重。
extern int UnifyCiCandidates(CANDIDATE *candidates, int count);

//增加新词
extern int AddCi(SYLLABLE *syllable, int syllable_length, HZ *ci, int hz_length);

//词候选排序
extern void SortCiCandidates(CANDIDATE *candidates, int count);

//基于音节处理词的候选。
extern int ProcessCiCandidate(SYLLABLE *syllable_array, int syllable_count, const TCHAR *letters, CANDIDATE *candidate_array, int candidate_length, int same_ci_syllable_length);

//获取混合解析候选词
extern int GetMixedParseCiCandidate(SYLLABLE *syllable_array, int syllable_count, CANDIDATE *candidate_array, int candidate_length);

//获得词的cache信息
extern int GetCiCacheInfo(HZ *hz, int length);
extern int GetCiUsedCount(int cache_pos);
extern void InsertCiToCache(HZ *hz, int length, int syllable_length, int set_fix_top);

//装载、保存词Cache数据
extern int LoadCiCacheData(const TCHAR *ci_cache_file_name);
extern int SaveCiCacheData(const TCHAR *ci_cache_file_name);
extern int FreeCiCacheData();
//extern int LoadNewWordTable();
//extern int SaveNewWordTable();
//extern void FreeNewWordTable();

extern int UpgradeWordLibFromV5ToV6(const TCHAR *);
extern int CheckAndUpgradeWordLibFromV5BKToV6(const char *);
extern WORDLIBITEMV6B1 *GetNextCiItemV6B1(WORDLIBITEMV6B1 *item);
extern WORDLIBITEM *GetNextCiItem(WORDLIBITEM *item);
extern HZ *GetItemHZPtr(WORDLIBITEM *item);
extern int GetItemLength(int hz_length, int syllable_length);

//检查是否为新词，并加入到新词表中
//extern void CheckNewUserWord(SYLLABLE *syllables, int syllable_length, HZ *ci_string, int ci_length);
//extern void DeleteNewWord(HZ *ci_str, int ci_length, SYLLABLE *syllable, int syllable_length);
//extern int  __stdcall GetNewestUserWords(char *buffer, int buffer_length);
//extern void __stdcall DelNewestUserWords(const char *buffer);
extern int TestNewWordTable();

#ifdef __cplusplus
}
#endif

#endif