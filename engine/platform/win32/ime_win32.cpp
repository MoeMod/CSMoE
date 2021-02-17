/*
ime_win32.cpp
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

extern "C" {
#include "common.h"
#include "keydefs.h"
#include "input_ime.h"
}
#include <WinUser.h>
#include <SDL.h>
#include <SDL_version.h>
#include <SDL_syswm.h>
#include <commctrl.h>
#include <string>
#include <vector>
#include <memory>
#include <assert.h>

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "comctl32.lib" )

static HWND Win32_GetHWND()
{
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(host.hWnd, &wmInfo);
    return wmInfo.info.win.window;
}

static std::string comp_str;
static std::vector<std::string> current_candidates;
static size_t page_start;
static size_t page_size;
static size_t selected_candicate;

const char* IME_GetCompositionString()
{
    return comp_str.c_str();
}

size_t IME_GetCandidateListCount()
{
    return current_candidates.size();
}

size_t IME_GetCandidateListPageStart()
{
    return page_start;
}

size_t IME_GetCandidateListPageSize()
{
    return page_size;
}

size_t IME_GetCandidateListSelectedItem()
{
    return selected_candicate;
}

int IME_CandidateListStartsAtOne()
{
    return 1;
}

const char* IME_GetCandidate(size_t i)
{
    return current_candidates[i].c_str();
}

void IME_SetInputScreenPos(int x, int y)
{
    HWND hwnd = Win32_GetHWND();
    if (HIMC himc = ::ImmGetContext(hwnd))
    {
        COMPOSITIONFORM cf;
        cf.ptCurrentPos.x = x;
        cf.ptCurrentPos.y = y;
        cf.dwStyle = CFS_FORCE_POSITION;
        ::ImmSetCompositionWindow(himc, &cf);
        ::ImmReleaseContext(hwnd, himc);
    }
}

void IME_GetInputScreenPos(int* x, int* y)
{
    HWND hwnd = Win32_GetHWND();
    if (HIMC himc = ::ImmGetContext(hwnd))
    {
        COMPOSITIONFORM cf;
        ::ImmGetCompositionWindow(himc, &cf);

        if (x) *x = cf.ptCurrentPos.x;
        if (y) *y = cf.ptCurrentPos.y;

        ::ImmReleaseContext(hwnd, himc);
    }
}

int IME_ShouldShowBuiltInCandidateWindow()
{
    return true;
}

static void ClearCandidateList()
{
    current_candidates.clear();
    selected_candicate = 0;
}

static void CompositionString()
{
    comp_str.clear();
}

static void UpdateCandidateList(const CANDIDATELIST* const src, const size_t src_size)
{
    assert(nullptr != src);
    assert(sizeof(CANDIDATELIST) <= src->dwSize);
    assert(src->dwSelection < src->dwCount);

    ClearCandidateList();

    const char* const baseaddr = reinterpret_cast<const char*>(src);

    for (size_t i = 0; i < src->dwCount; ++i) {
        const wchar_t* const item = reinterpret_cast<const wchar_t*>(baseaddr + src->dwOffset[i]);
        const int require_byte = WideCharToMultiByte(CP_UTF8, 0, item, -1, nullptr, 0, NULL, NULL);
        if (0 < require_byte) {
            std::unique_ptr<char[]> utf8buf{ new char[require_byte] };
            if (require_byte == WideCharToMultiByte(CP_UTF8, 0, item, -1, utf8buf.get(), require_byte, NULL, NULL)) {
                current_candidates.emplace_back(utf8buf.get());
                continue;
            }
        }
        current_candidates.emplace_back("??");
    }

    selected_candicate = src->dwSelection;
    page_start = src->dwPageStart;
    page_size = src->dwPageSize;
}

 // ���줾�� UTF-8 �ˉ�Q���뤿��˥����v�����⤹��
/*
    std::wstring �� std::unique_ptr<char[]> �� UTF-8 ��null �K�������Фˤ��Ɖ�Q����
    �������������ФΈ��Ϥ� nullptr �����
*/
static std::string to_utf8(const std::wstring& arg) {
    std::string utf8buf;

    if (arg.empty()) {
        return utf8buf;
    }
    const int require_byte = WideCharToMultiByte(CP_UTF8, 0, arg.c_str(), -1, nullptr, 0, NULL, NULL);
    if (0 == require_byte) {
        const DWORD lastError = GetLastError();
        (void)(lastError);
        assert(ERROR_INSUFFICIENT_BUFFER != lastError);
        assert(ERROR_INVALID_FLAGS != lastError);
        assert(ERROR_INVALID_PARAMETER != lastError);
        assert(ERROR_NO_UNICODE_TRANSLATION != lastError);
    }
    assert(0 != require_byte);
    if (!(0 < require_byte)) {
        return utf8buf;
    }

    utf8buf.resize(require_byte, '\0');

    const int conversion_result =
        WideCharToMultiByte(CP_UTF8, 0, arg.c_str(), -1, utf8buf.data(), require_byte, NULL, NULL);
    if (conversion_result == 0) {
        const DWORD lastError = GetLastError();
        (void)(lastError);
        assert(ERROR_INSUFFICIENT_BUFFER != lastError);
        assert(ERROR_INVALID_FLAGS != lastError);
        assert(ERROR_INVALID_PARAMETER != lastError);
        assert(ERROR_NO_UNICODE_TRANSLATION != lastError);
    }

    assert(require_byte == conversion_result);
    if (require_byte != conversion_result) {
        utf8buf.clear();
    }
    return utf8buf;
};

static void OnIMEChangeCandidates()
{
    HWND hwnd = Win32_GetHWND();
    HIMC const hImc = ImmGetContext(hwnd);
    if (hImc) {
        DWORD dwSize = ImmGetCandidateListW(hImc, 0, NULL, 0);
        if (dwSize) {
            assert(sizeof(CANDIDATELIST) <= dwSize);

            std::vector<char> candidatelist((size_t)dwSize);
            if ((DWORD)(std::size(candidatelist) * sizeof(typename decltype(candidatelist)::value_type))
                == ImmGetCandidateListW(hImc, 0,
                    reinterpret_cast<CANDIDATELIST*>(candidatelist.data()),
                    (DWORD)(std::size(candidatelist) * sizeof(typename decltype(candidatelist)::value_type)))) {
                const CANDIDATELIST* const cl = reinterpret_cast<CANDIDATELIST*>(candidatelist.data());

                UpdateCandidateList(cl, dwSize);
            }
        }
        ImmReleaseContext(hwnd, hImc);
    }
}

static WNDPROC g_ImeWndProc;
static char s_szInputBuffer[4096];
static LRESULT CALLBACK ImeWndProc(HWND hWnd, UINT uMsg, WPARAM wParam,
    LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch (uMsg)
    {
        case WM_INPUTLANGCHANGE:
        {
            //OnInputLanguageChanged();
            //return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
            break;
        }

        case WM_IME_CONTROL:
        {
            switch (wParam)
            {
            case IMC_SETCANDIDATEPOS:
            {
            }
            }

            break;
        }

        case WM_IME_SETCONTEXT:
        {
            lParam &= ~(ISC_SHOWUICOMPOSITIONWINDOW |
                (ISC_SHOWUICANDIDATEWINDOW) |
                (ISC_SHOWUICANDIDATEWINDOW << 1) |
                (ISC_SHOWUICANDIDATEWINDOW << 2) |
                (ISC_SHOWUICANDIDATEWINDOW << 3));
            //return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
            break;
        }

        case WM_IME_STARTCOMPOSITION:
        {
            //OnIMEStartComposition();
            ClearCandidateList();
            [[fallthrouth]];
        }

        case WM_IME_ENDCOMPOSITION:
        {
            //OnIMEEndComposition();
            ClearCandidateList();
            [[fallthrouth]];
        }

        case WM_IME_COMPOSITION:
        {
            HIMC const hImc = ImmGetContext(hWnd);
            if (hImc) {
                if (lParam & GCS_RESULTSTR) {
                    comp_str.clear();
                }
                if (lParam & GCS_COMPSTR) {

                    /* һ���AĿ�� IME ���� �磻�����֤������Ф���äƤ��� */
                    /* ����ϥХ��ȅgλ�Ǥ��Ȥꤹ��Τǡ�ע�� */
                    const DWORD compstr_length_in_byte = ImmGetCompositionStringW(hImc, GCS_COMPSTR, nullptr, 0);
                    switch (compstr_length_in_byte) {
                    case IMM_ERROR_NODATA:
                    case IMM_ERROR_GENERAL:
                        break;
                    default:
                    {
                        /* �Х��ȅgλ�Ǥ��äƤ����Τǡ�wchar_t �gλ��ֱ���ơ� null���֤���ԣ��Ӥ��ƥХåե������⤹�� */
                        size_t const buf_length_in_wchar = (size_t(compstr_length_in_byte) / sizeof(wchar_t)) + 1;
                        assert(0 < buf_length_in_wchar);
                        std::unique_ptr<wchar_t[]> buf{ new wchar_t[buf_length_in_wchar] };
                        if (buf) {
                            //std::fill( &buf[0] , &buf[buf_length_in_wchar-1] , L'\0' );
                            const LONG buf_length_in_byte = LONG(buf_length_in_wchar * sizeof(wchar_t));
                            const DWORD l = ImmGetCompositionStringW(hImc, GCS_COMPSTR,
                                (LPVOID)(buf.get()), buf_length_in_byte);

                            const DWORD attribute_size = ImmGetCompositionStringW(hImc, GCS_COMPATTR, NULL, 0);
                            std::vector<char> attribute_vec(attribute_size, 0);
                            const DWORD attribute_end =
                                ImmGetCompositionStringW(hImc, GCS_COMPATTR, attribute_vec.data(), (DWORD)std::size(attribute_vec));
                            assert(attribute_end == (DWORD)(std::size(attribute_vec)));
                            {
                                std::wstring comp_converted;
                                std::wstring comp_target;
                                std::wstring comp_unconveted;
                                size_t begin = 0;
                                size_t end = 0;
                                // ��Q�g�ߤ�ȡ�����
                                for (end = begin; end < attribute_end; ++end) {
                                    if ((ATTR_TARGET_CONVERTED == attribute_vec[end] ||
                                        ATTR_TARGET_NOTCONVERTED == attribute_vec[end])) {
                                        break;
                                    }
                                    else {
                                        comp_converted.push_back(buf[end]);
                                    }
                                }
                                // ��Q�g�ߤ��I��[begin,end) 

                                // ��Q�Ф������Ф�ȡ�����
                                for (begin = end; end < attribute_end; ++end) {
                                    if (!(ATTR_TARGET_CONVERTED == attribute_vec[end] ||
                                        ATTR_TARGET_NOTCONVERTED == attribute_vec[end])) {
                                        break;
                                    }
                                    else {
                                        comp_target.push_back(buf[end]);
                                    }
                                }
                                // ��Q�Ф��I�� [begin,end)

                                // δ��Q�������Ф�ȡ�����
                                for (; end < attribute_end; ++end) {
                                    comp_unconveted.push_back(buf[end]);
                                }

                                comp_str = to_utf8(comp_converted) + to_utf8(comp_target) + to_utf8(comp_unconveted);

                            }
                        }
                    }
                    break;
                    }
                }
                ImmReleaseContext(hWnd, hImc);
                OnIMEChangeCandidates();
            }
    #if defined( UNICODE )
            // UNICODE ���ɤΕr�ˤϡ�DefWindowProc ��ֱ��IME�����������ƴ��ɷ�
            return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
            // �ޥ���Х��� ���ɤΕr�ˤϡ�Window ���֥��饹�� �ץ��`���㤬�I����Τǡ� DefSubclassProc �����Ҫ�����롣
    #else
            return ::DefSubclassProc(hWnd, uMsg, wParam, lParam);
    #endif 
        }

        case WM_IME_NOTIFY:
        {
            switch (wParam)
            {
            case IMN_OPENCANDIDATE:
            {
                //OnIMEShowCandidates();
                [[fallthrough]];
            }

            case IMN_CHANGECANDIDATE:
            {
                //OnIMEChangeCandidates();
                OnIMEChangeCandidates();
                break;
            }

            case IMN_CLOSECANDIDATE:
            {
                //OnIMECloseCandidates();
                ClearCandidateList();
                break;
            }

            case IMN_SETCONVERSIONMODE:
            {
                //OnChangeIMEConversionModeByHandle(lParam);
                break;
            }

            case IMN_SETSENTENCEMODE:
            {
                //OnChangeIMESentenceModeByHandle(lParam);
                break;
            }

            case IMN_OPENSTATUSWINDOW: break;
            case IMN_CLOSESTATUSWINDOW: break;
            }
            //return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
            break;
        }
        case WM_IME_REQUEST:
        {
            //return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
            break;
        }
    }

    return ::DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

static HIMC s_himc;

void IME_CreateContext()
{
    
    HWND hwnd = Win32_GetHWND();
    /*
    HIMC himc = ImmGetContext(hwnd);
    if (!himc)
        himc = ImmCreateContext();

    s_himc = himc;
    */
    SetWindowSubclass(hwnd, ImeWndProc,
        reinterpret_cast<UINT_PTR>(ImeWndProc),
        NULL);

    //ImmAssociateContextEx(hwnd, nullptr, IACE_IGNORENOCONTEXT);
}

void IME_SetIMEEnabled(int enable, int force)
{
    /*
    HWND hwnd = Win32_GetHWND();
    ImmAssociateContext(hwnd, enable ? s_himc : NULL);
    */
    if (enable)
        SDL_StartTextInput();
    else
    {
        ClearCandidateList();
        CompositionString();
        SDL_StopTextInput();
    }
}