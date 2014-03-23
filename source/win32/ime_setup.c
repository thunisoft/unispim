/*	输入法系统设置程序。
 *	参数：
 *	imesetup install		--安装输入法
 *	imesetup uninstall		--卸载输入法
 *	卸载方式：
 *		参照Google输入法的卸载方法：
 *		1. Unload本输入法
 *		2. Delete Preload中的本输入法
 *		3. 用SHELL调用删除本输入法的项
 *	在这种调用顺序下，输入法得以正常卸载，感谢IDag.exe，呵呵。
 *
 *	返回值：
 *	-1		参数错
 *	-2		IME安装调用失败
 *	-3		系统目录获取错误
 *	-4		装载IME输入法文件失败
 *
 *	关于Vista登陆界面的输入法列表：
 *	在HKEY_USERS\.DEFAULT\Keyboard Layout\Preload中加入本输入法
 *	即可以在列表中列出我们的输入法
 */
#include <string.h>
#include <stdio.h>
#include <windows.h>
#include <regstr.h>
#include <imm.h>
#include <shlwapi.h>
#include <tchar.h>

#define	MAX_ENTRY			0x100			//最大搜索的注册表条目（用于卸载）
#define	MAX_PRELOADS		0x10			//最大的预装输入法项数目

#define	ERR_PARAMETER		-1
#define	ERR_INSTALL			-2
#define	ERR_DIR				-3
#define	ERR_LOAD			-4

#define _SizeOf(x) (sizeof((x)) / sizeof((x)[0]))

const TCHAR *ime_file_name;
const TCHAR *ime_text = _T("中文 - 华宇拼音输入法V6");

TCHAR *usage_msg = _T("ime_install [/i ime_file_name]\n")
				   _T("            [/u ime_file_name]\n");

void error_exit(int no)
{
	switch(no)
	{
	case ERR_PARAMETER:		//参数错误
		fprintf(stderr, "%s\n", usage_msg);
		exit(-1);

	case ERR_INSTALL:
		fprintf(stderr, "输入法安装错误。错误号:%d\n", GetLastError());
		exit(-2);

	case ERR_DIR:
		fprintf(stderr, "获取系统目录错误。错误号：%d\n", GetLastError());
		exit(-3);

	case ERR_LOAD:
		fprintf(stderr, "无法装载IME文件。错误号: %d\n", GetLastError());
		exit(-4);
	}

	exit(no);
}

//在默认用户中设置本输入法，这样在Vista Login界面中可见我们的输入法
void set_to_default_user(int hkl_id)
{
	TCHAR id_str[0x10];
	TCHAR no_str[0x10];
	TCHAR hkl_str[0x10];
	HKEY hkey;
	int last_key_no, key_no;
	DWORD size, type;

	_stprintf_s(id_str, _SizeOf(id_str), _T("%X"), hkl_id);

	//在HKEY_USERS\.DEFAULT\Keyboard Layout\Preload中加入本输入法
	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_USERS, _T(".DEFAULT\\Keyboard Layout\\Preload"), 0, KEY_ALL_ACCESS, &hkey))
		return;

	last_key_no = 0;
	for (key_no = 0; key_no < MAX_PRELOADS; key_no++)
	{
		_stprintf_s(no_str, _SizeOf(no_str), _T("%d"), key_no + 1);

		//关于RegQueryValueEx最后一个参数的MSDN解释，注意最后一个参数是倒数第二个参数
		//指向的buffer的大小，这一点与RegSetValueEx有明显区别(见下面的注释)
		//A pointer to a variable that specifies the size of the buffer pointed to by
		//the lpData parameter, in bytes. When the function returns, this variable contains 
		//the size of the data copied to lpData. If the data has the REG_SZ, REG_MULTI_SZ 
		//or REG_EXPAND_SZ type, this size includes any terminating null character or 
		//characters unless the data was stored without them.
		hkl_str[0] = 0;
		size = sizeof(hkl_str);
		type = REG_SZ;
		if (ERROR_SUCCESS == RegQueryValueEx(hkey, no_str, 0, &type, (LPBYTE)hkl_str, &size))
		{
			last_key_no = key_no + 1;
			//找到我们自己的，返回
			if (!_tcsicmp(hkl_str, id_str))
				break;
		}
	}

	if (key_no == MAX_PRELOADS)		//未找到我们的需要设定
	{
		//关于RegSetValueEx最后一个参数的MSDN解释，注意最后一个参数是倒数第二个参数
		//中信息(如字符串)的字节数，而不是倒数第二个参数所对应的buffer大小
		//The size of the information pointed to by the lpData parameter, in bytes. 
		//If the data is of type REG_SZ, REG_EXPAND_SZ, or REG_MULTI_SZ, cbData must
		//include the size of the terminating null character or characters
		_stprintf_s(no_str, _SizeOf(no_str), _T("%d"), last_key_no + 1);
		RegSetValueEx(hkey, no_str, 0, REG_SZ, (LPBYTE)id_str, (_tcslen(id_str) + 1) * sizeof(TCHAR));
	}

	RegCloseKey(hkey);

	return;
}

int install_ime()
{
	TCHAR sys_dir_name[MAX_PATH];
	TCHAR ime_name[MAX_PATH];
	HKL keyboard_layout_id;

	//获得系统目录位置
	if (!GetSystemDirectory(sys_dir_name, _SizeOf(sys_dir_name)))
		error_exit(ERR_DIR);
  
	_stprintf_s(ime_name, _SizeOf(ime_name), _T("%s\\%s"), sys_dir_name, ime_file_name);

	SetLastError(0);
	keyboard_layout_id = ImmInstallIME(ime_name, ime_text);
	if (!keyboard_layout_id)
		error_exit(ERR_INSTALL);

	set_to_default_user(*(int*)&keyboard_layout_id);

	return 0;
}

int uninstall_ime()
{
	HKEY hkey, found_key;
	DWORD size, type;
	TCHAR key_str[256];
	TCHAR key_str_save[256];
	TCHAR ime_name[256];
	TCHAR hkl_value[256], hkl_str[256], hkl_tag_str[256];
	TCHAR no_str[0x10];
	int deleted;				//已经删除的输入法项
	unsigned int key_no, cur_key = 0xe01f0804;
	LONG lres;
	TCHAR *layout_id;

	for (key_no = 0; key_no < MAX_ENTRY; key_no++)
	{
		cur_key += 0x00010000;
		_stprintf_s(key_str, _SizeOf(key_str), REGSTR_PATH_CURRENT_CONTROL_SET _T("\\Keyboard Layouts\\%08X"), cur_key);

		if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, key_str, 0, KEY_READ, &hkey))
		{
			ime_name[0] = 0;
			size = sizeof(ime_name);
			type = REG_SZ;
			RegQueryValueEx(hkey, _T("Ime File"), 0, &type, (LPBYTE)ime_name, &size);
			RegCloseKey(hkey);

			//寻找相同的ime文件
			if (!_tcsicmp(ime_name, ime_file_name))
				break;
		}
	}

	_tcscpy_s(key_str_save, _SizeOf(key_str_save), key_str);
	UnloadKeyboardLayout(*(HKL*)&cur_key);

	_stprintf_s(hkl_tag_str, _SizeOf(hkl_tag_str), _T("%08X"), cur_key);

	//在HKEY_CURRENT_USER/Keyboard Layout/Preload中删除我们的输入法
	deleted = 0;
	_stprintf_s(key_str, _SizeOf(key_str), _T("Keyboard Layout\\Preload"));
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, key_str, 0, KEY_ALL_ACCESS, &hkey))
	{
		for (key_no = 0; key_no < MAX_PRELOADS; key_no++)
		{
			_stprintf_s(no_str, _SizeOf(no_str), _T("%d"), key_no + 1);
			hkl_str[0] = 0;
			size = sizeof(hkl_str);
			type = REG_SZ;
			if (ERROR_SUCCESS == RegQueryValueEx(hkey, no_str, 0, &type, (LPBYTE)hkl_str, &size))
			{
				//找到我们自己的，删除
				if (!_tcsicmp(hkl_str, hkl_tag_str))
				{
					RegDeleteValue(hkey, no_str);
					deleted++;
					continue;
				}
				//删除之后，需要对后面的注册表项提前
				//找到其他的要判断是否为正确的输入法，否则也需要删除
				_stprintf_s(key_str, _SizeOf(key_str), REGSTR_PATH_CURRENT_CONTROL_SET _T("\\Keyboard Layouts\\%s"), hkl_str);
				if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, key_str, 0, KEY_READ, &found_key))
				{
					RegCloseKey(found_key);
					//如果前面删除过输入法，则需要将该输入法提前
					if (deleted)
					{
						//SHDeleteKey之类的函数不能立刻生效，当重新启动的时候，才进行删除!
						//1. 删除原来的value
						//SHDeleteValue(HKEY_CURRENT_USER, "Keyboard Layouts\\Preload", no_str);
						RegDeleteValue(hkey, no_str);
						//2. 设定当前的value
						_stprintf_s(no_str, _SizeOf(no_str), _T("%d"), key_no + 1 - deleted);
						//SHSetValue(HKEY_CURRENT_USER, "Keyboard Layouts\\Preload", no_str, type, hkl_str, size); 
						RegSetValueEx(hkey, no_str, 0, type, (LPBYTE)hkl_str, size);
					}
				}
				else
				{
					RegDeleteValue(hkey, no_str);
					deleted++;
				}
			}
		}
	}

	//在HKEY_USERS/.DEFAULT/Keyboard Layout/Preload中删除我们的输入法
	deleted = 0;
	_stprintf_s(key_str, _SizeOf(key_str), _T(".DEFAULT\\Keyboard Layout\\Preload"));
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_USERS, key_str, 0, KEY_ALL_ACCESS, &hkey))
	{
		for (key_no = 0; key_no < MAX_PRELOADS; key_no++)
		{
			_stprintf_s(no_str, _SizeOf(no_str), _T("%d"), key_no + 1);
			hkl_str[0] = 0;
			size = sizeof(hkl_str);
			type = REG_SZ;
			if (ERROR_SUCCESS == RegQueryValueEx(hkey, no_str, 0, &type, (LPBYTE)hkl_str, &size))
			{
				//找到我们自己的，删除
				if (!_tcsicmp(hkl_str, hkl_tag_str))
				{
					RegDeleteValue(hkey, no_str);
					deleted++;
					continue;
				}
				//删除之后，需要对后面的注册表项提前
				//找到其他的要判断是否为正确的输入法，否则也需要删除
				_stprintf_s(key_str, _SizeOf(key_str), REGSTR_PATH_CURRENT_CONTROL_SET _T("\\Keyboard Layouts\\%s"), hkl_str);
				if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, key_str, 0, KEY_READ, &found_key))
				{
					RegCloseKey(found_key);
					//如果前面删除过输入法，则需要将该输入法提前
					if (deleted)
					{
						//SHDeleteKey之类的函数不能立刻生效，当重新启动的时候，才进行删除!
						//1. 删除原来的value
						//SHDeleteValue(HKEY_CURRENT_USER, "Keyboard Layouts\\Preload", no_str);
						RegDeleteValue(hkey, no_str);
						//2. 设定当前的value
						_stprintf_s(no_str, _SizeOf(no_str), _T("%d"), key_no + 1 - deleted);
						//SHSetValue(HKEY_CURRENT_USER, "Keyboard Layouts\\Preload", no_str, type, hkl_str, size); 
						RegSetValueEx(hkey, no_str, 0, type, (LPBYTE)hkl_str, size);
					}
				}
				else
				{
					RegDeleteValue(hkey, no_str);
					deleted++;
				}
			}
		}
	}

	//在HKEY_CURRENT_USER\Control Panel\International\User Profile\zh-Hans-CN中删除我们的输入法
	//注：此项为WIN8下新增，WIN7和XP没有该项。该项中的键值并非输入法注册时加入，而是只要打控制面
	//板\时钟、语言和区域\语言\语言选项时，系统就会自动将我们的输入法id作为一个键值添加到该项中，
	//但卸载了我们的输入法后系统并不会清除该项，造成再次打开控制面板\时钟、语言和区域\语言\语言
	//选项时，其中残留着一个“不可用的输入法”。注意，每个输入法对应的键值的数据表示该输入法在语
	//言选项中输入法的排序序号，我们将键值删除后可能造成序号不连续，但并无关系，系统会在下一次开
	//打控制面板\时钟、语言和区域\语言\语言选项时自动根据剩余的输入法重新为键值指定序号
	_stprintf_s(key_str, _SizeOf(key_str), _T("Control Panel\\International\\User Profile\\zh-Hans-CN"));
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, key_str, 0, KEY_ALL_ACCESS, &hkey))
	{
		for (key_no = 0;; key_no++)
		{
			//注意此处的size是字符数，和RegSetValueEx及RegQueryValueEx均不同
			//A pointer to a variable that specifies the size of the buffer pointed to by the 
			//lpValueName parameter, in characters. When the function returns, the variable
			//receives the number of characters stored in the buffer, not including the terminating
			//null character.
			size = _SizeOf(hkl_value);
			lres = RegEnumValue(hkey, key_no, hkl_value, &size, 0, NULL, NULL, NULL);

			if (ERROR_NO_MORE_ITEMS == lres)
			{
				break;
			}
			else if (ERROR_SUCCESS == lres)
			{
				layout_id = _tcsstr(hkl_value, _T("0804:"));

				if (layout_id && !(layout_id - hkl_value))
				{
					layout_id += _tcslen(_T("0804:"));

					//找到我们自己的，删除
					if (!_tcscmp(layout_id, hkl_tag_str))
					{
						RegDeleteValue(hkey, hkl_value);
					}
				}
			}
		}
	}

	//在HKEY_CURRENT_USER\Control Panel\International\User Profile\zh-Hans中删除我们的输入法
	//有时HKEY_CURRENT_USER\Control Panel\International\User Profile下只有zh-Hans-CN，目前尚不清楚此
	//二者的区别
	_stprintf_s(key_str, _SizeOf(key_str), _T("Control Panel\\International\\User Profile\\zh-Hans"));
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, key_str, 0, KEY_ALL_ACCESS, &hkey))
	{
		for (key_no = 0;; key_no++)
		{
			size = _SizeOf(hkl_value);
			lres = RegEnumValue(hkey, key_no, hkl_value, &size, 0, NULL, NULL, NULL);

			if (ERROR_NO_MORE_ITEMS == lres)
			{
				break;
			}
			else if (ERROR_SUCCESS == lres)
			{
				layout_id = _tcsstr(hkl_value, _T("0804:"));

				if (layout_id && !(layout_id - hkl_value))
				{
					layout_id += _tcslen(_T("0804:"));

					//找到我们自己的，删除
					if (!_tcscmp(layout_id, hkl_tag_str))
					{
						RegDeleteValue(hkey, hkl_value);
					}
				}
			}
		}
	}

	RegCloseKey(hkey);

	//删除输入法项
	RegDeleteKey(HKEY_LOCAL_MACHINE, key_str_save);

	return 0;
}

int _tmain(int argc, TCHAR* argv[])
{
	if (argc != 3)
		error_exit(ERR_PARAMETER);

	if (!_tcscmp(argv[1], _T("/u")))
	{
		ime_file_name = argv[2];
		uninstall_ime();
		return 0;
	}

	//安装输入法
	if (!_tcscmp(argv[1], _T("/i")))
	{
		ime_file_name = argv[2];
		install_ime();
		return 0;
	}

	error_exit(ERR_PARAMETER);
	return 0;
}
