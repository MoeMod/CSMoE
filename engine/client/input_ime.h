/*
input_ime.h
Copyright (C) 2020 Moemod Haoyuan

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#pragma once

const char* IME_GetCompositionString();
size_t IME_GetCandidateListCount();
size_t IME_GetCandidateListPageStart();
size_t IME_GetCandidateListPageSize();
size_t IME_GetCandidateListSelectedItem();
int IME_CandidateListStartsAtOne();
const char* IME_GetCandidate(size_t i);
void IME_SetInputScreenPos(int x, int y);
int IME_ShouldShowBuiltInCandidateWindow();
void IME_CreateContext();
void IME_SetIMEEnabled(int enable, int force);