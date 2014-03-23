/*	Èí¼üÅÌÄ£¿é
 */

#include <config.h>
#include <utility.h>
#include <win32/softkbd.h>
#include <assert.h>

static int current_index = -1;	/* µ±Ç°Èí¼üÅÌË÷Òý */

#pragma data_seg(HYPIM_SHARED_SEGMENT)

/* Èí¼üÅÌ¶ÔÓ¦¼ü±í */
static const TCHAR softkbd_vkey_map[] =
{
	' ', '0', '1', '2', '3', '4', '5','6', '7', '8', '9',
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 
	'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
	0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xc0, 0xdb, 0xdc, 0xdd, 0xde
};

/* Õý³£×Ö·û */
static TCHAR softkbd_char_map[SOFTKBD_NUMBER * 2][SOFTKBD_KEY_NUMBER + 1] =
{
/*  "¡¡£°£±£²£³£´£µ£¶£·£¸£¹£á£â£ã£ä£å£æ£ç£è£é£ê£ë£ì£í£î£ï£ð£ñ£ò£ó£ô£õ£ö£÷£ø£ù£ú£»£½£¬£­£®£¯£à£Û£Ü£Ý£§", */
	TEXT(" ¢Î¢Å¢Æ¢Ç¢È¢É¢Ê¢Ë¢Ì¢Í¡ö¦Á¡Æ¡ø¢³¡ñ¡ï¡¾¢¸¡¿¡º¡®¦Ã¦Â¢¹¢º¢±¢´¡ô¢µ¢·¡ì¢²¡ë¢¶¡ç£»£½£¬£­¡££¯£à£Û¡¢£Ý¡¯"),
	TEXT(" ¢î¢å¢æ¢ç¢è¢é¢ê¢ë¢ì¢í¡õ¡À¡Ç¡÷¢ó¡ð¡î¡¼¢ø¡½¡»¡°¡Â¡Á¢ù¢ú¢ñ¢ô¡ó¢õ¢÷¡ù¢ò¡æ¢ö£¤£º£«¡¶£ß¡·£¿¡«£û¡­£ý¡±"),
	TEXT(" ¡¥£¬¡¢£»£º£¿£¡¡­¡ª¡¤¡²£¨¡¾¡´¡°¡µ¡¶¡·¡Ã¡¸¡¹¡º£Û£©£¢£§¡®¡±¡³¡©¡¬¡¿¡¯¡½¡«¡¼¡»¡§£Ý¡¦£û£ý¡££à¡¨£ü£®"),
	TEXT("                                                "),
	TEXT("           ¡ö£Þ£À¡ø¡î¡ù¡ú¡û¡ó¡ü¡ý¡þ£þ£ß¡ô¡õ¡ì¡ï¡÷¡ð¡ò£Ü¡í£¦¡ñ££           "),
	TEXT("                                                "),
	TEXT(" ©¨©±©²©³©´©µ©¶©· ©¤©Ð©ä©â©Ò©Â©Ó©Ô©Õ©Ç©Ö©× ©æ©å ©¦©À©Ã©Ñ©Ä©Æ©ã©Á©á©Å©à  ©ç©¬  ©°©ª ©® "),
	TEXT(" ©©©¹©º©»©¼©½©¾©¿ ©¥©Ø©ì©ê©Ú©Ê©Û©Ü©Ý©Ï©Þ©ß ©î©í ©§©È©Ë©Ù©Ì©Î©ë©É©é©Í©è  ©ï©­  ©¸©« ©¯ "),
	TEXT(" ¢û¢ò¢ó¢ô¢õ¢ö¢÷¢ø¢ù¢ú¢å¢É¢Ç¢ç¢³¢è¢é¢ê¢¸¢ë¢ì¢í¢Ë¢Ê¢¹¢º¢±¢´¢æ¢µ¢·¢È¢²¢Æ¢¶¢Å¢î ¢Ì¢ü¢Í¢Î¢ñ    "),
	TEXT("           ¢Ù¢Ó¢Ñ¢Û¢½¢Ü¢Ý¢Þ¢Â¢ß¢à¢á¢Õ¢Ô¢Ã¢Ä¢»¢¾¢Ú¢¿¢Á¢Ò¢¼¢Ð¢À¢Ï¢â ¢Ö ¢×¢Ø     "),
	TEXT("  ¡Ô¡Ù£½¡Ü¡Ý£¼£¾¡Ú¡Û¡Ä¡Ñ¡Ï¡Æ£­¡Ç¡È¡É¡Ò¡Ê ¡ß¡×¡Õ¡Ó¡Ø¡À¡Á¡Å¡Â ¡Ð£«¡Î£¯¡Í¡à  ¡Ë¡Ì ¡Ö¡Þ   "),
	TEXT("                                                "),
	TEXT(" ¡è¡ã¡ä¡å¡ç¡ê£¤¡ë£¥¡æ°Ù ºÁÍò¶þÒÚÕ×¼ªÆßÌ«ÅÄ°¬  °Ë¾Å¡ðÈýÇ§ËÄÁùÎ¢Ò»ÀåÎå·Ö   ¡é   Ê®   "),
	TEXT("           °Û   ·¡   Æâ     °Æ¾ÁÁãÈþÇªËÁÂ½ Ò¼ Îé        Ê°   "),
	TEXT("           ¨¥ ¨³¨§¨£¨¨ ¨©¨¯¨ª¨«¨¬¨¶¨µ¨° ¨¡¨¤¨¦ ¨®¨´¨¢¨²¨­¨±  ¨· ¨¸¨¹ ¨º   "),
	TEXT("                                                "),
	TEXT(" ¨â¨Å¨É¡¦¨A¨Ó¨@¨B¨Ú¨Þ¨Ç¨Ö¨Ï¨Î¨Í¨Ñ¨Õ¨Ø¨Û¨è¨Ü¨à¨é¨Ù¨ß¨ã¨Æ¨Ð¨Ë¨Ô¨ç¨Ò¨Ê¨Ì¨×¨È¨ä ¨Ý¨æ¨á¨å    "),
	TEXT("                                                "),
	TEXT("  ¤£¤¥¤§¤©¤«¤­¤¯¤±¤³¤Ê¤â¤à¤Ì¤¹¤Í¤Î¤Ï¤Ä¤Ò¤Õ¤Ø¤å¤ã¤Ã¤Æ¤µ¤»¤Ë¤½¤Á¤á¤·¤ß¤¿¤Þ¤Û ¤ç¤ó¤î¤ò¤¡¤È ¤ð¤ñ"),
	TEXT("  ¤¤¤¦¤¨¤ª¤¬¤®¤°¤²¤´¤Ñ¤í¤ë¤×¤º¤Ú¤Ý¤Ð¤Å¤Ó¤Ö¤Ù¤æ¤ä ¤Ç¤¶¤¼¤Ô¤¾¤Â¤ì¤¸¤ê¤À¤é¤Ü ¤è ¤ï ¤¢¤É   "),
	TEXT(" ¥±¥£¥¥¥ô¥§¥©¥«¥õ¥­¥¯¥Ê¥â¥à¥Ì¥¹¥Í¥Î¥Ï¥Ä¥Ò¥Õ¥Ø¥å¥ã¥Ã¥Æ¥µ¥»¥Ë¥½¥Á¥á¥·¥ß¥¿¥Þ¥Û¥³¥ç¥ö¥î¥ò¥¡¥È¥ó¥ð¥ñ"),
	TEXT(" ¥²¥¤¥¦ ¥¨¥ª¥¬ ¥®¥°¥Ñ¥í¥ë¥×¥º¥Ú¥Ý¥Ð¥Å¥Ó¥Ö¥Ù¥æ¥ä ¥Ç¥¶¥¼¥Ô¥¾¥Â¥ì¥¸¥ê¥À¥é¥Ü¥´¥è ¥ï ¥¢¥É   "),
	TEXT("           ¦Ê¦Ö¦Ô¦Ì¦Ã¦Í¦Î¦Ï¦È¦Ð¦Ñ ¦Ø¦×¦É ¦Á¦Ä¦Ë¦Å¦Ç¦Õ¦Â¦Ó¦Æ¦Ò           "),
	TEXT("           ¦ª¦¶¦´¦¬¦£¦­¦®¦¯¦¨¦°¦± ¦¸¦·¦© ¦¡¦¤¦«¦¥¦§¦µ¦¢¦³¦¦¦²           "),
	TEXT("           §Ý§ì§ê§ß§Ó§à§á§â§Ø§ã§ä§å§î§í§Ù§Ú§Ñ§Ô§Þ§Õ§×§ë§Ò§é§Ö§è§æ §ï §ð§ñ §Û §Ü§ç"),
	TEXT("           §­§¼§º§¯§£§°§±§²§¨§³§´§µ§¾§½§©§ª§¡§¤§®§¥§§§»§¢§¹§¦§¸§¶ §¿ §À§Á §« §¬§·"),
    TEXT(" 0123456789abcdefghijklmnopqrstuvwxyz;=,-./`[\\]'"),
	TEXT(" )!@#$%^&*(ABCDEFGHIJKLMNOPQRSTUVWXYZ:+<_>?~{|}\""),
    TEXT(" 0123456789abcdefghijklmnopqrstuvwxyz;=,-./`[\\]'"),
	TEXT(" )!@#$%^&*(ABCDEFGHIJKLMNOPQRSTUVWXYZ:+<_>?~{|}\""),
};
#pragma data_seg()

static HWND	softkbd_window;

/*	¶¨ÒåÈí¼üÅÌ×Ö·û
 *	²ÎÊý£º
 *		index		µ±Ç°Èí¼üÅÌµÄË÷Òý
 */
static void SetSoftKBDData(PIMCONTEXT *context, int index)
{
	int i;
	SOFTKBDDATA softkbd_data[2];
	LPSOFTKBDDATA data;

	if (current_index == index)
		return;

	current_index = index;

	data = &softkbd_data[0];
	data->uCount = 2;
    for (i = 0; i < SOFTKBD_KEY_NUMBER; i++)
	{
		data->wCode[0][softkbd_vkey_map[i]] = softkbd_char_map[index * 2][i];
		data->wCode[1][softkbd_vkey_map[i]] = softkbd_char_map[index * 2 + 1][i];
    }

	pim_config->soft_kbd_index = index;
	SaveConfigInternal(pim_config);
    SendMessage(softkbd_window, WM_IME_CONTROL, IMC_SETSOFTKBDDATA, (LPARAM) data);
}

/*	´´½¨²¢Èí¼üÅÌ´°¿Ú
 *	²ÎÊý£º
 *		ui_window		UI´°¿Ú¾ä±ú
 */
void CreateSoftKBDWindow(PIMCONTEXT *context)
{
	RECT softkbd_rect, status_rect, work_rect;
	POINT point;
	int	 x, y;

	softkbd_window = ImmCreateSoftKeyboard(SOFTKEYBOARD_TYPE_C1, context->ui_context->ui_window, 0, 0);

	GetWindowRect(softkbd_window, &softkbd_rect);
	GetWindowRect(context->ui_context->status_window, &status_rect);

	point.x = status_rect.left;
	point.y = status_rect.top;

	work_rect = GetMonitorRectFromPoint(point);

	x = work_rect.right - (softkbd_rect.right - softkbd_rect.left);
	y = status_rect.top - (softkbd_rect.bottom - softkbd_rect.top);

	if (y < work_rect.top)
		y = status_rect.bottom;

	SetWindowPos(softkbd_window, 0, x, y, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
}

/* ÏÔÊ¾Èí¼üÅÌ
 */
void ShowSoftKBDWindow()
{
	if (softkbd_window)
		ImmShowSoftKeyboard(softkbd_window, SW_SHOWNOACTIVATE);
}

/*	Òþ²ØÈí¼üÅÌ
 */
void HideSoftKBDWindow()
{
	if (softkbd_window)
		ImmShowSoftKeyboard(softkbd_window, SW_HIDE);
}

/*	²»Ñ¡ÔñÈí¼üÅÌ
 */
void DeSelectSoftKBD()
{
	if (softkbd_window)
		ImmDestroySoftKeyboard(softkbd_window);

	softkbd_window = 0;
	current_index  = -1;
}

/* Ñ¡ÔñÈí¼üÅÌ */
void SelectSoftKBD(PIMCONTEXT *context, HIMC hIMC, int index)
{
	if (current_index != index)
	{
		if (softkbd_window)
			DeSelectSoftKBD();
	}

	if (!softkbd_window)
		CreateSoftKBDWindow(context);

	SetSoftKBDData(context, index);
}

/*	¼ìË÷Èí¼üÅÌµÄ·ûºÅ
 */
int GetSoftKeyboardSymbol(PIMCONTEXT *context, int virtual_key, int key_flag, TCHAR *symbol)
{
	int i;

	if (key_flag & KEY_CONTROL)
		return 0;

	for (i = 0; i < SOFTKBD_KEY_NUMBER; i++)
		if ((BYTE)virtual_key == softkbd_vkey_map[i])
			break;

	if (i == SOFTKBD_KEY_NUMBER)
		return 0;

	if (((key_flag & KEY_SHIFT) && !(key_flag & KEY_CAPITAL)) ||
		(!(key_flag & KEY_SHIFT) && (key_flag & KEY_CAPITAL)))
	{
		symbol[0] = softkbd_char_map[context->softkbd_index * 2 + 1][i];
		symbol[1] = 0;
	}
	else
	{
		symbol[0] = softkbd_char_map[context->softkbd_index * 2][i];
		symbol[1] = 0;
	}

	if (symbol[0] == ' ' && symbol[1] == ' ')
		symbol[0] = 0;

	//Ó¢ÎÄ¼üÅÌ
	if (context->softkbd_index == SOFTKBD_NUMBER - 2)
		symbol[1] = 0;

	return 1;
}