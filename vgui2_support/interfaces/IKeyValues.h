#ifndef IKEYVALUES_H
#define IKEYVALUES_H

#include <interface.h>
#include <vgui/VGUI2.h>
#include <vstdlib/IKeyValuesSystem.h>

namespace vgui2 {
	using ::HKeySymbol; // IKeyValuesSystem.h
}

namespace vgui2 {
	class IKeyValues : public IBaseInterface {
	public:
		virtual void RegisterSizeofKeyValues(int) = 0;
		virtual void * AllocKeyValuesMemory(int) = 0;
		virtual void FreeKeyValuesMemory(void *) = 0;
		virtual HKeySymbol GetSymbolForString(const char  *) = 0;
		virtual const char  * GetStringForSymbol(HKeySymbol) = 0;
		virtual void GetLocalizedFromANSI(const char  *, wchar_t *, int) = 0;
		virtual void GetANSIFromLocalized(const wchar_t  *, char *, int) = 0;
		virtual void AddKeyValuesToMemoryLeakList(void *, HKeySymbol) = 0;
		virtual void RemoveKeyValuesFromMemoryLeakList(void *) = 0;
	};
}

#define VGUI_KEYVALUES_INTERFACE_VERSION "KeyValues003"

#endif // IKEYVALUES_H
