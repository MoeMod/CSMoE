#include <string.h>
#include "wrect.h"
#include "vgui/IVGui.h"
#include "vgui/IPanel.h"
#include "vgui/ISurface.h"
#include "vgui/ISchemeManager.h"
#include "controls.h"
#include "Panel.h"

namespace vgui2 {
	Panel::Panel() {
		_vpanel = ivgui()->AllocPanel();
		_scheme = 0;
		ipanel()->Init(_vpanel, this);
	}

	Panel::~Panel() {
		if (_vpanel) {
			ivgui()->FreePanel(_vpanel);
			_vpanel = 0;
		}
	}

	VPANEL Panel::GetVPanel() {
		return _vpanel;
	}

	void Panel::Think() {
		//
	}

	void Panel::PerformApplySchemeSettings() {
		//
	}

	void Panel::PaintTraverse(bool repaint, bool allowForce) {
		if (!IsVisible()) {
			return;
		}

		if (_needsRepaint && allowForce) {
			repaint = true;
		}

		if (allowForce) {
			_needsRepaint = false;
		}

		VPANEL vpanel = GetVPanel();

		int clipRect[4];
		ipanel()->GetClipRect(vpanel, clipRect[0], clipRect[1], clipRect[2], clipRect[3]);

		if ((clipRect[2] <= clipRect[0]) || (clipRect[3] <= clipRect[1])) {
			repaint = false;
		}

		int childCount = GetChildCount();

		for (int i = 0; i < childCount; ++i) {
			VPANEL child = GetChild(i);

			if (surface()->ShouldPaintChildPanel(child)) {
				ipanel()->PaintTraverse(child, repaint, allowForce);
			}
			else {
				surface()->Invalidate(child);
				ipanel()->PaintTraverse(child, false, false);
			}
		}

		surface()->SwapBuffers(vpanel);
	}

	void Panel::Repaint() {
		_needsRepaint = true;
		surface()->Invalidate(GetVPanel());
	}

	VPANEL Panel::IsWithinTraverse(int x, int y, bool traversePopups) {
		if (!IsVisible() || !IsMouseInputEnabled()) {
			return 0;
		}

		if (traversePopups) {
			int childCount = GetChildCount();

			for (int i = childCount - 1; i >= 0; i--) {
				VPANEL panel = GetChild(i);

				if (ipanel()->IsPopup(panel)) {
					panel = ipanel()->IsWithinTraverse(panel, x, y, true);

					if (panel) {
						return panel;
					}
				}
			}

			for (int i = childCount - 1; i >= 0; i--) {
				VPANEL panel = GetChild(i);

				if (!ipanel()->IsPopup(panel)) {
					panel = ipanel()->IsWithinTraverse(panel, x, y, true);

					if (panel) {
						return panel;
					}
				}
			}

			if (!IsMouseInputDisabledForThisPanel() && IsWithin(x, y)) {
				return GetVPanel();
			}
		}
		else {
			if (IsWithin(x, y)) {
				int childCount = GetChildCount();

				for (int i = childCount - 1; i >= 0; i--) {
					VPANEL panel = GetChild(i);

					if (!ipanel()->IsPopup(panel)) {
						panel = ipanel()->IsWithinTraverse(panel, x, y, true);

						if (panel) {
							return panel;
						}
					}
				}

				if (!IsMouseInputDisabledForThisPanel()) {
					return GetVPanel();
				}
			}
		}

		return 0;
	}

	void Panel::GetInset(int &left, int &top, int &right, int &bottom) {
		ipanel()->GetInset(GetVPanel(), left, top, right, bottom);
	}

	void Panel::GetClipRect(int &x0, int &y0, int &x1, int &y1) {
		ipanel()->GetClipRect(GetVPanel(), x0, y0, x1, y1);
	}

	void Panel::OnChildAdded(VPANEL) {
		//
	}

	void Panel::OnSizeChanged(int, int) {
		InvalidateLayout();
	}

	void Panel::InternalFocusChanged(bool) {
		//
	}

	bool Panel::RequestInfo(class KeyValues *outputData) {
		return false;
	}

	void Panel::RequestFocus(int) {
		// 
	}

	bool Panel::RequestFocusPrev(VPANEL panel) {
		return false;
	}

	bool Panel::RequestFocusNext(VPANEL panel) {
		return false;
	}

	void Panel::OnMessage(const KeyValues *, VPANEL) {
		//
	}

	VPANEL Panel::GetCurrentKeyFocus() {
		return 0;
	}

	int Panel::GetTabPosition() {
		return _tabPosition;
	}

	const char * Panel::GetName() {
		if (_panelName) {
			return _panelName;
		}

		return "";
	}

	const char * Panel::GetClassName() {
		return "ClientPanel";
	}

	HScheme Panel::GetScheme() {
		if (_scheme) {
			return _scheme;
		}

		return scheme()->GetDefaultScheme();
	}

	bool Panel::IsProportional() {
		return _isPropotional;
	}

	bool Panel::IsAutoDeleteSet() {
		return _autoDeleteEnabled;
	}

	void Panel::DeletePanel() {
		delete this;
	}

	void *Panel::QueryInterface(EInterfaceID id) {
		if (id == ICLIENTPANEL_STANDARD_INTERFACE) {
			return this;
		}

		return NULL;
	}

	Panel * Panel::GetPanel() {
		return (class Panel *)this;
	}

	const char * Panel::GetModuleName() {
		return "VGUI2Support";
	}

	void Panel::SetScheme(const char * tag) {
		if (strlen(tag) > 0 && scheme()->GetScheme(tag)) {
			SetScheme(scheme()->GetScheme(tag));
		}
	}

	void Panel::SetScheme(HScheme scheme) {
		if (_scheme != scheme) {
			_scheme = scheme;
		}
	}

	bool Panel::IsWithin(int x, int y) {
		int clipRect[4];
		ipanel()->GetClipRect(_vpanel, clipRect[0], clipRect[1], clipRect[2], clipRect[3]);

		if (x < clipRect[0]) {
			return false;
		}

		if (y < clipRect[1]) {
			return false;
		}

		if (x >= clipRect[2]) {
			return false;
		}

		if (y >= clipRect[3]) {
			return false;
		}

		return true;
	}

	bool Panel::IsVisible() {
		return ipanel()->IsVisible(GetVPanel());
	}

	bool Panel::IsMouseInputEnabled() {
		return ipanel()->IsMouseInputEnabled(GetVPanel());
	}

	bool Panel::IsMouseInputDisabledForThisPanel() {
		return _mouseInputDisabledForThisPanelOnly;
	}

	int Panel::GetChildCount() {
		return ipanel()->GetChildCount(GetVPanel());
	}

	VPANEL Panel::GetChild(int childId) {
		return ipanel()->GetChild(GetVPanel(), childId);
	}

	void Panel::InvalidateLayout(bool layoutNow, bool reloadScheme) {
		if (layoutNow) {
			Repaint();
		}
	}

	void Panel::SetVisible(bool state) {
		ipanel()->SetVisible(GetVPanel(), state);
	}

	void Panel::SetZPos(int zpos) {
		ipanel()->SetZPos(GetVPanel(), zpos);
	}

	void Panel::SetBounds(int x, int y, int wide, int tall) {
		ipanel()->SetPos(GetVPanel(), x, y);
		ipanel()->SetSize(GetVPanel(), wide, tall);
	}
}
