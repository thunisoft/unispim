/*	IME启动代码
 */
#include <config.h>
#include <utility.h>
#include <pim_resource.h>
#include <win32/pim_ime.h>
#include <time.h>
#include <icw.h>
#include <share_segment.h>

//全局instance
HINSTANCE	global_instance = 0;

//#pragma data_seg(HYPIM_SHARED_SEGMENT)
//static int	process_count = 0;			//与IME连接的进程计数
//static int	first_use	  = 1; 			//第一次使用IME
//#pragma data_seg()

int resource_thread_finished = 0;

DWORD WINAPI LoadResources(LPVOID dummy)
{
	Log(LOG_ID, L"启动装载资源线程");
	PIM_LoadResources();
	resource_thread_finished = 1;
	Log(LOG_ID, L"结束装载资源线程");

	//加快bigram的装载速度
	MakeBigramFaster();

	return 0;
}

/**	DLL入口
 */
int WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID dummy)
{
	static HANDLE h_thread;
	extern int LoadSPResource();		//由于Hint信息可能在资源没有装载之前显示，所以必须早期调用本函数

	if (!share_segment)		//不存在share_segment就初始化log
		LogInit(1);

	LoadSharedSegment();

	switch (reason)
	{
		case DLL_PROCESS_ATTACH:
			Log(LOG_ID, L"Dll attach, instance:0x%x", instance);

			GetProgramName();

			//创建全局互斥量
			global_instance = instance;

			//配置更新
			MaintainConfig();

			LoadSPResource();					//装载双拼资源

			//装载资源线程
			h_thread = CreateThread(0, 0, LoadResources, 0, 0, 0);

			Log(LOG_ID, L"res thread return:%x", h_thread);

			//增加进程计数
			share_segment->process_count++;

			//注册UI窗口Class
			RegisterUIWindowClass(instance);

			break;

		case DLL_PROCESS_DETACH:
			Log(LOG_ID, L"Dll detach, instance:0x%x", instance);

			share_segment->process_count--;			//减少进程计数

			//清除UI窗口类
			UnregisterUIWindowClass(instance);

			//由于RunDll32之类的程序Load、Free输入法DLL的速度太快
			//可能在DETACH的时候，线程并没有结束，所以必须关闭已经
			//运行的线程
			CloseHandle(h_thread);

			//为避免U盘插入时的RunDll32死机现象，这里应该对resource_thread_finished
			//进行判断
			if (resource_thread_finished)
			{
				//清理输入法环境，保存数据
				if (share_segment->process_count)
					PIM_SaveResources();
				else
					PIM_FreeResources();
			}

			SaveConfigInternal(pim_config);

			if (!share_segment->process_count)
			{
				FreeSharedSegment();
				FreeLog();
			}

			break;
    }

    return 1;
}