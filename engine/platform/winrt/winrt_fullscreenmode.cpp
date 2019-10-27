
#include <wrl\implements.h>
#include <wrl\module.h>
#include <wrl\event.h>
#include <windows.ui.ViewManagement.h>
#include <windows.ui.viewmanagement.core.h>

#include "winrt_fullscreenmode.h"

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::UI::Core;
using namespace ABI::Windows::UI::ViewManagement;

extern "C" void WinRT_FullscreenMode_Install(IInspectable * hwnd)
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
	hr = applicationViewStatics3->put_PreferredLaunchWindowingMode(ApplicationViewWindowingMode_FullScreen);


	ComPtr <IApplicationView> appView;
	hr = applicationViewStatics2->GetForCurrentView(&appView);
	if (SUCCEEDED(hr))
	{
		ComPtr <IApplicationView3> appView3;
		hr = appView.As(&appView3);
		if (SUCCEEDED(hr))
		{
			boolean success;
			hr = appView3->TryEnterFullScreenMode(&success);
		}
	}
}