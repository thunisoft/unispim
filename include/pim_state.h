#ifndef	_PIM_STATE_H_
#define	_PIM_STATE_H_

#include <context.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int IsNeedKey(PIMCONTEXT *context, UINT virtual_key, UINT_PTR scan_code, LPBYTE key_state);
extern void ProcessKey(PIMCONTEXT *context, int key_flag, int virtual_key, TCHAR ch);

#ifdef __cplusplus
}
#endif

#endif