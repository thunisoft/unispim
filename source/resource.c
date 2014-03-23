/*	系统的资源管理。
 *		华宇输入法安装目录设计
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

#include <config.h>
#include <utility.h>
