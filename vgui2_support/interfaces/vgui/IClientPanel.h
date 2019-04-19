#ifndef ICLIENTPANEL_H
#define ICLIENTPANEL_H

#include "VGUI2.h"

class KeyValues;

namespace vgui2 {
	enum EInterfaceID {
		ICLIENTPANEL_STANDARD_INTERFACE = 0,
	};

	class IClientPanel {
	public:
		virtual VPANEL GetVPanel() = 0;
		virtual void Think() = 0;
		virtual void PerformApplySchemeSettings() = 0;
		virtual void PaintTraverse(bool, bool) = 0;
		virtual void Repaint() = 0;
		virtual VPANEL IsWithinTraverse(int, int, bool) = 0;
		virtual void GetInset(int &, int &, int &, int &) = 0;
		virtual void GetClipRect(int &, int &, int &, int &) = 0;
		virtual void OnChildAdded(VPANEL) = 0;
		virtual void OnSizeChanged(int, int) = 0;
		virtual void InternalFocusChanged(bool) = 0;
		virtual bool RequestInfo(KeyValues *) = 0;
		virtual void RequestFocus(int) = 0;
		virtual bool RequestFocusPrev(VPANEL) = 0;
		virtual bool RequestFocusNext(VPANEL) = 0;
		virtual void OnMessage(const KeyValues  *, VPANEL) = 0;
		virtual VPANEL GetCurrentKeyFocus() = 0;
		virtual int GetTabPosition() = 0;
		virtual const char  * GetName() = 0;
		virtual const char  * GetClassName() = 0;
		virtual HScheme GetScheme() = 0;
		virtual bool IsProportional() = 0;
		virtual bool IsAutoDeleteSet() = 0;
		virtual void DeletePanel() = 0;
		virtual void * QueryInterface(enum EInterfaceID) = 0;
		virtual class Panel * GetPanel() = 0;
		virtual const char  * GetModuleName() = 0;
	};
}

#endif // ICLIENTPANEL_H
