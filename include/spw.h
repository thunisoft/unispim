#ifndef	_SPW_H_
#define	_SPW_H_

#include <kernel.h>
#include <context.h>

#ifdef __cplusplus
extern "C" {
#endif

#define		SYS_SPW_FILE_NAME				TEXT("unispim6\\phrase\\系统短语库.ini")	//短语文件名称
#define		SPW_COMMENT_CHAR			';'							//短语注释符号
#define		SPW_ASSIGN_CHAR				'='							//短语赋值符号
#define		SPW_HINT_LEFT_CHAR			'['							//短语提示符号_左
#define		SPW_HINT_RIGHT_CHAR			']'							//短语提示符号_右
#define		SPW_HINT_NULL_STR			TEXT("[]")					//短语提示空串
#define		SPW_NAME_LENGTH				16							//短语名字最大长度
#define		SPW_HINT_LENGTH				64							//短语提示最大长度
#define		SPW_CONTENT_LENGTH			MAX_SPW_LENGTH				//短语内容最大长度
#define		SPW_BUFFER_SIZE				0x500000					//短语存储区大小
#define		SPW_MAX_ITEMS				400000						//短语最多数目

#define		SPW_TYPE_NAME				1							//短语名字
#define		SPW_TYPE_CONTENT			2							//短语内容
#define		SPW_TYPE_COMMENT			4							//短语注释
#define		SPW_TYPE_NONE				8							//出错！非短语相关

#define		SPW_STRING_NORMAL			1							//普通的短语
#define		SPW_STRING_EXEC				2							//执行程序类型
#define		SPW_STRING_SPECIAL			3							//特殊类型，如I,D,H等
#define		SPW_STRING_BH				4							//笔划候选
#define		SPW_STIRNG_ENGLISH			5							//英文单词

#define		MAX_SPW_COUNT				32							//短语文件最多数目

//u命令的保留关键字
static const TCHAR u_reserved_word[][MAX_SPW_HINT_STRING] =
{
	TEXT("setup"),		TEXT("启动输入法配置程序*"),		TEXT(""),
	TEXT("abc"),		TEXT("启动智能ABC输入方式*"),		TEXT(""),
	TEXT("bsjz"),		TEXT("打开部首检字功能*"),			TEXT(""),
	TEXT("qj"),			TEXT("启动全集输入*"),         		TEXT(""),
	//TEXT("ckgl"),		TEXT("打开词库管理程序*"),			TEXT(""),
	TEXT("cstar"),		TEXT("启动CStar输入方式*"),			TEXT(""),
	TEXT("czsr"),		TEXT("打开拆字输入功能*"),			TEXT(""),
	TEXT("dos"),		TEXT("打开Dos窗口"),				TEXT("cmd"),
	TEXT("dxdiag"),		TEXT("DirectX 诊断工具"),			TEXT("dxdiag"),	
	TEXT("dy"),			TEXT("打开符号(短语)输入*"),		TEXT(""),
	TEXT("excel"),		TEXT("打开MS Excel"),				TEXT("excel"),
	TEXT("ft"),			TEXT("启动繁体输入*"),				TEXT(""),
	TEXT("ht"),			TEXT("打开画图软件"),				TEXT("mspaint"),
	TEXT("hs"),			TEXT("切换横竖排显示*"),			TEXT(""),
	TEXT("hzgl"),		TEXT("汉字管理*"),					TEXT(""),
	TEXT("ie"),			TEXT("打开浏览器"),					TEXT("iexplore"),
//	TEXT("imetool"),	TEXT("输入法管理器*"),				TEXT(""),
	TEXT("jsb"),		TEXT("打开记事本"),					TEXT("notepad"),
	TEXT("jsq"),		TEXT("打开计算器"),					TEXT("calc"),
	TEXT("jt"),			TEXT("启动简体输入*"),				TEXT(""),
	TEXT("msconfig"),	TEXT("查看本机配置"),				TEXT("msconfig"),
	TEXT("pathalluser"),TEXT("打开所有用户数据目录*"),		TEXT(""),
	TEXT("pathapp"),	TEXT("打开输入法安装目录*"),		TEXT(""),
	TEXT("pathime"),	TEXT("打开ime所在目录*"),			TEXT(""),
	TEXT("pathtemp"),	TEXT("打开系统临时目录"),			TEXT(""),
	TEXT("pathuser"),	TEXT("打开当前用户数据目录*"),		TEXT(""),
	TEXT("pfzz"),		TEXT("主题制作*"),					TEXT(""),
	TEXT("plzc"),		TEXT("批量造词*"),					TEXT(""),
	TEXT("qjsr"),		TEXT("打开/关闭全角输入中文功能*"),	TEXT(""),
	TEXT("qp"),			TEXT("启动全拼输入*"),				TEXT(""),
	TEXT("services"),	TEXT("查看本机服务"),				TEXT("services.msc"),
	TEXT("sp"),			TEXT("启动双拼输入*"),				TEXT(""),
	TEXT("status"),		TEXT("切换状态栏显示与不显示*"),	TEXT(""),
	TEXT("szxd"),		TEXT("打开设置向导*"),				TEXT(""),
	//TEXT("url"),		TEXT("打开网址管理*"),				TEXT(""),
	TEXT("ver"),		TEXT("查看输入法版本信息*"),		TEXT(""),
	//TEXT("version"),	TEXT("查看输入法版本信息*"),		TEXT(""),
	TEXT("word"),		TEXT("打开MS Word"),				TEXT("winword"),
	TEXT("yczm"),		TEXT("打开远程桌面程序"),			TEXT("mstsc"),
	TEXT("zcb"),		TEXT("打开注册表"),					TEXT("regedit"),
	TEXT("ztzz"),		TEXT("主题制作*"),					TEXT(""),
};

#define RESERVED_WORD_COUNT	 (sizeof(u_reserved_word) / _SizeOf(u_reserved_word[0]) / sizeof(TCHAR))

//获得短语候选
int GetSpwCandidates(PIMCONTEXT *context, const TCHAR *name, CANDIDATE *candidate_array, int array_length);

//获得特殊输入的提示信息
const TCHAR *GetSPWHintString(const TCHAR *input_string);

//获得U命令的显示字符串
void GetUDisplayString(CANDIDATE *candidate, TCHAR *buffer, int length);

//加载用户自定义短语到内存
//extern int LoadSpwData(const TCHAR *spw_file_name);
extern int LoadAllSpwData();
extern int FreeSpwData();

#ifdef __cplusplus
}
#endif

#endif