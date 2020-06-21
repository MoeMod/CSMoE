#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Text.h>
#include <winrt/Windows.UI.Text.Core.h>
#include <winrt/Windows.UI.ViewManagement.h>
#include <winrt/Windows.Applicationmodel.Core.h>
#include <winrt/Windows.ApplicationModel.UserDataAccounts.h>
#include <winrt/Windows.ApplicationModel.UserDataAccounts.Provider.h>
#include <winrt/Windows.ApplicationModel.UserDataAccounts.SystemAccess.h>
#include <winrt/Windows.Graphics.Display.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Pickers.h>
#include <winrt/Windows.Storage.AccessCache.h>
#include <winrt/Windows.ApplicationModel.DataTransfer.h>

#include <msctf.h>
#include <ctffunc.h>

#include <SDL_Video.h>

#include "winrt_interop.h"
extern "C" {
#include "common.h"
#include "keydefs.h"
#include "gl_local.h"

	void Key_Event(int key, int down);
}

#include <functional>
#include <future>

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::UI;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::UI::Text;
using namespace winrt::Windows::UI::Text::Core;
using namespace winrt::Windows::UI::ViewManagement;
using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::ApplicationModel::UserDataAccounts;
using namespace winrt::Windows::Graphics::Display;
using namespace winrt::Windows::System;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Pickers;
using namespace winrt::Windows::Storage::AccessCache;
using namespace winrt::Windows::ApplicationModel::DataTransfer;

void WinRT_FullscreenMode_Install(int fullscreen)
{
	auto coreWindow = CoreWindow::GetForCurrentThread();
	auto appView = ApplicationView::GetForCurrentView();
	/*
	coreWindow.SizeChanged([appView](auto, auto e) {
		if(bool tablet_mode = UIViewSettings::GetForCurrentView().UserInteractionMode() == UserInteractionMode::Touch)
			Cvar_Set("fullscreen", "1");
	});
	*/
	if (fullscreen)
		appView.TryEnterFullScreenMode();
	appView.FullScreenSystemOverlayMode(FullScreenSystemOverlayMode::Minimal);

	auto titleBar = appView.TitleBar();
	Color red = { 255, 232, 16, 35 };
	Color red2 = { 255, 181, 113, 113 };
	Color red3 = { 255, 188, 47, 46 };
	Color red4 = { 255, 241, 112, 122 };
	Color white = { 255, 255, 255, 255 };
	Color black = { 255, 0, 0, 0 };
	titleBar.BackgroundColor(red3);
	titleBar.ButtonBackgroundColor(red3);
	titleBar.ButtonInactiveBackgroundColor(red2);
	titleBar.InactiveBackgroundColor(red2);
	titleBar.ButtonHoverBackgroundColor(red);
	titleBar.ButtonPressedBackgroundColor(red4);

	titleBar.ForegroundColor(white);
	titleBar.ButtonForegroundColor(white);
	titleBar.ButtonHoverForegroundColor(white);
	titleBar.ButtonPressedForegroundColor(black);
	titleBar.ButtonInactiveForegroundColor(white);
	titleBar.InactiveForegroundColor(white);

	auto coreApplicationView = CoreApplication::GetCurrentView();
	coreApplicationView.TitleBar().ExtendViewIntoTitleBar(false);
}

void WinRT_SaveVideoMode(int w, int h)
{
	ApplicationView::PreferredLaunchViewSize(Size{ (float)w,(float)h });
	ApplicationView::PreferredLaunchWindowingMode(vid_fullscreen->value != 0.0f ? ApplicationViewWindowingMode::FullScreen : ApplicationViewWindowingMode::PreferredLaunchViewSize);
}

void WinRT_OnBackRequested(const BackRequestedEventArgs &e)
{
	Key_Event(K_ESCAPE, 1);
}

void WinRT_BackButton_Install()
{
	auto systemNavigationManager = SystemNavigationManager::GetForCurrentView();
	systemNavigationManager.AppViewBackButtonVisibility(AppViewBackButtonVisibility::Visible);
	[[maybe_unused]] winrt::event_token ev = systemNavigationManager.BackRequested(std::bind(WinRT_OnBackRequested, std::placeholders::_2));
}

float WinRT_GetDisplayDPI()
{
	auto displayInformation = DisplayInformation::GetForCurrentView();
	ResolutionScale resolutionScale = displayInformation.ResolutionScale();
	double rawPixelsPerViewPixel = displayInformation.RawPixelsPerViewPixel();
	return static_cast<float>(rawPixelsPerViewPixel);
}

char *WinRT_GetUserName()
{
	static char buffer[1024] = "Player";
	// TODO
	return buffer;
}

void WinRT_ShellExecute(const char* path)
{
	wchar_t buffer[1024]{};
	MultiByteToWideChar(CP_ACP, 0, path, -1, buffer, 1024);

	Uri uri(buffer);
	LauncherOptions launcherOptions;
	Launcher::LaunchUriAsync(uri, launcherOptions).get();
}

void WinRT_OpenGameFolderWithExplorer()
{
	Launcher::LaunchFolderAsync(ApplicationData::Current().LocalFolder()).get();
}

void WinRT_SetClipboardData(const char* buffer, size_t size)
{
	DataPackage dp;
	dp.SetText(winrt::to_hstring(std::string_view(buffer, size)));
	Clipboard::SetContent(dp);
}

void WinRT_GetClipboardData(char* buffer, size_t size)
{
	DataPackageView con = Clipboard::GetContent();
	if (con.Contains(StandardDataFormats::Text()))
	{
		auto hstr = std::async(std::launch::async, [con] { return con.GetTextAsync().get(); }).get();
		winrt::to_string(hstr).copy(buffer, size);
	}
}

static std::shared_ptr<CoreTextEditContext> s_pImeContext;
static bool s_ImeContextCompositionStarted = false;
static std::string s_textUpdating;
static float s_inputX;
static float s_inputY;

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
		*pbShow = TRUE;
		return S_OK;
	}
	STDMETHODIMP UpdateUIElement(DWORD dwUIElementId)
	{
		auto pElement = GetUIElement(dwUIElementId);

		ITfCandidateListUIElement* pcandidate = nullptr;
		pElement->QueryInterface(__uuidof(ITfCandidateListUIElement), (void**)&pcandidate);

		UINT count;
		pcandidate->GetCount(&count);

		std::vector<std::wstring> current_candidates;
		for (UINT i = 0; i < count; ++i)
		{
			BSTR bstr;
			pcandidate->GetString(i, &bstr);
			current_candidates.push_back(bstr ? bstr : L"");
		}

		pcandidate->Release();
		pElement->Release();
		return S_OK;
	}
	STDMETHODIMP EndUIElement(DWORD dwUIElementId)
	{
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
	ITfThreadMgr2* m_tm;
	hr = CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER, __uuidof(ITfThreadMgr2), (void**)&m_tm);

	// ready to start interacting
	TfClientId cid;// not used
	hr = m_tm->ActivateEx(&cid, 0);

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
void WinRT_ImeCreateContext()
{
	auto manager = CoreTextServicesManager::GetForCurrentView();
	s_pImeContext = std::make_shared<CoreTextEditContext>(manager.CreateEditContext());
	auto& s_ImeContext = *s_pImeContext;
	s_pImeContext->InputPaneDisplayPolicy(CoreTextInputPaneDisplayPolicy::Automatic);
	s_pImeContext->InputScope(CoreTextInputScope::Text);

	s_pImeContext->CompositionStarted([](CoreTextEditContext, CoreTextCompositionStartedEventArgs)
			{
				s_ImeContextCompositionStarted = true;
			});

	s_pImeContext->CompositionCompleted([](CoreTextEditContext, CoreTextCompositionCompletedEventArgs args)
			{
				CL_CharEventUTF(std::exchange(s_textUpdating, {}).c_str());
				s_ImeContextCompositionStarted = false;
			});

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
			});

	s_pImeContext->TextUpdating([](CoreTextEditContext, CoreTextTextUpdatingEventArgs args)
			{
				auto text = winrt::to_string(args.Text());
				if (s_ImeContextCompositionStarted)
					s_textUpdating = text;
				else
					CL_CharEventUTF(text.c_str());
			});

	// Get CTF Candidate List
	SetupSinks();
}

void WinRT_ImeSetInputScreenPos(int x, int y)
{
	std::tie(s_inputX, s_inputY) = std::make_tuple(x, y);
	s_pImeContext->NotifyLayoutChanged();
}

void WinRT_ImeEnableTextInput(int enable, int force)
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