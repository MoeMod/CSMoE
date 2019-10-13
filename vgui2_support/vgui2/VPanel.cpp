#include <vgui/IClientPanel.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Controls.h>

#include "VPanel.h"

namespace vgui2
{
	VPanel::VPanel()
	{
	}

	VPanel::~VPanel()
	{
	}

	void VPanel::Init(IClientPanel* attachedClientPanel)
	{
		_clientPanel = attachedClientPanel;
	}

	SurfacePlat* VPanel::Plat()
	{
		return _plat;
	}

	void VPanel::SetPlat(SurfacePlat* Plat)
	{
		_plat = Plat;
	}

	HPanelList VPanel::GetListEntry()
	{
		return _listEntry;
	}

	void VPanel::SetListEntry(HPanelList listEntry)
	{
		_listEntry = listEntry;
	}

	bool VPanel::IsPopup()
	{
		return _popup;
	}

	void VPanel::SetPopup(bool state)
	{
		_popup = state;
	}

	bool VPanel::Render_IsPopupPanelVisible()
	{
		return _popupVisible;
	}

	void VPanel::Render_SetPopupVisible(bool state)
	{
		_popupVisible = state;
	}

	void VPanel::SetPos(int x, int y)
	{
		_pos[0] = x;
		_pos[1] = y;
	}

	void VPanel::GetPos(int& x, int& y)
	{
		x = _pos[0];
		y = _pos[1];
	}

	void VPanel::SetSize(int wide, int tall)
	{
		if (wide < _minimumSize[0])
			wide = _minimumSize[0];

		if (tall < _minimumSize[1])
			tall = _minimumSize[1];

		if (wide != _size[0] || tall != _size[1])
		{
			_size[0] = wide;
			_size[1] = tall;

			Client()->OnSizeChanged(wide, tall);
		}
	}

	void VPanel::GetSize(int& wide, int& tall)
	{
		wide = _size[0];
		tall = _size[1];
	}

	void VPanel::SetMinimumSize(int wide, int tall)
	{
		_minimumSize[0] = wide;
		_minimumSize[1] = tall;

		if (wide < _size[0])
			wide = _size[0];

		if (tall < _size[1])
			tall = _size[1];

		if (wide != _size[0] || tall != _size[1])
			SetSize(wide, tall);
	}

	void VPanel::GetMinimumSize(int& wide, int& tall)
	{
		wide = _minimumSize[0];
		tall = _minimumSize[1];
	}

	void VPanel::SetZPos(int z)
	{
		_zpos = z;

		if (!_parent)
			return;

		if (_parent->GetChildCount() <= 0)
			return;

		int i;

		for (i = 0; i < _parent->GetChildCount() && this != _parent->GetChild(i); ++i)
		{
		}

		if (i == _parent->GetChildCount())
			return;

		if (i > 0)
		{
			//See if it needs to be moved back.
			for (int j = i - 1; j >= 0; --j)
			{
				if (_zpos <= _parent->_childDar[j]->_zpos)
				{
					if (j + 1 != i)
					{
						//We need to be moved back.
						_parent->_childDar.InsertElementAt(this, j + 1);
						_parent->_childDar.RemoveElement(this);
						i = j + 1;
					}

					break;
				}
			}
		}

		if (i >= _parent->_childDar.Count())
			return;

		//See if we need to be moved forward.
		for (int j = i + 1; j < _parent->_childDar.Count(); ++j)
		{
			if (_zpos >= _parent->_childDar[j]->_zpos)
			{
				if (j - 1 != i)
				{
					//We need to be moved forward.
					//Insert first so index is still valid.
					_parent->_childDar.InsertElementAt(this, j - 1);
					_parent->_childDar.RemoveElement(this);
				}

				break;
			}
		}
	}

	int VPanel::GetZPos()
	{
		return _zpos;
	}

	void VPanel::GetAbsPos(int& x, int& y)
	{
		x = _absPos[0];
		y = _absPos[1];
	}

	void VPanel::GetClipRect(int& x0, int& y0, int& x1, int& y1)
	{
		x0 = _clipRect[0];
		y0 = _clipRect[1];
		x1 = _clipRect[2];
		y1 = _clipRect[3];
	}

	void VPanel::SetInset(int left, int top, int right, int bottom)
	{
		_inset[0] = left;
		_inset[1] = top;
		_inset[2] = right;
		_inset[3] = bottom;
	}

	void VPanel::GetInset(int& left, int& top, int& right, int& bottom)
	{
		left = _inset[0];
		top = _inset[1];
		right = _inset[2];
		bottom = _inset[3];
	}

	void VPanel::Solve()
	{
		_absPos[0] = _pos[0];
		_absPos[1] = _pos[1];

		auto pParent = GetParent();

		if (IsPopup())
			pParent = reinterpret_cast<VPanel*>(vgui2::surface()->GetEmbeddedPanel());

		int pinset[4] = {};

		if (pParent)
		{
			pParent->GetInset(
				pinset[0],
				pinset[1],
				pinset[2],
				pinset[3]
			);

			int pabsX, pabsY;
			pParent->GetAbsPos(pabsX, pabsY);

			_absPos[0] += pabsX + pinset[0];
			_absPos[1] += pabsY + pinset[1];
		}

		_clipRect[0] = _absPos[0];
		_clipRect[1] = _absPos[1];

		int wide, tall;
		GetSize(wide, tall);

		_clipRect[2] = _absPos[0] + wide;
		_clipRect[3] = _absPos[1] + tall;

		if (pParent && !IsPopup())
		{
			int pclip[4];

			pParent->GetClipRect(
				pclip[0],
				pclip[1],
				pclip[2],
				pclip[3]
			);

			if (_clipRect[0] < pclip[0])
				_clipRect[0] = pclip[0];

			if (_clipRect[1] < pclip[1])
				_clipRect[1] = pclip[1];

			if (_clipRect[2] > pclip[2])
				_clipRect[2] = pclip[2] - pinset[2];

			if (_clipRect[3] > pclip[3])
				_clipRect[3] = pclip[3] - pinset[3];
		}
	}

	void VPanel::SetVisible(bool state)
	{
		if (_visible != state)
		{
			vgui2::surface()->SetPanelVisible(reinterpret_cast<VPANEL>(this), state);

			_visible = state;

			if (IsPopup())
				vgui2::surface()->CalculateMouseVisible();
		}
	}

	void VPanel::SetEnabled(bool state)
	{
		_enabled = state;
	}

	bool VPanel::IsVisible()
	{
		return _visible;
	}

	bool VPanel::IsEnabled()
	{
		return _enabled;
	}

	void VPanel::SetParent(VPanel* newParent)
	{
		if (this == newParent ||
			_parent == newParent)
			return;

		if (_parent)
		{
			_parent->_childDar.RemoveElement(this);
			_parent = nullptr;
		}

		if (newParent)
		{
			_parent = newParent;

			if (_parent->_childDar.Find(this) == _parent->_childDar.InvalidIndex())
			{
				_parent->_childDar.AddElement(this);
			}

			SetZPos(_zpos);

			if (_parent->Client())
			{
				_parent->Client()->OnChildAdded(reinterpret_cast<VPANEL>(this));
			}
		}
	}

	int VPanel::GetChildCount()
	{
		return _childDar.Count();
	}

	VPanel* VPanel::GetChild(int index)
	{
		if (index < 0 || index >= _childDar.Count())
			return nullptr;

		return _childDar[index];
	}

	VPanel* VPanel::GetParent()
	{
		return _parent;
	}

	void VPanel::MoveToFront()
	{
		vgui2::surface()->MovePopupToFront(reinterpret_cast<VPANEL>(this));

		if (!_parent)
			return;

		//Move to front.
		_parent->_childDar.MoveElementToEnd(this);

		//Move the panel back until the first panel
		//that is either on the same Z pos or behind it.
		for (int i = _parent->_childDar.Count() - 2; i >= 0; --i)
		{
			auto pSibling = _parent->_childDar[i];

			if (_zpos >= pSibling->_zpos)
			{
				_parent->_childDar.InsertElementAt(this, i + 1);
				_parent->_childDar.RemoveElement(this);
				break;
			}
		}
	}

	void VPanel::MoveToBack()
	{
		if (!_parent)
			return;

		//Move to back.
		_parent->_childDar.RemoveElement(this);
		_parent->_childDar.InsertElementAt(this, 0);

		//Move the panel forward until the first panel
		//that is either on the same Z pos or in front of it.
		for (int i = 1; i < _parent->_childDar.Count(); ++i)
		{
			auto pSibling = _parent->_childDar[i];

			if (_zpos <= pSibling->_zpos)
			{
				_parent->_childDar.InsertElementAt(this, i - 1);
				_parent->_childDar.RemoveElement(this);
				break;
			}
		}
	}

	bool VPanel::HasParent(VPanel* potentialParent)
	{
		if (this == potentialParent)
			return true;

		if (!_parent)
			return false;

		return _parent->HasParent(potentialParent);
	}

	const char* VPanel::GetName()
	{
		return Client()->GetName();
	}

	const char* VPanel::GetClassName()
	{
		return Client()->GetClassName();
	}

	HScheme VPanel::GetScheme()
	{
		return Client()->GetScheme();
	}

	void VPanel::SendMessage(KeyValues* params, VPANEL ifrompanel)
	{
		Client()->OnMessage(params, ifrompanel);
	}

	IClientPanel* VPanel::Client()
	{
		return _clientPanel;
	}

	void VPanel::SetKeyBoardInputEnabled(bool state)
	{
		_kbInput = state;
	}

	void VPanel::SetMouseInputEnabled(bool state)
	{
		_mouseInput = state;
	}

	bool VPanel::IsKeyBoardInputEnabled()
	{
		return _kbInput;
	}

	bool VPanel::IsMouseInputEnabled()
	{
		return _mouseInput;
	}
}
