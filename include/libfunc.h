/*********************************************************************
* 版权所有 
* 
* 文件名称： libfunc.h
* 文件标识： 
* 内容摘要： 重新实现upimlib.dproj文件中的五个输出函数
* 其它说明： 无
* 当前版本： V1.0
* 作    者： xiongh
* 建立日期： 2010年4月21日
* 完成日期： 2010年5月23日
**********************************************************************/

#ifndef	_LibFun_H_
#define	_LibFun_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <string.h>
#include <config.h>

//根据Theme名称，读取Theme信息到Config
int MakeTheme(PIMCONFIG *Config);

//生成Theme列表
int GetThemeList(TCHAR ThemeList[MAX_THEME_COUNT][MAX_THEME_NAME_LENGTH]);

//备份用户数据
int BackupUserData(const TCHAR *BackupDir);// BackUpDir:传入保存路径

//能否运行自动更新程序(判断当前是否允许更新、且是否已更新)
//int CanRunUpdateApp(INT_PTR handlIme);

//获取智能提示文字
int GetHintMessage(TCHAR *MsgBuffer,int BufferSize);//随机返回一条提示信息给MsgBuffer，BufferSize;单条提示信息的大小

#ifdef __cplusplus
}
#endif

#endif