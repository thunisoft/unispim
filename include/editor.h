#ifndef	_EDITOR_H_
#define	_EDITOR_H_

#include <kernel.h>
#include <context.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define	ZFW_NONE		0
#define	ZFW_LEFT		1
#define	ZFW_RIGHT		2

extern void ChangeTone(PIMCONTEXT *context, TCHAR ch);
extern void AddChar(PIMCONTEXT *context, TCHAR ch, int is_numpad);
extern void DelChar(PIMCONTEXT *context);
extern void BackspaceChar(PIMCONTEXT *context);
extern void ProcessContext(PIMCONTEXT *context);
extern void ProcessCandidateStuff(PIMCONTEXT *context);
extern void PrevCandidatePage(PIMCONTEXT *context);
extern void NextCandidatePage(PIMCONTEXT *context);
extern void EndCandidateItem(PIMCONTEXT *context);
extern void HomeCandidateItem(PIMCONTEXT *context);
extern void SelectCandidate(PIMCONTEXT *context, int index);
extern int  SelectZFWCandidate(PIMCONTEXT *context, int index, int left_or_right);
extern void SelectInputString(PIMCONTEXT *context, int is_space);
extern void SelectInputStringWithSelectedItem(PIMCONTEXT *context);
extern void OutURLString(PIMCONTEXT *context);
extern void PrevCandidateItem(PIMCONTEXT *context);
extern void NextCandidateItem(PIMCONTEXT *context);
extern void NextCandidateLine(PIMCONTEXT *context);
//extern void SetSelectedCandidateItem(PIMCONTEXT *context, int n);
extern void PrevCandidateLine(PIMCONTEXT *context);
extern void MoveCursorHead(PIMCONTEXT *context);
extern void MoveCursorTail(PIMCONTEXT *context);
extern void MoveCursorLeft(PIMCONTEXT *context);
extern void MoveCursorRight(PIMCONTEXT *context);
extern void MoveCursorByLetter(PIMCONTEXT *context, TCHAR ch);
extern void PrevSyllable(PIMCONTEXT *context);
extern void NextSyllable(PIMCONTEXT *context);
extern void ProcessDigitalKey(PIMCONTEXT *context, TCHAR ch);
extern void JumpToZiOrNextPage(PIMCONTEXT *context);
extern void ClearSelectedDigital(PIMCONTEXT *context);
extern void DeleteCi(PIMCONTEXT *context, int selected_index);
extern void MakeCandidate(PIMCONTEXT *context);
extern int BackSelectedCandidate(PIMCONTEXT *context);
extern int GetCandidateSyllable(CANDIDATE *candidate, SYLLABLE *syllables, int length);
extern int GetCandidateSyllableCount(CANDIDATE *candidate);
extern int GetCandidateDisplayString(PIMCONTEXT *context, CANDIDATE *candidate, TCHAR *buffer, int length, int first_candidate);
extern void CheckDeleteNewCi(int key);
extern void PrepareDeleteNewCi(HZ *new_ci, int ci_length, SYLLABLE *syllable, int syllable_length);
extern void RunCommand(PIMCONTEXT *context, const TCHAR *cmd_string);
extern int GetInputPos(PIMCONTEXT *context);
extern int GetSyllableIndexByPosition(PIMCONTEXT *context, int pos);
extern int GetSyllableIndexByComposeCursor(PIMCONTEXT *context, int pos);
extern int GetSyllableIndexInDefaultString(PIMCONTEXT *context, int syllable_index);

extern int IEditModeStart(PIMCONTEXT *context);
extern int IEditModeEnd(PIMCONTEXT *context);
extern void IEditPrevSyllable(PIMCONTEXT *context);
extern void IEditNextSyllable(PIMCONTEXT *context);
extern void IEditMoveCursorHead(PIMCONTEXT *context);
extern void IEditMoveCursorTail(PIMCONTEXT *context);
extern void IEditSelectCandidate(PIMCONTEXT *context, int index);
extern void IEditSelectResult(PIMCONTEXT *context);

#ifdef __cplusplus
}
#endif

#endif