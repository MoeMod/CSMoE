#ifndef VPANEL_H
#define VPANEL_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/Dar.h>
#include <vgui/IPanel.h>

#ifdef GetClassName
#undef GetClassName
#endif

namespace vgui2
{

class SurfaceBase;
class IClientPanel;
struct SerialPanel_t;

typedef unsigned short HPanelList;
const HPanelList INVALID_PANELLIST = 0xFFFF;

class VPanel
{
public:
	VPanel(void);
	virtual ~VPanel(void);

public:
	virtual void Init(IClientPanel *attachedClientPanel);
	virtual SurfacePlat *Plat(void);
	virtual void SetPlat(SurfacePlat *pl);
	virtual HPanelList GetListEntry(void);
	virtual void SetListEntry(HPanelList listEntry);
	virtual bool IsPopup(void);
	virtual void SetPopup(bool state);
	virtual bool Render_IsPopupPanelVisible(void);
	virtual void Render_SetPopupVisible(bool state);
	virtual void SetPos(int x, int y);
	virtual void GetPos(int &x, int &y);
	virtual void SetSize(int wide,int tall);
	virtual void GetSize(int &wide,int &tall);
	virtual void SetMinimumSize(int wide,int tall);
	virtual void GetMinimumSize(int &wide,int &tall);
	virtual void SetZPos(int z);
	virtual int GetZPos(void);
	virtual void GetAbsPos(int &x, int &y);
	virtual void GetClipRect(int &x0, int &y0, int &x1, int &y1);
	virtual void SetInset(int left, int top, int right, int bottom);
	virtual void GetInset(int &left, int &top, int &right, int &bottom);
	virtual void Solve(void);
	virtual void SetVisible(bool state);
	virtual void SetEnabled(bool state);
	virtual bool IsVisible(void);
	virtual bool IsEnabled(void);
	virtual void SetParent(VPanel *newParent);
	virtual int GetChildCount(void);
	virtual VPanel *GetChild(int index);
	virtual VPanel *GetParent(void);
	virtual void MoveToFront(void);
	virtual void MoveToBack(void);
	virtual bool HasParent(VPanel *potentialParent);
	virtual const char *GetName(void);
	virtual const char *GetClassName(void);
	virtual HScheme GetScheme(void);
	virtual void SendMessage(KeyValues *params, VPANEL ifromPanel);
	virtual IClientPanel *Client(void);
	virtual void SetKeyBoardInputEnabled(bool state);
	virtual void SetMouseInputEnabled(bool state);
	virtual bool IsKeyBoardInputEnabled(void);
	virtual bool IsMouseInputEnabled(void);

private:
	Dar<VPanel*> _childDar;
	VPanel* _parent = nullptr;
	SurfacePlat* _plat = nullptr;
	HPanelList _listEntry = INVALID_PANELLIST;

	IClientPanel* _clientPanel = nullptr;

	short _pos[2] = {};
	short _size[2] = {};
	short _minimumSize[2] = {};

	short _inset[4] = {};
	short _clipRect[4] = {};
	short _absPos[2] = {};

	short _zpos = 0;

	bool _visible = true;
	bool _enabled = true;
	bool _popup = false;
	bool _popupVisible = false;
	bool _mouseInput = true;
	bool _kbInput = true;
	//bool _isTopmostPopup : 1;


private:
	VPanel(const VPanel&) = delete;
	VPanel& operator=(const VPanel&) = delete;
};
}

#endif