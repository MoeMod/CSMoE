
#include <wrl\implements.h>
#include <wrl\module.h>
#include <wrl\event.h>
#include <windows.ui.ViewManagement.h>
#include <windows.System.h>

#include "winrt_interop.h"
#include "common.h"
#include "keydefs.h"

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::UI::Core;
using namespace ABI::Windows::UI::ViewManagement;

void WinRT_FullscreenMode_Install(IInspectable * hwnd, BOOL fullscreen)
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
	hr = applicationViewStatics3->put_PreferredLaunchWindowingMode(fullscreen ? ApplicationViewWindowingMode_FullScreen : ApplicationViewWindowingMode_Auto);


	ComPtr <IApplicationView> appView;
	hr = applicationViewStatics2->GetForCurrentView(&appView);
	if (SUCCEEDED(hr))
	{
		ComPtr <IApplicationView3> appView3;
		hr = appView.As(&appView3);
		if (SUCCEEDED(hr))
		{
			//boolean success;
			//hr = appView3->TryEnterFullScreenMode(&success);

			// disable the system gestures...
			hr = appView3->put_FullScreenSystemOverlayMode(FullScreenSystemOverlayMode_Minimal);
		}
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
	systemNavigationManagerStatics->GetForCurrentView(&systemNavigationManager);

	
	ComPtr<ISystemNavigationManager2> systemNavigationManager2;
	if(hr = systemNavigationManager.As(&systemNavigationManager2), SUCCEEDED(hr))
	{
		systemNavigationManager2->put_AppViewBackButtonVisibility(AppViewBackButtonVisibility_Visible);
	}
	
	auto callback = Callback<__FIEventHandler_1_Windows__CUI__CCore__CBackRequestedEventArgs>(WinRT_OnBackRequested);
	EventRegistrationToken token;
	hr = systemNavigationManager->add_BackRequested(callback.Get(), &token);
}