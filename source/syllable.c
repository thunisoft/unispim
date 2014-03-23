/*	处理音节函数组。
 */
#include <config.h>
#include <utility.h>
#include <assert.h>
#include <zi.h>
#include <tchar.h>
#include <share_segment.h>

//#pragma data_seg(HYPIM_SHARED_SEGMENT)

//int sp_used_u = 1;			//双拼是否使用u作为开头的字母
//int sp_used_v = 0;			//双拼是否使用V作为开头的字母
//int sp_used_i = 0;			//双拼是否使用I作为开头的字母
//int sp_loaded = 0;

//双拼声母表
//TCHAR con_sp_string_save[][8] =
//{
//	TEXT(""),						//CON_NULL
//	TEXT("b"),						//CON_B
//	TEXT("c"),						//CON_C
//	TEXT("ch"),						//CON_CH
//	TEXT("d"),						//CON_D
//	TEXT("f"),						//CON_F
//	TEXT("g"),						//CON_G
//	TEXT("h"),						//CON_H
//	TEXT("j"),						//CON_J
//	TEXT("k"),						//CON_K
//	TEXT("l"),						//CON_L
//	TEXT("m"),						//CON_M
//	TEXT("n"),						//CON_N
//	TEXT("p"),						//CON_P
//	TEXT("q"),						//CON_Q
//	TEXT("r"),						//CON_R
//	TEXT("s"),						//CON_S
//	TEXT("sh"),						//CON_SH
//	TEXT("t"),						//CON_T
//	TEXT("w"),						//CON_W
//	TEXT("x"),						//CON_X
//	TEXT("y"),						//CON_Y
//	TEXT("z"),						//CON_Z
//	TEXT("zh"),						//CON_ZH
//	TEXT(""),						//CON_END
//	TEXT("*"),						//CON_ANY
//};
//TCHAR con_sp_string[sizeof(con_sp_string_save) / _SizeOf(con_sp_string_save[0]) / sizeof(TCHAR)][8] = { 0 };

//双拼韵母表
//TCHAR vow_sp_string_save[][8] =
//{
//	TEXT(""),						//VOW_NULL,
//	TEXT("a"),						//VOW_A,
//	TEXT("ai"),						//VOW_AI
//	TEXT("an"),						//VOW_AN
//	TEXT("ang"),					//VOW_ANG
//	TEXT("ao"),						//VOW_AO
//	TEXT("e"),						//VOW_E
//	TEXT("ei"),						//VOW_EI
//	TEXT("en"),						//VOW_EN
//	TEXT("eng"),					//VOW_ENG
//	TEXT("er"),						//VOW_ER
//	TEXT("i"),						//VOW_I
//	TEXT("ia"),						//VOW_IA
//	TEXT("ian"),					//VOW_IAN
//	TEXT("iang"),					//VOW_IANG
//	TEXT("iao"),					//VOW_IAO
//	TEXT("ie"),						//VOW_IE
//	TEXT("in"),						//VOW_IN
//	TEXT("ing"),					//VOW_ING
//	TEXT("iong"),					//VOW_IONG
//	TEXT("iu"),						//VOW_IU
//	TEXT("o"),						//VOW_O
//	TEXT("ong"),					//VOW_ONG
//	TEXT("ou"),						//VOW_OU
//	TEXT("u"),						//VOW_U
//	TEXT("ua"),						//VOW_UA
//	TEXT("uai"),					//VOW_UAI
//	TEXT("uan"),					//VOW_UAN
//	TEXT("uang"),					//VOW_UANG
//	TEXT("ue"),						//VOW_UE
//	TEXT("ui"),						//VOW_UI
//	TEXT("un"),						//VOW_UN
//	TEXT("uo"),						//VOW_UO
//	TEXT("v"),						//VOW_V
//	TEXT(""),						//VOW_END
//	TEXT(""),						//VOW_ANY
//};
//TCHAR vow_sp_string[sizeof(vow_sp_string_save) / _SizeOf(vow_sp_string_save[0]) / sizeof(TCHAR)][8] = { 0 };

//双拼音节表
//TCHAR vow_sp_string_single_save[][8] =
//{
//	TEXT(""),						//VOW_NULL,
//	TEXT("a"),						//VOW_A,
//	TEXT("ai"),						//VOW_AI
//	TEXT("an"),						//VOW_AN
//	TEXT("ang"),					//VOW_ANG
//	TEXT("ao"),						//VOW_AO
//	TEXT("e"),						//VOW_E
//	TEXT("ei"),						//VOW_EI
//	TEXT("en"),						//VOW_EN
//	TEXT("eng"),					//VOW_ENG
//	TEXT("er"),						//VOW_ER
//	TEXT("i"),						//VOW_I
//	TEXT("ia"),						//VOW_IA
//	TEXT("ian"),					//VOW_IAN
//	TEXT("iang"),					//VOW_IANG
//	TEXT("iao"),					//VOW_IAO
//	TEXT("ie"),						//VOW_IE
//	TEXT("in"),						//VOW_IN
//	TEXT("ing"),					//VOW_ING
//	TEXT("iong"),					//VOW_IONG
//	TEXT("iu"),						//VOW_IU
//	TEXT("o"),						//VOW_O
//	TEXT("ong"),					//VOW_ONG
//	TEXT("ou"),						//VOW_OU
//	TEXT("u"),						//VOW_U
//	TEXT("ua"),						//VOW_UA
//	TEXT("uai"),					//VOW_UAI
//	TEXT("uan"),					//VOW_UAN
//	TEXT("uang"),					//VOW_UANG
//	TEXT("ue"),						//VOW_UE
//	TEXT("ui"),						//VOW_UI
//	TEXT("un"),						//VOW_UN
//	TEXT("uo"),						//VOW_UO
//	TEXT("v"),						//VOW_V
//	TEXT(""),						//VOW_END
//	TEXT(""),						//VOW_ANY
//};
//TCHAR vow_sp_string_single[sizeof(vow_sp_string_single_save) / _SizeOf(vow_sp_string_single_save[0]) / sizeof(TCHAR)][8] = { 0 };

//#pragma data_seg()

/*	拼音音调符号
	ɑāáǎà
	eēéěè
	iīíǐì
	oōóǒò
	uūúǔù
	üǖǘǚǜ
	ˉˊˇˋ
*/

//SPECIALSYLLABLE special_syllables[] =
//{
//	//wa'nan->wan'an
//	2, CON_W,  VOW_A,    TONE_0, CON_N,    VOW_AN,  TONE_0, CON_NULL, VOW_NULL, TONE_0, CON_NULL, VOW_NULL, TONE_0, CON_NULL, VOW_NULL, TONE_0, 
//	2, CON_W,  VOW_AN,   TONE_0, CON_NULL, VOW_AN,  TONE_0, CON_NULL, VOW_NULL, TONE_0, CON_NULL, VOW_NULL, TONE_0, CON_NULL, VOW_NULL, TONE_0, 
//
//	//nian'pai->ni'an'pai
//	2, CON_N,  VOW_IAN,  TONE_0, CON_P,    VOW_AI,  TONE_0, CON_NULL, VOW_NULL, TONE_0, CON_NULL, VOW_NULL, TONE_0, CON_NULL, VOW_NULL, TONE_0, 
//	3, CON_N,  VOW_I,    TONE_0, CON_NULL, VOW_AN,  TONE_0, CON_P,    VOW_AI,   TONE_0, CON_NULL, VOW_NULL, TONE_0, CON_NULL, VOW_NULL, TONE_0, 
//
//	//la'qiao->la'qi'ao
//	//2, CON_L,  VOW_A,    TONE_0, CON_Q,    VOW_IAO, TONE_0, CON_NULL, VOW_NULL, TONE_0, CON_NULL, VOW_NULL, TONE_0, CON_NULL, VOW_NULL, TONE_0,
//	//3, CON_L,  VOW_A,    TONE_0, CON_Q,	   VOW_I,   TONE_0, CON_NULL, VOW_AO,   TONE_0, CON_NULL, VOW_NULL, TONE_0, CON_NULL, VOW_NULL, TONE_0, 
//
//	//xi'nan'zhuang->xin'an'zhuang
//	3, CON_X,  VOW_I,    TONE_0, CON_N,    VOW_AN,  TONE_0, CON_ZH,   VOW_UANG, TONE_0, CON_NULL, VOW_NULL, TONE_0, CON_NULL, VOW_NULL, TONE_0, 
//	3, CON_X,  VOW_IN,   TONE_0, CON_NULL, VOW_AN,  TONE_0, CON_ZH,   VOW_UANG, TONE_0, CON_NULL, VOW_NULL, TONE_0, CON_NULL, VOW_NULL, TONE_0, 
//
//	//shan'xian'kang->shan'xi'an'kang
//	3, CON_SH, VOW_AN,   TONE_0, CON_X,    VOW_IAN, TONE_0, CON_K,    VOW_ANG,  TONE_0, CON_NULL, VOW_NULL, TONE_0, CON_NULL, VOW_NULL, TONE_0, 
//	4, CON_SH, VOW_AN,   TONE_0, CON_X,    VOW_I,   TONE_0, CON_NULL, VOW_AN,   TONE_0, CON_K,    VOW_ANG,  TONE_0, CON_NULL, VOW_NULL, TONE_0, 
//
//	//gong'suan'jian->gong'su'an'jian
//	3, CON_G,  VOW_ONG,  TONE_0, CON_S,    VOW_UAN, TONE_0, CON_J,    VOW_IAN,  TONE_0, CON_NULL, VOW_NULL, TONE_0, CON_NULL, VOW_NULL, TONE_0, 
//	4, CON_G,  VOW_ONG,  TONE_0, CON_S,    VOW_U,   TONE_0, CON_NULL, VOW_AN,   TONE_0, CON_J,	  VOW_IAN,  TONE_0, CON_NULL, VOW_NULL, TONE_0, 
//
//	//qi'suan'jian->qi'su'an'jian
//	3, CON_Q,  VOW_I,    TONE_0, CON_S,    VOW_UAN, TONE_0, CON_J,    VOW_IAN,  TONE_0, CON_NULL, VOW_NULL, TONE_0, CON_NULL, VOW_NULL, TONE_0, 
//	4, CON_Q,  VOW_I,    TONE_0, CON_S,    VOW_U,   TONE_0, CON_NULL, VOW_AN,   TONE_0, CON_J,	  VOW_IAN,  TONE_0, CON_NULL, VOW_NULL, TONE_0, 
//
//	//qi'suan'jian->qi'su'an'jian
//	3, CON_SH, VOW_ANG,  TONE_0, CON_S,    VOW_UAN, TONE_0, CON_J,    VOW_IAN,  TONE_0, CON_NULL, VOW_NULL, TONE_0, CON_NULL, VOW_NULL, TONE_0, 
//	4, CON_SH, VOW_ANG,  TONE_0, CON_S,    VOW_U,   TONE_0, CON_NULL, VOW_AN,   TONE_0, CON_J,	  VOW_IAN,  TONE_0, CON_NULL, VOW_NULL, TONE_0, 
//
//	//sui'yue'ran->sui'yu'er'an
//	3, CON_S,  VOW_UI,   TONE_0, CON_Y,    VOW_UE,  TONE_0, CON_R,    VOW_AN,   TONE_0, CON_NULL, VOW_NULL, TONE_0, CON_NULL, VOW_NULL, TONE_0, 
//	4, CON_S,  VOW_UI,   TONE_0, CON_Y,    VOW_U,   TONE_0, CON_NULL, VOW_ER,   TONE_0, CON_NULL, VOW_AN,   TONE_0, CON_NULL, VOW_NULL, TONE_0, 
//
//	//xin'gao'qiao->xin'gao'qi'ao
//	3, CON_X,  VOW_IN,   TONE_0, CON_G,    VOW_AO,  TONE_0, CON_Q,    VOW_IAO,  TONE_0, CON_NULL, VOW_NULL, TONE_0, CON_NULL, VOW_NULL, TONE_0, 
//	4, CON_X,  VOW_IN,   TONE_0, CON_G,    VOW_AO,  TONE_0, CON_Q,    VOW_I,    TONE_0, CON_NULL, VOW_AO,   TONE_0, CON_NULL, VOW_NULL, TONE_0, 
//
//	//xiang'gua'nan'li->xiang'guan'an'li
//	4, CON_X,  VOW_IANG, TONE_0, CON_G,    VOW_UA,  TONE_0, CON_N,    VOW_AN,   TONE_0, CON_L,    VOW_I,    TONE_0, CON_NULL, VOW_NULL, TONE_0, 
//	4, CON_X,  VOW_IANG, TONE_0, CON_G,    VOW_UAN, TONE_0, CON_NULL, VOW_AN,   TONE_0, CON_L,    VOW_I,    TONE_0, CON_NULL, VOW_NULL, TONE_0, 
//
//	//xin'xian'quan->xin'xi'an'quan
//	3, CON_X,  VOW_IN,	 TONE_0, CON_X,	   VOW_IAN, TONE_0, CON_Q,	  VOW_UAN,  TONE_0, CON_NULL, VOW_NULL, TONE_0, CON_NULL, VOW_NULL, TONE_0, 
//	4, CON_X,  VOW_IN,	 TONE_0, CON_X,	   VOW_I,   TONE_0, CON_NULL, VOW_AN,   TONE_0, CON_Q,	  VOW_UAN,  TONE_0, CON_NULL, VOW_NULL, TONE_0, 
//
//	//zhen'zhen'gai'ni->zhen'zheng'ai'ni
//	4, CON_ZH, VOW_EN,   TONE_0, CON_ZH,   VOW_EN,  TONE_0, CON_G,    VOW_AI,   TONE_0, CON_N,    VOW_I,    TONE_0, CON_NULL, VOW_NULL, TONE_0, 
//	4, CON_ZH, VOW_EN,   TONE_0, CON_ZH,   VOW_ENG, TONE_0, CON_NULL, VOW_AI,   TONE_0, CON_N,    VOW_I,    TONE_0, CON_NULL, VOW_NULL, TONE_0, 
//
//	//zhen'gan'bu'jiu'ban->zheng'an'bu'jiu'ban
//	5, CON_ZH, VOW_EN,   TONE_0, CON_G,    VOW_AN,  TONE_0, CON_B,    VOW_U,    TONE_0, CON_J,    VOW_IU,   TONE_0, CON_B,    VOW_AN,   TONE_0, 
//	5, CON_ZH, VOW_ENG,  TONE_0, CON_NULL, VOW_AN,  TONE_0, CON_B,    VOW_U,    TONE_0, CON_J,    VOW_IU,   TONE_0, CON_B,    VOW_AN,   TONE_0, 
//};

typedef struct tagSMALLSYLLABLEITEM
{
	SYLLABLE syllable;
	SYLLABLE small_syllables[2];
}SMALLSYLLABLEITEM;

//似乎并未包含所有情况，如shuang->shu'ang，可以考虑酌情增加
static SMALLSYLLABLEITEM small_syllable_items[] =
{
	//ao -> a'o
	{ { CON_NULL, VOW_AO, },{ { CON_NULL, VOW_A, },	{ CON_NULL, VOW_O,  },  }, },
	//bian -> bi'an
	{ { CON_B, VOW_IAN, },	{ { CON_B, VOW_I, },	{ CON_NULL, VOW_AN, },  }, },
	//biao -> bi'ao
	{ { CON_B, VOW_IAO, },	{ { CON_B, VOW_I, },	{ CON_NULL, VOW_AO, },  }, },
	//cuan -> cu'an
	{ { CON_C, VOW_UAN, },	{ { CON_C, VOW_U, },	{ CON_NULL, VOW_AN, },  }, },
	//dia -> di'a
	{ { CON_D, VOW_IA, },	{ { CON_D, VOW_I, },	{ CON_NULL, VOW_A, },   }, },
	//dian -> di'an
	{ { CON_D, VOW_IAN, },	{ { CON_D, VOW_I, },	{ CON_NULL, VOW_AN, },  }, },
	//diao -> di'ao
	{ { CON_D, VOW_IAO, },	{ { CON_D, VOW_I, },	{ CON_NULL, VOW_AO, },	}, },
	//die -> di'e
	{ { CON_D, VOW_IE, },	{ { CON_D, VOW_I, },	{ CON_NULL, VOW_E, },	}, },
	//duan -> du'an
	{ { CON_D, VOW_UAN, },	{ { CON_D, VOW_U, },	{ CON_NULL, VOW_AN, },	}, },
	//gua -> gu'a
	{ { CON_G, VOW_UA, },	{ { CON_G, VOW_U, },	{ CON_NULL, VOW_A, },	}, },
	//guai -> gu'ai
	{ { CON_G, VOW_UAI, },	{ { CON_G, VOW_U, },	{ CON_NULL, VOW_AI, },	}, },
	//guan -> gu'an
	{ { CON_G, VOW_UAN, },	{ { CON_G, VOW_U, },	{ CON_NULL, VOW_AN, },	}, },
	//huan -> hu'a
	{ { CON_H, VOW_UA, },	{ { CON_H, VOW_U, },	{ CON_NULL, VOW_A, },	}, },
	//huai -> hu'ai
	{ { CON_H, VOW_UAI, },	{ { CON_H, VOW_U, },	{ CON_NULL, VOW_AI, },	}, },
	//huan -> hu'an
	{ { CON_H, VOW_UAN, },	{ { CON_H, VOW_U, },	{ CON_NULL, VOW_AN, },	}, },
	//huo -> hu'o
	{ { CON_H, VOW_UO, },	{ { CON_H, VOW_U, },	{ CON_NULL, VOW_O, },	}, },
	//jia -> ji'a
	{ { CON_J, VOW_IA, },	{ { CON_J, VOW_I, },	{ CON_NULL, VOW_A, },	}, },
	//jian -> ji'an
	{ { CON_J, VOW_IAN, },	{ { CON_J, VOW_I, },	{ CON_NULL, VOW_AN, },	}, },
	//jiang -> ji'ang
	{ { CON_J, VOW_IANG, }, { { CON_J, VOW_I, },	{ CON_NULL, VOW_ANG, }, }, },
	//jiao -> ji'ao
	{ { CON_J, VOW_IAO, },  { { CON_J, VOW_I, },	{ CON_NULL, VOW_AO, },  }, },
	//jie -> ji'e
	{ { CON_J, VOW_IE,  },	{ { CON_J, VOW_I, },	{ CON_NULL, VOW_E,  },  }, },
	//jue -> ju'e
	{ { CON_J, VOW_UE,  },	{ { CON_J, VOW_U, },	{ CON_NULL, VOW_E,  },  }, },
	//juan -> ju'an
	{ { CON_J, VOW_UAN, },	{ { CON_J, VOW_U, },	{ CON_NULL, VOW_AN, },	}, },
	//kua -> ku'a
	{ { CON_K, VOW_UA,  },  { { CON_K, VOW_U, },    { CON_NULL, VOW_A,  },  }, },
	//kuai -> ku'ai
	{ { CON_K, VOW_UAI,  }, { { CON_K, VOW_U, },	{ CON_NULL, VOW_AI,  }, }, },
	//kuo -> ku'o
	{ { CON_K, VOW_UO,  },  { { CON_K, VOW_U, },	{ CON_NULL, VOW_O,  },  }, },
	//lia -> li'a
	{ { CON_L, VOW_IA, },	{ { CON_L, VOW_I, },	{ CON_NULL, VOW_A, },  },  },
	//lian -> li'an
	{ { CON_L, VOW_IAN, },	{ { CON_L, VOW_I, },	{ CON_NULL, VOW_AN, },  }, },
	//liang -> li'ang
	{ { CON_L, VOW_IANG, }, { { CON_L, VOW_I, },	{ CON_NULL, VOW_ANG, }, }, },
	//liao -> li'ao
	{ { CON_L, VOW_IAO, },	{ { CON_L, VOW_I, },	{ CON_NULL, VOW_AO, },  }, },
	//lie -> li'e
	{ { CON_L, VOW_IE, },	{ { CON_L, VOW_I, },	{ CON_NULL, VOW_E, },  },  },
	//luan -> lu'an
	{ { CON_L, VOW_UAN, },	{ { CON_L, VOW_U, },	{ CON_NULL, VOW_AN, },  }, },
	//mian -> mi'an
	{ { CON_M, VOW_IAN, },	{ { CON_M, VOW_I, },	{ CON_NULL, VOW_AN, },  }, },
	//miao -> mi'ao
	{ { CON_M, VOW_IAO, },	{ { CON_M, VOW_I, },	{ CON_NULL, VOW_AO, },  }, },
	//mie -> mi'e
	{ { CON_M, VOW_IE, },	{ { CON_M, VOW_I, },	{ CON_NULL, VOW_E, },   }, },
	//nao -> na'o
	{ { CON_N, VOW_AO, },	{ { CON_N, VOW_A, },	{ CON_NULL, VOW_O, },  },  },
	//nian -> ni'an
	{ { CON_N, VOW_IAN, },	{ { CON_N, VOW_I, },	{ CON_NULL, VOW_AN, },  }, },
	//niao -> ni'ao
	{ { CON_N, VOW_IAO, },	{ { CON_N, VOW_I, },	{ CON_NULL, VOW_AO, },  }, },
	//nie -> ni'e
	{ { CON_N, VOW_IE, },	{ { CON_N, VOW_I, },	{ CON_NULL, VOW_E, },   }, },
	//pian -> pi'an
	{ { CON_P, VOW_IAN, },	{ { CON_P, VOW_I, },	{ CON_NULL, VOW_AN, },  }, },
	//piao -> pi'ao
	{ { CON_P, VOW_IAO, },	{ { CON_P, VOW_I, },	{ CON_NULL, VOW_AO, },  }, },
	//pie -> pi'e
	{ { CON_P, VOW_IE, },	{ { CON_P, VOW_I, },	{ CON_NULL, VOW_E, },   }, },
	//qia -> qi'a
	{ { CON_Q, VOW_IA,  }, { { CON_Q, VOW_I,  },	{ CON_NULL, VOW_A,  }, },  },
	//qian -> qi'an
	{ { CON_Q, VOW_IAN,  }, { { CON_Q, VOW_I, },	{ CON_NULL, VOW_AN,  }, }, },
	//qiang-> qi'ang
	{ { CON_Q, VOW_IANG, }, { { CON_Q, VOW_I, },	{ CON_NULL, VOW_ANG, }, }, },
	//qiao -> qi'ao
	{ { CON_Q, VOW_IAO,  }, { { CON_Q, VOW_I, },	{ CON_NULL, VOW_AO,  }, }, },
	//qie -> qi'e
	{ { CON_Q, VOW_IE,  },	{ { CON_Q, VOW_I, },	{ CON_NULL, VOW_E,  },  }, },
	//quan -> qu'an
	{ { CON_Q, VOW_UAN,  },	{ { CON_Q, VOW_U, },	{ CON_NULL, VOW_AN,  }, }, },
	//que -> qu'e
	{ { CON_Q, VOW_UE,  },	{ { CON_Q, VOW_U, },	{ CON_NULL, VOW_E,  },  }, },
	//suan -> su'an
	{ { CON_S, VOW_UAN, },  { { CON_S, VOW_U, },	{ CON_NULL, VOW_AN, },	}, },
	//shao -> sha'o
	{ { CON_SH, VOW_AO, },  { { CON_SH, VOW_A,},	{ CON_NULL, VOW_O, },	}, },
	//shua -> shu'a
	{ { CON_SH, VOW_UA, },  { { CON_SH, VOW_U,},	{ CON_NULL, VOW_A, },	}, },
	//shuan -> shu'an
	{ { CON_SH, VOW_UAN, }, { { CON_SH, VOW_U,},	{ CON_NULL, VOW_AN, },	}, },
	//shuo -> shu'o
	{ { CON_SH, VOW_UO, },  { { CON_SH, VOW_U,},	{ CON_NULL, VOW_O, },	}, },
	//tian -> ti'an
	{ { CON_T, VOW_IAN, },	{ { CON_T, VOW_I, },	{ CON_NULL, VOW_AN, },  }, },
	//tie -> ti'e
	{ { CON_T, VOW_IE, },	{ { CON_T, VOW_I, },	{ CON_NULL, VOW_E, },  },  },
	//tuan -> tu'an
	{ { CON_T, VOW_UAN, },	{ { CON_T, VOW_U, },	{ CON_NULL, VOW_AN, },  }, },
	//tuo -> tu'o
	{ { CON_T, VOW_UO, },	{ { CON_T, VOW_U, },	{ CON_NULL, VOW_O, },   }, },
	//xia -> xi'a
	{ { CON_X, VOW_IA, },	{ { CON_X, VOW_I, },	{ CON_NULL, VOW_A, },  },  },
	//xian -> xi'an
	{ { CON_X, VOW_IAN, },	{ { CON_X, VOW_I, },	{ CON_NULL, VOW_AN, },  }, },
	//xiao -> xi'ao
	{ { CON_X, VOW_IAO, },	{ { CON_X, VOW_I, },	{ CON_NULL, VOW_AO, },  }, },
	//xiang -> xi'ang
	{ { CON_X, VOW_IANG, }, { { CON_X, VOW_I, },	{ CON_NULL, VOW_ANG, }, }, },
	//xie -> xi'e
	{ { CON_X, VOW_IE, },	{ { CON_X, VOW_I, },	{ CON_NULL, VOW_E, },  },  },
	//xuan -> xu'an
	{ { CON_X, VOW_UAN, },  { { CON_X, VOW_U, },	{ CON_NULL, VOW_AN, },  }, },
	//xue -> xu'e
	{ { CON_X, VOW_UE, },   { { CON_X, VOW_U, },	{ CON_NULL, VOW_E, },   }, },
	//yao -> ya'o
	{ { CON_Y, VOW_AO, },	{ { CON_Y, VOW_A, },	{ CON_NULL, VOW_O, },   }, },
	//yuan -> yu'an
	{ { CON_Y, VOW_UAN, },	{ { CON_Y, VOW_U, },	{ CON_NULL, VOW_AN, },  }, },
	//yue -> yu'e
	{ { CON_Y, VOW_UE, },	{ { CON_Y, VOW_U, },	{ CON_NULL, VOW_E, },   }, },
	//zao -> za'o
	{ { CON_Z, VOW_AO, },	{ { CON_Z, VOW_A, },	{ CON_NULL, VOW_O, },  },  },
	//zuan -> zu'an
	{ { CON_Z, VOW_UAN, },	{ { CON_Z, VOW_U, },	{ CON_NULL, VOW_AN, },  }, },
	//zhao -> zha'o
	{ { CON_ZH, VOW_AO, },	{ { CON_ZH, VOW_A,},	{ CON_NULL, VOW_O, },   }, },
	//zhua -> zhu'a
	{ { CON_ZH, VOW_UA, },	{ { CON_ZH, VOW_U,},	{ CON_NULL, VOW_A, },   }, },
	//zhuan -> zhu'an
	{ { CON_ZH, VOW_UAN, },	{ { CON_ZH, VOW_U,},	{ CON_NULL, VOW_AN, },  }, },
};

//大部分双拼方案都存在同一个键对应两个不同韵母的情况，但通常这两个韵母不能和同一个声母组成
//音节，因此从双拼到音节的解析结果是唯一的。但也有一些双拼方案(如蓝天双拼)同一个键(k)对应的
//两个韵母(en，ia)可以和同一个声母组成音节(den，dia)，即从双拼到音节的解析结果不唯一。目前
//代码的解析部分ParsePinYinStringReverse不方便返回多种解析结果，改动起来代价过大，考虑到在
//一个合理的双拼方案中，这种解析结果不唯一的情况应该是少数，索性将其作为一种特殊的模糊音处
//理，即den->dia的模糊(按目前算法总是解析出den，因此设置单向模糊即可)，就如同配置面板中的
//wang->huang模糊一样。每一个双拼拼音被解析为sp_fuzzy_syllables中的一项(最多32项)，如果拼音
//存在多种解析，则作为模糊音，每项最多支持4个模糊音
#define MAX_SP_FUZZY_SYLLABLE_COUNT    4

typedef struct tagSPFUZZYMAPITEM 
{
	SYLLABLE sp_fuzzy_syllables[MAX_SP_FUZZY_SYLLABLE_COUNT];    //相互模糊的音节
	int      syllable_map_index[MAX_SP_FUZZY_SYLLABLE_COUNT];    //当前音节在share_segment->syllable_map中的下标
	int      sp_fuzzy_syllable_count;                            //fuzzy_syllable中实际音节个数
} SPFUZZYMAPITEM;

SPFUZZYMAPITEM sp_fuzzy_map[MAX_SYLLABLE_PER_INPUT];
int sp_fuzzy_map_items;

/*	判断第一个音节参数所指示的声母集合是否包含第二个音节参数的声母。
 *	参数：
 *		syllable				源音节
 *		checked_syllable		被检查音节
 *		fuzzy_mode				模糊设置
 *	返回：
 *		1			包含
 *		0			不包含
 */
int ContainCon(SYLLABLE syllable, SYLLABLE checked_syllable, int fuzzy_mode)
{
//方便程序编写，建立两个宏。
#define	CHECK_FUZZY(fuzzy_bit, con_value)	((fuzzy_mode & fuzzy_bit) && (checked_syllable.con == con_value))

	if (syllable.con == checked_syllable.con)
		return 1;

	if (syllable.con == CON_ANY)
		return 1;

	//被检查的声母为空则为不包含
	if (checked_syllable.con == CON_NULL)
		return 0;

	switch(syllable.con)
	{
	case CON_Z:
		if (syllable.vow == VOW_NULL &&	CHECK_FUZZY(FUZZY_ZCS_IN_CI, CON_ZH))
			return 1;

		return CHECK_FUZZY(FUZZY_Z_ZH, CON_ZH);

	case CON_ZH:
		return CHECK_FUZZY(FUZZY_REV_Z_ZH, CON_Z);

	case CON_C:
		if (syllable.vow == VOW_NULL &&	CHECK_FUZZY(FUZZY_ZCS_IN_CI, CON_CH))
			return 1;

		return CHECK_FUZZY(FUZZY_C_CH, CON_CH);

	case CON_CH:
		return CHECK_FUZZY(FUZZY_REV_C_CH, CON_C);

	case CON_S:
		if (syllable.vow == VOW_NULL &&	CHECK_FUZZY(FUZZY_ZCS_IN_CI, CON_SH))
			return 1;

		return CHECK_FUZZY(FUZZY_S_SH, CON_SH);

	case CON_SH:
		return CHECK_FUZZY(FUZZY_REV_S_SH, CON_S);

	case CON_G:
		return CHECK_FUZZY(FUZZY_G_K, CON_K);

	case CON_K:
		return CHECK_FUZZY(FUZZY_REV_G_K, CON_G);

	case CON_F:		//由于F与H、HU两个音都可以模糊，所以要特殊处理
		if (CHECK_FUZZY(FUZZY_F_H, CON_H))		//如果F、H模糊，并且被检查声母为H，则为包含
			return 1;

		//模糊F与HU，为了能够使“湖南”可以通过"fn"查到，必须假定f = h，再由韵母进行判断。
		return (fuzzy_mode & FUZZY_F_HU) && IS_START_WITH_HU(checked_syllable);

	case CON_H:		//与CON_F同样的理由需要特殊处理
		if (CHECK_FUZZY(FUZZY_REV_F_H, CON_F))
			return 1;

		//如果被检查声母不是F或者没有设置F与HU的模糊，为不包含
		return ((fuzzy_mode & FUZZY_REV_F_HU) && IS_START_WITH_HU(syllable) && checked_syllable.con == CON_F);

	case CON_L:		//L可以与N、R两个音相互模糊，所以特殊处理
		if (CHECK_FUZZY(FUZZY_L_R, CON_R))		//L与R
			return 1;

		return CHECK_FUZZY(FUZZY_L_N, CON_N);	//L与N

	case CON_N:		//不检查N-L/L-R的映射。
		return CHECK_FUZZY(FUZZY_REV_L_N, CON_L);

	case CON_R:		//不检查R-L/L-N的映射
		return CHECK_FUZZY(FUZZY_REV_L_R, CON_L);
	}

	return 0;

//不再使用这两个宏
#undef	CHECK_FUZZY
}

/*	判断HUANG与WANG之间的模糊
 *	参数：
 *		syllable				源音节
 *		checked_syllable		被检查音节
 *		fuzzy_mode				模糊设置
 *	返回：
 *		1			包含
 *		0			不包含
 */
INLINE static int ProcessHUANGAndWANG(SYLLABLE syllable, SYLLABLE checked_syllable, int fuzzy_mode)
{
	if (!(fuzzy_mode & FUZZY_HUANG_WANG))
		return 0;

	//源声母与目标声母都不是W与H
	return (syllable.con == CON_H && syllable.vow == VOW_UANG &&
			checked_syllable.con == CON_W && checked_syllable.vow == VOW_ANG);
}

/*	判断WANG与HUANG之间的模糊
 *	参数：
 *		syllable				源音节
 *		checked_syllable		被检查音节
 *		fuzzy_mode				模糊设置
 *	返回：
 *		1			包含
 *		0			不包含
 */
INLINE static int ProcessWANGAndHUANG(SYLLABLE syllable, SYLLABLE checked_syllable, int fuzzy_mode)
{
	if (!(fuzzy_mode & FUZZY_REV_HUANG_WANG))
		return 0;

	//源声母与目标声母都不是W与H
	return (checked_syllable.con == CON_H && checked_syllable.vow == VOW_UANG &&
			syllable.con == CON_W && syllable.vow == VOW_ANG);
}

/*	判断带有F与HU的模糊音包含关系
 *	U开头的韵母只有：UA, UAI, UAN, UANG, UE, UI, UN, UO七种。
 *	由于音节获取函数返回的音节保证正确性，所以本函数内不再进行音节合法性的判断。
 *	参数：
 *		syllable				源音节
 *		checked_syllable		被检查音节
 *		fuzzy_mode				模糊设置
 *	返回：
 *		1			包含
 *		0			不包含
 */
INLINE static int ProcessFAndHU(SYLLABLE syllable, SYLLABLE checked_syllable, int fuzzy_mode)
{
	//没有设置模糊音，或者被比较的音节不满足要求（F与HU）
	if (!((fuzzy_mode & FUZZY_F_HU) && syllable.con == CON_F && IS_START_WITH_HU(checked_syllable)))
		return 0;

	if (syllable.vow == VOW_NULL)
		return 1;

	switch(checked_syllable.vow)
	{
	case VOW_U:
		if (syllable.vow == VOW_NULL)
			return 1;

		return 0;

	case VOW_UA:
		return syllable.vow == VOW_A;

	case VOW_UAI:
		return syllable.vow == VOW_AI;

	case VOW_UAN:
		return syllable.vow == VOW_AN;

	case VOW_UANG:
		return syllable.vow == VOW_ANG;

	case VOW_UE:
		return syllable.vow == VOW_E;

	case VOW_UI:
		return syllable.vow == VOW_EI;

	case VOW_UN:
		return syllable.vow == VOW_EN;

	case VOW_UO:
		return syllable.vow == VOW_O;
	}

	return 0;
}

/*	判断HU与F的模糊音包含关系。
 *	U开头的韵母只有：UA, UAI, UAN, UANG, UE, UI, UN, UO七种。
 *	由于音节获取函数返回的音节保证正确性，所以本函数内不再进行音节合法性的判断。
 *	参数：
 *		syllable				源音节
 *		checked_syllable		被检查音节
 *		fuzzy_mode				模糊设置
 *	返回：
 *		1			包含
 *		0			不包含
 */
INLINE int ProcessHUAndF(SYLLABLE syllable, SYLLABLE checked_syllable, int fuzzy_mode)
{
	//模糊项没有设置或者输入参数不满足要求（HU与F）
	if (!((fuzzy_mode & FUZZY_REV_F_HU) && IS_START_WITH_HU(syllable) && checked_syllable.con == CON_F))
		return 0;

	switch(syllable.vow)
	{
	case VOW_U:				//HU包含任何一个以F开头的音节
		return 1;

	case VOW_UA:
		return checked_syllable.vow == VOW_A;

	case VOW_UAI:
		return checked_syllable.vow == VOW_AI;

	case VOW_UAN:
		return checked_syllable.vow == VOW_AN;

	case VOW_UANG:
		return checked_syllable.vow == VOW_ANG;

	case VOW_UE:
		return checked_syllable.vow == VOW_E;

	case VOW_UI:
		return checked_syllable.vow == VOW_EI;

	case VOW_UN:
		return checked_syllable.vow == VOW_EN;

	case VOW_UO:
		return checked_syllable.vow == VOW_O;
	}

	return 0;
}

/*	判断是否是双拼方案设这引起的模糊(见sp_fuzzy_syllables定义处注释)
 *	参数：
 *		syllable				源音节
 *		checked_syllable		被检查音节
 *	返回：
 *		1			包含
 *		0			不包含
 */
INLINE static int ProcessSPFuzzy(SYLLABLE syllable, SYLLABLE checked_syllable)
{
	int i, j;
	
	for(i = 0; i < sp_fuzzy_map_items; i++)
	{
		if (syllable.con == sp_fuzzy_map[i].sp_fuzzy_syllables[0].con &&
			syllable.vow ==  sp_fuzzy_map[i].sp_fuzzy_syllables[0].vow)
		{
			//不存在多种解析
			if (sp_fuzzy_map[i].sp_fuzzy_syllable_count <= 1)
				return 0;

			for(j = 1; j < sp_fuzzy_map[i].sp_fuzzy_syllable_count; j++)
				if (checked_syllable.con == sp_fuzzy_map[i].sp_fuzzy_syllables[j].con && 
					checked_syllable.vow == sp_fuzzy_map[i].sp_fuzzy_syllables[j].vow)
					return 1;
		}
	}

	return 0;
}

/*	判断第一个音节参数所指示的韵母集合是否包含第二个音节参数的韵母。
 *	参数：
 *		syllable				源音节
 *		checked_syllable		被检查音节
 *		fuzzy_mode				模糊设置
 *	返回：
 *		1			包含
 *		0			不包含
 */
int ContainVow(SYLLABLE syllable, SYLLABLE checked_syllable, int fuzzy_mode)
{
#define	CHECK_FUZZY(fuzzy_bit, vow_value)	((fuzzy_mode & fuzzy_bit) && (checked_syllable.vow == vow_value))

	//解决jqxy与u、v相同的问题
	if (syllable.con == CON_J || syllable.con == CON_Q || syllable.con == CON_X || syllable.con == CON_Y)
	{
		if (syllable.vow == VOW_V && checked_syllable.vow == VOW_U)
			return 1;

		if (syllable.vow == VOW_U && checked_syllable.vow == VOW_V)
			return 1;
	}

	//为了解决fang输入到hu的问题
	if (syllable.vow == checked_syllable.vow)
	{
		if (syllable.con == checked_syllable.con)
			return 1;

		if (!(fuzzy_mode & FUZZY_F_HU) && !(fuzzy_mode & FUZZY_REV_F_HU))
			return 1;

		if (fuzzy_mode & FUZZY_F_HU)
			if (syllable.con != CON_F || checked_syllable.con != CON_H)
				return 1;

		if (fuzzy_mode & FUZZY_REV_F_HU)
			if (syllable.con != CON_H || checked_syllable.con != CON_F)
				return 1;
	}

	if (syllable.vow == VOW_ANY)
		return 1;

	//处理F与HU
	if (ProcessFAndHU(syllable, checked_syllable, fuzzy_mode))
		return 1;

	//处理HU与F
	if (ProcessHUAndF(syllable, checked_syllable, fuzzy_mode))
		return 1;

	switch(syllable.vow)
	{
	case VOW_NULL:					//没有韵母标识，肯定包含
		return 1;

	case VOW_A:
		if (checked_syllable.con != CON_NULL)
			return 0;

		return (CHECK_FUZZY(FUZZY_SUPER, VOW_AI))  || (CHECK_FUZZY(FUZZY_SUPER, VOW_AN)) ||
			   (CHECK_FUZZY(FUZZY_SUPER, VOW_ANG)) || (CHECK_FUZZY(FUZZY_SUPER, VOW_AO));

	case VOW_E:
		if (checked_syllable.con != CON_NULL)
			return 0;

		return (CHECK_FUZZY(FUZZY_SUPER, VOW_EI)) || (CHECK_FUZZY(FUZZY_SUPER, VOW_EN)) ||
			   (CHECK_FUZZY(FUZZY_SUPER, VOW_ER)) || (CHECK_FUZZY(FUZZY_SUPER, VOW_ENG));

	case VOW_O:
		if (checked_syllable.con != CON_NULL)
			return 0;

		return (CHECK_FUZZY(FUZZY_SUPER, VOW_OU));

	case VOW_AN:
		return CHECK_FUZZY(FUZZY_AN_ANG, VOW_ANG);

	case VOW_IAN:
		return CHECK_FUZZY(FUZZY_AN_ANG, VOW_IANG);

	case VOW_UAN:
		return CHECK_FUZZY(FUZZY_AN_ANG, VOW_UANG);

	case VOW_ANG:
		return CHECK_FUZZY(FUZZY_REV_AN_ANG, VOW_AN);

	case VOW_IANG:
		return CHECK_FUZZY(FUZZY_REV_AN_ANG, VOW_IAN);

	case VOW_UANG:
		return CHECK_FUZZY(FUZZY_REV_AN_ANG, VOW_UAN);

	case VOW_EN:
		return CHECK_FUZZY(FUZZY_EN_ENG, VOW_ENG);

	case VOW_ENG:
		return CHECK_FUZZY(FUZZY_REV_EN_ENG, VOW_EN);

	case VOW_IN:
		return CHECK_FUZZY(FUZZY_IN_ING, VOW_ING);

	case VOW_ING:
		return CHECK_FUZZY(FUZZY_REV_IN_ING, VOW_IN);
	}

	return 0;

//不再使用这个宏
#undef	CHECK_FUZZY
}

/*	不使用音调的比较，判断第一个音节参数所指示的音节集合是否包含第二个音节参数。
 *	当源音节只有声母时，与相同声母的任何音节都是相同的。
 *	参数：
 *		syllable				源音节
 *		checked_syllable		被检查音节
 *		fuzzy_mode				模糊设置
 *	返回：
 *		1			包含
 *		0			不包含
 */
int ContainSyllable(SYLLABLE syllable, SYLLABLE checked_syllable, int fuzzy_mode)
{
	if (syllable.con == CON_F && syllable.vow == VOW_AN &&
		checked_syllable.con == CON_H && checked_syllable.vow == VOW_ANG)
		fuzzy_mode = fuzzy_mode;

	//如果第一个音节为空，则肯定为包含。即：没有音包含全部发音。
	if (syllable.con == CON_NULL && syllable.vow == VOW_NULL)
		return 1;

	//需要特殊判断“王”、“黄”的模糊音
	if (ProcessHUANGAndWANG(syllable, checked_syllable, fuzzy_mode))
		return 1;

	if (ProcessWANGAndHUANG(syllable, checked_syllable, fuzzy_mode))
		return 1;

	//处理双拼方案解析不唯一的情况
	if (pim_config->pinyin_mode == PINYIN_SHUANGPIN && ProcessSPFuzzy(syllable, checked_syllable))
		return 1;

	//首先判断音调，这样省时间
	//如果源音节带有音调，被检查音节不包含音调则为不符合。
	if (syllable.tone != TONE_0 && checked_syllable.tone != TONE_0 &&	//带有音调
		!(syllable.tone & checked_syllable.tone))						//不包含音调
		return 0;

	//如果声母不相同则返回不同
	if (!ContainCon(syllable, checked_syllable, fuzzy_mode))
		return 0;

	//如果源音节的声母为空，则比较结果为相同
	if (syllable.vow == VOW_NULL)
		return 1;

	return ContainVow(syllable, checked_syllable, fuzzy_mode);
}

/*	进行音调包含的比较，判断第一个音节参数所指示的音调集合是否包含第二个音节参数的音调。
 *	参数：
 *		syllable				源音节
 *		checked_syllable		被检查音节
 *	返回：
 *		1			包含
 *		0			不包含
 */
__inline static int ContainTone(SYLLABLE syllable, SYLLABLE checked_syllable)
{
	//如果两个音调中的一个为没有指定音调，则为包含关系
	if (syllable.tone == TONE_0 || checked_syllable.tone == TONE_0)
		return 1;

	return syllable.tone & checked_syllable.tone;
}

/*	使用带有音调的比较，判断第一个音节参数所指示的音节集合是否包含第二个音节参数。
 *	当源音节只有声母时，与相同声母的任何音节都是相同的。
 *	参数：
 *		syllable				源音节
 *		checked_syllable		被检查音节
 *		fuzzy_mode				模糊设置
 *	返回：
 *		1			包含
 *		0			不包含
 */
int ContainSyllableWithTone(SYLLABLE syllable, SYLLABLE checked_syllable, int fuzzy_mode)
{
	return ContainTone(syllable, checked_syllable) && ContainSyllable(syllable, checked_syllable, fuzzy_mode);
}

/*	获得拼音串的音调。
 *	如果拼音串长度为0，则音调为全部包括。
 *	参数：
 *		pin_yin			拼音串
 *		length			串长度
 *	返回：
 *		TONE_ERROR		非法的音调串
 *		其他			音调标识
 */
static int GetTone(const TCHAR *pin_yin)
{
	switch(*pin_yin)
	{
	case TONE_CHAR_1:
		return TONE_1;

	case TONE_CHAR_2:
		return TONE_2;

	case TONE_CHAR_3:
		return TONE_3;

	case TONE_CHAR_4:
		return TONE_4;
	}

	return TONE_0;
}

/*	获得拼音的音节标识。当前拼音作为完整的拼音进行检索，找出音节表达。
 *	此拼音串如果其中包含着错误（如：v、u、i开始）则肯定返回失败。
 *	如："wom"将返回错误。"pin4" -> 声母：CON_P，韵母：VOW_IN，音调：4声（10000B）。
 *
 *	由于部分拼音只能在模糊的方式下才是正确的，所以必须将模糊设置传递过来。
 *	如：cuang，在c、ch模糊的情况下是正确的发音。
 *
 *	参数：
 *		pin_yin					拼音串
 *		syllable				音节
 *		*syllable_str_length	音节使用的拼音长度，可以为0
 *		fuzzy_mode				模糊音选项
 *	返回：
 *		1				成功
 *		0				音节解析失败。如：vabcd等。
 */
int GetSyllable(const TCHAR *pin_yin, SYLLABLE *syllable, int *syllable_str_length, int fuzzy_mode/*, int *correct_flag, int *separator_flag*/)
{
	int hi, low, mid, ret;
	int py_length = (int)_tcslen(pin_yin);
	int has_tone = 0, has_separator = 0;	//是否有音调、音节分隔符
	TCHAR py[MAX_PINYIN_LENGTH + 1];		//备份拼音串

	if (!py_length || py_length > MAX_PINYIN_LENGTH)				//没有拼音串，则为非法音节
		return 0;

	if (!share_segment)
		return 0;

	//复制一份
	_tcscpy(py, pin_yin);

	//如果末尾为'，则为正确的音节
	if (py[py_length - 1] == SYLLABLE_SEPARATOR_CHAR)
	{
		py[py_length - 1] = 0;
		has_separator	  = 1;
		py_length--;

		if (!py_length)				//没有拼音串，则为非法音节
			return 0;
	}

	//通配符
	if (py_length == 1 && *py == SYLLABLE_ANY_CHAR)
	{
		syllable->con		 = CON_ANY;
		syllable->vow		 = VOW_ANY;
		syllable->tone		 = TONE_0;
		*syllable_str_length = 1;

		return 1;
	}

	//判断尾部是否有音调标志
	if (IS_TONE(py[py_length - 1]))
	{
		has_tone = 1;
		py_length--;

		if (!py_length)				//再次判断是否有拼音串
			return 0;
	}

	//二分法查找拼音
	low = 0;
	hi = share_segment->syllable_map_items - 1;
	while(low <= hi)
	{
		mid = (low + hi) / 2;

		//如果长度不相同，则肯定不能找到，必须进行完整拼音的比较
		//如：如果不进行完整比较men将与meng相同。
		//由于音调为数字，所以比较时不会产生错误。
		if (py_length != share_segment->syllable_map[mid].pin_yin_length)
			ret = _tcscmp(py, share_segment->syllable_map[mid].pin_yin);
		else
			ret = _tcsncmp(py, share_segment->syllable_map[mid].pin_yin, py_length);

		if (!ret)			//找到
			break;

		if (ret < 0)
			hi = mid - 1;
		else
			low = mid + 1;
	}

	if (low > hi)			//没有找到
		return 0;

	//如果该音需要在模糊的情况下才是有效的，则需要判断模糊标志是否设置
	if (share_segment->syllable_map[mid].fuzzy_flag && !(share_segment->syllable_map[mid].fuzzy_flag & fuzzy_mode))
		return 0;

	//如果该音需要在纠正的情况下才是有效的，则需要判断纠正标志是否设置
	//if (share_segment->syllable_map[mid].correct_flag && !(share_segment->syllable_map[mid].correct_flag & pim_config->correct_option))
	//	return 0;

	//存储结果
	syllable->con  = share_segment->syllable_map[mid].con;
	syllable->vow  = share_segment->syllable_map[mid].vow;
	syllable->tone = TONE_0;				//没有音调标识

	//if (correct_flag)
	//	*correct_flag = share_segment->syllable_map[mid].correct_flag;

	//if (separator_flag)
	//	*separator_flag = has_separator;

	if (has_tone)
		syllable->tone = GetTone(py + share_segment->syllable_map[mid].pin_yin_length);

	if (syllable_str_length)
	{
		*syllable_str_length = share_segment->syllable_map[mid].pin_yin_length;

		if (has_tone)
			(*syllable_str_length)++;

		if (has_separator)
			(*syllable_str_length)++;
	}

	return 1;
}

/*	获得双拼的音节
 */
int GetSPSyllable(const TCHAR *pin_yin, SYLLABLE *syllable, int *syllable_str_length, int fuzzy_mode)
{
	int  i, max_length, index;
	int  syllable_length;
	int  py_length = (int)_tcslen(pin_yin);
	int  has_tone = 0, has_separator = 0;		//是否有音调、音节分隔符
	TCHAR py[MAX_PINYIN_LENGTH + 1];			//备份拼音串

	if (!py_length || py_length > 3)			//没有拼音串，则为非法音节
		return 0;

	if (!share_segment)
		return 0;

	//复制一份
	_tcscpy(py, pin_yin);

	//如果末尾为'，则为正确的音节
	if (py[py_length - 1] == SYLLABLE_SEPARATOR_CHAR)
	{
		py[py_length - 1] = 0;
		has_separator	  = 1;
		py_length--;

		if (!py_length)				//没有拼音串，则为非法音节
			return 0;
	}

	//通配符
	if (py_length == 1 && *py == SYLLABLE_ANY_CHAR)
	{
		syllable->con		 = CON_ANY;
		syllable->vow		 = VOW_ANY;
		syllable->tone		 = TONE_0;
		*syllable_str_length = 1;

		return 1;
	}

	//判断尾部是否有音调标志
	if (pim_config->use_hz_tone && IS_TONE(py[py_length - 1]))
	{
		has_tone = 1;
		py[py_length - 1] = 0;
		py_length--;

		if (!py_length)				//再次判断是否有拼音串
			return 0;
	}

	//优先匹配pin_yin_length最大的，但对于有多个匹配项它们的pin_yin_length
	//最大相同的情况，将其加入到sp_fuzzy_syllables(目前同一个双拼组合最多支
	//持4个音节模糊的情况，如蓝天双拼方案中的双拼组合dk，可以解析为den或dia
	//，即2个音节模糊)

	//查找拼音
	max_length = 0;
	index = -1;

	//找到第一个最大匹配项的位置
	for (i = 0; i < share_segment->syllable_map_items; i++)
	{
		if (!_tcscmp(py, share_segment->syllable_map[i].sp_pin_yin) &&
			max_length < share_segment->syllable_map[i].pin_yin_length)
		{
			max_length = share_segment->syllable_map[i].pin_yin_length;
			index = i;
		}
	}

	//没有找到
	if (index == -1)	
		return 0;

	//理论上来说必然会进入if分支
	if (sp_fuzzy_map_items < MAX_SYLLABLE_PER_INPUT)
	{
		//把所有和上面找出的最大匹配项长度相等的匹配项(包括其自身)加入到sp_fuzzy_map
		for (i = 0; i < share_segment->syllable_map_items; i++)
		{
			//首次进入下面的if时必有i==index
			if (!_tcscmp(py, share_segment->syllable_map[i].sp_pin_yin) &&
				max_length == share_segment->syllable_map[i].pin_yin_length)
			{
				int count = sp_fuzzy_map[sp_fuzzy_map_items].sp_fuzzy_syllable_count;
				if (count < MAX_SP_FUZZY_SYLLABLE_COUNT)
				{
					sp_fuzzy_map[sp_fuzzy_map_items].sp_fuzzy_syllables[count].con = share_segment->syllable_map[i].con;
					sp_fuzzy_map[sp_fuzzy_map_items].sp_fuzzy_syllables[count].vow = share_segment->syllable_map[i].vow;
					sp_fuzzy_map[sp_fuzzy_map_items].sp_fuzzy_syllables[count].tone = TONE_0;
					sp_fuzzy_map[sp_fuzzy_map_items].syllable_map_index[count] = i;
					sp_fuzzy_map[sp_fuzzy_map_items].sp_fuzzy_syllable_count++; 
				}
			}
		}

		for (i = 0; i < sp_fuzzy_map[sp_fuzzy_map_items].sp_fuzzy_syllable_count; i++)
		{
			index = sp_fuzzy_map[sp_fuzzy_map_items].syllable_map_index[i];

			//如果该音需要在模糊的情况下才是有效的，则需要判断模糊标志是否设置，若未设置将其从sp_fuzzy_syllables中删除
			if (share_segment->syllable_map[index].fuzzy_flag && !(share_segment->syllable_map[index].fuzzy_flag & fuzzy_mode))
			{
				if (i < sp_fuzzy_map[sp_fuzzy_map_items].sp_fuzzy_syllable_count - 1)
				{
					memmove(sp_fuzzy_map[sp_fuzzy_map_items].sp_fuzzy_syllables + i,
						sp_fuzzy_map[sp_fuzzy_map_items].sp_fuzzy_syllables + i + 1,
						sp_fuzzy_map[sp_fuzzy_map_items].sp_fuzzy_syllable_count - i - 1);
					memmove(sp_fuzzy_map[sp_fuzzy_map_items].syllable_map_index + i, 
						sp_fuzzy_map[sp_fuzzy_map_items].syllable_map_index + i + 1,
						sp_fuzzy_map[sp_fuzzy_map_items].sp_fuzzy_syllable_count - i - 1);
				}

				sp_fuzzy_map[sp_fuzzy_map_items].sp_fuzzy_syllable_count--;
				i--;
			}
		}
		
		if (!sp_fuzzy_map[sp_fuzzy_map_items].sp_fuzzy_syllable_count)
			return 0;

		//如果存在多个合理的匹配项，设置解析出的首个为解析结果，其余作为模糊音
		i = sp_fuzzy_map[sp_fuzzy_map_items].syllable_map_index[0];

		sp_fuzzy_map_items++;
	}
	else
	{
		//如果该音需要在模糊的情况下才是有效的，则需要判断模糊标志是否设置
		if (share_segment->syllable_map[index].fuzzy_flag && !(share_segment->syllable_map[index].fuzzy_flag & fuzzy_mode))
			return 0;

		i = index;
	}


	//存储结果
	syllable->con  = share_segment->syllable_map[i].con;
	syllable->vow  = share_segment->syllable_map[i].vow;
	syllable->tone = TONE_0;			//没有音调标识

	syllable_length = (int)_tcslen(share_segment->syllable_map[i].sp_pin_yin);
	if (has_tone)
		syllable->tone = GetTone(pin_yin + syllable_length);

	if (syllable_str_length)
	{
		*syllable_str_length = syllable_length;

		if (has_tone)
			(*syllable_str_length)++;

		if (has_separator)
			(*syllable_str_length)++;
	}

	return 1;
}

int GetLegalPinYinLength(const TCHAR *pin_yin, int state, int english_state)
{
	int i;

	if (STATE_EDIT == state && ENGLISH_STATE_NONE == english_state)
	{
		for (i = 0; pin_yin[i]; i++)
		{
			if (pin_yin[i] >= 'A' && pin_yin[i] <= 'Z')
				break;
		}
	}
	else
		i = (int)_tcslen(pin_yin);

	return i;
}

/*	用递归的方式正向解析拼音串。
 *	参数：
 *		pin_yin				拼音串
 *		*syllables			音节存储数组
 *		array_length		音节数组长度
 *		fuzzy_mode			模糊设置
 *	返回值：
 *		0：没有解析出正确的结果
 *		> 0：解析出的音节数目
 */
static int ProcessParse(const TCHAR *pin_yin, SYLLABLE *syllables, int array_length, int fuzzy_mode/*, int *correct_flag, int *separator_flag*/)
{
	int i, ret;
	int py_length;									//拼音串长度
	int syllable_string_length;						//音节字符串长度
	TCHAR cur_py[MAX_PINYIN_LENGTH + 1];			//当前处理的临时拼音串
	TCHAR new_pin_yin[MAX_INPUT_LENGTH + 0x10];

	assert(pin_yin && syllables);

	//跳过音节切分符号
	if (*pin_yin == SYLLABLE_SEPARATOR_CHAR)
		pin_yin++;

	//减少递归次数直接发现错误。
	if (*pin_yin == 'i' || *pin_yin == 'u' || *pin_yin == 'v')
		return 0;

	if (IS_TONE(*pin_yin))
		return 0;

	//py_length = (int)_tcslen(pin_yin);
	py_length = GetLegalPinYinLength(pin_yin, STATE_EDIT, ENGLISH_STATE_NONE);
	//如果拼音字符长度大于最大长度限制，则退出
	if ( py_length > _SizeOf(new_pin_yin) )
		return 0;
	_tcsncpy_s(new_pin_yin, _SizeOf(new_pin_yin), pin_yin, py_length);

	for (i = min(MAX_PINYIN_LENGTH, py_length); i > 0; i--)
	{
		//复制第一个可能的音节
		CopyPartString(cur_py, new_pin_yin, i);

		//查找合法的拼音
		if (!GetSyllable(cur_py, syllables, &syllable_string_length, fuzzy_mode/*, correct_flag, separator_flag*/))
			continue;			//没有找到，减少一个音符，再次检索

		//没有需要检索的输入拼音了，则结束递归 || 没有空余的音节数组空间
		if (py_length == i || array_length == 1)
			return 1;

		//找到第一个音节，查找剩余音节
		ret = ProcessParse(new_pin_yin + syllable_string_length, 
						   syllables + 1, 
						   array_length - 1, 
						   fuzzy_mode//, 
						   /*correct_flag ? correct_flag + 1 : 0,*/
						   /*separator_flag ? separator_flag + 1 : 0*/);

		if (!ret)
			continue;			//没有合法的音节，如：li，如果第一个为l则永远没有合法的音节

		//现在获得了整个解
		return ret + 1;
	}

	//没有找到解
	return 0;
}

/*	对双拼串进行解析的递归过程
 */
static int ProcessParseSPInternal(const TCHAR *pin_yin, SYLLABLE *syllables, int array_length, int fuzzy_mode)
{
	int i, ret;
	int py_length;									//拼音串长度
	int syllable_string_length;						//音节字符串长度
	TCHAR cur_py[MAX_PINYIN_LENGTH + 1];			//当前处理的临时拼音串

	assert(pin_yin && syllables);

	//跳过音节切分符号
	if (*pin_yin == SYLLABLE_SEPARATOR_CHAR)
		pin_yin++;

	//减少递归次数直接发现错误。
	if (IS_TONE(*pin_yin))
		return 0;

	py_length = (int)_tcslen(pin_yin);
	for (i = min(3, py_length); i > 0; i--)
	{
		//复制第一个可能的音节
		CopyPartString(cur_py, pin_yin, i);

		//查找合法的拼音
		if (!GetSPSyllable(cur_py, syllables, &syllable_string_length, fuzzy_mode))
			continue;			//没有找到，减少一个音符，再次检索

		//没有需要检索的输入拼音了，则结束递归 || 没有空余的音节数组空间
		if (py_length == i || array_length == 1)
			return 1;

		//找到第一个音节，查找剩余音节
		ret = ProcessParseSPInternal(pin_yin + syllable_string_length, syllables + 1, array_length - 1, fuzzy_mode);
		if (!ret)
			continue;			//没有合法的音节，如：li，如果第一个为l则永远没有合法的音节

		//现在获得了整个解
		return ret + 1;
	}

	//没有找到解
	return 0;
}

/*	对双拼串进行解析
 *  由于sp_fuzzy_map的清0针对的是整个拼音串，而不是单个音节，
 *  因此不能放在递归函数里，需要把ProcessParseSP里的递归部分
 *  单独构成一个函数ProcessParseSPInternal
 */
static int ProcessParseSP(const TCHAR *pin_yin, SYLLABLE *syllables, int array_length, int fuzzy_mode)
{
	//在解析双拼拼音之前，把必要的数据结构清0
	memset(sp_fuzzy_map, 0, sizeof(sp_fuzzy_map));
	sp_fuzzy_map_items = 0;

	return ProcessParseSPInternal(pin_yin, syllables, array_length, fuzzy_mode);
}

/*	用递归的方式反向解析拼音串。
 *	参数：
 *		pin_yin				拼音串
 *		*syllables			音节存储数组
 *		array_length		音节数组长度
 *		fuzzy_mode			模糊设置
 *	返回值：
 *		0：没有解析出正确的结果
 *		> 0：解析出的音节数目
 */
static int ProcessParseReverse(const TCHAR *pin_yin, SYLLABLE *syllables, int array_length, int fuzzy_mode/* ,int *correct_flag, int *separator_flag*/)
{
	int i, ret;
	int py_length;													//拼音串长度
	int syllable_string_length;										//音节字符串长度
	TCHAR cur_py[MAX_PINYIN_LENGTH * MAX_SYLLABLE_PER_INPUT + 1];	//当前处理的临时拼音串
	TCHAR new_pin_yin[MAX_INPUT_LENGTH + 0x10];

	assert(pin_yin && syllables);

	//跳过音节切分符号
	if (*pin_yin == SYLLABLE_SEPARATOR_CHAR)
		pin_yin++;

	//减少递归次数直接发现错误。
	if (*pin_yin == 'i' || *pin_yin == 'u' || *pin_yin == 'v')
		return 0;

	if (IS_TONE(*pin_yin))
		return 0;

	//py_length = (int)_tcslen(pin_yin);
	py_length = GetLegalPinYinLength(pin_yin, STATE_EDIT, ENGLISH_STATE_NONE);
	//如果拼音字符长度大于最大长度限制，则退出
	if ( py_length > _SizeOf(new_pin_yin) )
		return 0;
	_tcsncpy_s(new_pin_yin, _SizeOf(new_pin_yin), pin_yin, py_length);

	for (i = min(MAX_PINYIN_LENGTH, py_length); i > 0; i--)
	{
		//查找合法的拼音
		if (!GetSyllable(new_pin_yin + py_length - i, syllables, &syllable_string_length, fuzzy_mode/*, correct_flag, separator_flag*/))
			continue;			//没有找到，减少一个音符，再次检索

		//没有需要检索的输入拼音了，则结束递归 || 没有空余的音节数组空间
		if (py_length == i || array_length == 1)
			return 1;

		//复制第一个可能的音节
		CopyPartString(cur_py, new_pin_yin, py_length - i);

		//找到第一个音节，查找剩余音节
		ret = ProcessParseReverse(cur_py, 
								  syllables + 1, 
								  array_length - 1, 
								  fuzzy_mode//, 
								  /*correct_flag ? correct_flag + 1 : 0, */
								  /*separator_flag ? separator_flag + 1 : 0*/);

		if (!ret)
			continue;			//没有合法的音节，如：li，如果第一个为l则永远没有合法的音节

		//现在获得了整个解
		return ret + 1;
	}

	//没有找到解
	return 0;
}

/*	判断拼音串是否为合法的。
 *	参数：
 *		pin_yin				拼音串
 *	返回值：
 *		1：合法
 *		0：非法
 */
static int LegalPinYin(const TCHAR *pin_yin)
{
	int i, nlen = GetLegalPinYinLength(pin_yin, STATE_EDIT, ENGLISH_STATE_NONE);

	if (!pin_yin)
		return 0;

	//进行比较粗浅的合法性检查，主要集中与输入串中是否包含不合法的
	//字符以及uu、vv、ii、uv、vi、vu。
	//该检查的目的在于避免递归分析数据的时间过长。
	//v的合法前缀：jlnqxy
	//u的非法前缀：{NUL}aeuv{TONE}
	//i的非法前缀：{NUL}fgikovw{TONE}		h需要特殊处理，shi,zhi,chi合法。
	//TONE的非法前缀：{NUL}{TONE}{SEPARATOR}
	//SEPARATOR的非法前缀：{NUL}{SEPARATOR}

	//打头的应该是a-z，并且不是uvi
	if (*pin_yin < 'a' || *pin_yin > 'z' ||	*pin_yin == 'u' || *pin_yin == 'v' || *pin_yin == 'i')
		return 0;

	for (i = 1; pin_yin[i] && i < nlen; i++)
	{
		//拼音串中出现非法的字符
		if (!((pin_yin[i] >= 'a' && pin_yin[i] <= 'z') || (pin_yin[i] == SYLLABLE_SEPARATOR_CHAR) ||
			  (pin_yin[i] == SYLLABLE_ANY_CHAR) || IS_TONE(pin_yin[i])))
			return 0;

		switch(pin_yin[i])
		{
		case SYLLABLE_ANY_CHAR:
			if (!(pim_config->ci_option & CI_WILDCARD))
				return 0;

			if (pin_yin[i - 1] == SYLLABLE_ANY_CHAR)
				return 0;

			break;

		case SYLLABLE_SEPARATOR_CHAR:
			if (pin_yin[i - 1] == SYLLABLE_SEPARATOR_CHAR)
				return 0;

			break;

		case 'v':
			{
				int wrong = 1;
				switch(pin_yin[i - 1])
				{
				case  'j':	case  'l':	case  'n':	case  'q':	case  'x':	case  'y':
					wrong = 0;
				}

				if (wrong)
					return 0;
			}
			break;

		case 'u':
			switch(pin_yin[i - 1])
			{
			case 'a':	case 'e':	case 'v':	case 'u':
				return 0;
			}

			break;

		case 'i':
			switch(pin_yin[i - 1])
			{
			case 'f':
				if (pin_yin[i + 1] == 'a' && pin_yin[i + 2] == 'o')
					break;

				return 0;

			case 'g':	case 'i':	case 'o':	case 'v':	case 'w':
				return 0;

			case 'h':
				if (i < 2 || (pin_yin[i - 2] != 's' && pin_yin[i - 2] != 'c' &&	pin_yin[i - 2] != 'z'))
					return 0;

				break;

			default:
				if (IS_TONE(pin_yin[i - 1]))
					return 0;
			}

			break;

		case TONE_CHAR_1:
		case TONE_CHAR_2:
		case TONE_CHAR_3:
		case TONE_CHAR_4:
			if (!pim_config->use_hz_tone)
				return 0;

			if (IS_TONE(pin_yin[i - 1]))
				return 0;

			if (pin_yin[i - 1] == SYLLABLE_SEPARATOR_CHAR || pin_yin[i - 1] == SYLLABLE_ANY_CHAR)
				return 0;

			break;
		}
	}

	return 1;
}

/*	解析拼音串到音节数组。采用正向最大匹配的方式进行。
 *	参数：
 *		pin_yin				拼音串（可能包含音节分隔符）
 *		*syllable_array		解析后的音节数组，以0为标识
 *		syllable_length		音节数组长度
 *		fuzzy_mode			模糊音设置
 *	返回：
 *		音节的数目，等于0的时候一般为错误输入
 */
int ParsePinYinString(const TCHAR *pin_yin, SYLLABLE *syllables, int array_length, int fuzzy_mode/*, int *correct_flag, int *separator_flag*/)
{
	if (pim_config->pinyin_mode == PINYIN_SHUANGPIN)
		return ProcessParseSP(pin_yin, syllables, array_length, fuzzy_mode);

	//全拼
	if (!LegalPinYin(pin_yin))
		return 0;

	return ProcessParse(pin_yin, syllables, array_length, fuzzy_mode/*, correct_flag, separator_flag*/);
}

//是否有需要特殊解析的音节
//int HasSpecialSyllable(SYLLABLE *syllables, int *separator_flag, int syllable_count)
//{
//	int i, j, found;
//	
//	if (syllable_count < special_syllables[0].old_syllable_count)
//		return -1;
//
//	for (i = 0; i < _SizeOf(special_syllables); i++)
//	{
//		if (syllable_count < special_syllables[i].old_syllable_count)
//			continue;
//
//		found = 1;
//
//		for (j = 0; j < syllable_count && j < special_syllables[i].old_syllable_count; j++)
//		{
//			if (syllables[j].con != special_syllables[i].old_syllables[j].con ||
//				syllables[j].vow != special_syllables[i].old_syllables[j].vow || separator_flag[j])
//			{
//				found = 0;
//				break;
//			}
//		}
//
//		if (found)
//			return i;
//	}
//
//	return -1;
//}

//int ProcessSpecialSyllables(SYLLABLE *syllables, int *separator_flag, int array_length, int syllable_count)
//{
//	int i, j, index, count;
//
//	if (syllable_count < special_syllables[0].old_syllable_count)
//		return syllable_count;
//
//	for (i = 0; i < syllable_count; i++)
//	{
//		index = HasSpecialSyllable(&syllables[i], &separator_flag[i], syllable_count - i);
//		if (index < 0)
//			continue;
//
//		count = special_syllables[index].new_syllable_count - special_syllables[index].old_syllable_count;
//		if ((syllable_count + count) > array_length)
//			break;
//
//		//向后移动
//		if (count > 0)
//		{
//			for (j = syllable_count - 1 + count; j > i + count; j--)
//				syllables[j] = syllables[j - 1];
//		}
//
//		for (j = 0; j < special_syllables[index].new_syllable_count; j++)
//		{
//			syllables[i + j].con = special_syllables[index].new_syllables[j].con;
//			syllables[i + j].vow = special_syllables[index].new_syllables[j].vow;
//		}
//
//		return syllable_count + count;
//	}
//
//	return syllable_count;
//}

/*	解析拼音串到音节数组。采用反向最大匹配的方式进行。
 *	参数：
 *		pin_yin				拼音串（可能包含音节分隔符）
 *		*syllable_array		解析后的音节数组，以0为标识
 *		syllable_length		音节数组长度
 *		fuzzy_mode			模糊音设置
 *		pinyin_mode			全拼、双拼
 *	返回：
 *		音节的数目，等于0的时候一般为错误输入
 */
int ParsePinYinStringReverse(const TCHAR *pin_yin, SYLLABLE *syllables, int array_length, int fuzzy_mode, int pinyin_mode/* ,int *correct_flag,int *separator_flag*/)
{
	int i;
	int count, other_count;		//返回音节数目
	SYLLABLE tmp_syllable;		//交换临时音节
	SYLLABLE tmp_syllables[MAX_SYLLABLE_PER_INPUT];

	//int tmp_correct_flag = 0;
	//int tmp_correct_flags[MAX_SYLLABLE_PER_INPUT] = {0};
	//int tmp_separator_flag = 0;
	//int tmp_separator_flags[MAX_SYLLABLE_PER_INPUT] = {0};

	if (pinyin_mode == PINYIN_SHUANGPIN)
		return ProcessParseSP(pin_yin, syllables, array_length, fuzzy_mode);

	//全拼
	if (!LegalPinYin(pin_yin))
		return 0;

	//解析，采用两种解析的方法进行，哪一种的音节数目少使用哪一个，如果相同，使用反向的。
	count = ProcessParseReverse(pin_yin, syllables, array_length, fuzzy_mode/*, correct_flag, separator_flag*/);
	other_count = ProcessParse(pin_yin, tmp_syllables, array_length, fuzzy_mode/*, tmp_correct_flags, tmp_separator_flags*/);

	if ((other_count < count)) //IsSpecialPinYinStr(pin_yin))		//正向的结果好
	{
		for (i = 0; i < other_count; i++)
		{
			syllables[i] = tmp_syllables[i];
			/*if (correct_flag)
				correct_flag[i] = tmp_correct_flags[i];*/
			//if (separator_flag)
			//	separator_flag[i] = tmp_separator_flags[i];
		}

		//if (separator_flag)
		//	other_count = ProcessSpecialSyllables(syllables, tmp_separator_flags, array_length, other_count);

		return other_count;
	}

	//反向的结果好，将解析音节数组倒序
	for (i = 0; i < count / 2; i++)
	{
		tmp_syllable			 = syllables[i];
		syllables[i]			 = syllables[count - 1 - i];
		syllables[count - 1 - i] = tmp_syllable;
		//if (correct_flag)
		//{
		//	tmp_correct_flag			= correct_flag[i];
		//	correct_flag[i]				= correct_flag[count - 1 - i];
		//	correct_flag[count - 1 - i] = tmp_correct_flag;
		//}
		//if (separator_flag)
		//{
		//	tmp_separator_flag 			  = separator_flag[i];
		//	separator_flag[i]			  = separator_flag[count - 1 - i];
		//	separator_flag[count - 1 - i] = tmp_separator_flag;
		//}
	}

	//if (separator_flag)
	//	count = ProcessSpecialSyllables(syllables, separator_flag, array_length, count);

	return count;
}

//声母转换到字符串
static TCHAR *con_to_string[] =
{
	TEXT(""),						//CON_NULL
	TEXT("b"),						//CON_B
	TEXT("c"),						//CON_C
	TEXT("ch"),						//CON_CH
	TEXT("d"),						//CON_D
	TEXT("f"),						//CON_F
	TEXT("g"),						//CON_G
	TEXT("h"),						//CON_H
	TEXT("j"),						//CON_J
	TEXT("k"),						//CON_K
	TEXT("l"),						//CON_L
	TEXT("m"),						//CON_M
	TEXT("n"),						//CON_N
	TEXT("p"),						//CON_P
	TEXT("q"),						//CON_Q
	TEXT("r"),						//CON_R
	TEXT("s"),						//CON_S
	TEXT("sh"),						//CON_SH
	TEXT("t"),						//CON_T
	TEXT("w"),						//CON_W
	TEXT("x"),						//CON_X
	TEXT("y"),						//CON_Y
	TEXT("z"),						//CON_Z
	TEXT("zh"),						//CON_ZH
	TEXT(""),						//CON_END
	TEXT("*"),						//CON_ANY
};

//韵母转换到字符串
static TCHAR *vow_to_string[] =
{
	TEXT(""),						//VOW_NULL,
	TEXT("a"),						//VOW_A,
	TEXT("ai"),						//VOW_AI
	TEXT("an"),						//VOW_AN
	TEXT("ang"),					//VOW_ANG
	TEXT("ao"),						//VOW_AO
	TEXT("e"),						//VOW_E
	TEXT("ei"),						//VOW_EI
	TEXT("en"),						//VOW_EN
	TEXT("eng"),					//VOW_ENG
	TEXT("er"),						//VOW_ER
	TEXT("i"),						//VOW_I
	TEXT("ia"),						//VOW_IA
	TEXT("ian"),					//VOW_IAN
	TEXT("iang"),					//VOW_IANG
	TEXT("iao"),					//VOW_IAO
	TEXT("ie"),						//VOW_IE
	TEXT("in"),						//VOW_IN
	TEXT("ing"),					//VOW_ING
	TEXT("iong"),					//VOW_IONG
	TEXT("iu"),						//VOW_IU
	TEXT("o"),						//VOW_O
	TEXT("ong"),					//VOW_ONG
	TEXT("ou"),						//VOW_OU
	TEXT("u"),						//VOW_U
	TEXT("ua"),						//VOW_UA
	TEXT("uai"),					//VOW_UAI
	TEXT("uan"),					//VOW_UAN
	TEXT("uang"),					//VOW_UANG
	TEXT("ue"),						//VOW_UE
	TEXT("ui"),						//VOW_UI
	TEXT("un"),						//VOW_UN
	TEXT("uo"),						//VOW_UO
	TEXT("v"),						//VOW_V
	TEXT(""),						//VOW_END
	TEXT(""),						//VOW_ANY
};

//音调转换成字符串
static TCHAR *tone_to_string[] =
{
/* 0 */		TEXT(""),		//没有音
/* 1 */		TEXT("1"),		//(1 << 0)	1
/* 2 */		TEXT("2"),		//(1 << 1)	2
/* 3 */		TEXT(""),
/* 4 */		TEXT("3"),		//(1 << 2)	3
/* 5 */		TEXT(""),
/* 6 */		TEXT(""),
/* 7 */		TEXT(""),
/* 8 */		TEXT("4"),		//(1 << 3)	4
};

/*	获得音节的拼音串。
 *	参数：
 *		syllable		音节
 *		str				输出串
 *		length			输入串的长度
 *		correct_flag	纠错标记
 *		tone_char		使用音调(注意不是使用syllable.h中的TONE_CHAR_1等字符，而是将拼音表示为SYLLABLEMAP中的带声调形式，如ā等)
 *	返回值：
 *		字符串长度，0标识没有字符。
 */
int GetSyllableString(SYLLABLE syllable, TCHAR *str, int length, /*int correct_flag, */int tone_char)
{
	TCHAR *con_str, *vow_str;
	TCHAR tones[10] = {0};
	TCHAR py_str[0x10] = {0};
 	int count = 0;
	int i, j;

	if (!share_segment)
		return 0;

	//CON_ERROR，VOW_ERROR
	if (syllable.con < CON_NULL || syllable.vow < VOW_NULL || syllable.con > CON_ANY || syllable.vow > VOW_ANY)
	{
		*str++ = '?';
		*str++ = 0;
		return 1;
	}

	//非正常的拼音，直接转换，比如*
	if ((CON_NULL == syllable.con || CON_END == syllable.con || CON_ANY == syllable.con) &&
		(VOW_NULL == syllable.vow || VOW_END == syllable.vow || VOW_ANY == syllable.vow))
	{
		con_str = con_to_string[syllable.con];
		vow_str = vow_to_string[syllable.vow];

		while(*con_str && count < length - 1)
			str[count++] = *con_str++;

		while(*vow_str && count < length - 1)
			str[count++] = *vow_str++;
	}
	//正常的拼音，查表获取，主要是为了实现拼音容错
	else
	{
		str[0] = 0;

		for (i = 0; i < share_segment->syllable_map_items; i++)
		{
			if (syllable.con != share_segment->syllable_map[i].con || 
				syllable.vow != share_segment->syllable_map[i].vow/* ||
				correct_flag != share_segment->syllable_map[i].correct_flag*/)
				continue;

			for (j = 0; j <= 3; j++)
			{
				if (!(syllable.tone & (1 << j)))
					continue;

				if (count++)
					_tcscat_s(str, length, TEXT(","));

				if (tone_char)
				{
					switch (j)
					{
					case 0:
						_tcscat_s(str, length, share_segment->syllable_map[i].pin_yin_tone1);
						break;

					case 1:
						_tcscat_s(str, length, share_segment->syllable_map[i].pin_yin_tone2);
						break;

					case 2:
						_tcscat_s(str, length, share_segment->syllable_map[i].pin_yin_tone3);
						break;

					case 3:
						_tcscat_s(str, length, share_segment->syllable_map[i].pin_yin_tone4);
						break;
					}
				}
				else
				{
					_tcscat_s(str, length, share_segment->syllable_map[i].pin_yin);
					_tcscat_s(str, length, tone_to_string[1i64 << j]);
				}
			}

			if (!count)
				_tcscpy_s(str, length, share_segment->syllable_map[i].pin_yin);

			count = (int)_tcslen(str);
			break;
		}
	}

	str[count] = 0;

	return count;
}

//得到全拼的字符串（拼音全串）
//这个函数很有用，不过暂时不需要，先注释掉
//void GetAllSyllableStringQP(PIMCONTEXT *context, TCHAR *pinyin ,unsigned int pinyin_len)
//{
//	int i;
//	TCHAR py[0x10];
//	for (i = 0; i < context->syllable_count; i++)
//	{
//		GetSyllableString(context->syllables[i], py, _SizeOf(py), context->syllable_correct_flag[i], 0);
//		if (_tcslen(pinyin) + _tcslen(py) < pinyin_len - 1)
//			_tcscat_s(pinyin, pinyin_len, py);
//	}
//}

/*	获得音节的双拼拼音串。
 *	参数：
 *		syllable		音节
 *		str				输出串
 *		length			输入串的长度
 *	返回值：
 *		字符串长度，0标识没有字符。
 */
int GetSyllableStringSP(SYLLABLE syllable, TCHAR *str, int length)
{
	TCHAR *tone_str;
 	int  count = 0;
	int  i;

	if (!share_segment)
		return 0;

	//CON_ERROR，VOW_ERROR
	if (syllable.con < CON_NULL || syllable.vow < VOW_NULL || syllable.con > CON_ANY || syllable.vow > VOW_ANY ||
		syllable.tone < 0 || syllable.tone > 8)
	{
		*str++ = '?';
		*str++ = 0;
		return 1;
	}

	if (syllable.con == CON_ANY && syllable.vow == VOW_ANY)
	{
		*str++ = '*';
		*str++ = 0;
		return 1;
	}

	for (i = 0; i < share_segment->syllable_map_items; i++)
	{
		if (share_segment->syllable_map[i].con == syllable.con && 
			share_segment->syllable_map[i].vow == syllable.vow)
			break;
	}

	if (i == share_segment->syllable_map_items)
	{
		str[0] = 0;
		return 0;
	}

	_tcscpy_s(str, length, share_segment->syllable_map[i].sp_pin_yin);
	tone_str = tone_to_string[syllable.tone];
	_tcscat_s(str, length, tone_str);

	return (int)_tcslen(str);
}

/*	判断音节首字母是否与参数字母相同。用于单键取词。
 *	参数：
 *		ch				进行判断的字母
 *		syllable		进行判断的音节
 *	返回：
 *		1：包含，0：不包含
 */
int SyllableStartWithLetter(TCHAR ch, SYLLABLE syllable)
{
	if (ch == SYLLABLE_ANY_CHAR)
		return 1;

	if (syllable.con != CON_NULL)			//有声母
		return ch == con_to_string[syllable.con][0];

	if (syllable.vow != VOW_NULL)			//没有声母
		return ch == vow_to_string[syllable.vow][0];

	return 0;
}

/*	比较一组音节，进行通配符的比较，不进行音调比较。
 *	检查音节可能含有通配符，但被检查音节中不包括这些。
 *	不应有连续两个以上的通配符。
 *	参数：
 *		syllables					比较的音节
 *		length						比较音节长度
 *		checked_syllables			被比较的音节数组
 *		ci							被比较的词数组
 *		checked_length				被比较的词长度
 *		fuzzy_mode					模糊方式
 *	返回：
 *		相符：1
 *		不相符：0
 */
int WildCompareSyllablesWithCi(SYLLABLE *syllables, int length, SYLLABLE *checked_syllables, HZ *ci, int checked_length, int fuzzy_mode)
{
	int i, j;

	//被比较音节长度大于等于比较音节
	if (length > checked_length)
		return 0;

	for (i = 0; i < length; i++)
	{
		if (syllables[i].con != CON_ANY)		//不是通配符
		{
			//不匹配
			if (syllables[i].tone != TONE_0 && !ZiContainTone(ci[i], checked_syllables[i], syllables[i].tone))
				return 0;

			continue;
		}

		//通配符
		if (i == length - 1)		//最后一个是通配符，肯定匹配
			return 1;

		//需要使用递归来处理
		for (j = i + 1; j < checked_length; j++)
		{
			if (WildCompareSyllablesWithCi(
				syllables + i + 1,				//跳过通配符
				length - i - 1,					//长度减少
				checked_syllables + j,			//被比较音节
				ci + j,							//被比较的词
				checked_length - j,				//被比较长度
				fuzzy_mode
				))
				return 1;
		}
		break;
	}

	//最后的比较必须加上比较程度等于被比较长度，否则会
	//出现部分匹配，如：zg*j  与 zgrmjcy相互匹配。
	return i == length && length == checked_length;
}

/*	比较一组音节，进行通配符的比较，不进行音调比较。
 *	检查音节可能含有通配符，但被检查音节中不包括这些。
 *	不应有连续两个以上的通配符。
 *	参数：
 *		syllables					比较的音节
 *		length						比较音节长度
 *		checked_syllables			被比较音节
 *		checked_length				被比较音节的长度
 *		fuzzy_mode					模糊方式
 *	返回：
 *		相符：1
 *		不相符：0
 */
int WildCompareSyllables(SYLLABLE *syllables, int length, SYLLABLE *checked_syllables, int checked_length, int fuzzy_mode)
{
	int i, j;

	//被比较音节长度大于等于比较音节
	if (length > checked_length)
		return 0;

	for (i = 0; i < length; i++)
	{
		if (syllables[i].con != CON_ANY)		//不是通配符
		{
			if (!ContainSyllable(syllables[i], checked_syllables[i], fuzzy_mode))
				return 0;		//不匹配
			continue;
		}

		//通配符
		if (i == length - 1)		//最后一个是通配符，肯定匹配
			return 1;

		//需要使用递归来处理
		for (j = i + 1; j < checked_length; j++)
		{
			if (WildCompareSyllables(
				syllables + i + 1,				//跳过通配符
				length - i - 1,					//长度减少
				checked_syllables + j,			//跳过通配符
				checked_length - j,				//长度减少
				fuzzy_mode))
				return 1;
		}
		break;
	}

	//最后的比较必须加上比较程度等于被比较长度，否则会
	//出现部分匹配，如：zg*j  与 zgrmjcy相互匹配。
	return i == length && length == checked_length;
}

/*	比较一组音节，不进行通配符的比较，不进行音调比较。
 *	注：不能有两个以上的通配符。
 *	参数：
 *		syllables					比较的音节
 *		checked_syllables			被比较音节
 *		length						比较音节长度
 *		fuzzy_mode					模糊方式
 *	返回：
 *		相符：1
 *		不相符：0
 */
int CompareSyllables(SYLLABLE *syllables, SYLLABLE *checked_syllables, int length, int fuzzy_mode)
{
	int i;

	//依次进行比较
	for (i = 0; i < length; i++)
		if (!ContainSyllable(syllables[i], checked_syllables[i], fuzzy_mode))
			return 0;		//不匹配

	return 1;
}

/*	比较一组音节的首字母是否与目标相符，进行通配符的比较，
 *	不进行音调比较。
 *	参数：
 *		letters						比较的首字母数组
 *		length						比较首字母长度
 *		checked_syllables			被比较音节
 *		checked_length				被比较音节的长度
 *	返回：
 *		相符：1
 *		不相符：0
 */
int WildCompareSyllablesAndLetters(const TCHAR *letters, int length, SYLLABLE *checked_syllables, int checked_length)
{
	int i, j;

	//被比较音节长度必须大于等于比较音节
	if (checked_length < length)
		return 0;

	for (i = 0; i < length; i++)
	{
		if (letters[i] != SYLLABLE_ANY_CHAR)		//不是通配符
		{
			if (!SyllableStartWithLetter(letters[i], checked_syllables[i]))
				return 0;			//不匹配
			continue;
		}

		if (i == length - 1)		//最后一个是通配符
			return 1;

		//通配符
		for (j = i + 1; j < checked_length; j++)
			if (WildCompareSyllablesAndLetters(
				letters + i + 1,
				length - i - 1,
				checked_syllables + j,
				checked_length - j))
				return 1;

		//不匹配
		return 0;
	}

	return i == length && length == checked_length;
}

/*	比较一组音节的首字母是否与目标相符，不进行通配符的比较，不进行音调比较。
 *	参数：
 *		letters						比较的首字母数组
 *		checked_syllables			被比较音节
 *		length						比较首字母长度
 *	返回：
 *		相符：1
 *		不相符：0
 */
int CompareSyllablesAndLetters(const TCHAR *letters, SYLLABLE *checked_syllables, int length)
{
	int i;

	for (i = 0; i < length; i++)
		if (!SyllableStartWithLetter(letters[i], checked_syllables[i]))
			return 0;			//不匹配

	return 1;
}

/*	判断是否为音调的字符
 *	参数：
 *		ch			字符
 *	返回：
 *		是：1
 *		否：0
 */
int IsToneChar(char ch)
{
	return (ch == TONE_CHAR_1 || ch == TONE_CHAR_2 || ch == TONE_CHAR_3 || ch == TONE_CHAR_4);
}

/**	判断两个音节是否完全相符
 */
int SameSyllable(SYLLABLE s1, SYLLABLE s2)
{
	return *(unsigned short*)&s1 == *(unsigned short*)&s2;
}

/**	是否包含音节分隔符
 */
int HasSyllableSeparator(const TCHAR *str)
{
	while(*str)
	{
		if (*str++ == SYLLABLE_SEPARATOR_CHAR)
			return 1;
	}

	return 0;
}

#define	SECTION1		TEXT("[声母]")
#define	SECTION2		TEXT("[韵母]")
#define	SECTION3		TEXT("[零声母音节的韵母]")

/*	装载双拼INI文件数据
 */
int LoadSPIniData(const TCHAR *ini_name)
{
	TCHAR *buffer;
	TCHAR line[0x400];
	int  length, count, index, i;
	TCHAR *sp_string1, *sp_string2;
	int  section = 0;

	if (share_segment->sp_loaded)
		return 1;

	memcpy(share_segment->vow_sp_string_single, share_segment->vow_sp_string_single_save, sizeof(share_segment->vow_sp_string_single));
	memcpy(share_segment->vow_sp_string, share_segment->vow_sp_string_save, sizeof(share_segment->vow_sp_string));
	memcpy(share_segment->con_sp_string, share_segment->con_sp_string_save, sizeof(share_segment->con_sp_string));

	share_segment->sp_used_v = share_segment->sp_used_i = 0;

	length = GetFileLength(ini_name);
	if (length <= 0)
		return 0;

	buffer = (TCHAR *)malloc(length);
	if (!LoadFromFile(ini_name, buffer, length))
	{
		free(buffer);
		return 0;
	}

	//获得双拼定义条目
	index = 1;
	length = length / sizeof(TCHAR);

	while(index < length)
	{
		count = 0;
		while(index < length && count < _SizeOf(line) - 1)
		{
			line[count++] = buffer[index++];
			if (buffer[index - 1] == 0xa)
			{
				count--;
				break;
			}
		}

		line[count] = 0;		//得到一行数据

		if ('#' == line[0] || ';' == line[0])		//注释
			continue;

		if (count && (line[count - 1] == 0xd || line[count - 1] == 0xa))
			line[count - 1] = 0;

		if (!_tcscmp(SECTION1, line))
			section = 1;
		else if (!_tcscmp(SECTION2, line))
			section = 2;
		else if (!_tcscmp(SECTION3, line))
			section = 3;

		if (!section)
			continue;

		sp_string1 = _tcstok(line, TEXT("="));
		sp_string2 = _tcstok(0, TEXT("="));
		if (!sp_string1 || !sp_string2)
			continue;

		_tcslwr_s(sp_string2, 8);	//变为小写

		//查找CON、VOW，进行替换
		if (section == 1)
		{
			for (i = 0; i < CON_ANY; i++)
			{
				if (!_tcscmp(sp_string1, share_segment->con_sp_string_save[i]))
				{
					_tcscpy_s(share_segment->con_sp_string[i], 8, sp_string2);
					if (sp_string2[0] == 'v')
						share_segment->sp_used_v = 1;
					else if (sp_string2[0] == 'i')
						share_segment->sp_used_i = 1;
					else if (sp_string2[0] == 'u')
						share_segment->sp_used_u = 1;

					break;
				}
			}

			continue;
		}

		if (section == 2)
		{
			for (i = 0; i < VOW_ANY; i++)
			{
				if (!_tcscmp(sp_string1, share_segment->vow_sp_string_save[i]))
				{
					_tcscpy_s(share_segment->vow_sp_string[i], 8, sp_string2);
					if (sp_string2[0] == 'v')
						share_segment->sp_used_v = 1;
					else if (sp_string2[0] == 'i')
						share_segment->sp_used_i = 1;
					else if (sp_string2[0] == 'u')
						share_segment->sp_used_u = 1;

					break;
				}
			}

			continue;
		}

		if (section == 3)
		{
			for (i = 0; i < VOW_ANY; i++)
			{
				if (!_tcscmp(sp_string1, share_segment->vow_sp_string_single_save[i]))
				{
					_tcscpy_s(share_segment->vow_sp_string_single[i], 8, sp_string2);
					break;
				}
			}

			continue;
		}
	}

	free(buffer);

	//对音节表进行字符串组合
	for (i = 0; i < share_segment->syllable_map_items; i++)
	{
		int con = share_segment->syllable_map[i].con;
		int vow = share_segment->syllable_map[i].vow;

		if (con == CON_NULL)
		{
			_tcscpy_s(line, _SizeOf(line), share_segment->vow_sp_string_single[vow]);
		}
		else
		{
			_tcscpy_s(line, _SizeOf(line), share_segment->con_sp_string[con]);
			_tcscat_s(line, _SizeOf(line), share_segment->vow_sp_string[vow]);
		}

		if (_tcslen(line) <= 2)
			_tcscpy_s(share_segment->syllable_map[i].sp_pin_yin, _SizeOf(share_segment->syllable_map[0].sp_pin_yin), line);
		else
			Log(LOG_ID, L"双拼编码存在错误, %s", share_segment->con_sp_string[con], share_segment->vow_sp_string[vow]);
	}

	share_segment->sp_loaded = 1;
	return 1;
}

/*	卸载双拼数据
 */
int FreeSPIniData()
{
	share_segment->sp_used_v = share_segment->sp_used_i = share_segment->sp_loaded = 0;
	return 1;
}

void InsertHint(TCHAR *hint_buffer, int buffer_length, const TCHAR *src, const TCHAR *tag)
{
	TCHAR upper_str[0x100];
	int  i;
	static int count;

	if (!*hint_buffer)
		count = 0;

	if (!_tcscmp(src, tag))
		return;

	_tcscpy_s(upper_str, _SizeOf(upper_str), src);
	for (i = 0; upper_str[i]; i++)
		if (upper_str[i] >= 'a' && upper_str[i] <= 'z')
			upper_str[i] = upper_str[i] ^ 0x20;

	_tcscat_s(hint_buffer, buffer_length, tag);
	_tcscat_s(hint_buffer, buffer_length, TEXT("="));
	_tcscat_s(hint_buffer, buffer_length, upper_str);
	_tcscat_s(hint_buffer, buffer_length, TEXT(" "));

	count++;
	if (!(count % 8))
		_tcscat_s(hint_buffer, buffer_length, TEXT("\n"));
}

/**	获取双拼提示信息
 */
void GetSPHintString(PIMCONTEXT *context, TCHAR *hint_buffer, int buffer_length)
{
	TCHAR last_ch = 0;
	int  mode;		//0: input start, 1: single vow, 2: has con
	int  i;
	int  con, vow;
	const TCHAR *src_string, *tag_string;

	if (!share_segment)
		return;

	*hint_buffer = 0;

	if (!context->input_length)
	{
		mode = 0;
	}
	else if (context->syllable_count)
	{
		con = context->syllables[context->syllable_count - 1].con;
		vow = context->syllables[context->syllable_count - 1].vow;

		if (vow == VOW_NULL)
			mode = 2;
		else
			mode = 0;
	}
	else
	{
		last_ch = context->input_string[context->input_length - 1];
		if (last_ch < 'a' || last_ch > 'z')
			return;

		mode = 1;
	}

	if (mode == 0)		//目前没有输入
	{
		for (i = 0; i < share_segment->syllable_map_items; i++)
		{
			/*if (share_segment->syllable_map[i].correct_flag)
				continue;*/

			if (share_segment->syllable_map[i].con == CON_NULL)	//单韵母
			{
				vow = share_segment->syllable_map[i].vow;

				src_string = share_segment->vow_sp_string_single[vow];
				tag_string = share_segment->vow_sp_string_single_save[vow];
				InsertHint(hint_buffer, buffer_length, src_string, tag_string);
				continue;
			}

			if (share_segment->syllable_map[i].vow != VOW_NULL)
				continue;

			//存在声母情况
			con = share_segment->syllable_map[i].con;
			src_string = share_segment->con_sp_string[con];
			tag_string = share_segment->con_sp_string_save[con];

			InsertHint(hint_buffer, buffer_length, src_string, tag_string);
		}

		return;
	}

	//mode1, 单韵母情况
	if (mode == 1)
	{
		for (i = 0; i < share_segment->syllable_map_items; i++)
		{
			/*if (share_segment->syllable_map[i].correct_flag)
				continue;*/

			if (share_segment->syllable_map[i].con != CON_NULL)
				continue;

			vow = share_segment->syllable_map[i].vow;
			src_string = share_segment->vow_sp_string_single[vow];
			tag_string = share_segment->vow_sp_string_single_save[vow];

			if (src_string[0] != last_ch)
				continue;

			InsertHint(hint_buffer, buffer_length, src_string, tag_string);
		}

		return;
	}

	//mode2, 存在声母的情况
	if (mode == 2)
	{
		for (i = 0; i < share_segment->syllable_map_items; i++)
		{
			/*if (share_segment->syllable_map[i].correct_flag)
				continue;*/

			if (share_segment->syllable_map[i].con != con)
				continue;

			if (share_segment->syllable_map[i].vow == VOW_UE && 
				share_segment->syllable_map[i].pin_yin_length >= 2)
			{
				if (share_segment->syllable_map[i].pin_yin[share_segment->syllable_map[i].pin_yin_length - 2] == 'v' &&
					share_segment->syllable_map[i].pin_yin[share_segment->syllable_map[i].pin_yin_length - 1] == 'e')
					continue;
			}

			vow = share_segment->syllable_map[i].vow;
			src_string = share_segment->vow_sp_string[vow];
			tag_string = share_segment->vow_sp_string_save[vow];

			InsertHint(hint_buffer, buffer_length, src_string, tag_string);
		}

		return;
	}

	return;
}

/**	判断是否为完整的拼音音节。
 *	完整音节，不能包含通配符，以及生母简拼的音节
 */
int IsFullSyllable(SYLLABLE *syllable_array, int syllable_count)
{
	int i;

	for (i = 0; i < syllable_count; i++)
	{
		if (syllable_array[i].vow == VOW_NULL)		//不完整音节
			return 0;

		if (syllable_array[i].con == CON_ANY)		//通配符，直接返回
			return 0;
	}
	return 1;
}

/**	判断是否存在另外一种解析方式。
 *	如：lianai  -> lia'nai(ori)
 *				-> lian'ai(other)
 *		dangan	-> dan'gan(ori)
 *				-> dang'an(other)
 *		xian	-> xi'an
 *		xiang	-> xi'ang
 *	采用方式：华宇拼音采用反向最大匹配算法进行音节拆分，现在
 *	需要进行正向的最大匹配；解析后的的结果需要判断是否为完整
 *	拼音。
 *
 *	参数：
 *		context
 *		syllable_start
 *		syllable_count
 *		other_syllable_array
 *		other_syllable_count
 *	返回：
 *		存在另一种拆分方式：返回音节数目
 *		不存在：0
 */
int GetOtherSyllableParse(PIMCONTEXT *context, int syllable_start, int syllable_count, SYLLABLE *other_syllables, int other_length)
{
	TCHAR syllable_string[MAX_SYLLABLE_PER_INPUT * MAX_PINYIN_LENGTH];
	int  string_length = 0;
	int  i, start_pos, end_pos;
	int  other_count;

	//双拼不需要进行计算
	if (pim_config->pinyin_mode == PINYIN_SHUANGPIN/* || !pim_config->special_parse_pin_yin*/)
		return 0;

	//目前MAX_OTHER_PARSE_SYLLABLE为32，即未作限制
	if (syllable_count > MAX_OTHER_PARSE_SYLLABLE)
		return 0;

	//获取输入字符串
	start_pos = context->syllable_start_pos[syllable_start];
	end_pos   = (syllable_count == context->syllable_count - syllable_start) ?
				 -1 : context->syllable_start_pos[syllable_start + syllable_count];

	if (end_pos == -1)
		_tcscpy_s(syllable_string, _SizeOf(syllable_string), context->input_string + start_pos);
	else
		_tcsncpy_s(syllable_string, _SizeOf(syllable_string), context->input_string + start_pos, end_pos - start_pos);

	//具有分隔符的音节串，不予解析
	if (HasSyllableSeparator(syllable_string))
		return 0;

	//现在已经拥有了拼音串，可以进行反向查找了。
	other_count = ParsePinYinString(syllable_string, other_syllables, other_length, pim_config->use_fuzzy ? pim_config->fuzzy_mode : 0/*, 0, 0*/);

	//非完整拼音串
	if (!IsFullSyllable(other_syllables, other_count))
		return 0;

	if (other_count != syllable_count)		//解析结果不相同
		return other_count;

	//判断是否音节完全相同
	for (i = 0; i < syllable_count; i++)
		if (*(short*)&other_syllables[i] != *(short*)&context->syllables[i + syllable_start])
			return other_count;

	return 0;
}

/**	判断一个音节是否可以拆分为两个小音节
 *
 *	返回：
 *		可拆分：1
 *		不可：  0
 */
static int IsSmallSyllable(SYLLABLE syllable)
{
	int i;

	for (i = 0; i < _SizeOf(small_syllable_items); i++)
	{
		if (small_syllable_items[i].syllable.con == syllable.con &&
			small_syllable_items[i].syllable.vow == syllable.vow)
		{
			return 1;
		}
	}
	
	return 0;
}

/** 辅助GetSmallSyllablesParse，完成递归算法
*/
static int ProcessSmallSyllables(SYLLABLE *syllables, int syllable_count, SYLLABLE *other_arrays, int *other_lengths, int syllable_pos, int other_count)
{
	SYLLABLE small_syllables[2];
	SYLLABLE ori_syllable;
	int i, j;

	for (i = syllable_pos; i < syllable_count; i++)
	{
		//每一个可拆分的单字音节都可看做是一个结点，由此结点开始分成两种
		//情况进行递归，形成一个类似于二叉树的结构
		if (IsSmallSyllable(syllables[i]))
		{
			other_count = ProcessSmallSyllables(syllables, syllable_count, other_arrays, other_lengths, i + 1, other_count);

			GetSmallSyllables(syllables[i], small_syllables);

			//若进行拆分，将导致溢出，直接返回
			if (syllable_count + 1 > MAX_SYLLABLE_PER_INPUT)
			{
				return other_count;
			}

			//用small_syllables替换掉原音节
			ori_syllable = syllables[i];
			for (j = syllable_count - 1; j >= i + 1; j--)
			{
				syllables[j + 1] = syllables[j];
			}
			syllables[i] = small_syllables[0];
			syllables[i + 1] = small_syllables[1];

			other_count = ProcessSmallSyllables(syllables, syllable_count + 1, other_arrays, other_lengths, i + 2, other_count);

		    //用原音节替换掉small_syllables(这样做非常重要，因为所有递归函数
			//使用的拼音串都是syllables，如果不还原，将对其他递归分支产生影响)
			for (j = i + 1; j <= syllable_count + 1 - 1; j++) //注意此时实际的音节数为syllable_count+1
			{
				syllables[j - 1] = syllables[j];
			}
			syllables[i] = ori_syllable;

			//特别注意，递归完成后立即终止循环，因为此时以当前结点为根的分支
			//已经遍历完成，应立即返回树的上一层；否则将造成重复访问左子树或
			//右子树
			break;
		}
	}

	//到达拼音串末尾，增加一个解析结果
	if (i >= syllable_count)
	{
		for (i = 0; i < syllable_count; i++)
		{
			(other_arrays + other_count * MAX_SYLLABLE_PER_INPUT)[i] = syllables[i];
		}
		*(other_lengths + other_count) = syllable_count;
		other_count++;
	}

	return other_count;
}

/**	判断是否可以通过拆分单字的音节获得音节的另一种解析方式
 *	如：lianaiting  -> lianting(ori)
 *				    -> lian'an'ting(other)
 *		xianshi  	-> xian'shi(ori)
 *				    -> xi'an'shi(other)
 *		jieruchou   -> jie'ru'chou(ori)
 *                  -> ji'e'ru'chou(other)
 *	采用方式：依次检查每一个音节，看是否在small_syllable_items中，
 *  如果在则尝试着将该音节拆分成两个音节，对于拆分前后的拼音串，分
 *  别作为一种情况，从下一个音节开始递归，最终遍历到所有的拆分结果
 *
 *	参数：
 *		syllables               待分析的拼音串
 *		syllable_count          syllables中音节的数目
 *		syallable_arrays        结果数组，每个元素是一个拼音串
 *		other_length            syallable_arrays每个串的音节数目
 *
 *	返回：
 *		存在另外的解析方式：    解析方式的数目
 *		不存在：                0
 */
int GetSmallSyllablesParse(SYLLABLE *syllables, int syllable_count, SYLLABLE *other_arrays, int *other_lengths)
{
	int  small_syllable_count = 0;
	int  other_count = 0;
	int  i;

	//双拼不需要进行计算
	if (pim_config->pinyin_mode == PINYIN_SHUANGPIN/* || !pim_config->special_parse_pin_yin*/)
		return 0;

	//不用处理单字的情况GetCandidates::ProcessZiCandidates里有专门处理单个音节的代码
	if (syllable_count <= 1)
	{
		return 0;
	}

	//最大能处理的可拆分音节数，如果一个单字的音节有两种拆分方式，
	//某句中有k个单字可拆分，共有2^k种不同的解析方式需要检查，这
	//无疑会造成计算量过大，因此在找到更优的算法前需要作出一些限
	//制
	for (i = 0; i < syllable_count; i++)
	{
		if (IsSmallSyllable(syllables[i]))
		{
			small_syllable_count++;
		}
	}
	if (small_syllable_count > MAX_SMALL_SYLLABLES)		
		return 0;

	//递归算法，注意下面的函数在调用过程中可能改变syllables，但调用
	//完成后会将其恢复到和调用之前一样
	other_count = ProcessSmallSyllables(syllables, syllable_count, other_arrays, other_lengths, 0, 0);

	if (other_count)
	{
		other_count--; //根据递归算法，第一个解析结果就是原拼音串，应排除
		memmove(other_arrays, other_arrays + MAX_SYLLABLE_PER_INPUT, other_count * MAX_SYLLABLE_PER_INPUT * sizeof(SYLLABLE));
		memmove(other_lengths, other_lengths + 1, other_count * sizeof(int));
	}

	return other_count;
}

//获得单个音节拆分成的两个小音节
int GetSmallSyllables(SYLLABLE syllable, SYLLABLE *small_syllables)
{
	int i;

	for (i = 0; i < sizeof(small_syllable_items) / sizeof(small_syllable_items[0]); i++)
	{
		if (small_syllable_items[i].syllable.con == syllable.con &&
			small_syllable_items[i].syllable.vow == syllable.vow)
		{
			*(short*)&small_syllables[0] = *(short*)&small_syllable_items[i].small_syllables[0];
			*(short*)&small_syllables[1] = *(short*)&small_syllable_items[i].small_syllables[1];
			small_syllables[1].tone = syllable.tone;

			return 1;
		}
	}

	return 0;
}