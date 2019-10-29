
#include <wrl.h>
#include <windows.ui.ViewManagement.h>
#include <windows.System.h>

#include "winrt_interop.h"
extern "C" {
#include "common.h"
#include "keydefs.h"
#include "gl_local.h"
}

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::UI::Core;
using namespace ABI::Windows::UI::ViewManagement;

void WinRT_FullscreenMode_Install(boolean fullscreen)
{
	HRESULT hr;
	ComPtr<ICoreWindowStatic> coreWindowStatic;
	hr = Windows::Foundation::GetActivationFactory(
		HStringReference(RuntimeClass_Windows_UI_Core_CoreWindow).Get(),
		&coreWindowStatic);

	ComPtr<ICoreWindow> coreWindow;
	hr = coreWindowStatic->GetForCurrentThread(&coreWindow);

	ComPtr<IApplicationViewStatics2> applicationViewStatics2;
	hr = Windows::Foundation::GetActivationFactory(
		HStringReference(RuntimeClass_Windows_UI_ViewManagement_ApplicationView).Get(),
		&applicationViewStatics2);

	ComPtr<IApplicationViewStatics3> applicationViewStatics3;
	hr = Windows::Foundation::GetActivationFactory(
		HStringReference(RuntimeClass_Windows_UI_ViewManagement_ApplicationView).Get(),
		&applicationViewStatics3);

	ComPtr <IApplicationView> appView;
	if (hr = applicationViewStatics2->GetForCurrentView(&appView), SUCCEEDED(hr))
	{
		ComPtr <IApplicationView3> appView3;
		if (hr = appView.As(&appView3), SUCCEEDED(hr))
		{
			if(fullscreen)
			{
				boolean success;
				hr = appView3->TryEnterFullScreenMode(&success);
			}
			
			// disable the system gestures...
			hr = appView3->put_FullScreenSystemOverlayMode(FullScreenSystemOverlayMode_Minimal);
		}
	}
}

void WinRT_SaveVideoMode(int w, int h)
{
	HRESULT hr;
	ComPtr<IApplicationViewStatics3> applicationViewStatics3;
	hr = Windows::Foundation::GetActivationFactory(
		HStringReference(RuntimeClass_Windows_UI_ViewManagement_ApplicationView).Get(),
		&applicationViewStatics3);
	if(SUCCEEDED(hr))
	{
		hr = applicationViewStatics3->put_PreferredLaunchViewSize(Size{ (float)w,(float)h });
		hr = applicationViewStatics3->put_PreferredLaunchWindowingMode(vid_fullscreen->integer ? ApplicationViewWindowingMode_FullScreen : ApplicationViewWindowingMode_PreferredLaunchViewSize);
	}
}

HRESULT WinRT_OnBackRequested(IInspectable *sender, IBackRequestedEventArgs *e)
{
	Key_Event(K_ESCAPE, 1);
	return S_OK;
}

void WinRT_BackButton_Install()
{
	HRESULT hr;
	ComPtr<ISystemNavigationManagerStatics> systemNavigationManagerStatics;
	hr = Windows::Foundation::GetActivationFactory(
		HStringReference(RuntimeClass_Windows_UI_Core_SystemNavigationManager).Get(),
		&systemNavigationManagerStatics);

	ComPtr<ISystemNavigationManager> systemNavigationManager;
	hr = systemNavigationManagerStatics->GetForCurrentView(&systemNavigationManager);

	
	ComPtr<ISystemNavigationManager2> systemNavigationManager2;
	if(hr = systemNavigationManager.As(&systemNavigationManager2), SUCCEEDED(hr))
	{
		systemNavigationManager2->put_AppViewBackButtonVisibility(AppViewBackButtonVisibility_Visible);
	}
	
	auto callback = Callback<__FIEventHandler_1_Windows__CUI__CCore__CBackRequestedEventArgs>(WinRT_OnBackRequested);
	EventRegistrationToken token;
	hr = systemNavigationManager->add_BackRequested(callback.Get(), &token);
}