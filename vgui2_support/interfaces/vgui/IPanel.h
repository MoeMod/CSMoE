#ifndef IPANEL_H
#define IPANEL_H

#include <interface.h>
#include "vgui/IClientPanel.h"

class KeyValues;

namespace vgui2 {
	class SurfacePlat;

	class IPanel : public IBaseInterface {
	public:
		virtual void Init(VPANEL, IClientPanel *) = 0;
		virtual void SetPos(VPANEL, int, int) = 0;
		virtual void GetPos(VPANEL, int &, int &) = 0;
		virtual void SetSize(VPANEL, int, int) = 0;
		virtual void GetSize(VPANEL, int &, int &) = 0;
		virtual void SetMinimumSize(VPANEL, int, int) = 0;
		virtual void GetMinimumSize(VPANEL, int &, int &) = 0;
		virtual void SetZPos(VPANEL, int) = 0;
		virtual int GetZPos(VPANEL) = 0;
		virtual void GetAbsPos(VPANEL, int &, int &) = 0;
		virtual void GetClipRect(VPANEL, int &, int &, int &, int &) = 0;
		virtual void SetInset(VPANEL, int, int, int, int) = 0;
		virtual void GetInset(VPANEL, int &, int &, int &, int &) = 0;
		virtual void SetVisible(VPANEL, bool) = 0;
		virtual bool IsVisible(VPANEL) = 0;
		virtual void SetParent(VPANEL, VPANEL) = 0;
		virtual int GetChildCount(VPANEL) = 0;
		virtual VPANEL GetChild(VPANEL, int) = 0;
		virtual VPANEL GetParent(VPANEL) = 0;
		virtual void MoveToFront(VPANEL) = 0;
		virtual void MoveToBack(VPANEL) = 0;
		virtual bool HasParent(VPANEL, VPANEL) = 0;
		virtual bool IsPopup(VPANEL) = 0;
		virtual void SetPopup(VPANEL, bool) = 0;
		virtual bool Render_GetPopupVisible(VPANEL) = 0;
		virtual void Render_SetPopupVisible(VPANEL, bool) = 0;
		virtual HScheme GetScheme(VPANEL) = 0;
		virtual bool IsProportional(VPANEL) = 0;
		virtual bool IsAutoDeleteSet(VPANEL) = 0;
		virtual void DeletePanel(VPANEL) = 0;
		virtual void SetKeyBoardInputEnabled(VPANEL, bool) = 0;
		virtual void SetMouseInputEnabled(VPANEL, bool) = 0;
		virtual bool IsKeyBoardInputEnabled(VPANEL) = 0;
		virtual bool IsMouseInputEnabled(VPANEL) = 0;
		virtual void Solve(VPANEL) = 0;
		virtual const char  * GetName(VPANEL) = 0;
		virtual const char  * GetClassName(VPANEL) = 0;
		virtual void SendMessage(VPANEL, KeyValues *, VPANEL) = 0;
		virtual void Think(VPANEL) = 0;
		virtual void PerformApplySchemeSettings(VPANEL) = 0;
		virtual void PaintTraverse(VPANEL, bool, bool) = 0;
		virtual void Repaint(VPANEL) = 0;
		virtual VPANEL IsWithinTraverse(VPANEL, int, int, bool) = 0;
		virtual void OnChildAdded(VPANEL, VPANEL) = 0;
		virtual void OnSizeChanged(VPANEL, int, int) = 0;
		virtual void InternalFocusChanged(VPANEL, bool) = 0;
		virtual bool RequestInfo(VPANEL, KeyValues *) = 0;
		virtual void RequestFocus(VPANEL, int) = 0;
		virtual bool RequestFocusPrev(VPANEL, VPANEL) = 0;
		virtual bool RequestFocusNext(VPANEL, VPANEL) = 0;
		virtual VPANEL GetCurrentKeyFocus(VPANEL) = 0;
		virtual int GetTabPosition(VPANEL) = 0;
		virtual class SurfacePlat * Plat(VPANEL) = 0;
		virtual void SetPlat(VPANEL, class SurfacePlat *) = 0;
		virtual class Panel * GetPanel(VPANEL, const char  *) = 0;
		virtual bool IsEnabled(VPANEL) = 0;
		virtual void SetEnabled(VPANEL, bool) = 0;
		virtual IClientPanel* Client(VPANEL vguiPanel) = 0;
		virtual const char* GetModuleName(VPANEL vguiPanel) = 0;

	public:
		// deadipanel.cpp
		bool IsFullyVisible(VPANEL vguiPanel);
		void SetTopmostPopup(VPANEL vguiPanel, bool state);
	};
}

#define VPANEL_NORMAL	((vgui2::SurfacePlat *) NULL)
#define VPANEL_MINIMIZED ((vgui2::SurfacePlat *) 0x00000001)

#define VGUI_PANEL_INTERFACE_VERSION "VGUI_Panel007"

#endif // IPANEL_H
