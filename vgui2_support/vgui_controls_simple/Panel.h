#ifndef PANEL_H
#define PANEL_H

#include "vgui/IClientPanel.h"

namespace vgui2 {
	class Panel : public IClientPanel {
	public:
		Panel();
		~Panel();

		VPANEL GetVPanel();
		void Think();
		void PerformApplySchemeSettings();
		void PaintTraverse(bool, bool);
		void Repaint();
		VPANEL IsWithinTraverse(int, int, bool);
		void GetInset(int &, int &, int &, int &);
		void GetClipRect(int &, int &, int &, int &);
		void OnChildAdded(VPANEL);
		void OnSizeChanged(int, int);
		void InternalFocusChanged(bool);
		bool RequestInfo(class KeyValues *);
		void RequestFocus(int);
		bool RequestFocusPrev(VPANEL);
		bool RequestFocusNext(VPANEL);
		void OnMessage(const class KeyValues  *, VPANEL);
		VPANEL GetCurrentKeyFocus();
		int GetTabPosition();
		const char  * GetName();
		const char  * GetClassName();
		HScheme GetScheme();
		bool IsProportional();
		bool IsAutoDeleteSet();
		void DeletePanel();
		void * QueryInterface(enum EInterfaceID);
		class Panel * GetPanel();
		const char  * GetModuleName();

		// 
		void SetScheme(const char *tag);
		void SetScheme(HScheme scheme);
		bool IsWithin(int x, int y);
		bool IsVisible();
		bool IsMouseInputEnabled();
		bool IsMouseInputDisabledForThisPanel();
		int GetChildCount();
		VPANEL GetChild(int childId);
		void InvalidateLayout(bool layoutNow = false, bool reloadScheme = false);
		void SetVisible(bool state);
		void SetZPos(int zpos);
		void SetBounds(int x, int y, int wide, int tall);

	private:
		VPANEL _vpanel;
		HScheme _scheme;
		bool _needsRepaint;
		bool _mouseInputDisabledForThisPanelOnly;
		bool _isPropotional;
		bool _autoDeleteEnabled;
		int _tabPosition;
		const char *_panelName;
	};
}

#endif // PANEL_H
