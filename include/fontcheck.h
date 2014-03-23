#ifndef	_FONTCHECK_H_
#define	_FONTCHECK_H_

#include <kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

#define	FONTMAP_FILE_NAME			TEXT("unispim6\\zi\\cmap.dat")
extern int LoadFontMapData(const TCHAR *file_name);
extern int FreeFontMapData();
extern int FontCanSupport(UC zi);

#ifdef __cplusplus
}
#endif

#endif