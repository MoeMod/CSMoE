#include "DeviceListener.h"
#include "SurfaceDial.h"
#include <windows.h>
#include <Windowsx.h>
#include <tchar.h>
#include <StrSafe.h>
#include <wrl\implements.h>
#include <wrl\module.h>
#include <wrl\event.h>
#include <roapi.h>
#include <Pathcch.h>
#include <windows.foundation.h>
#include <windows.foundation.numerics.h>
#include <windows.foundation.collections.h>
#include <windows.ui.input.h>
#include <windows.ui.core.h>
#include <windows.ui.ViewManagement.h>
#include <windows.ui.viewmanagement.core.h>
#include <UIViewSettingsInterop.h>
#include <wrl\client.h>

#pragma comment(lib, "onecoreuap.lib")

#define RETURN_IF_FAILED(hr) { if(FAILED(hr)) return hr; }

using namespace ABI::Windows::UI::Input;
using namespace ABI::Windows::UI::Core;
using namespace ABI::Windows::UI::ViewManagement;
using namespace ABI::Windows::Foundation;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

DeviceListener::DeviceListener(HANDLE console)
{
	_console = console;
}

HRESULT DeviceListener::Init(HWND hwnd)
{
	bool isRoInit = false;
	HRESULT hr = Windows::Foundation::Initialize(RO_INIT_SINGLETHREADED);

	if (SUCCEEDED(hr))
	{
		hr = RegisterForEvents(hwnd);
		if (SUCCEEDED(hr))
		{
			hr = PopulateMenuItems();
		}
	}

	ComPtr<ICoreWindowStatic> coreWindowStatic;
	RETURN_IF_FAILED(Windows::Foundation::GetActivationFactory(
		HStringReference(RuntimeClass_Windows_UI_Core_CoreWindow).Get(),
		&coreWindowStatic));

	ComPtr<ICoreWindow> coreWindow;
	hr = coreWindowStatic->GetForCurrentThread(&coreWindow);
	
	ComPtr<IApplicationViewStatics2> applicationViewStatics2;
	RETURN_IF_FAILED(Windows::Foundation::GetActivationFactory(
		HStringReference(RuntimeClass_Windows_UI_ViewManagement_ApplicationView).Get(),
		&applicationViewStatics2));
	
	ComPtr<IApplicationViewStatics3> applicationViewStatics3;
	RETURN_IF_FAILED(Windows::Foundation::GetActivationFactory(
		HStringReference(RuntimeClass_Windows_UI_ViewManagement_ApplicationView).Get(),
		&applicationViewStatics3));
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
	
	return hr;
}

HRESULT DeviceListener::SetRotationResolution(double res)
{
	return _controller->put_RotationResolutionInDegrees(res);
}

HRESULT DeviceListener::RegisterForEvents(HWND hwnd)
{
	RETURN_IF_FAILED(Windows::Foundation::GetActivationFactory(
		HStringReference(RuntimeClass_Windows_UI_Input_RadialController).Get(),
		&_controllerInterop));

	RETURN_IF_FAILED(_controllerInterop->CreateForWindow(hwnd, IID_PPV_ARGS(&_controller)));

	// Wire events
	RETURN_IF_FAILED(_controller->add_ScreenContactContinued(
		Callback<ITypedEventHandler<RadialController*, RadialControllerScreenContactContinuedEventArgs*>>(this, &DeviceListener::OnScreenContactContinued).Get(),
		&_screenContactContinuedToken));

	RETURN_IF_FAILED(_controller->add_ScreenContactStarted(
		Callback<ITypedEventHandler<RadialController*, RadialControllerScreenContactStartedEventArgs*>>(this, &DeviceListener::OnScreenContactStarted).Get(),
		&_screenContactStartedToken));

	RETURN_IF_FAILED(_controller->add_ScreenContactEnded(
		Callback<ITypedEventHandler<RadialController*, IInspectable*>>(this, &DeviceListener::OnScreenContactEnded).Get(),
		&_screenContactEndedToken));

	RETURN_IF_FAILED(_controller->add_ControlLost(
		Callback<ITypedEventHandler<RadialController*, IInspectable*>>(this, &DeviceListener::OnControlLost).Get(),
		&_controlLostToken));

	RETURN_IF_FAILED(_controller->add_ControlAcquired(
		Callback<ITypedEventHandler<RadialController*, RadialControllerControlAcquiredEventArgs*>>(this, &DeviceListener::OnControlAcquired).Get(),
		&_controlAcquiredToken));

	RETURN_IF_FAILED(_controller->add_RotationChanged(
		Callback<ITypedEventHandler<RadialController*, RadialControllerRotationChangedEventArgs*>>(this, &DeviceListener::OnRotationChanged).Get(),
		&_rotatedToken));

	// Lambda callback
	RETURN_IF_FAILED(_controller->add_ButtonClicked(
		Callback<ITypedEventHandler<RadialController*, RadialControllerButtonClickedEventArgs*>>(this, &DeviceListener::OnButtonClicked).Get(),
		&_buttonClickedToken));

	return S_OK;
}

HRESULT DeviceListener::PopulateMenuItems()
{
	RETURN_IF_FAILED(_controller->get_Menu(&_menu));


	RETURN_IF_FAILED(Windows::Foundation::GetActivationFactory(
		HStringReference(RuntimeClass_Windows_UI_Input_RadialControllerMenuItem).Get(),
		&_menuItemStatics));


	/*RETURN_IF_FAILED(AddMenuItemFromKnownIcon(HStringReference(L"My Ink").Get(),
		RadialControllerMenuKnownIcon::RadialControllerMenuKnownIcon_InkColor,
		_knownIconItem1Token));
	RETURN_IF_FAILED(AddMenuItemFromKnownIcon(HStringReference(L"Ruler").Get(),
		RadialControllerMenuKnownIcon::RadialControllerMenuKnownIcon_Ruler,
		_knownIconItem2Token));*/

	RETURN_IF_FAILED(AddMenuItemFromSystemFont());

	return S_OK;
}

HRESULT DeviceListener::AddMenuItemFromKnownIcon(_In_ HSTRING itemName, _In_ RadialControllerMenuKnownIcon icon, _In_ EventRegistrationToken registrationToken)
{
	// Get menu items
	ComPtr<Collections::IVector<RadialControllerMenuItem*>> menuItems;
	RETURN_IF_FAILED(_menu->get_Items(&menuItems));

	// Create item from known icon
	ComPtr<IRadialControllerMenuItem> menuItem;
	RETURN_IF_FAILED(_menuItemStatics->CreateFromKnownIcon(itemName, icon, &menuItem));

	RETURN_IF_FAILED(AddMenuItem(menuItem, itemName, registrationToken));

	return S_OK;
}

HRESULT DeviceListener::AddMenuItemFromSystemFont()
{
	// Create item from system font
	ComPtr<IRadialControllerMenuItemStatics2> menuItemStatics2;
	RETURN_IF_FAILED(Windows::Foundation::GetActivationFactory(
		HStringReference(RuntimeClass_Windows_UI_Input_RadialControllerMenuItem).Get(),
		&menuItemStatics2));

	HSTRING toolDisplayName = HStringReference(L"CSMoE Test").Get();
	ComPtr<IRadialControllerMenuItem> systemFontItem;
	menuItemStatics2->CreateFromFontGlyph(toolDisplayName, HStringReference(L"🔪").Get(), HStringReference(L"Segoe UI Emoji").Get(), &systemFontItem);

	RETURN_IF_FAILED(AddMenuItem(systemFontItem, toolDisplayName, _systemFontItemToken));
	return S_OK;
}

HRESULT DeviceListener::AddMenuItem(_In_ ComPtr<IRadialControllerMenuItem> item, _In_ HSTRING itemName, _In_ EventRegistrationToken registrationToken)
{
	// Set Callback
	RETURN_IF_FAILED(item->add_Invoked(
		Callback<ITypedEventHandler<RadialControllerMenuItem*, IInspectable*>>(this, &DeviceListener::OnItemInvoked).Get(),
		&registrationToken));

	// Get menu items
	ComPtr<Collections::IVector<RadialControllerMenuItem*>> menuItems;
	RETURN_IF_FAILED(_menu->get_Items(&menuItems));

	// Add item to menu
	RETURN_IF_FAILED(menuItems->Append(item.Get()));


	return S_OK;
}

HRESULT DeviceListener::GetFontUri(_Out_ ComPtr<IUriRuntimeClass>* fontUri)
{
	WCHAR currentPath[MAX_PATH];
	GetCurrentDirectoryW(MAX_PATH, currentPath);
	WCHAR fontFile[] = L"..\\shared\\Symbols.ttf";

	WCHAR fontPath[MAX_PATH];
	RETURN_IF_FAILED(PathCchCombine(fontPath, MAX_PATH, currentPath, fontFile));

	ComPtr<IUriRuntimeClassFactory> uriRuntimeClassFactory;
	RETURN_IF_FAILED(GetActivationFactory(HStringReference(RuntimeClass_Windows_Foundation_Uri).Get(), &uriRuntimeClassFactory));

	RETURN_IF_FAILED(uriRuntimeClassFactory->CreateUri(HStringReference(fontPath).Get(), &(*fontUri)));

	return S_OK;
}

HRESULT DeviceListener::OnItemInvoked(_In_ IRadialControllerMenuItem* item, _In_ IInspectable* args)
{
	if (item != nullptr)
	{
		_controller->put_UseAutomaticHapticFeedback(false);
	}

	return S_OK;
}

HRESULT DeviceListener::OnScreenContactContinued(_In_ IRadialController* /*sender*/, _In_ IRadialControllerScreenContactContinuedEventArgs* args)
{

	return S_OK;
}

HRESULT DeviceListener::OnScreenContactStarted(_In_ IRadialController* /*sender*/, _In_ IRadialControllerScreenContactStartedEventArgs* args)
{

	// Get contact
	ComPtr<IRadialControllerScreenContact> contact;
	RETURN_IF_FAILED(args->get_Contact(&contact));

	if (contact != nullptr)
	{
		// Get contact info
		Point position;
		Rect bounds;
		RETURN_IF_FAILED(contact->get_Position(&position));
		RETURN_IF_FAILED(contact->get_Bounds(&bounds));


		//Input_OnPosChanged(position.X, position.Y);
	}

	return S_OK;
}

HRESULT DeviceListener::OnScreenContactEnded(_In_ IRadialController* /*sender*/, _In_ IInspectable* args)
{

	return S_OK;
}

HRESULT DeviceListener::OnControlLost(_In_ IRadialController* /*sender*/, _In_ IInspectable* args)
{

	return S_OK;
}

HRESULT DeviceListener::OnControlAcquired(_In_ IRadialController* /*sender*/, _In_ IRadialControllerControlAcquiredEventArgs* args)
{

	return S_OK;
}

HRESULT DeviceListener::OnRotationChanged(_In_ IRadialController* /*sender*/, _In_ IRadialControllerRotationChangedEventArgs* args)
{
	double delta = 0;

	args->get_RotationDeltaInDegrees(&delta);

	//Input_OnRotated(delta);

	return S_OK;
}

HRESULT DeviceListener::OnButtonClicked(_In_ IRadialController* /*sender*/, _In_ ABI::Windows::UI::Input::IRadialControllerButtonClickedEventArgs* args)
{
	//Input_OnClicked();
	return S_OK;
}