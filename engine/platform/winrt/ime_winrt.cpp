/*
ime_winrt.cpp
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

#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Text.h>
#include <winrt/Windows.UI.Text.Core.h>
#include <winrt/Windows.UI.ViewManagement.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.Applicationmodel.Core.h>

#include "winrt_interop.h"

#include <msctf.h>
#include <ctffunc.h>
#include <memory>
#include <string>
#include <numeric>

extern "C" {
#include "common.h"
#include "keydefs.h"
#include "input_ime.h"
}
#include "minmax.h"

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::UI;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::UI::ViewManagement;
using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::UI::Text;
using namespace winrt::Windows::UI::Text::Core;

static std::shared_ptr<CoreTextEditContext> s_pImeContext;
static std::wstring s_textCompleted;
static std::wstring s_textUpdating;
static bool s_contextCompositionStarted;
static float s_inputX;
static float s_inputY;
static std::vector<std::string> current_candidates;
static size_t selected_candicate;
static ITfThreadMgr2* m_tm;
static TfClientId tf_client_id;

size_t IME_GetCandidateListCount()
{
	return current_candidates.size();
}

size_t IME_GetCandidateListPageStart()
{
	return 0;
}

size_t IME_GetCandidateListPageSize()
{
	return current_candidates.size();
}

size_t IME_GetCandidateListSelectedItem()
{
	return selected_candicate;
}

int IME_CandidateListStartsAtOne()
{
	return true;
}

const char *IME_GetCandidate(size_t i)
{
	return current_candidates[i].c_str();
}

int IME_ShouldShowBuiltInCandidateWindow()
{
	return true;
}

// Sink receives event notifications
class CUIElementSink : public ITfUIElementSink
{
public:
	CUIElementSink(ITfThreadMgr2* tm) : _cRef(0), m_tm(tm) { m_tm->AddRef(); }
	~CUIElementSink() = default;

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj)
	{
		if (!ppvObj)
			return E_INVALIDARG;

		*ppvObj = nullptr;

		if (IsEqualIID(riid, IID_IUnknown))
		{
			*ppvObj = static_cast<IUnknown*>(static_cast<ITfUIElementSink*>(this));
		}
		else if (IsEqualIID(riid, __uuidof(ITfUIElementSink)))
		{
			*ppvObj = (ITfUIElementSink*)this;
		}

		if (*ppvObj)
		{
			AddRef();
			return S_OK;
		}

		return E_NOINTERFACE;
	}
	STDMETHODIMP_(ULONG) AddRef(void) { return ++_cRef; }
	STDMETHODIMP_(ULONG) Release(void) {
		LONG cr = --_cRef;
		if (_cRef == 0)
		{
			delete this;
		}
		return cr;
	}

	// ITfUIElementSink
	//   Notifications for Reading Window events. We could process candidate as well, but we'll use IMM for simplicity sake.
	STDMETHODIMP BeginUIElement(DWORD dwUIElementId, BOOL* pbShow)
	{
		*pbShow = !IME_ShouldShowBuiltInCandidateWindow();
		selected_candicate = 0;
		current_candidates.clear();
		return S_OK;
	}
	STDMETHODIMP UpdateUIElement(DWORD dwUIElementId)
	{
		auto pElement = GetUIElement(dwUIElementId);
		
		ITfCandidateListUIElement* pcandidate = nullptr;
		pElement->QueryInterface(__uuidof(ITfCandidateListUIElement), (void**)&pcandidate);

		UINT count;
		pcandidate->GetCount(&count);

		UINT selection;
		pcandidate->GetSelection(&selection);
		selected_candicate = selection;

		current_candidates.clear();
		current_candidates.resize(count);
		for (UINT i = 0; i < count; ++i)
		{
			BSTR bstr = nullptr;
			pcandidate->GetString(i, &bstr);
			current_candidates[i] = winrt::to_string(bstr ? bstr : L"");
		}

		{
			ITfDocumentMgr* pDocumentMgr;
			m_tm->CreateDocumentMgr(&pDocumentMgr);

			ITfContext* pContextCandidateWindow;
			TfEditCookie ecTmp;
			pDocumentMgr->CreateContext(tf_client_id, 0, NULL, &pContextCandidateWindow, &ecTmp);

			// get pointer to ITfContextComposition interface
			ITfContextComposition* pContextComposition;
			pContextCandidateWindow->QueryInterface(IID_ITfContextComposition, (void**)&pContextComposition);

			// creates enumerator object for all compositions in context
			IEnumITfCompositionView* pEnumComposition;
			pContextComposition->EnumCompositions(&pEnumComposition);

			// obtain pointer to the composition view from the current 
			// position to the end
			ITfCompositionView* pCompositionView;
			while (pEnumComposition->Next(1, &pCompositionView, NULL) == S_OK) {
				ITfRange* pvRange;
				ULONG writeSize;
				WCHAR wStr[WCHAR_MAX];

				// get range object and receive the text into buffer (wStr)
				pCompositionView->GetRange(&pvRange);
				pvRange->GetText(ecTmp, TF_TF_IGNOREEND, wStr, WCHAR_MAX, &writeSize);
				wStr[writeSize] = NULL;

				// store the composition string into bstr
				// TODO

				pvRange->Release();
				pCompositionView->Release();
			}
			pEnumComposition->Release();
			pContextComposition->Release();

			pContextCandidateWindow->Release();
			pDocumentMgr->Release();
		}

		pcandidate->Release();
		pElement->Release();
		return S_OK;
	}
	STDMETHODIMP EndUIElement(DWORD dwUIElementId)
	{
		selected_candicate = 0;
		current_candidates.clear();
		return S_OK;
	}

protected:
	ITfUIElement* GetUIElement(DWORD dwUIElementId)
	{
		ITfUIElementMgr* puiem;
		ITfUIElement* pElement = nullptr;

		m_tm->QueryInterface(__uuidof(ITfUIElementMgr), (void**)&puiem);
		puiem->GetUIElement(dwUIElementId, &pElement);
		puiem->Release();

		return pElement;
	}

private:
	LONG _cRef;
	ITfThreadMgr2* m_tm;
};
static std::shared_ptr<CUIElementSink> s_pTsfSink;
static DWORD m_dwUIElementSinkCookie;

static void SetupSinks()
{
	// ITfThreadMgrEx is available on Vista or later.
	HRESULT hr;
	hr = CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER, __uuidof(ITfThreadMgr2), (void**)&m_tm);

	// ready to start interacting
	hr = m_tm->ActivateEx(&tf_client_id, 0);

	// Setup sinks
	ITfSource* srcTm;
	hr = m_tm->QueryInterface(__uuidof(ITfSource), (void**)&srcTm);

	s_pTsfSink = std::make_shared<CUIElementSink>(m_tm);

	m_tm->Release();

	// Sink for reading window change
	hr = srcTm->AdviseSink(__uuidof(ITfUIElementSink), (ITfUIElementSink*)s_pTsfSink.get(), &m_dwUIElementSinkCookie);
	srcTm->Release();
}

// Reference : https://social.msdn.microsoft.com/Forums/zh-CN/d1fadb61-6f8b-4ce3-8c30-b93dfca38d7a/uwp-229142030929992-ccx-20195307212101924314textbox?forum=window10app
void IME_CreateContext()
{
	auto manager = CoreTextServicesManager::GetForCurrentView();
	s_pImeContext = std::make_shared<CoreTextEditContext>(manager.CreateEditContext());
	auto& s_ImeContext = *s_pImeContext;
	s_pImeContext->InputPaneDisplayPolicy(CoreTextInputPaneDisplayPolicy::Automatic);
	s_pImeContext->InputScope(CoreTextInputScope::Text);

	s_pImeContext->FocusRemoved([](CoreTextEditContext, IInspectable)
		{
		});

	s_pImeContext->FormatUpdating([](CoreTextEditContext, CoreTextFormatUpdatingEventArgs)
		{
		});

	s_pImeContext->LayoutRequested([](CoreTextEditContext, CoreTextLayoutRequestedEventArgs args)
		{
			auto vb = ApplicationView::GetForCurrentView().VisibleBounds();
			float dpi = WinRT_GetDisplayDPI();
			float x = vb.X * dpi;
			float y = (vb.Y + CoreApplication::GetCurrentView().TitleBar().Height()) * dpi;

			Rect rc;
			rc.X = float(s_inputX) + x;
			rc.Y = float(s_inputY) + y + 16 * dpi; // add some random offset
			rc.Width = float(1);
			rc.Height = float(1);
			args.Request().LayoutBounds().TextBounds(rc);
		});

	s_pImeContext->NotifyFocusLeaveCompleted([](CoreTextEditContext, IInspectable)
		{
		});

	s_pImeContext->SelectionRequested([](CoreTextEditContext, CoreTextSelectionRequestedEventArgs)
		{
		});

	s_pImeContext->SelectionUpdating([](CoreTextEditContext, CoreTextSelectionUpdatingEventArgs)
		{
		});

	s_pImeContext->TextRequested([](CoreTextEditContext, CoreTextTextRequestedEventArgs args)
		{
			args.Request().Text(winrt::to_hstring(""));
		});

	s_pImeContext->TextUpdating([](CoreTextEditContext, CoreTextTextUpdatingEventArgs args)
		{
			auto htext = args.Text();
			
			{
				auto range = args.Range();
				auto size = range.EndCaretPosition - range.StartCaretPosition;
				if (size && range.StartCaretPosition < s_textCompleted.size())
					s_textCompleted.erase(range.StartCaretPosition, size);
				else if (range.StartCaretPosition <= s_textCompleted.size())
					s_textCompleted.insert(range.StartCaretPosition, htext);
			}

			s_textUpdating = htext;
			if (!s_textUpdating.empty() && !s_contextCompositionStarted)
			{
				CL_CharEventUTF(winrt::to_string(s_textUpdating).c_str());
				s_textUpdating.clear();
				s_textCompleted.clear();
				s_pImeContext->NotifyTextChanged(CoreTextRange{ 0, int32_t(s_textUpdating.size()) - 1 }, 0, {});
			}

			args.Result(CoreTextTextUpdatingResult::Succeeded);
		});

	s_pImeContext->CompositionStarted([](CoreTextEditContext, CoreTextCompositionStartedEventArgs)
		{
			s_contextCompositionStarted = true;
		});

	s_pImeContext->CompositionCompleted([](CoreTextEditContext, CoreTextCompositionCompletedEventArgs args)
		{
			if (!s_textUpdating.empty() && s_contextCompositionStarted)
			{
				CL_CharEventUTF(winrt::to_string(s_textUpdating).c_str());
				s_textUpdating.clear();
				s_textCompleted.clear();
				s_pImeContext->NotifyTextChanged(CoreTextRange{ 0, int32_t(s_textUpdating.size()) - 1 }, 0, {});
			}
			s_contextCompositionStarted = false;
		});

	// Get CTF Candidate List
	SetupSinks();
}

const char* IME_GetCompositionString()
{
	static char buf[4096];
	return strncpy(buf, winrt::to_string(s_textCompleted).c_str(), sizeof(buf));
}

void IME_SetInputScreenPos(int x, int y)
{
	std::tie(s_inputX, s_inputY) = std::make_tuple(x, y);
	s_pImeContext->NotifyLayoutChanged();
}

void IME_GetInputScreenPos(int *x, int *y)
{
	if (x) *x = s_inputX;
	if (y) *y = s_inputY;
}

void IME_SetIMEEnabled(int enable, int force)
{
	if (force)
	{
		if (enable)
		{
			InputPane::GetForCurrentView().Visible(true);
			s_pImeContext->NotifyFocusEnter();
		}
		else
		{
			InputPane::GetForCurrentView().Visible(false);
			s_pImeContext->NotifyFocusLeave();
		}
	}
	else if (enable)
	{
		if (!host.textmode)
		{
			InputPane::GetForCurrentView().TryShow();
			s_pImeContext->NotifyFocusEnter();
		}
		host.textmode = true;
	}
	else
	{
		InputPane::GetForCurrentView().TryHide();
		s_pImeContext->NotifyFocusLeave();
		host.textmode = false;
	}
}
