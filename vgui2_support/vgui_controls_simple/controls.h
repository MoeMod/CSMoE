#ifndef CONTROLS_H
#define CONTROLS_H

#include <interface.h>
#include "Panel.h"

class IFileSystem;

namespace vgui2 {
	bool VGuiControls_Init( const char *moduleName, CreateInterfaceFn *factoryList, int numFactories );
	const char *GetControlsModuleName();

	class IPanel *ipanel();
	class IInput *input();
	class ISchemeManager *scheme();
	class ISurface *surface();
	class ISystem *system();
	class IVGui *ivgui();
	class ILocalize *localize();
	class IKeyValues *keyvalues();
	IFileSystem *filesystem();
}

#endif // CONTROLS_H
