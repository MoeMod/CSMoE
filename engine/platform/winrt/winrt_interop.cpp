#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Text.h>
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
