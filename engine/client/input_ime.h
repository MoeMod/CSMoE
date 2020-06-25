#pragma once

const char* IME_GetCompositionString();
size_t IME_GetCandidateListCount();
size_t IME_GetCandidateListPageStart();
size_t IME_GetCandidateListPageSize();
size_t IME_GetCandidateListSelectedItem();
int IME_CandidateListStartsAtOne();
const char* IME_GetCandidate(size_t i);
void IME_SetInputScreenPos(int x, int y);
void IME_GetInputScreenPos(int *x, int *y);
int IME_ShouldShowBuiltInCandidateWindow();

// related to window, not scaled
void WinRT_ImeCreateContext();
void WinRT_ImeEnableTextInput(int enable, int force);
