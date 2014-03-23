#ifndef	_PIM_RESOURCE_H
#define	_PIM_RESOURCE_H

#ifdef __cplusplus
extern "C"
{
#endif

extern int PIM_LoadResources();
extern int PIM_FreeResources();
extern int PIM_SaveResources();
extern int PIM_ReloadINIResource();
extern int PIM_ReloadWordlibResource();
extern int PIM_ForceReloadWordlibResource();

extern int PIM_ReloadEnglishResource();
extern int PIM_ReloadEnglishTransResource();
extern int PIM_ReloadEnglishAllResource();

//extern int __stdcall PIM_ReloadURLResource();
extern int __stdcall PIM_ReloadBHResource();
extern int __stdcall PIM_ReloadZiResource();
//extern int __stdcall PIM_ReloadSPWResource();
extern int __stdcall PIM_ReloadFontMapResource();

extern int LoadWordLibraryResource();
extern int LoadHZDataResource();
extern int LoadSpwResource();

#ifdef __cplusplus
}
#endif

#endif
