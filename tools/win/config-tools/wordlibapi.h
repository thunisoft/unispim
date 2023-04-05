#ifndef WORDLIBAPI_H
#define WORDLIBAPI_H
#include <Windows.h>
#include <tchar.h>
#include "syllableapi.h"
#include "wordlibdef.h"



int ExportWordLibrary(const TCHAR *wordlib_file_name, const TCHAR *text_file_name, int *ci_count, TCHAR *err_file_name, int export_all, void *call_back);
int ImportWordLibrary(const TCHAR *wordlib_file_name, const TCHAR *text_file_name, int *ok_count, int *err_count, TCHAR *err_file_name, void *call_back);
int CreateWordLibrary(const TCHAR *wordlib_file_name, const TCHAR *text_file_name, int *ok_count, int *err_count, TCHAR *err_file_name);
int GetWordLibInformation(const TCHAR *name, int *version, TCHAR *wl_name, TCHAR *author_name, int *can_be_edit, int *items);
int CreateEmptyWordLibFile(const TCHAR *wordlib_file_name, const TCHAR *name, const TCHAR *author, int can_be_edit);



int SaveWordLibrary(int wordlib_id,const TCHAR*filePath=NULL);
int SaveWordLibarayToFile(WORDLIB* wordlib, const TCHAR* filePath);
void CloseWordLibrary(int wordlib_id);

#endif // WORDLIBAPI_H
