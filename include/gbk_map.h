#ifndef	_GBK_MAP_H_
#define	_GBK_MAP_H_

#include <kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GBKMAP_SIZE 0xFFFF
//#define GBKMAP_SCOPE_NUM 284

extern int GenGBKMapData();
extern int FreeGBKMapData();
extern int IsGBK(UC zi);

#ifdef __cplusplus
}
#endif

#endif