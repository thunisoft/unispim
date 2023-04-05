#include "wordlibapi.h"
#include "utils.h"
#include "wordlibapi.h"
#include "wordlibutils.h"
#include "config.h"

#include <malloc.h>
#include <io.h>
#include <fcntl.h>
#include <iostream>
#include <tchar.h>
#include <winnt.h>
#include <fstream>
#include <minwindef.h>
#include <malloc.h>
#include <assert.h>
#include <fstream>

#include "share_segment.h"

static int wordlib_create_extra_length = 0x1000000;

/*	装载词库文件。模块内部使用。
 *	词库文件将装载到内存的共享区。对于同一个用户，所有的应用程序共享相同的
 *	词库（包括多个词库）。
 *	参数：
 *		lib_name			词库的完整文件名字（包含路径）
 *		extra_length		词库所需要的扩展数据长度
 *		check_exist			检查是否已经存在（用于词库的更新）
 *	返回值：
 *		成功：词库序号 >= 0
 *		失败：-1
 */
int LoadWordLibraryWithExtraLength(const TCHAR *lib_name, int extra_length, int check_exist, FILE* fr = NULL)
{
    Q_UNUSED(check_exist);
    int  length,length1;				//词库长度
    char *buffer;						//词库的指针
    int  error = 0;						//是否出错
    WORDLIB *wl ;						//词库指针

    error = 1;                          //默认出错

    //内存中没有词库数据，需要进行装载
    length = GetFileLength(lib_name);				//获得词库的长度
    if (length <= 0)									//文件不存在？
    {
        if(fr)
        {
            _ftprintf(fr, TEXT("获取文件长度失败!"));
        }
       return -1;
    }
    //临时解决系统词库清零问题，固定系统词库内存为2M+1M，不足2M的直接申请2M，外挂1M作为扩展（不断的增加单词）
    //大概能保存20W左右的数据
    if(!_tcscmp(lib_name + _tcslen(lib_name) - 8,TEXT("user.uwl")) && length < 0x200000)
        length1 = 0x200000;
    else
        length1 = length;

    //分配共享内存
    buffer = (char*)malloc(length1 + extra_length);
    if (!buffer)		//分配失败
    {
        if(fr)
        {
            _ftprintf(fr, TEXT("分配内存失败!"));
        }
        return -1;
    }

    if (!LoadFromFile(lib_name, buffer, length))
    {	delete buffer;
        if(fr)
        {
            _ftprintf(fr, TEXT("读取文件失败!"));
        }
        return -1;
    }

    wl = (WORDLIB*)buffer;
    if (wl->header.signature != HYPIM_WORDLIB_V66_SIGNATURE)
    {
        if(fr)
        {
            _ftprintf(fr, TEXT("文件不是v6版本!"));
        }
        return -1;
    }

    wordlib_buffer[0]	= wl;

    return 0;
}


/*	获得词汇候选。
 *	参数：
 *		wordlib_id				词库标识
 *		syllable_array			音节序列
 *		syllable_count			音节长度
 *		candidate_array			候选数组
 *		candidate_length		候选数组长度
 *		fuzzy_mode				模糊音选项
 *	返回：
 *		候选词汇数目
 */
int GetCiCandidates(int wordlib_id, SYLLABLE *syllable_array, int syllable_count, CANDIDATE *candidate_array, int candidate_length, int fuzzy_mode)
{
    WORDLIB *wordlib;			//词库指针
    int page_no;				//页号
    int count = 0;				//候选计数
    int i, j;
    SYLLABLE syllable_tmp1, syllable_tmp2;
    int wild_compare = 0;								//是否使用通配比较

    if (!candidate_length || wordlib_id < 0)
        return 0;

    //音节过少或者过大
    if (syllable_count < 2 || syllable_count > MAX_WORD_LENGTH)
        return 0;

    wordlib = wordlib_buffer[0];
    if (!wordlib)				//没有这个词库
        return 0;

    //查看是否需要进行通配比较
    for (i = 0; i < syllable_count; i++)
        if (syllable_array[i].con == CON_ANY)		//带有通配符
            break;

    wild_compare = i != syllable_count;

    //用于比较的临时音节初始化
    syllable_tmp1.con  = syllable_tmp2.con  = CON_NULL;
    syllable_tmp1.vow  = syllable_tmp2.vow  = VOW_NULL;
    syllable_tmp1.tone = syllable_tmp2.tone = TONE_0;

    //由于有模糊音的存在，所以必须进行声母的遍历才能找出全部正确的声母。
    for (i = CON_NULL; i < CON_END; i++)
    {
        syllable_tmp1.con = i;
        if (!ContainCon(syllable_array[0], syllable_tmp1, fuzzy_mode))
            continue;

        //找到第一个相同的音节
        for (j = CON_NULL; j < CON_END; j++)
        {
            syllable_tmp2.con = j;
            if (!ContainCon(syllable_array[1], syllable_tmp2, fuzzy_mode))
                continue;

            //音节序列的词库页索引
            page_no = wordlib->header.index[i][j];

            //遍历页表找出词候选
            while(page_no != PAGE_END)
            {
                count += wild_compare
                    ?
                        GetCiInPageWild(			//使用通配符
                            &wordlib->pages[page_no],
                            syllable_array,
                            syllable_count,
                            candidate_array + count,
                            candidate_length - count,
                            fuzzy_mode)
                    :
                        GetCiInPage(				//不使用通配符
                            &wordlib->pages[page_no],
                            syllable_array,
                            syllable_count,
                            candidate_array + count,
                            candidate_length - count,
                            fuzzy_mode);

                if (count >= candidate_length)
                    break;

                page_no = wordlib->pages[page_no].next_page_no;
            }

            if (count >= candidate_length)
                break;
        }
    }

    //设置词库标识
    for (i = 0; i < count; i++)
    {
        candidate_array[i].word.source = wordlib_id;
        candidate_array[i].type		   = CAND_TYPE_CI;
    }

    return count;
}

/*	判断词是否已经在词库中
 *	参数：
 *	返回：
 *		在词库中：指向词条的指针
 *		不在：0
 */
WORDLIBITEM *GetCiInWordLibrary(int wordlib_id, HZ *hz, int hz_length, SYLLABLE *syllable, int syllable_length)
{
    int count = 0, i= 0;

    CANDIDATE	*candidates = NULL;
    WORDLIBITEM	*item = 0;
    int candiateSize = sizeof(CANDIDATE);

    candidates = new CANDIDATE[candiateSize * MAX_CANDIDATES];


    count = GetCiCandidates(wordlib_id, syllable, syllable_length, candidates, MAX_CANDIDATES, FUZZY_CI_SYLLABLE_LENGTH);

    for (i = 0; i < count; i++)
        if (candidates[i].word.item->ci_length == hz_length && !memcmp(candidates[i].word.hz, hz, hz_length * sizeof(HZ)))
        {
            item = candidates[i].word.item;			//找到
            break;
        }

    delete [] candidates;

    return item;
}



/**	判断汉字能否插入词库
 */
int IsAllCanInLibrary(HZ *hz, int length)
{
    int i;

    for (i = 0; i < length; i++)
        if (!_CanInLibrary(hz[i]))
            return 0;

    return 1;
}

/*	向词库中增加词汇（用户自造词）。
 *	要求：词的音节必须与字一一对应，不能出现“分隔符”以及“通配符”。
 *	参数：
 *		wordlib_id	词库标识
 *		hz			词
 *		length		词长度
 *		freq		词频
 *	返回：
 *		成功加入：1
 *		失败：0
 */
int AddCiToWordLibrary(int wordlib_id, HZ *hz, int hz_length, SYLLABLE *syllable, int syllable_length, int freq, FILE *fr = NULL)
{
    WORDLIBITEM	*item;							//词项指针
    WORDLIB		*wordlib;						//词库指针
    PAGE		*page;							//页指针
    int			page_no, new_page_no;			//页号
    int			item_length;					//词项长度
    int			i;

    //检查词频，避免越界
    if (freq > WORDLIB_MAX_FREQ)
        freq = WORDLIB_MAX_FREQ;

    if (wordlib_id < 0 || wordlib_id >= MAX_WORDLIBS * 2)
    {
        if(fr)
        {
            _ftprintf(fr, TEXT("词库ID错误 :<%d>\n"), wordlib_id);
        }
        return 0;
    }

    if (syllable_length < 2 || syllable_length > MAX_WORD_LENGTH ||	hz_length < 2 || hz_length > MAX_WORD_LENGTH)
    {
        if(fr)
        {
            _ftprintf(fr, TEXT("音节过少或者过大 :<%d>\n"), syllable_length);
        }
        return 0;				//音节过少或者过大
    }

    //判断是否都是汉字
    if (!IsAllCanInLibrary(hz, hz_length))
    {
        if(fr)
        {
            _ftprintf(fr, TEXT("不全为汉字 :<%s>\n"), (TCHAR*)hz);
        }
        return 0;
    }

    //进行插入

    wordlib = wordlib_buffer[0];
    if (!wordlib)				//没有这个词库
    {
        if(fr)
        {
            _ftprintf(fr, TEXT("wordlib不存在\n"));
        }
        return 0;
    }
   /*
    //判断该词是否在词库中存在，如果存在不做插入，但将有效置为1，并且增加一次词频
    if ((item = GetCiInWordLibrary(wordlib_id, hz, hz_length, syllable, syllable_length)) != 0)
    {
        if (!item->effective)
        {
            item->effective = 1;
            wordlib->header.word_count++;
        }

        if (wordlib_id == 0)
        {
            if (freq > (int)item->freq)
                item->freq = (unsigned int)freq;
            else
                item->freq++;
            return 1;
        }

        if (freq > (int)item->freq)
            item->freq = (unsigned int)freq;

        return 1;
    }
    */
    item_length = GetItemLength(hz_length, syllable_length);

    //找出音节序列的词库页索引
    page_no = wordlib->header.index[syllable[0].con][syllable[1].con];
    if (page_no == PAGE_END)							//索引没有指向页
    {
        new_page_no = NewWordLibPage(wordlib_buffer[0]);		//分配新页
        if (new_page_no == -1)							//未能分配成功，只好返回
        {
            if(fr)
            {
                _ftprintf(fr, TEXT("分配新页失败1。\n"));
            }
            return 0;
        }

        wordlib->header.index[syllable[0].con][syllable[1].con] = new_page_no;		//索引联接
        page_no = new_page_no;
    }

    //遍历页表找出最后一页。
    //不进行已删除词汇空洞的填补工作，省力（好编）并且省心（程序健壮）。
    while(wordlib->pages[page_no].next_page_no != PAGE_END)
        page_no = wordlib->pages[page_no].next_page_no;

    //获得页
    page = &wordlib->pages[page_no];

    //如果本页的数据不能满足加入要求
    if (page->data_length + item_length > WORDLIB_PAGE_DATA_LENGTH)
    {//需要分配新页
        new_page_no = NewWordLibPage(wordlib);
        if (new_page_no == -1)		//未能分配成功，只好返回
        {
            if(fr)
            {
                _ftprintf(fr, TEXT("分配新页失败2。\n"));
            }
            return 0;
        }

        //分配成功，维护页链表
        page->next_page_no	= new_page_no;
        page_no				= new_page_no;
        page				= &wordlib->pages[page_no];
    }

    assert(page->data_length + item_length <= WORDLIB_PAGE_DATA_LENGTH);

    //词汇长度
    page->length_flag |= (1 << syllable_length);

    //在本页中插入输入
    item = (WORDLIBITEM*)&page->data[page->data_length];

    item->effective		  = 1;					//有效
    item->ci_length		  = hz_length;			//词长度
    item->syllable_length = syllable_length;	//音节长度
    item->freq			  = freq;				//词频

    for (i = 0; i < syllable_length; i++)
        item->syllable[i] = syllable[i];		//音节

    for (i = 0; i < hz_length; i++)
        GetItemHZPtr(item)[i] = hz[i];			//汉字

    //增加页的数据长度
    page->data_length += item_length;

    //增加了一条记录
    wordlib->header.word_count++;

    //成功插入
    return 1;
}

//词库维护外部接口
#define	ERR_FILE_NAME		TEXT("err.log")
#define	DEFAULT_FREQ		0

/**	向词库导入词条
 *	参数：
 *		wordlib_name
 *		text_name
 *		ok_count
 *		err_count
 *		err_file_name
 *	返回：
 *		1：成功
 *		0：失败
 */
int ImportWordLibrary(const TCHAR *wordlib_file_name, const TCHAR *text_file_name, int *ok_count, int *err_count, TCHAR *err_file_name, void *call_back)
{
    TCHAR line[1024];                   //一行1000个字节
    TCHAR ci_str[256];                  //词串
    int  ci_length = 0;					//词长
    TCHAR pinyin_str[256];              //拼音串
    int  syllable_count = 0;			//音节长度
    FILE *fr = NULL, *ferr = NULL;		//导入文件、错误文件
    SYLLABLE syllables[256];            //音节数组
    WORDLIB	*wl;                        //被导出词库
    WORDLIBITEM *item;                  //
    int  wl_id = 0, user_wl_id = 0;		//被导入词库标识、用户词库标识
    int  freq = 0;						//词频
    int  i = 0, j = 0, line_count = 0;	//行数
    int  old_ci_count = 0;
    int  import_to_user_lib = 0;
    int  wl_reloaded = 0;

    progress_indicator pi = (progress_indicator)call_back;

    *ok_count = 0;
    *err_count = 0;

    //创建err文件，在text文件所在目录中
    for (i = 0, j = 0; text_file_name[i]; i++)
    {
        if (text_file_name[i] == '\\')
            j = i;
    }

    _tcsncpy_s(err_file_name, MAX_PATH, text_file_name, j + 1);
    if (j)
        _tcscpy_s(err_file_name + j + 1, MAX_PATH - j - 1, ERR_FILE_NAME);
    else
        _tcscpy_s(err_file_name, MAX_PATH - j, ERR_FILE_NAME);

    ferr = _tfopen(err_file_name, TEXT("wt"));
    if (!ferr)
    {
        err_file_name[0] = 0;
        return 0;
    }

    _setmode(_fileno(ferr), _O_U16TEXT);
    _ftprintf(ferr, TEXT("%c"), 0xFEFF);

    //装载词库，如果内存中有这个词库，不需要重新进行加载
    wl_reloaded = 0;
   QVector<QString> wordlib_vector = Config::Instance()->GetSelectedWordlibContainer();

    wl_id = wordlib_vector.indexOf(QString::fromWCharArray((wchar_t*)wordlib_file_name));
//    wl_id = GetWordLibraryLoaded(wordlib_file_name);
    if (wl_id == -1)		//内存中没有这个库
    {
        wl_id = LoadWordLibraryWithExtraLength(wordlib_file_name, WORDLIB_EXTRA_LENGTH, 0);
        wl_reloaded = 1;
    }

    user_wl_id = 0;
    if (user_wl_id == wl_id)
        import_to_user_lib = 1;

    wl = GetWordLibrary(wl_id);
    if (!wl)		//装载失败
    {
        _ftprintf(ferr, TEXT("词库<%s>装载失败。可能您装载的词库过多\n"), wordlib_file_name);
        fclose(ferr);
        return 0;
    }

    old_ci_count = wl->header.word_count;

    if (!wl->header.can_be_edit)
    {
        _ftprintf(ferr, TEXT("词库<%s>不允许导入。\n"), wordlib_file_name);
        fclose(ferr);
        return 0;
    }

    fr = _tfopen(text_file_name, TEXT("rb"));
    if (!fr)
    {
        _ftprintf(ferr, TEXT("文件<%s>无法打开。\n"), text_file_name);
        fclose(ferr);
        return 0;			//无法打开导出文件
    }

    //跳过FFFE
    fseek(fr, 2, SEEK_SET);

    line_count = 0;
    do
    {
        //读取一行数据
        if (!GetLineFromFile(fr, line, _SizeOf(line)))
            break;			//可能到结尾了

        line_count++;

        //注释行或空行
        if (line[0] == 0 || line[0] == '#' || line[0] == ';' || line[0] == 0xd || line[0] == 0xa)
            continue;

        if (_tcsstr(line, TEXT("=")))			//跳过前面的作者等信息
            continue;

        if (1 != _stscanf_s(line, TEXT("%s"), ci_str, _SizeOf(ci_str)))
        {
            _ftprintf(ferr, TEXT("第<%d>行没有内容\n"), line_count);
            (*err_count)++;
            continue;
        }

        if (2 != _stscanf_s(line, TEXT("%s%s"), ci_str, _SizeOf(ci_str), pinyin_str, _SizeOf(pinyin_str)))
        {
            _ftprintf(ferr, TEXT("第<%d>行没有拼音内容\n"), line_count);
            (*err_count)++;
            continue;
        }

        if( _tcslen(pinyin_str) > MAX_INPUT_LENGTH){
            _ftprintf(ferr, TEXT("第<%d>行拼音超过%d字符的限制\n"), line_count, MAX_INPUT_LENGTH);
            (*err_count)++;
            continue;
        }

        if (3 != _stscanf_s(line, TEXT("%s%s%d"), ci_str, _SizeOf(ci_str), pinyin_str, _SizeOf(pinyin_str), &freq))
            freq = DEFAULT_FREQ;

        syllable_count = ParsePinYinStringReverse(pinyin_str, syllables, _SizeOf(syllables), 0, PINYIN_QUANPIN/*, 0, 0*/);

        if (!syllable_count)
        {
            _ftprintf(ferr, TEXT("第<%d>行拼音<%s>有错误\n"), line_count, pinyin_str);
            (*err_count)++;
            continue;
        }

        ci_length = (int)_tcslen(ci_str);
        if (ci_length < syllable_count)
        {
            _ftprintf(ferr, TEXT("第<%d>行词条<%s>音节长度<%d>与词长度<%d>不匹配\n"), line_count, ci_str, syllable_count, ci_length);
            (*err_count)++;
            continue;
        }

        //判断词是否都是汉字
        for (i = 0; i < ci_length; i++)
            if (!_CanInLibrary(ci_str[i]))
                break;

        if (i != ci_length)
        {
            _ftprintf(ferr, TEXT("第<%d>行词条<%s>不全是汉字\n"), line_count, ci_str);
            (*err_count)++;
            continue;
        }

        if (ci_length > MAX_WORD_LENGTH)
        {
            _ftprintf(ferr, TEXT("第<%d>行词条<%s>长度超出最大长度\n"), line_count, ci_str);
            (*err_count)++;
            continue;
        }

        if (syllable_count > MAX_WORD_LENGTH)
        {
            _ftprintf(ferr, TEXT("第<%d>行词条音节<%s>长度超出最大长度\n"), line_count, pinyin_str);
            (*err_count)++;
            continue;
        }

        //重复词汇
        if (item = GetCiInWordLibrary(wl_id, (HZ*)ci_str, ci_length, syllables, syllable_count))
        {
            if (item->effective)
                continue;
        }

        //向词库中追加
        if (AddCiToWordLibrary(wl_id, (HZ*)ci_str, ci_length, syllables, syllable_count, freq))
        {
            (*ok_count)++;
            if (pi && !(*ok_count % 0x100))
                (*pi)(-1, *ok_count);
            continue;
        }

        //可能词库满，重新装载词库
        SaveWordLibrary(wl_id);					//保存词库数据
        if (wl_reloaded)						//必须在词库重装载后，进行Close操作
            CloseWordLibrary(wl_id);			//关闭词库

        wl_id = LoadWordLibraryWithExtraLength(wordlib_file_name, WORDLIB_EXTRA_LENGTH, 0);
        wl_reloaded = 1;
        wl = GetWordLibrary(wl_id);

        if (!wl) 		//装载失败
        {
            _ftprintf(ferr, TEXT("词库<%s>重新装载失败。\n"), wordlib_file_name);
            fclose(fr);
            fclose(ferr);
            return 0;
        }

        if (AddCiToWordLibrary(wl_id, (HZ*)ci_str, ci_length, syllables, syllable_count, freq))
        {
            (*ok_count)++;
            if (pi && !(*ok_count % 0x100))
                (*pi)(-1, *ok_count);
            continue;
        }

        _ftprintf(ferr, TEXT("发生未知错误，词库<%s>可能毁坏\n"), wordlib_file_name);
        fclose(fr);
        fclose(ferr);
        return 0;

    }while(1);

    fclose(fr);

    if (wl)
        *ok_count = wl->header.word_count - old_ci_count;

    if (pi)
        (*pi)(-1, *ok_count);

    SaveWordLibrary(wl_id);					//保存词库数据
    if (wl_reloaded)
    {
        CloseWordLibrary(wl_id);			//删除共享
//        if (import_to_user_lib)
//            share_segment->can_save_user_wordlib = 0;
    }

    fclose(ferr);
    //_tunlink(err_file_name);					//删除错误记录文件

    return 1;
}

/**	导出词库词条
 *	参数：
 *		wordlib_name			被导出词库名称
 *		text_name				导出词条文件名称
 *		ci_count				被导出词计数
 *		err_file_name			错误文件名称
 *		export_all				是否输出全部词条（包含被删除词？）
 *	返回：
 *		1：成功
 *		0：失败
 */
int ExportWordLibrary(const TCHAR *wordlib_file_name, const TCHAR *text_file_name, int *ci_count, TCHAR *err_file_name, int	export_all, void *call_back)
{
	export_all;
#define	MAX_DELETED_COUNT	10000

    WORDLIBITEM *item = NULL;										//词项
    WORDLIB *wl = NULL;											//被导出词库
    PAGE	*page = NULL;											//当前页
    FILE *fw = NULL, *ferr=NULL;										//导出文件
    int  wl_id = 0;												//被导出词库标识
    TCHAR ci_str[MAX_WORD_LENGTH + 1] ={0};
    TCHAR pinyin_str[MAX_WORD_LENGTH * MAX_PINYIN_LENGTH + 1] ={0};
    int	 pinyin_index = 0;
    int  i = 0, j = 0, err = 0;											//返回错误
    progress_indicator pi = (progress_indicator)call_back;

    share_segment.syllable_map_items   = sizeof(share_segment.syllable_map) / sizeof(share_segment.syllable_map[0]);
    share_segment.syllablev5_map_items = sizeof(share_segment.syllablev5_map) / sizeof(share_segment.syllablev5_map[0]);

    fw = ferr = 0;
    *ci_count = 0;

    ferr = _tfopen(err_file_name, TEXT("wt"));
    if (!ferr)
    {
        err_file_name[0] = 0;
        return 0;
    }

    _setmode(_fileno(ferr), _O_U16TEXT);
    _ftprintf(ferr, TEXT("%c"), 0xFEFF);


    wl_id = LoadWordLibraryWithExtraLength(wordlib_file_name, 0, 0);
    wl = wordlib_buffer[0];
    if (!wl)		//装载失败
    {
        _ftprintf(ferr, TEXT("词库<%s>装载失败\n"), wordlib_file_name);
        fclose(ferr);
        return 0;
    }

    err = 1;
    do
    {
//        if (!wl->header.can_be_edit)
//        {
//            _ftprintf(ferr, TEXT("词库<%s>不允许导出。\n"), wordlib_file_name);
//            break;
//        }

        fw = _tfopen(text_file_name, TEXT("wt"));
        if (!fw)
        {
            _ftprintf(ferr, TEXT("文件<%s>无法创建。\n"), text_file_name);
            break;			//无法打开导出文件
        }

        _setmode(_fileno(fw), _O_U16TEXT);
        _ftprintf(fw, TEXT("%c"), 0xFEFF);

        _ftprintf(fw, TEXT("名称=%s\n"), wl->header.name);
        _ftprintf(fw, TEXT("作者=%s\n"), wl->header.author_name);
        _ftprintf(fw, TEXT("编辑=%d\n\n"), wl->header.can_be_edit);

        for (i = 0; i < wl->header.page_count; i++)
        {	//遍历页表
            for (page = &wl->pages[i], item = (WORDLIBITEM*) page->data; (char*)item < (char*) &page->data + page->data_length; item = GetNextCiItem(item))
            {
                //是否有效
                if (!item->effective)
                    continue;

                memcpy(ci_str, GetItemHZPtr(item), item->ci_length * sizeof(HZ));
                ci_str[item->ci_length] = 0;

                pinyin_index = 0;
                for (j = 0; j < (int)item->syllable_length; j++)
                {
                    SYLLABLE s;
                    if (j)
                        pinyin_str[pinyin_index++] = SYLLABLE_SEPARATOR_CHAR;

                    s = item->syllable[j];
                    s.tone = TONE_0;
                    pinyin_index += GetSyllableString(s, pinyin_str + pinyin_index, _SizeOf(pinyin_str) - pinyin_index,/* 0,*/ 0);
                    pinyin_str[pinyin_index] = 0;
                }

                _ftprintf(fw, TEXT("%s\t%s\t%d\n"), ci_str, pinyin_str, item->freq);

                (*ci_count)++;
                if (pi && !(*ci_count % 0x100))
                    (*pi)(wl->header.word_count, *ci_count);
            }
        }
        err = 0;

    }while(0);

    if (fw)
        fclose(fw);

    if (ferr)
        fclose(ferr);

    if (pi)
        (*pi)(wl->header.word_count, *ci_count);

    CloseWordLibrary(wl_id);

    if (!err)		//没有错误
        _tunlink(err_file_name);

    return !err;
}

/*	基于词条数据文件，创建词库。
 *	参数：
 *		wordlib_file_name		词库文件名字（全路径）
 *		text_file_name			词库词条文件名字（如果为0则为创建新词库）
 *		can_be_edit				是否允许编辑
 *		ok_count				正确词条数目
 *		err_count				错误词条数目
 *		err_file_name			错误文件名称
 *	返回：
 *		成功创建：1
 *		失败：0
 */

int  CreateWordLibrary(const TCHAR *wordlib_file_name, const TCHAR *text_file_name, int *ok_count, int *err_count, TCHAR *err_file_name)
{
    FILE *fr = NULL, *ferr = NULL;				//导入文件、错误文件
    SYLLABLE syllables[256];					//音节数组
    WORDLIB	*wl;								//被导出词库
    TCHAR line[1024] = {0};						//一行1000个字节
    TCHAR ci_str[256] = {0};					//词串
    TCHAR pinyin_str[256] = {0};				//拼音串
    TCHAR wl_name[WORDLIB_NAME_LENGTH];			//词库名称
    TCHAR author_name[WORDLIB_AUTHOR_LENGTH];	//作者名称
    int  can_be_edit = 0;						//是否可以编辑
    int  ci_length = 0;							//词长
    int  syllable_count = 0;					//音节长度
    int  wl_id = 0;                             //被导入词库标识
    int  freq = 0;								//词频
    int  i = 0, line_count = 0;					//行数

    share_segment.syllable_map_items   = sizeof(share_segment.syllable_map) / sizeof(share_segment.syllable_map[0]);
    share_segment.syllablev5_map_items = sizeof(share_segment.syllablev5_map) / sizeof(share_segment.syllablev5_map[0]);

    // 文件过大，直接返回
    do
    {
        std::ifstream in(text_file_name);
        if (!in.is_open()) return 0;
        in.seekg(0, std::ios_base::end);
        std::streampos sp = in.tellg();

        // 将词库内存设定为文件大小的 n 倍
        // 注意！这里是危险的代码，有可能造成内存不足导致程序崩溃
        // 这是历史遗留问题的临时解决方案，需要重构
        // added by liuxinyang
        int n = 8;
        wordlib_create_extra_length = sp * n;

        if (wordlib_create_extra_length > 1024 * 1024 * 200)
        {
            return -99;
        }
    } while (false);

    //如果传入的文本文件地址为空则返回错误
    if (!text_file_name)		//创建空的词库文件
    {
        return 0;
    }

    *ok_count = 0;
    *err_count = 0;

    //error文件目录由外部传入，在内部不再进行处理
    ferr = _tfopen(err_file_name, TEXT("wt"));
    if (!ferr)
    {
        err_file_name[0] = 0;
        return 0;
    }

    _setmode(_fileno(ferr), _O_U16TEXT);
    _ftprintf(ferr, TEXT("%c"), 0xFEFF);

    //打开词条文件
    fr = _tfopen(text_file_name, TEXT("rb"));
    if (!fr)
    {
        _ftprintf(ferr, TEXT("词条文件<%s>无法打开。\n"), text_file_name);
        fclose(ferr);
        return 0;			//无法打开导出文件
    }

    //跳过FFFE
    fseek(fr, 2, SEEK_SET);

    //读取词库名称
    line_count = 0;
    do
    {
        if (!GetLineFromFile(fr, line, _SizeOf(line)))
            break;

        line_count++;
    }while(line[0] == 0 || line[0] == '#' || line[0] == ';' || line[0] == 0xd || line[0] == 0xa);

    if (!line[0] || _tcsncmp(TEXT("名称="), line, 3))
    {
        _ftprintf(ferr, TEXT("没有定义词库名称。词库名称定义格式：名称=Test\n名称长度最大15个汉字"));
        fclose(fr);
        fclose(ferr);
        return 0;
    }

    MakeStringFitLength(line + 3, WORDLIB_NAME_LENGTH);
    _tcscpy_s(wl_name, _SizeOf(wl_name), line + 3);

    //读取作者名称
    do
    {
        if (!GetLineFromFile(fr, line, _SizeOf(line)))
            break;

        line_count++;
    }while(line[0] == '#' || line[0] == ';' || line[0] == 0xd || line[0] == 0xa);

    if (!line[0] || _tcsncmp(TEXT("作者="), line, 3))	//没有数据
    {
        _ftprintf(ferr, TEXT("没有定义作者名字。作者定义格式：作者=ceshi\n名字长度最大15个汉字"));
        fclose(fr);
        fclose(ferr);

        return 0;
    }

    MakeStringFitLength(line + 3, WORDLIB_AUTHOR_LENGTH);
    _tcscpy_s(author_name, _SizeOf(author_name), line + 3);

    //读取是否可以编辑信息
    do
    {
        if (!GetLineFromFile(fr, line, _SizeOf(line)))
            break;

        line_count++;
    }while(line[0] == '#' || line[0] == ';' || line[0] == 0xd || line[0] == 0xa);

    if (!line[0] || _tcsncmp(TEXT("编辑="), line, 3))	//没有数据
    {
        _ftprintf(ferr, TEXT("没有定义是否可以编辑。定义格式：编辑=1\n可以被他人编辑：1，不可以编辑：0"));
        fclose(fr);
        fclose(ferr);
        return 0;
    }

    can_be_edit = line[3] == '1' ? 1 : 0;

    //创建空词库文件
    if (!CreateEmptyWordLibFile(wordlib_file_name, wl_name, author_name, can_be_edit))
    {
        fclose(fr);
        fclose(ferr);
        return 0;
    }
    //将词库加载到内存中
    int length = GetFileLength(wordlib_file_name);

    // <<<
    // 注意！下面的代码是危险地带！
    // wordlib_create_extra_length 只是一个预估的值！
    // 有可能造成分配的内存不足，导致崩溃！
    // 这是对历史遗留代码的临时解决方案！
    // 必须重构！必须重构！必须重构！
    // added by liuxinyang
    char *buffer = (char*)malloc(length + wordlib_create_extra_length);
    memset(buffer, 0, length + wordlib_create_extra_length);
    // >>>

    LoadFromFile(wordlib_file_name,buffer,length);
    wl = (WORDLIB*)buffer;
    wordlib_buffer[0] = (WORDLIB*)buffer;

    //配置工具只对词库进行生成，不对词库进行装载
    do
    {
        //读取一行数据
        if (!GetLineFromFile(fr, line, _SizeOf(line)))
            break;			//可能到结尾了

        line_count++;

        //注释行或空行
        if (line[0] == 0 || line[0] == '#' || line[0] == ';' || line[0] == 0xd || line[0] == 0xa)
            continue;

        if (!_tcscmp(TEXT("人生得意须尽欢"), ci_str))
            line_count = line_count;

        memset((char*)ci_str, 0, _SizeOf(ci_str) * sizeof(WCHAR));
        if (1 != _stscanf_s(line, TEXT("%s"), ci_str, _SizeOf(ci_str)))
        {
            _ftprintf(ferr, TEXT("第<%d>行没有内容\n"), line_count);
            (*err_count)++;
            continue;
        }
        ci_str[_SizeOf(ci_str) - 1] = _T('\0');

        memset((char*)ci_str, 0, _SizeOf(ci_str) * sizeof(WCHAR));
        memset((char*)pinyin_str, 0, _SizeOf(pinyin_str) * sizeof(WCHAR));
        if (2 != _stscanf_s(line, TEXT("%s%s"), ci_str, _SizeOf(ci_str), pinyin_str, _SizeOf(pinyin_str)))
        {
            _ftprintf(ferr, TEXT("第<%d>行没有拼音内容\n"), line_count);
            (*err_count)++;
            continue;
        }
        ci_str[_SizeOf(ci_str) - 1] = _T('\0');
        pinyin_str[_SizeOf(pinyin_str) - 1] = _T('\0');

        memset((char*)ci_str, 0, _SizeOf(ci_str) * sizeof(WCHAR));
        memset((char*)pinyin_str, 0, _SizeOf(pinyin_str) * sizeof(WCHAR));
        freq = 0;
        if (3 != _stscanf_s(line, TEXT("%s%s%d"), ci_str, _SizeOf(ci_str), pinyin_str, _SizeOf(pinyin_str), &freq))
            freq = DEFAULT_FREQ;
        ci_str[_SizeOf(ci_str) - 1] = _T('\0');
        pinyin_str[_SizeOf(pinyin_str) - 1] = _T('\0');

        syllable_count = ParsePinYinStringReverse(pinyin_str, syllables, MAX_WORD_LENGTH, 0, PINYIN_QUANPIN);

        if (!syllable_count)
        {
            _ftprintf(ferr, TEXT("第<%d>行拼音<%s>有错误\n"), line_count, pinyin_str);
            (*err_count)++;
            continue;
        }

        ci_length = (int)_tcslen(ci_str);
        if (ci_length < syllable_count)
        {
            _ftprintf(ferr, TEXT("第<%d>行词条<%s>音节长度<%d>与词长度<%d>不匹配\n"), line_count, ci_str, syllable_count, ci_length);
            (*err_count)++;
            continue;
        }

        //判断词是否都是汉字
        for (i = 0; i < ci_length; i++)
            if (!_CanInLibrary(ci_str[i]))
                break;

        if (i != ci_length)
        {
            _ftprintf(ferr, TEXT("第<%d>行词条<%s>不全是汉字\n"), line_count, ci_str);
            (*err_count)++;
            continue;
        }
        if(*ok_count == 960)
        {
            //int ceshi = 22;
        }

        //向词库中追加
        if (AddCiToWordLibrary(wl_id, (HZ*)ci_str, ci_length, syllables, syllable_count, freq, ferr))
        {
            (*ok_count)++;

            continue;
        }
        //可能词库满，重新装载词库
        if(!SaveWordLibarayToFile(wl,wordlib_file_name))				//保存词库数据
        {
            _ftprintf(ferr, TEXT("保存文件失败"));
        }
        free(wl);

        wl_id = LoadWordLibraryWithExtraLength(wordlib_file_name, wordlib_create_extra_length, 0, ferr);
        _ftprintf(ferr, TEXT("词库名为:<%s>, 词库ID为: <%d>"), wordlib_file_name, wl_id);
        wl = wordlib_buffer[0];
        if (!wl)		//装载失败
        {
            CloseWordLibrary(wl_id);				//关闭词库
            _ftprintf(ferr, TEXT("词库<%s>重新装载失败。\n"), wordlib_file_name);
            fclose(fr);
            fclose(ferr);
            return 0;
        }

        if (AddCiToWordLibrary(wl_id, (HZ*)ci_str, ci_length, syllables, syllable_count, freq, ferr))
        {
            (*ok_count)++;
            continue;
        }
        else
        {
             _ftprintf(ferr, TEXT("第<%d>行词条<%s>无法加入词库，保存词库后仍然无法写入词库\n"), line_count, ci_str);
             continue;
        }

        _ftprintf(ferr, TEXT("发生未知错误，词库<%s>可能毁坏\n"), wordlib_file_name);
        fclose(fr);
        fclose(ferr);
        CloseWordLibrary(wl_id);				//关闭词库
        return 0;

    }while(1);

    fclose(fr);
    fclose(ferr);

    if (wl)
        *ok_count = wl->header.word_count;

    SaveWordLibarayToFile(wl,wordlib_file_name);

    if (*err_count == 0)
        _tunlink(err_file_name);

    return 1;
}




/**	获得词库信息
 *	当参数为0的时候，不对参数进行赋值
 *	参数：
 *		name		词库文件
 *		version		词库版本号
 *		wl_name		词库文件名称
 *		author_name	词库作者名称
 *		can_be_edit	是否可以被编辑
 *		items		词库中的词条数目
 *	返回：
 *		成功：1
 *		失败：0（词库可能不存在）
 */

int GetWordLibInformation(const TCHAR *name, int *version, TCHAR *wl_name, TCHAR *author_name, int *can_be_edit, int *items)
{
    WORDLIBHEADER header;
   // extern int IsV3WordLibBK(const TCHAR*);

    if (LoadFromFile(name, (char*)&header, sizeof(header)) < sizeof(header))
        return 0;

    if (version)
    {
        switch (header.signature)
        {
        case HYPIM_WORDLIB_V66_SIGNATURE:
            *version = WORDLIB_V66;
            break;

        case HYPIM_WORDLIB_V6_SIGNATURE:
            *version = WORDLIB_V6;
            break;

        case HYPIM_WORDLIB_V6B1_SIGNATURE:
            *version = WORDLIB_V6B1;
            break;

        case HYPIM_WORDLIB_V6B2_SIGNATURE:
            *version = WORDLIB_V6B2;
            break;

        case HYPIM_WORDLIB_V5_SIGNATURE:
            *version = WORDLIB_V5;
            return 1;

        default:
            //需要判断是否为V3/V5的备份词库文件
           // *version = IsV3WordLibBK(name) ? WORDLIB_V5_BKUP : WORDLIB_WRONG;
            return 1;
        }
    }

    if (wl_name)
        _tcscpy_s(wl_name, WORDLIB_NAME_LENGTH, header.name);

    if (author_name)
        _tcscpy_s(author_name, WORDLIB_AUTHOR_LENGTH, header.author_name);

    if (can_be_edit)
        *can_be_edit = header.can_be_edit;

    if (items)
        *items = header.word_count;

    return 1;
}

/*	创建新的空的词库文件。
 *	参数：
 *		wordlib_file_name		词库文件名字（全路径）
 *		name					词库名字（放在词库内部）
 *		author					作者
 *		can_be_edit				是否允许编辑
 *	返回：
 *		成功创建：1
 *		失败：0
 */
int CreateEmptyWordLibFile(const TCHAR *wordlib_file_name, const TCHAR *name, const TCHAR *author, int can_be_edit)
{
    WORDLIB		wordlib;
    int i, j;

    //清零
    memset(&wordlib, 0, sizeof(wordlib));

    //作者名字
    _tcsncpy(wordlib.header.author_name, author, _SizeOf(wordlib.header.author_name));

    //词库名字
    _tcsncpy(wordlib.header.name, name, _SizeOf(wordlib.header.name));

    wordlib.header.can_be_edit = can_be_edit;

    //将Index置为没有页
    for (i = CON_NULL; i < CON_END; i++)
        for (j = CON_NULL; j < CON_END; j++)
            wordlib.header.index[i][j] = PAGE_END;

    //没有已分配的页
    wordlib.header.page_count	= 0;
    wordlib.header.pim_version	= HYPIM_VERSION;
    wordlib.header.signature	= HYPIM_WORDLIB_V66_SIGNATURE;
    wordlib.header.word_count	= 0;

    if (!SaveToFile(wordlib_file_name, &wordlib, sizeof(wordlib)))
        return 0;

    return 1;
}

int SaveWordLibarayToFile(WORDLIB* wordlib, const TCHAR* filePath)
{

    if (!wordlib)
    {
        return 0;
    }

    int length = GetWordLibSize(wordlib);

    if (!SaveToFile(filePath, wordlib, length))
    {
        return 0;
    }
    return 1;
}

/*	保存词库文件。
 *	参数：
 *		wordlib_id			词库标识
 *	返回：
 *		成功：1
 *		失败：0
 */
int SaveWordLibrary(int wordlib_id,const TCHAR*filePath)
{
    WORDLIB *wordlib = NULL;
    int		length = NULL;			//词库长度

    //词库指针获取
    wordlib = GetWordLibrary(wordlib_id);
    if (!wordlib)
    {
        return 0;
    }

    length = GetWordLibSize(wordlib);

    if (!SaveToFile(filePath, wordlib, length))
    {
        return 0;
    }

    return 1;
}

/*	释放词库数据。
 *	参数：无
 *	返回：无
 */
void CloseWordLibrary(int wordlib_id)
{
    if (wordlib_id < 0 || wordlib_id >= MAX_WORDLIBS * 2)
        return;

    wordlib_buffer[wordlib_id]					  = 0;		//指针清零

}
