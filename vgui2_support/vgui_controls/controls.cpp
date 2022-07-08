#include <string.h>
#include <locale.h>
#include <vgui/IInputInternal.h>
#include <vgui/ISchemeManager.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui/IVGui.h>
#include <vgui/IPanel.h>
#include <vgui/ILocalize.h>
#include <vgui/IKeyValues.h>
#include <tier1/KeyValues.h>
#include <FileSystem.h>
#include "controls.h"

IKeyValues* g_pKeyValuesInterface = nullptr;
IFileSystem *g_pFullFileSystem = nullptr;

IKeyValues* keyvalues()
{
	return g_pKeyValuesInterface;
}

namespace vgui2
{
	vgui2::IInputInternal *g_pInputInterface = NULL;
	vgui2::ISchemeManager *g_pSchemeInterface = NULL;
	vgui2::ISurface *g_pSurfaceInterface = NULL;
	vgui2::ISystem *g_pSystemInterface = NULL;
	vgui2::IVGui *g_pVGuiInterface = NULL;
	vgui2::IPanel *g_pPanelInterface = NULL;
	vgui2::ILocalize *g_pLocalizeInterface = NULL;

	vgui2::IInputInternal *input() {
		return g_pInputInterface;
	}

	vgui2::ISchemeManager *scheme() {
		return g_pSchemeInterface;
	}

	vgui2::ISurface *surface() {
		return g_pSurfaceInterface;
	}

	vgui2::ISystem *system() {
		return g_pSystemInterface;
	}
	
	vgui2::IVGui *ivgui() {
		return g_pVGuiInterface;
	}

	vgui2::IPanel *ipanel() {
		return g_pPanelInterface;
	}

	vgui2::ILocalize *localize() {
		return g_pLocalizeInterface;
	}
	
	IFileSystem *filesystem() {
		return g_pFullFileSystem;
	}

	static void *InitializeInterface(char const *interfaceName, CreateInterfaceFn *factoryList, int numFactories) {
		void *retval;

		for (int i = 0; i < numFactories; i++) {
			CreateInterfaceFn factory = factoryList[i];
			if (!factory)
				continue;

			retval = factory(interfaceName, NULL);
			if (retval)
				return retval;
		}

		return NULL;
	}

	static char g_szControlsModuleName[256] = "root";

	bool VGuiControls_Init(const char *moduleName, CreateInterfaceFn *factoryList, int numFactories) {
#ifndef XASH_STATIC_GAMELIB
		strncpy(g_szControlsModuleName, moduleName, sizeof(g_szControlsModuleName));
		g_szControlsModuleName[sizeof(g_szControlsModuleName) - 1] = 0;
#endif

		setlocale(LC_CTYPE, "");
		setlocale(LC_TIME, "");
		setlocale(LC_COLLATE, "");
		setlocale(LC_MONETARY, "");

		g_pVGuiInterface = (IVGui *)InitializeInterface(VGUI_IVGUI_INTERFACE_VERSION, factoryList, numFactories);
		g_pPanelInterface = (IPanel *)InitializeInterface(VGUI_PANEL_INTERFACE_VERSION, factoryList, numFactories);
		g_pSurfaceInterface = (ISurface *)InitializeInterface(VGUI_SURFACE_INTERFACE_VERSION, factoryList, numFactories);
		g_pSchemeInterface = (ISchemeManager *)InitializeInterface(VGUI_SCHEME_INTERFACE_VERSION, factoryList, numFactories);
		g_pSystemInterface = (ISystem *)InitializeInterface(VGUI_SYSTEM_INTERFACE_VERSION, factoryList, numFactories);
		g_pInputInterface = (IInputInternal *)InitializeInterface(VGUI_INPUTINTERNAL_INTERFACE_VERSION, factoryList, numFactories);
		g_pLocalizeInterface = (ILocalize *)InitializeInterface(VGUI_LOCALIZE_INTERFACE_VERSION, factoryList, numFactories);
        g_pFullFileSystem = (IFileSystem *)InitializeInterface(FILESYSTEM_INTERFACE_VERSION, factoryList, numFactories);

		g_pKeyValuesInterface = static_cast<IKeyValues*>(InitializeInterface(KEYVALUES_INTERFACE_VERSION, factoryList, numFactories));
		
		if (!g_pVGuiInterface) {
			return false;
		}

		g_pVGuiInterface->Init(factoryList, numFactories);

		if (g_pKeyValuesInterface)
			g_pKeyValuesInterface->RegisterSizeofKeyValues(sizeof(KeyValues));

		if (g_pSchemeInterface &&
			g_pSurfaceInterface &&
			g_pSystemInterface &&
			g_pInputInterface &&
			g_pVGuiInterface &&
			g_pFullFileSystem &&
			g_pLocalizeInterface &&
			g_pPanelInterface)
			return true;

		return false;
	}

	const char *GetControlsModuleName() {
		return g_szControlsModuleName;
	}
}
