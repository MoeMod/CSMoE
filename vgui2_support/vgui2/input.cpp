

#include <tier1/KeyValues.h>
#include <vgui/IClientPanel.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui/IInput.h>
#include <vgui/IInputInternal.h>
#include <vgui/KeyCode.h>
#include <vgui_controls/Controls.h>

#include "input.h"
#include "VPanel.h"

using vgui2::IInput;
using vgui2::IInputInternal;
using vgui2::HInputContext;
using vgui2::DEFAULT_INPUT_CONTEXT;

namespace vgui2
{
	inline VPANEL VPanelToHandle(VPanel *vguiPanel)
	{
		return (VPANEL)vguiPanel;
	}

	inline VPanel *VHandleToPanel(VPANEL vguiPanel)
	{
		return (VPanel *)vguiPanel;
	}
}


static CInputWin32 g_Input;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CInputWin32, IInput, VGUI_INPUT_INTERFACE_VERSION, g_Input);
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CInputWin32, IInputInternal, VGUI_INPUTINTERNAL_INTERFACE_VERSION, g_Input);

CInputWin32::CInputWin32()
{
	InitInputContext(&m_DefaultInputContext);
}

CInputWin32::~CInputWin32()
{
}

void CInputWin32::SetMouseFocus(vgui2::VPANEL newMouseFocus)
{
	if (!IsChildOfModalPanel(newMouseFocus))
		return;

	auto pCtx = GetCurrentContext();

	auto pNewMouseFocus = vgui2::VHandleToPanel(newMouseFocus);

	if (newMouseFocus)
	{
		auto pFocus = pNewMouseFocus;

		while (true)
		{
			if (!pFocus->IsMouseInputEnabled())
				return;

			auto isPopup = pFocus->IsPopup();

			pFocus = pFocus->GetParent();

			if (isPopup || !pFocus || !pFocus->GetParent())
				break;
		}
	}

	if (pCtx->_mouseOver == pNewMouseFocus)
	{
		if (pCtx->_mouseCapture || pCtx->_mouseFocus == pNewMouseFocus)
			return;
	}

	pCtx->_oldMouseFocus = pCtx->_mouseOver;
	pCtx->_mouseOver = pNewMouseFocus;

	if (pCtx->_oldMouseFocus)
	{
		if (!pCtx->_mouseCapture ||
			pCtx->_oldMouseFocus->HasParent(pCtx->_mouseCapture))
		{
			vgui2::ivgui()->PostMessage(
				vgui2::VPanelToHandle(pCtx->_oldMouseFocus),
				new KeyValues("CursorExited"),
				NULL_HANDLE
			);
		}
	}

	if (pNewMouseFocus)
	{
		if (!pCtx->_mouseCapture ||
			pCtx->_mouseCapture->HasParent(pCtx->_mouseCapture))
		{
			vgui2::ivgui()->PostMessage(
				vgui2::VPanelToHandle(pCtx->_mouseOver),
				new KeyValues("CursorEntered"),
				NULL_HANDLE
			);
		}

		auto pFocus = pCtx->_mouseCapture;

		if (!pCtx->_mouseCapture)
			pFocus = pCtx->_mouseOver;

		pCtx->_mouseFocus = pFocus;
	}
}

void CInputWin32::SetMouseCapture(vgui2::VPANEL panel)
{
	if (!IsChildOfModalPanel(panel))
		return;

	auto pCtx = GetCurrentContext();

	if (pCtx->_mouseCapture)
	{
		if (pCtx->_mouseCapture == vgui2::VHandleToPanel(panel))
		{
			vgui2::surface()->EnableMouseCapture(panel, true);
			pCtx->_mouseCapture = vgui2::VHandleToPanel(panel);
			return;
		}

		vgui2::ivgui()->PostMessage(
			vgui2::VPanelToHandle(pCtx->_mouseCapture),
			new KeyValues("MouseCaptureLost"),
			NULL_HANDLE
		);
	}

	if (!panel)
	{
		if (pCtx->_mouseCapture)
		{
			vgui2::surface()->EnableMouseCapture(
				vgui2::VPanelToHandle(pCtx->_mouseCapture), false
			);
		}

		pCtx->_mouseCapture = vgui2::VHandleToPanel(panel);
		return;
	}

	vgui2::surface()->EnableMouseCapture(panel, true);
	pCtx->_mouseCapture = vgui2::VHandleToPanel(panel);
}

void CInputWin32::GetKeyCodeText(vgui2::KeyCode code, char *buf, int buflen)
{
	if (buf && buflen > 0)
	{
		for (int i = 0; i < buflen; ++i)
		{
			buf[i] = _keyTrans[code][i + 2];
		}
	}
}

vgui2::VPANEL CInputWin32::GetFocus()
{
	return vgui2::VPanelToHandle(GetCurrentContext()->_keyFocus);
}

vgui2::VPANEL CInputWin32::GetMouseOver()
{
	return vgui2::VPanelToHandle(GetCurrentContext()->_mouseOver);
}

void CInputWin32::SetCursorPos(int x, int y)
{
	if (!vgui2::surface()->HasCursorPosFunctions())
	{
		int px, py, pw, pt;
		vgui2::surface()->GetAbsoluteWindowBounds(px, py, pw, pt);
	}
	else
	{
		vgui2::surface()->SurfaceSetCursorPos(x, y);
	}
}

void CInputWin32::GetCursorPos(int &x, int &y)
{
	//if( vgui2::surface()->HasCursorPosFunctions() )
	//	vgui2::surface()->SurfaceGetCursorPos( x, y );
	return GetCursorPosition(x, y);
}

bool CInputWin32::WasMousePressed(vgui2::MouseCode code)
{
	return GetCurrentContext()->_mousePressed[code];
}

bool CInputWin32::WasMouseDoublePressed(vgui2::MouseCode code)
{
	return GetCurrentContext()->_mouseDoublePressed[code];
}

bool CInputWin32::IsMouseDown(vgui2::MouseCode code)
{
	return GetCurrentContext()->_mouseDown[code];
}

void CInputWin32::SetCursorOveride(vgui2::HCursor cursor)
{
	//Nothing
}

vgui2::HCursor CInputWin32::GetCursorOveride()
{
	//Nothing
	return NULL_HANDLE;
}

bool CInputWin32::WasMouseReleased(vgui2::MouseCode code)
{
	return GetCurrentContext()->_mousePressed[code];
}

bool CInputWin32::WasKeyPressed(vgui2::KeyCode code)
{
	return GetCurrentContext()->_keyPressed[code];
}

bool CInputWin32::IsKeyDown(vgui2::KeyCode code)
{
	return GetCurrentContext()->_keyDown[code];
}

bool CInputWin32::WasKeyTyped(vgui2::KeyCode code)
{
	return GetCurrentContext()->_keyTyped[code];
}

bool CInputWin32::WasKeyReleased(vgui2::KeyCode code)
{
	return GetCurrentContext()->_keyReleased[code];
}

vgui2::VPANEL CInputWin32::GetAppModalSurface()
{
	return vgui2::VPanelToHandle(m_DefaultInputContext._appModalPanel);
}

void CInputWin32::SetAppModalSurface(vgui2::VPANEL panel)
{
	m_DefaultInputContext._appModalPanel = vgui2::VHandleToPanel(panel);
}

void CInputWin32::ReleaseAppModalSurface()
{
	m_DefaultInputContext._appModalPanel = nullptr;
}

void CInputWin32::GetCursorPosition(int &x, int &y)
{
	auto pCtx = GetCurrentContext();

	x = pCtx->m_nCursorX;
	y = pCtx->m_nCursorY;
}

void CInputWin32::RunFrame()
{
	if (m_hContext == DEFAULT_INPUT_CONTEXT)
		_updateToggleButtonState = true;

	auto pCtx = GetCurrentContext();

	if (pCtx->_keyFocus &&
		IsChildOfModalPanel(vgui2::VPanelToHandle(pCtx->_keyFocus)))
	{
		vgui2::ivgui()->PostMessage(
			vgui2::VPanelToHandle(pCtx->_keyFocus),
			new KeyValues("KeyFocusTicked"),
			NULL_HANDLE
		);
	}

	if (pCtx->_mouseFocus &&
		IsChildOfModalPanel(vgui2::VPanelToHandle(pCtx->_mouseFocus)))
	{
		vgui2::ivgui()->PostMessage(
			vgui2::VPanelToHandle(pCtx->_mouseFocus),
			new KeyValues("MouseFocusTicked"),
			NULL_HANDLE
		);
	}

	memset(pCtx->_mousePressed, 0, sizeof(pCtx->_mousePressed));
	memset(pCtx->_mouseDoublePressed, 0, sizeof(pCtx->_mouseDoublePressed));
	memset(pCtx->_mouseReleased, 0, sizeof(pCtx->_mouseReleased));
	memset(pCtx->_keyPressed, 0, sizeof(pCtx->_keyPressed));
	memset(pCtx->_keyTyped, 0, sizeof(pCtx->_keyTyped));
	memset(pCtx->_keyReleased, 0, sizeof(pCtx->_keyReleased));

	auto pNewFocus = CalculateNewKeyFocus();

	if (pCtx->_keyFocus != pNewFocus)
	{
		if (pCtx->_keyFocus)
		{
			pCtx->_keyFocus->Client()->InternalFocusChanged(true);

			vgui2::ivgui()->PostMessage(
				vgui2::VPanelToHandle(pCtx->_keyFocus),
				new KeyValues("KillFocus"),
				NULL_HANDLE
			);

			pCtx->_keyFocus->Client()->Repaint();
		}

		if (pCtx->_keyFocus)
		{
			auto pFocus = pCtx->_keyFocus;

			while (pFocus && !pFocus->IsPopup())
			{
				pFocus = pFocus->GetParent();
			}

			if (pFocus)
			{
				pFocus->Client()->Repaint();
			}
		}

		if (!pNewFocus)
		{
			pCtx->_keyFocus = nullptr;
			return;
		}

		pNewFocus->Client()->InternalFocusChanged(false);

		vgui2::ivgui()->PostMessage(
			vgui2::VPanelToHandle(pNewFocus),
			new KeyValues("SetFocus"),
			NULL_HANDLE
		);

		pNewFocus->Client()->Repaint();

		auto pFocus = pNewFocus;

		while (pFocus && !pFocus->IsPopup())
		{
			pFocus = pFocus->GetParent();
		}

		if (pFocus)
		{
			pFocus->Client()->Repaint();
		}

		pCtx->_keyFocus = pNewFocus;

		pNewFocus->MoveToFront();
	}
}

void CInputWin32::UpdateMouseFocus(int x, int y)
{
	auto pCtx = GetCurrentContext();

	vgui2::VPANEL topMost = NULL_HANDLE;

	if (pCtx->_rootPanel)
	{
		vgui2::IClientPanel* pClient = vgui2::VHandleToPanel(pCtx->_rootPanel)->Client();

		if (!pClient)
		{
			SetMouseFocus(NULL_HANDLE);
			return;
		}

		topMost = pClient->IsWithinTraverse(x, y, false);
	}
	else
	{
		if (!vgui2::surface()->IsCursorVisible() ||
			!vgui2::surface()->IsWithin(x, y))
		{
			SetMouseFocus(NULL_HANDLE);
			return;
		}

		for (auto i = vgui2::surface()->GetPopupCount() - 1; i >= 0; --i)
		{
			auto pPopup = vgui2::VHandleToPanel(vgui2::surface()->GetPopup(i));

			auto wantsMouse = pPopup->IsMouseInputEnabled();

			auto isVisible = !vgui2::surface()->IsMinimized(vgui2::VPanelToHandle(pPopup));

			auto pParent = pPopup->GetParent();

			while (isVisible && pParent)
			{
				isVisible = pParent->IsVisible();
				pParent = pParent->GetParent();
			}

			if (wantsMouse && isVisible)
			{
				topMost = pPopup->Client()->IsWithinTraverse(x, y, false);

				if (topMost)
					break;
			}
		}

		if (!topMost)
		{
			topMost = vgui2::VHandleToPanel(
				vgui2::surface()->GetEmbeddedPanel()
			)->Client()->IsWithinTraverse(x, y, false);
		}
	}

	if (!topMost)
	{
		SetMouseFocus(NULL_HANDLE);
		return;
	}

	vgui2::VPANEL focus = NULL_HANDLE;

	if (IsChildOfModalPanel(topMost))
		focus = topMost;

	SetMouseFocus(focus);
}

void CInputWin32::PanelDeleted(vgui2::VPANEL panel)
{
	auto pPanel = vgui2::VHandleToPanel(panel);

	for (int i = m_Contexts.Head(); i != m_Contexts.InvalidIndex(); i = m_Contexts.Next(i))
	{
		PanelDeleted(pPanel, m_Contexts[i]);
	}

	PanelDeleted(pPanel, m_DefaultInputContext);
}

void CInputWin32::InternalCursorMoved(int x, int y)
{
	auto pCtx = GetCurrentContext();

	if (pCtx->m_nCursorX != x || pCtx->m_nCursorY != y)
	{
		pCtx->m_nCursorX = x;
		pCtx->m_nCursorY = y;

		UpdateMouseFocus(x, y);

		if (pCtx->_mouseCapture)
		{
			if (IsChildOfModalPanel(vgui2::VPanelToHandle(pCtx->_mouseCapture)))
			{
				if (pCtx->_mouseOver &&
					pCtx->_mouseOver != pCtx->_mouseCapture &&
					pCtx->_mouseOver->HasParent(pCtx->_mouseCapture))
				{
					vgui2::ivgui()->PostMessage(
						vgui2::VPanelToHandle(pCtx->_mouseOver),
						new KeyValues("CursorMoved", "xpos", x, "ypos", y),
						NULL_HANDLE
					);
				}

				vgui2::ivgui()->PostMessage(
					vgui2::VPanelToHandle(pCtx->_mouseCapture),
					new KeyValues("CursorMoved", "xpos", x, "ypos", y),
					NULL_HANDLE
				);
			}
		}
		else if (pCtx->_mouseFocus)
		{
			vgui2::ivgui()->PostMessage(
				vgui2::VPanelToHandle(pCtx->_mouseFocus),
				new KeyValues("CursorMoved", "xpos", x, "ypos", y),
				NULL_HANDLE
			);
		}
	}
}

void CInputWin32::InternalMousePressed(vgui2::MouseCode code)
{
	auto pCtx = GetCurrentContext();

	pCtx->_mousePressed[code] = true;
	pCtx->_mouseDown[code] = true;

	if (pCtx->_mouseCapture &&
		IsChildOfModalPanel(vgui2::VPanelToHandle(pCtx->_mouseCapture)))
	{
		bool bShouldSetCapture = false;

		if (pCtx->_mouseOver &&
			pCtx->_mouseOver != pCtx->_mouseCapture &&
			pCtx->_mouseOver->HasParent(pCtx->_mouseCapture))
		{
			vgui2::ivgui()->PostMessage(
				vgui2::VPanelToHandle(pCtx->_mouseOver),
				new KeyValues("MousePressed", "code", code),
				NULL_HANDLE
			);
		}
		else
			bShouldSetCapture = true;

		vgui2::ivgui()->PostMessage(
			vgui2::VPanelToHandle(pCtx->_mouseCapture),
			new KeyValues("MousePressed", "code", code),
			NULL_HANDLE
		);

		if (bShouldSetCapture)
			SetMouseCapture(NULL_HANDLE);
	}
	else if (pCtx->_mouseFocus &&
		IsChildOfModalPanel(vgui2::VPanelToHandle(pCtx->_mouseFocus)))
	{
		vgui2::ivgui()->PostMessage(
			vgui2::VPanelToHandle(pCtx->_mouseFocus),
			new KeyValues("MousePressed", "code", code),
			NULL_HANDLE
		);
	}

	if (IsChildOfModalPanel(vgui2::VPanelToHandle(pCtx->_mouseOver)))
	{
		vgui2::surface()->SetTopLevelFocus(vgui2::VPanelToHandle(pCtx->_mouseOver));
	}

	if (m_hContext == DEFAULT_INPUT_CONTEXT)
	{
		if (_updateToggleButtonState)
			_updateToggleButtonState = false;
	}
}

void CInputWin32::InternalMouseDoublePressed(vgui2::MouseCode code)
{
	auto pCtx = GetCurrentContext();

	pCtx->_mouseDoublePressed[code] = true;

	if (pCtx->_mouseCapture &&
		IsChildOfModalPanel(vgui2::VPanelToHandle(pCtx->_mouseCapture)))
	{
		if (pCtx->_mouseOver &&
			pCtx->_mouseOver != pCtx->_mouseCapture &&
			pCtx->_mouseOver->HasParent(pCtx->_mouseCapture))
		{
			vgui2::ivgui()->PostMessage(
				vgui2::VPanelToHandle(pCtx->_mouseOver),
				new KeyValues("MouseDoublePressed", "code", code),
				NULL_HANDLE
			);
		}

		vgui2::ivgui()->PostMessage(
			vgui2::VPanelToHandle(pCtx->_mouseCapture),
			new KeyValues("MouseDoublePressed", "code", code),
			NULL_HANDLE
		);
	}
	else if (pCtx->_mouseFocus &&
		IsChildOfModalPanel(vgui2::VPanelToHandle(pCtx->_mouseFocus)))
	{
		vgui2::ivgui()->PostMessage(
			vgui2::VPanelToHandle(pCtx->_mouseFocus),
			new KeyValues("MouseDoublePressed", "code", code),
			NULL_HANDLE
		);
	}

	if (IsChildOfModalPanel(vgui2::VPanelToHandle(pCtx->_mouseOver)))
	{
		vgui2::surface()->SetTopLevelFocus(vgui2::VPanelToHandle(pCtx->_mouseOver));
	}
}

void CInputWin32::InternalMouseReleased(vgui2::MouseCode code)
{
	auto pCtx = GetCurrentContext();

	pCtx->_mouseReleased[code] = true;
	pCtx->_mouseDown[code] = false;

	if (pCtx->_mouseCapture &&
		IsChildOfModalPanel(vgui2::VPanelToHandle(pCtx->_mouseCapture)))
	{
		if (pCtx->_mouseOver &&
			pCtx->_mouseOver != pCtx->_mouseCapture &&
			pCtx->_mouseOver->HasParent(pCtx->_mouseCapture))
		{
			vgui2::ivgui()->PostMessage(
				vgui2::VPanelToHandle(pCtx->_mouseOver),
				new KeyValues("MouseReleased", "code", code),
				NULL_HANDLE
			);
		}

		vgui2::ivgui()->PostMessage(
			vgui2::VPanelToHandle(pCtx->_mouseCapture),
			new KeyValues("MouseReleased", "code", code),
			NULL_HANDLE
		);
	}
	else if (pCtx->_mouseFocus &&
		IsChildOfModalPanel(vgui2::VPanelToHandle(pCtx->_mouseFocus)))
	{
		vgui2::ivgui()->PostMessage(
			vgui2::VPanelToHandle(pCtx->_mouseFocus),
			new KeyValues("MouseReleased", "code", code),
			NULL_HANDLE
		);
	}
}

void CInputWin32::InternalMouseWheeled(int delta)
{
	auto pCtx = GetCurrentContext();

	if (pCtx->_mouseFocus &&
		IsChildOfModalPanel(vgui2::VPanelToHandle(pCtx->_mouseFocus)))
	{
		vgui2::ivgui()->PostMessage(
			vgui2::VPanelToHandle(pCtx->_mouseFocus),
			new KeyValues("MouseWheeled", "delta", delta),
			NULL_HANDLE
		);
	}
}

void CInputWin32::InternalKeyCodePressed(vgui2::KeyCode code)
{
	if (code < 0 || code >= MAX_KEYS)
		return;

	auto pCtx = GetCurrentContext();

	pCtx->_keyDown[code] = true;
	pCtx->_keyPressed[code] = true;

	auto pMessage = new KeyValues("KeyCodePressed", "code", code);

	if (pCtx->_keyFocus &&
		IsChildOfModalPanel(vgui2::VPanelToHandle(pCtx->_keyFocus)))
	{
		vgui2::ivgui()->PostMessage(
			vgui2::VPanelToHandle(pCtx->_keyFocus),
			pMessage,
			NULL_HANDLE
		);
	}
	else
	{
		pMessage->deleteThis();
	}

	if (m_hContext == DEFAULT_INPUT_CONTEXT)
	{
		if (_updateToggleButtonState)
			_updateToggleButtonState = false;
	}
}

void CInputWin32::InternalKeyCodeTyped(vgui2::KeyCode code)
{
	if (code < 0 || code >= MAX_KEYS)
		return;

	auto pCtx = GetCurrentContext();

	pCtx->_keyTyped[code] = true;

	auto pMessage = new KeyValues("KeyCodeTyped", "code", code);

	if (pCtx->_keyFocus &&
		IsChildOfModalPanel(vgui2::VPanelToHandle(pCtx->_keyFocus)))
	{
		vgui2::ivgui()->PostMessage(
			vgui2::VPanelToHandle(pCtx->_keyFocus),
			pMessage,
			NULL_HANDLE
		);
	}
	else
	{
		pMessage->deleteThis();
	}
}

void CInputWin32::InternalKeyTyped(wchar_t unichar)
{
#if 0
	// MoeMod : dont worry, no loop here, just for jump out multiple if
	while (g_dwEngineBuildnum < 5953)
	{
		InputContext_t *pContext = GetContext(m_hContext);
		// set key state
		if (unichar <= vgui2::KEY_LAST)
		{
			pContext->_keyTyped[unichar] = 1;
		}

		/*// tell the current focused panel that a key was typed
		PostKeyMessage(new KeyValues("KeyTyped", "unichar", unichar));*/

		char cChar = (char)unichar;
		static bool bDoubleChar = false;
		static char sInpub[3] = { 0 };
		wchar_t *pComplete;

		if (bDoubleChar)
		{
			vgui2::VPANEL focus = GetFocus();

			if (focus)
			{
				const char *mod = vgui2::ipanel()->GetModuleName(focus);
				const char *name = vgui2::ipanel()->GetName(focus);

				//if (!strcmp(name, "ConsoleEntry") || !strcmp(name, "NameEntry"))
				{
					void *panel = vgui2::ipanel()->GetPanel(focus, mod);

					if (panel)
					{
						//void **pVtable = *(void ***)panel;

						sInpub[1] = cChar;
						bDoubleChar = false;
						pComplete = ANSIToUnicode(sInpub);

						//*(bool *)((char *)panel + 303) = true;
						//reinterpret_cast<void(__fastcall *)(void *, int, wchar_t)>(pVtable[128])(panel, 0, pComplete[0]);

						//PostKeyMessage(new KeyValues("KeyTyped", "unichar", pComplete[0]));
						unichar = static_cast<int>(pComplete[0]);
						break;
					}
				}
			}

			bDoubleChar = false;
		}
		else if (IsDBCSLeadByte(cChar))
		{
			bDoubleChar = true;
			sInpub[0] = cChar;
			return;
		}
		break;
	} // while
#endif

	{
		auto pMessage = new KeyValues("KeyTyped", "unichar", unichar);

		auto pCtx = GetCurrentContext();

		if (pCtx->_keyFocus &&
			IsChildOfModalPanel(vgui2::VPanelToHandle(pCtx->_keyFocus)))
		{
			vgui2::ivgui()->PostMessage(
				vgui2::VPanelToHandle(pCtx->_keyFocus),
				pMessage,
				NULL_HANDLE
			);
		}
		else
		{
			pMessage->deleteThis();
		}
	}

}

void CInputWin32::InternalKeyCodeReleased(vgui2::KeyCode code)
{
	if (code < vgui2::KEY_NONE || code >= MAX_KEYS)
		return;

	auto pCtx = GetCurrentContext();

	pCtx->_keyReleased[code] = true;
	pCtx->_keyDown[code] = false;

	//Could avoid allocating this by doing this inside the if body
	auto pMessage = new KeyValues("KeyCodeReleased", "code", code);

	if (pCtx->_keyFocus &&
		IsChildOfModalPanel(vgui2::VPanelToHandle(pCtx->_keyFocus)))
	{
		vgui2::ivgui()->PostMessage(
			vgui2::VPanelToHandle(pCtx->_keyFocus),
			pMessage,
			NULL_HANDLE
		);
	}
	else
	{
		pMessage->deleteThis();
	}
}

vgui2::HInputContext CInputWin32::CreateInputContext()
{
	auto index = m_Contexts.AddToTail();

	InitInputContext(&m_Contexts[index]);

	return static_cast<vgui2::HInputContext>(index);
}

void CInputWin32::DestroyInputContext(vgui2::HInputContext context)
{
	if (m_hContext == context)
		ActivateInputContext(DEFAULT_INPUT_CONTEXT);

	m_Contexts.Remove(context);
}

void CInputWin32::AssociatePanelWithInputContext(vgui2::HInputContext context, vgui2::VPANEL pRoot)
{
	auto pCtx = GetContext(context);

	if (pCtx->_rootPanel != pRoot)
	{
		ResetInputContext(context);
		pCtx->_rootPanel = pRoot;
	}
}

void CInputWin32::ActivateInputContext(vgui2::HInputContext context)
{
	m_hContext = context;
}

vgui2::VPANEL CInputWin32::GetMouseCapture()
{
	return vgui2::VPanelToHandle(m_DefaultInputContext._mouseCapture);
}

bool CInputWin32::IsChildOfModalPanel(vgui2::VPANEL panel)
{
	if (panel)
	{
		auto pModal = GetCurrentContext()->_appModalPanel;

		if (pModal)
		{
			return vgui2::VHandleToPanel(panel)->HasParent(pModal);
		}
	}

	return true;
}

void CInputWin32::ResetInputContext(vgui2::HInputContext context)
{
	InitInputContext(GetCurrentContext());
}

void CInputWin32::InitInputContext(InputContext_t* pContext)
{
	pContext->_rootPanel = NULL_HANDLE;

	pContext->_keyFocus = nullptr;
	pContext->_oldMouseFocus = nullptr;
	pContext->_mouseFocus = nullptr;
	pContext->_mouseOver = nullptr;
	pContext->_mouseCapture = nullptr;
	pContext->_appModalPanel = nullptr;

	pContext->m_nCursorX = 0;
	pContext->m_nCursorY = 0;

	memset(pContext->_mousePressed, 0, sizeof(pContext->_mousePressed));
	memset(pContext->_mouseDoublePressed, 0, sizeof(pContext->_mouseDoublePressed));
	memset(pContext->_mouseDown, 0, sizeof(pContext->_mouseDown));
	memset(pContext->_mouseReleased, 0, sizeof(pContext->_mouseReleased));
	memset(pContext->_keyPressed, 0, sizeof(pContext->_keyPressed));
	memset(pContext->_keyTyped, 0, sizeof(pContext->_keyTyped));
	memset(pContext->_keyDown, 0, sizeof(pContext->_keyDown));
	memset(pContext->_keyReleased, 0, sizeof(pContext->_keyReleased));
}

InputContext_t* CInputWin32::GetContext(vgui2::HInputContext context)
{
	if (context == DEFAULT_INPUT_CONTEXT)
		return &m_DefaultInputContext;
	else
		return &m_Contexts[m_hContext];
}

InputContext_t* CInputWin32::GetCurrentContext()
{
	return GetContext(m_hContext);
}

void CInputWin32::PanelDeleted(vgui2::VPanel* pFocus, InputContext_t& context)
{
	if (context._keyFocus == pFocus)
	{
		context._keyFocus = nullptr;
	}

	if (context._oldMouseFocus == pFocus)
	{
		context._oldMouseFocus = nullptr;
	}

	if (context._mouseFocus == pFocus)
	{
		context._mouseFocus = nullptr;
	}

	if (context._mouseOver == pFocus)
	{
		context._mouseOver = nullptr;
	}

	if (context._mouseCapture == pFocus)
	{
		SetMouseCapture(NULL_HANDLE);
		context._mouseCapture = nullptr;
	}

	if (context._appModalPanel == pFocus)
	{
		ReleaseAppModalSurface();
	}
}

vgui2::VPanel* CInputWin32::CalculateNewKeyFocus()
{
	auto pCtx = GetCurrentContext();

	vgui2::VPanel* pFocus;

	if (pCtx->_rootPanel)
	{
		pFocus = vgui2::VHandleToPanel(
			vgui2::VHandleToPanel(
				pCtx->_rootPanel
			)->Client()->GetCurrentKeyFocus()
		);

		if (!pFocus)
			pFocus = vgui2::VHandleToPanel(pCtx->_rootPanel);
	}
	else
	{
		pFocus = nullptr;

		for (auto i = vgui2::surface()->GetPopupCount() - 1; i >= 0; --i)
		{
			auto pPopup = vgui2::VHandleToPanel(vgui2::surface()->GetPopup(i));

			if (!pPopup->IsPopup() ||
				!pPopup->IsVisible() ||
				!pPopup->IsKeyBoardInputEnabled() ||
				vgui2::surface()->IsMinimized(vgui2::VPanelToHandle(pPopup)))
				continue;

			auto isVisible = pPopup->IsVisible();

			auto pParent = pPopup->GetParent();

			while (pParent && (isVisible = pParent->IsVisible()))
			{
				pParent = pParent->GetParent();
			}

			if (!isVisible)
				continue;

			if (!vgui2::surface()->IsMinimized(vgui2::VPanelToHandle(pPopup)))
			{
				pFocus = vgui2::VHandleToPanel(
					pPopup->Client()->GetCurrentKeyFocus()
				);

				if (!pFocus)
					pFocus = pPopup;
				break;
			}
		}
	}

	if (!vgui2::surface()->HasFocus())
		pFocus = nullptr;

	if (!IsChildOfModalPanel(vgui2::VPanelToHandle(pFocus)))
		pFocus = nullptr;

	return pFocus;
}
