#pragma warning(disable: 4310)

#define _WIN32_WINNT 0x0502
#include <oleidl.h>
#include <winerror.h>
#include <comdef.h>
#include <assert.h>
#include "HtmlWindow.h"
#include <tier0/dbg.h>

#include <mshtml.h>
#include <mshtmdid.h>
#include <mshtmhst.h>

#include <vgui/ISurface.h>
#include "vgui/iinput.h"
#include "vgui/iinputinternal.h"
#include "vgui_key_translation.h"

#include <vgui_controls/Controls.h>

#include <tchar.h>
#include <stdio.h>

#ifdef UNICODE
#define GetClassName GetClassNameW
#else
#define GetClassName GetClassNameA
#endif

#ifdef _DEBUG
#ifdef DEBUG
#undef DEBUG
#endif
#define DEBUG(x) OutputDebugString(#x "\n")
#else
#define DEBUG(x)
#endif

#define ASSERT assert

class HtmlWindow;
class FS_IOleInPlaceFrame;
class FS_IOleInPlaceSiteWindowless;
class FS_IOleClientSite;
class FS_IOleControlSite;
class FS_IOleCommandTarget;
class FS_IOleItemContainer;
class FS_IDispatch;
class FS_DWebBrowserEvents2;
class FS_DHTMLDocumentEvents2;
class FS_IAdviseSink2;
class FS_IAdviseSinkEx;
class FS_IDocHostUIHandler;

class FrameSite : public IUnknown
{
	friend class HtmlWindow;
	friend class FS_IOleInPlaceFrame;
	friend class FS_IOleInPlaceSiteWindowless;
	friend class FS_IOleClientSite;
	friend class FS_IOleControlSite;
	friend class FS_IOleCommandTarget;
	friend class FS_IOleItemContainer;
	friend class FS_IDispatch;
	friend class FS_DWebBrowserEvents2;
	friend class FS_DHTMLDocumentEvents2;
	friend class FS_IAdviseSink2;
	friend class FS_IAdviseSinkEx;
	friend class FS_IDocHostUIHandler;

public:
	FrameSite(HtmlWindow *win, bool AllowJavaScript);
	~FrameSite(void);

	STDMETHODIMP QueryInterface(REFIID iid, void **ppvObject);
	ULONG STDMETHODCALLTYPE AddRef(void);
	ULONG STDMETHODCALLTYPE Release(void);

	FS_IAdviseSinkEx *m_IAdviseSinkEx;

protected:
	int m_cRef;

	FS_IOleInPlaceFrame *m_IOleInPlaceFrame;
	FS_IOleInPlaceSiteWindowless *m_IOleInPlaceSiteWindowless;
	FS_IOleClientSite *m_IOleClientSite;
	FS_IOleControlSite *m_IOleControlSite;
	FS_IOleCommandTarget *m_IOleCommandTarget;
	FS_IOleItemContainer *m_IOleItemContainer;
	FS_IDispatch *m_IDispatch;
	FS_DWebBrowserEvents2 *m_DWebBrowserEvents2;
	FS_DHTMLDocumentEvents2 *m_DHTMLDocumentEvents2;
	FS_IAdviseSink2 *m_IAdviseSink2;
	FS_IDocHostUIHandler *m_IDocHostUIHandler;
	HtmlWindow *m_window;

	HDC m_hDCBuffer;
	HWND m_hWndParent;

	bool m_bSupportsWindowlessActivation;
	bool m_bInPlaceLocked;
	bool m_bInPlaceActive;
	bool m_bUIActive;
	bool m_bWindowless;

	LCID m_nAmbientLocale;
	COLORREF m_clrAmbientForeColor;
	COLORREF m_clrAmbientBackColor;
	bool m_bAmbientShowHatching;
	bool m_bAmbientShowGrabHandles;
	bool m_bAmbientUserMode;
	bool m_bAmbientAppearance;
};

class FS_IOleInPlaceFrame : public IOleInPlaceFrame
{
public:
	FS_IOleInPlaceFrame(FrameSite *fs) { m_fs = fs; }
	~FS_IOleInPlaceFrame(void) {}

	STDMETHODIMP QueryInterface(REFIID iid, void **ppvObject) { return m_fs->QueryInterface(iid, ppvObject); }
	ULONG STDMETHODCALLTYPE AddRef(void) { return m_fs->AddRef(); }
	ULONG STDMETHODCALLTYPE Release(void) { return m_fs->Release(); }

	STDMETHODIMP GetWindow(HWND *);
	STDMETHODIMP ContextSensitiveHelp(BOOL);

	STDMETHODIMP GetBorder(LPRECT);
	STDMETHODIMP RequestBorderSpace(LPCBORDERWIDTHS);
	STDMETHODIMP SetBorderSpace(LPCBORDERWIDTHS);
	STDMETHODIMP SetActiveObject(IOleInPlaceActiveObject *, LPCOLESTR);

	STDMETHODIMP InsertMenus(HMENU, LPOLEMENUGROUPWIDTHS);
	STDMETHODIMP SetMenu(HMENU, HOLEMENU, HWND);
	STDMETHODIMP RemoveMenus(HMENU);
	STDMETHODIMP SetStatusText(LPCOLESTR);
	STDMETHODIMP EnableModeless(BOOL);
	STDMETHODIMP TranslateAccelerator(LPMSG, WORD);

protected:
	FrameSite *m_fs;
};

class FS_IOleInPlaceSiteWindowless : public IOleInPlaceSiteWindowless
{
public:
	FS_IOleInPlaceSiteWindowless(FrameSite *fs) { m_fs = fs; }
	~FS_IOleInPlaceSiteWindowless(void) {}

	STDMETHODIMP QueryInterface(REFIID iid, void **ppvObject) { return m_fs->QueryInterface(iid, ppvObject); }
	ULONG STDMETHODCALLTYPE AddRef(void) { return m_fs->AddRef(); }
	ULONG STDMETHODCALLTYPE Release(void) { return m_fs->Release(); }

	STDMETHODIMP GetWindow(HWND *h) { return m_fs->m_IOleInPlaceFrame->GetWindow(h); }
	STDMETHODIMP ContextSensitiveHelp(BOOL b) { return m_fs->m_IOleInPlaceFrame->ContextSensitiveHelp(b); }

	STDMETHODIMP CanInPlaceActivate(void);
	STDMETHODIMP OnInPlaceActivate(void);
	STDMETHODIMP OnUIActivate(void);
	STDMETHODIMP GetWindowContext(IOleInPlaceFrame **, IOleInPlaceUIWindow **, LPRECT, LPRECT, LPOLEINPLACEFRAMEINFO);
	STDMETHODIMP Scroll(SIZE);
	STDMETHODIMP OnUIDeactivate(BOOL);
	STDMETHODIMP OnInPlaceDeactivate(void);
	STDMETHODIMP DiscardUndoState(void);
	STDMETHODIMP DeactivateAndUndo(void);
	STDMETHODIMP OnPosRectChange(LPCRECT);
	STDMETHODIMP OnInPlaceActivateEx(BOOL *, DWORD);
	STDMETHODIMP OnInPlaceDeactivateEx(BOOL);
	STDMETHODIMP RequestUIActivate(void);
	STDMETHODIMP CanWindowlessActivate(void);
	STDMETHODIMP GetCapture(void);
	STDMETHODIMP SetCapture(BOOL);
	STDMETHODIMP GetFocus(void);
	STDMETHODIMP SetFocus(BOOL);
	STDMETHODIMP GetDC(LPCRECT, DWORD, HDC *);
	STDMETHODIMP ReleaseDC(HDC);
	STDMETHODIMP InvalidateRect(LPCRECT, BOOL);
	STDMETHODIMP InvalidateRgn(HRGN, BOOL);
	STDMETHODIMP ScrollRect(INT, INT, LPCRECT, LPCRECT);
	STDMETHODIMP AdjustRect(LPRECT);
	STDMETHODIMP OnDefWindowMessage(UINT, WPARAM, LPARAM, LRESULT *);

protected:
	FrameSite *m_fs;
};

class FS_IOleClientSite : public IOleClientSite
{
public:
	FS_IOleClientSite(FrameSite *fs) { m_fs = fs; }
	~FS_IOleClientSite(void) {}

	STDMETHODIMP QueryInterface(REFIID iid, void **ppvObject) { return m_fs->QueryInterface(iid, ppvObject); }
	ULONG STDMETHODCALLTYPE AddRef(void) { return m_fs->AddRef(); }
	ULONG STDMETHODCALLTYPE Release(void) { return m_fs->Release(); }

	STDMETHODIMP SaveObject(void);
	STDMETHODIMP GetMoniker(DWORD, DWORD, IMoniker**);
	STDMETHODIMP GetContainer(LPOLECONTAINER FAR*);
	STDMETHODIMP ShowObject(void);
	STDMETHODIMP OnShowWindow(BOOL);
	STDMETHODIMP RequestNewObjectLayout(void);

protected:
	FrameSite *m_fs;
};

class FS_IOleControlSite : public IOleControlSite
{
public:
	FS_IOleControlSite(FrameSite *fs) { m_fs = fs; }
	~FS_IOleControlSite(void) {}

	STDMETHODIMP QueryInterface(REFIID iid, void **ppvObject) { return m_fs->QueryInterface(iid, ppvObject); }
	ULONG STDMETHODCALLTYPE AddRef(void) { return m_fs->AddRef(); }
	ULONG STDMETHODCALLTYPE Release(void) { return m_fs->Release(); }

	STDMETHODIMP OnControlInfoChanged(void);
	STDMETHODIMP LockInPlaceActive(BOOL);
	STDMETHODIMP GetExtendedControl(IDispatch **);
	STDMETHODIMP TransformCoords(POINTL*, POINTF *, DWORD);
	STDMETHODIMP ShowContextMenu(DWORD dwID,POINT *ppt, IUnknown *pcmdtReserved, IDispatch **pdispReserved);
	STDMETHODIMP TranslateAccelerator(LPMSG, DWORD);
	STDMETHODIMP OnFocus(BOOL);
	STDMETHODIMP ShowPropertyFrame(void);

protected:
	FrameSite *m_fs;
};

class FS_IOleCommandTarget : public IOleCommandTarget
{
public:
	FS_IOleCommandTarget(FrameSite *fs) { m_fs = fs; }
	~FS_IOleCommandTarget(void) {}

	STDMETHODIMP QueryInterface(REFIID iid, void **ppvObject) { return m_fs->QueryInterface(iid, ppvObject); }
	ULONG STDMETHODCALLTYPE AddRef(void) { return m_fs->AddRef(); }
	ULONG STDMETHODCALLTYPE Release(void) { return m_fs->Release(); }
	STDMETHODIMP QueryStatus(const GUID *, ULONG, OLECMD[], OLECMDTEXT *);
	STDMETHODIMP Exec(const GUID *, DWORD, DWORD, VARIANTARG *, VARIANTARG *);

protected:
	FrameSite *m_fs;
};

class FS_IOleItemContainer : public IOleItemContainer
{
public:
	FS_IOleItemContainer(FrameSite *fs) { m_fs = fs; }
	~FS_IOleItemContainer(void) {}

	STDMETHODIMP QueryInterface(REFIID iid, void **ppvObject) { return m_fs->QueryInterface(iid, ppvObject); }
	ULONG STDMETHODCALLTYPE AddRef(void) { return m_fs->AddRef(); }
	ULONG STDMETHODCALLTYPE Release(void) { return m_fs->Release(); }

	STDMETHODIMP ParseDisplayName(IBindCtx *, LPOLESTR, ULONG *, IMoniker **);
	STDMETHODIMP EnumObjects(DWORD, IEnumUnknown **);
	STDMETHODIMP LockContainer(BOOL);

	STDMETHODIMP GetObject(LPOLESTR, DWORD, IBindCtx *, REFIID, void **);
	STDMETHODIMP GetObjectStorage(LPOLESTR, IBindCtx *, REFIID, void **);
	STDMETHODIMP IsRunning(LPOLESTR);

protected:
	FrameSite *m_fs;
};

class FS_IDispatch : public IDispatch
{
public:
	FS_IDispatch(FrameSite *fs) { m_fs = fs; m_bNewURL = 0; m_bOnLink = false; m_bAllowJavaScript = true;}
	~FS_IDispatch(void) {}

	STDMETHODIMP QueryInterface(REFIID iid, void **ppvObject) { return m_fs->QueryInterface(iid, ppvObject); }
	ULONG STDMETHODCALLTYPE AddRef(void) { return m_fs->AddRef(); }
	ULONG STDMETHODCALLTYPE Release(void) { return m_fs->Release(); }

	STDMETHODIMP GetIDsOfNames(REFIID, OLECHAR **, unsigned int, LCID, DISPID *);
	STDMETHODIMP GetTypeInfo(unsigned int, LCID, ITypeInfo **);
	STDMETHODIMP GetTypeInfoCount(unsigned int *);
	STDMETHODIMP Invoke(DISPID, REFIID, LCID, WORD, DISPPARAMS *, VARIANT *, EXCEPINFO *, UINT *);

	void SetAllowJavaScript(bool state) { m_bAllowJavaScript = state; }

protected:
	FrameSite *m_fs;
	unsigned int m_bNewURL;
	bool m_bAllowJavaScript;
	bool m_bOnLink;
};

class FS_DWebBrowserEvents2 : public DWebBrowserEvents2
{
public:
	FS_DWebBrowserEvents2(FrameSite *fs) { m_fs = fs; }
	~FS_DWebBrowserEvents2(void) {}

	STDMETHODIMP QueryInterface(REFIID iid, void **ppvObject) { return m_fs->QueryInterface(iid, ppvObject); }
	ULONG STDMETHODCALLTYPE AddRef(void) { return m_fs->AddRef(); }
	ULONG STDMETHODCALLTYPE Release(void) { return m_fs->Release(); }

	STDMETHODIMP GetIDsOfNames(REFIID r, OLECHAR **o, unsigned int i, LCID l, DISPID *d) { return m_fs->m_IDispatch->GetIDsOfNames(r, o, i, l, d); }
	STDMETHODIMP GetTypeInfo(unsigned int i, LCID l, ITypeInfo **t) { return m_fs->m_IDispatch->GetTypeInfo(i, l, t); }
	STDMETHODIMP GetTypeInfoCount(unsigned int *i) { return m_fs->m_IDispatch->GetTypeInfoCount(i); }
	STDMETHODIMP Invoke(DISPID d, REFIID r, LCID l, WORD w, DISPPARAMS *dp, VARIANT *v, EXCEPINFO *e, UINT *u) { return m_fs->m_IDispatch->Invoke(d, r, l, w, dp, v, e, u); }

protected:
	FrameSite *m_fs;
};

class FS_DHTMLDocumentEvents2 : public HTMLDocumentEvents
{
public:
	FS_DHTMLDocumentEvents2(FrameSite *fs) { m_fs = fs; }
	~FS_DHTMLDocumentEvents2(void) {}

	STDMETHODIMP QueryInterface(REFIID iid, void **ppvObject) { return m_fs->QueryInterface(iid, ppvObject); }
	ULONG STDMETHODCALLTYPE AddRef(void) { return m_fs->AddRef(); }
	ULONG STDMETHODCALLTYPE Release(void) { return m_fs->Release(); }

	STDMETHODIMP GetIDsOfNames(REFIID r, OLECHAR **o, unsigned int i, LCID l, DISPID *d) { return m_fs->m_IDispatch->GetIDsOfNames(r, o, i, l, d); }
	STDMETHODIMP GetTypeInfo(unsigned int i, LCID l, ITypeInfo **t) { return m_fs->m_IDispatch->GetTypeInfo(i, l, t); }
	STDMETHODIMP GetTypeInfoCount(unsigned int *i) { return m_fs->m_IDispatch->GetTypeInfoCount(i); }
	STDMETHODIMP Invoke(DISPID d, REFIID r, LCID l, WORD w, DISPPARAMS *dp, VARIANT *v, EXCEPINFO *e, UINT *u) { return m_fs->m_IDispatch->Invoke(d, r, l, w, dp, v, e, u); }

protected:
	FrameSite *m_fs;
};

class FS_IAdviseSink2 : public IAdviseSink2
{
public:
	FS_IAdviseSink2(FrameSite *fs) { m_fs = fs; }
	~FS_IAdviseSink2(void) {}

	STDMETHODIMP QueryInterface(REFIID iid, void **ppvObject) { return m_fs->QueryInterface(iid, ppvObject); }
	ULONG STDMETHODCALLTYPE AddRef() { return m_fs->AddRef(); }
	ULONG STDMETHODCALLTYPE Release() { return m_fs->Release(); }

	void STDMETHODCALLTYPE OnDataChange(FORMATETC *, STGMEDIUM *);
	void STDMETHODCALLTYPE OnViewChange(DWORD, LONG);
	void STDMETHODCALLTYPE OnRename(IMoniker *);
	void STDMETHODCALLTYPE OnSave(void);
	void STDMETHODCALLTYPE OnClose(void);
	void STDMETHODCALLTYPE OnLinkSrcChange(IMoniker *);

protected:
	FrameSite *m_fs;
};

class FS_IAdviseSinkEx : public IAdviseSinkEx
{
public:
	FS_IAdviseSinkEx(FrameSite *fs) { m_fs = fs; }
	~FS_IAdviseSinkEx(void) {}

	STDMETHODIMP QueryInterface(REFIID iid, void **ppvObject) { return m_fs->QueryInterface(iid, ppvObject); }
	ULONG STDMETHODCALLTYPE AddRef(void) { return m_fs->AddRef(); }
	ULONG STDMETHODCALLTYPE Release(void) { return m_fs->Release(); }

	void STDMETHODCALLTYPE OnDataChange(FORMATETC *f, STGMEDIUM *s) { m_fs->m_IAdviseSink2->OnDataChange(f, s); }
	void STDMETHODCALLTYPE OnViewChange(DWORD d, LONG l) { m_fs->m_IAdviseSink2->OnViewChange(d, l); }
	void STDMETHODCALLTYPE OnRename(IMoniker *i) { m_fs->m_IAdviseSink2->OnRename(i); }
	void STDMETHODCALLTYPE OnSave(void) { m_fs->m_IAdviseSink2->OnSave(); }
	void STDMETHODCALLTYPE OnClose(void) { m_fs->m_IAdviseSink2->OnClose(); }
	void STDMETHODCALLTYPE OnViewStatusChange(DWORD);

protected:
	FrameSite *m_fs;
};

class FS_IDocHostUIHandler : public IDocHostUIHandler
{
public:
	FS_IDocHostUIHandler(FrameSite *fs) { m_fs = fs; }
	~FS_IDocHostUIHandler(void) {}

	STDMETHODIMP QueryInterface(REFIID iid, void **ppvObject) { return m_fs->QueryInterface(iid, ppvObject); }
	ULONG STDMETHODCALLTYPE AddRef(void) { return m_fs->AddRef(); }
	ULONG STDMETHODCALLTYPE Release(void) { return m_fs->Release(); }

	STDMETHODIMP GetHostInfo(DOCHOSTUIINFO __RPC_FAR *pInfo);
	STDMETHODIMP ShowContextMenu(DWORD dwID, POINT __RPC_FAR *ppt, IUnknown __RPC_FAR *pcmdtReserved, IDispatch __RPC_FAR *pdispReserved) { return S_FALSE; }
	STDMETHODIMP ShowUI(DWORD dwID, IOleInPlaceActiveObject __RPC_FAR *pActiveObject, IOleCommandTarget __RPC_FAR *pCommandTarget, IOleInPlaceFrame __RPC_FAR *pFrame, IOleInPlaceUIWindow __RPC_FAR *pDoc) { return S_FALSE; }
	STDMETHODIMP HideUI(void) { return S_OK; }
	STDMETHODIMP UpdateUI(void) { return S_OK; }
	STDMETHODIMP EnableModeless(BOOL fEnable) { return S_OK; }
	STDMETHODIMP OnDocWindowActivate(BOOL fActivate) { return S_OK; }
	STDMETHODIMP OnFrameWindowActivate(BOOL fActivate) { return S_OK; }
	STDMETHODIMP ResizeBorder(LPCRECT prcBorder, IOleInPlaceUIWindow __RPC_FAR *pUIWindow, BOOL fRameWindow) { return S_OK; }
	STDMETHODIMP TranslateAccelerator(LPMSG lpMsg, const GUID __RPC_FAR *pguidCmdGroup, DWORD nCmdID) { return S_FALSE; }
	STDMETHODIMP GetOptionKeyPath(LPOLESTR __RPC_FAR *pchKey, DWORD dw) { return S_FALSE; }
	STDMETHODIMP GetDropTarget(IDropTarget __RPC_FAR *pDropTarget, IDropTarget __RPC_FAR *__RPC_FAR *ppDropTarget) { return E_FAIL; }
	STDMETHODIMP GetExternal(IDispatch __RPC_FAR *__RPC_FAR *ppDispatch) { ppDispatch = NULL; return S_FALSE; }
	STDMETHODIMP TranslateUrl(DWORD dwTranslate, OLECHAR __RPC_FAR *pchURLIn, OLECHAR __RPC_FAR *__RPC_FAR *ppchURLOut) { ppchURLOut = NULL; return S_FALSE; }
	STDMETHODIMP FilterDataObject(IDataObject __RPC_FAR *pDO, IDataObject __RPC_FAR *__RPC_FAR *ppDORet) { ppDORet = NULL; return S_FALSE; }

protected:
	FrameSite *m_fs;
};

HRESULT FS_IDocHostUIHandler::GetHostInfo(DOCHOSTUIINFO __RPC_FAR *pInfo)
{
	pInfo->cbSize = sizeof(DOCHOSTUIINFO);
	pInfo->dwFlags = DOCHOSTUIFLAG_NO3DBORDER | DOCHOSTUIFLAG_SCROLL_NO;
	return S_OK;
}

CUtlVector<HtmlWindow *> html_windows;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HtmlWindow *win = NULL;

	for (int i = 0; i < html_windows.Count(); i++)
	{
		if (html_windows[i] && html_windows[i]->GetIEHWND() == hwnd)
		{
			win = html_windows[i];
			break;
		}
	}

	if (win)
	{
		switch (uMsg)
		{
			case WM_KILLFOCUS:
			{
				return 0;
				break;
			}

			case WM_SETFOCUS:
			{
				::PostMessage(win->GetHWND(), WM_APP, 0, 0);
				return 0;
				break;
			}

			default:
			{
				return ::CallWindowProc((WNDPROC)win->GetIEWndProc(), hwnd, uMsg, wParam, lParam);
				break;
			}
		}
	}

	return 0;
}

LRESULT CALLBACK GetMsgProc(int code, WPARAM wParam, LPARAM lParam)
{
	MSG *msg = reinterpret_cast<MSG *>(lParam);
	HtmlWindow *win=NULL;

	for (int i = 0; i < html_windows.Count(); i++)
	{
		if (html_windows[i] && html_windows[i]->GetIEHWND() == msg->hwnd)
		{
			win = html_windows[i];
			break;
		}
	}

	if (msg && win)
	{
		switch (msg->message)
		{
			case WM_TIMER:
			{
				win->OnUpdate();
				break;
			}

			default:break;
		}

		return CallNextHookEx(win->GetHook(), code, wParam, lParam);
	}

	return 0;
}

BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam)
{
	POINT pt;
	pt.y = lParam >> 16;
	pt.x = lParam & 0xffff;

	HtmlWindow *win = NULL;
	HWND parent = ::GetParent(hwnd);

	for (int i = 0;i < html_windows.Count(); i++)
	{
		if (html_windows[i] && html_windows[i]->GetIEHWND() == parent)
		{
			win = html_windows[i];
			break;
		}
	}

	if (win)
	{
		RECT win_rect;
		::GetWindowRect(hwnd, &win_rect);
		RECT ie_rect;
		::GetWindowRect(win->GetIEHWND(), &ie_rect);

		pt.x -= (win_rect.left - ie_rect.left);
		pt.y -= (win_rect.top - ie_rect.top);

		lParam = ((int)pt.y) << 16;
		lParam |= (((int)pt.x) & 0xffff);

		win_rect.bottom = (win_rect.bottom - win_rect.top);
		win_rect.top = 0;
		win_rect.right = (win_rect.right - win_rect.left);
		win_rect.left = 0;

		if (::PtInRect(&win_rect,pt))
		{
			if (::PostMessage(hwnd, win->GetMouseMessage(), 1, lParam) == 0)
			{
				DEBUG("msg2 not delivered");
				return true;
			}

			if (::PostMessage(hwnd, WM_SETFOCUS, reinterpret_cast<uint>(win->GetIEHWND()), lParam) == 0)
			{
				DEBUG("msg2 not delivered");
				return true;
			}

		}
	}

	return true;
}

HtmlWindow::HtmlWindow(vgui2::IHTMLEvents *events, vgui2::VPANEL c, HWND parent, bool AllowJavaScript, bool DirectToHWND)
{
	m_oleObject = NULL;
	m_oleInPlaceObject = NULL;
	m_webBrowser = NULL;

	m_events = events;
	m_ieHWND = 0;

	w = 0;
	h = 0;
	window_x = 0;
	window_y = 0;
	textureID = 0;
	m_HtmlEventsAdviseCookie = 0;
	m_Bitmap = NULL;
	m_bVisible = false;

	hdcMem = NULL;
	lasthDC = NULL;
	hBitmap = NULL;
	m_hHook = NULL;

	m_cleared = false;
	m_newpage = false;
	m_bHooked = false;
	m_bDirectToHWND = DirectToHWND;
	m_bParentCreated = false;
	m_hIEWndProc = NULL;

	strcpy(m_currentUrl, "");
	m_specificallyOpened = false;
	m_parent = parent;
	m_vcontext = c;

	if (!DirectToHWND)
	{
		char tmp[50];

		if (::GetClassName(parent,tmp,50) == 0)
		{
			WNDCLASS wc;
			memset(&wc, 0, sizeof(wc));

			wc.style = CS_OWNDC;
			wc.lpfnWndProc = WindowProc;
			wc.hInstance = GetModuleHandle(NULL);
			wc.lpszClassName = "VGUI_HTML";

			UnregisterClass("VGUI_HTML", GetModuleHandle(NULL));

			RegisterClass(&wc);
			strcpy(tmp, "VGUI_HTML");
		}

		m_parent = CreateWindowEx(0, tmp, "", WS_CHILD | WS_DISABLED, 0, 0, 1, 1, parent, NULL,GetModuleHandle(NULL), NULL);
		m_bParentCreated = true;
	}

	CreateBrowser(AllowJavaScript);
}

void HtmlWindow::CreateBrowser(bool AllowJavaScript)
{
	IUnknown *p;
	HRESULT hret = CoCreateInstance(CLSID_WebBrowser, NULL, CLSCTX_ALL, IID_IUnknown, (void **)(&p));

	ASSERT(SUCCEEDED(hret));

	hret = p->QueryInterface(IID_IViewObject, (void **)(&m_viewObject));
	ASSERT(SUCCEEDED(hret));
	hret = p->QueryInterface(IID_IOleObject, (void **)(&m_oleObject));
	ASSERT(SUCCEEDED(hret));

	FrameSite *c = new FrameSite(this, AllowJavaScript);
	c->AddRef();

	m_fs = c;
	m_viewObject->SetAdvise(DVASPECT_CONTENT, ADVF_PRIMEFIRST, c->m_IAdviseSinkEx);

	DWORD dwMiscStatus;
	m_oleObject->GetMiscStatus(DVASPECT_CONTENT, &dwMiscStatus);

	if (dwMiscStatus & OLEMISC_SETCLIENTSITEFIRST)
		m_bSetClientSiteFirst = true;
	else
		m_bSetClientSiteFirst = false;

	if (dwMiscStatus & OLEMISC_INVISIBLEATRUNTIME)
		m_bVisibleAtRuntime = false;
	else
		m_bVisibleAtRuntime = true;

	if (m_bSetClientSiteFirst)
		m_oleObject->SetClientSite(c->m_IOleClientSite);

	IPersistStreamInit *psInit = NULL;
	hret = p->QueryInterface(IID_IPersistStreamInit, (void **)(&psInit));

	if (SUCCEEDED(hret) && psInit != NULL)
	{
		hret = psInit->InitNew();
		ASSERT(SUCCEEDED(hret));
	}

	psInit->Release();

	hret = p->QueryInterface(IID_IOleInPlaceObject, (void **)(&m_oleInPlaceObject));
	assert(SUCCEEDED(hret));

	RECT posRect;
	posRect.left = 0;
	posRect.top = 0;
	posRect.right = w;
	posRect.bottom = h;

	m_oleInPlaceObject->SetObjectRects(&posRect, &posRect);

	MSG msg;

	if (m_bVisibleAtRuntime)
	{
		hret = m_oleObject->DoVerb(OLEIVERB_INPLACEACTIVATE, &msg, c->m_IOleClientSite, 0, m_parent, &posRect);
		assert(SUCCEEDED(hret));
	}

	hret = m_oleInPlaceObject->GetWindow(&m_oleObjectHWND);
	ASSERT(SUCCEEDED(hret));

	if (!m_bSetClientSiteFirst)
		m_oleObject->SetClientSite(c->m_IOleClientSite);

	hret = p->QueryInterface(IID_IWebBrowser2, (void **)(&m_webBrowser));
	assert(SUCCEEDED(hret));

	IConnectionPointContainer *cpContainer;
	hret = p->QueryInterface(IID_IConnectionPointContainer, (void **)(&cpContainer));
	assert(SUCCEEDED(hret));
	hret = cpContainer->FindConnectionPoint(DIID_DWebBrowserEvents2, &m_connectionPoint);
	assert(SUCCEEDED(hret));

	m_connectionPoint->Advise(c->m_DWebBrowserEvents2, &m_adviseCookie);
	cpContainer->Release();

	p->Release();

	m_webBrowser->put_MenuBar(VARIANT_FALSE);
	m_webBrowser->put_AddressBar(VARIANT_FALSE);
	m_webBrowser->put_StatusBar(VARIANT_FALSE);
	m_webBrowser->put_ToolBar(VARIANT_FALSE);

	m_webBrowser->put_RegisterAsBrowser(VARIANT_TRUE);
	m_webBrowser->put_RegisterAsDropTarget(VARIANT_TRUE);
	m_webBrowser->put_Silent(VARIANT_TRUE);

	if (m_bDirectToHWND)
		::ShowWindow(m_oleObjectHWND, SW_HIDE);

	html_windows.AddToTail(this);
}

HtmlWindow::~HtmlWindow(void)
{
	html_windows.FindAndRemove(this);

	if (m_hHook)
	{
		::UnhookWindowsHookEx(m_hHook);
	}

	if (m_webBrowser)
	{
		m_webBrowser->Release();
	}

	if (m_oleInPlaceObject)
	{
		if (!m_bVisibleAtRuntime)
			m_oleInPlaceObject->InPlaceDeactivate();

		m_oleInPlaceObject->UIDeactivate();
		m_oleInPlaceObject->Release();
	}

	if (m_connectionPoint)
	{
		m_connectionPoint->Unadvise(m_adviseCookie);
		m_connectionPoint->Unadvise(m_HtmlEventsAdviseCookie);
		m_connectionPoint->Release();
	}

	if (m_oleObject)
	{
		if (!m_bVisibleAtRuntime)
			m_oleObject->Close(OLECLOSE_NOSAVE);

		m_oleObject->SetClientSite(NULL);
		m_oleObject->Release();
	}

	if (m_viewObject)
	{
		m_viewObject->Release();
	}

	if (m_fs)
	{
		delete (FrameSite *)m_fs;
	}

	if (lasthDC)
	{
		::DeleteObject(hBitmap);
		::DeleteDC(hdcMem);
	}

	if (m_bParentCreated)
	{
		DestroyWindow(m_parent);
	}

	delete m_Bitmap;
}

void HtmlWindow::OnSize(int x, int y, int w_in, int h_in)
{
	w = w_in;
	h = h_in;

	html_x = x;
	html_y = y;

	int panel_x, panel_y;

#if defined (GAMEUI_EXPORTS) || defined(GAMEOVERLAYUI_EXPORTS)
	vgui2::ipanel()->GetAbsPos(m_vcontext, panel_x, panel_y);

	::SetWindowPos(m_parent, HWND_TOP, panel_x, panel_y, w, h, SWP_HIDEWINDOW | SWP_NOACTIVATE);
#else
	if (m_bDirectToHWND)
	{
		panel_x = 0;
		panel_y = 0;

		int x_off, y_off;
		vgui2::VPANEL panel = m_vcontext;

		while (!((vgui2::VPanel *)panel)->IsPopup())
		{
			((vgui2::VPanel *)panel)->GetPos(x_off, y_off);
			panel_x += x_off;
			panel_y += y_off;
			panel = (vgui2::VPANEL) (((vgui2::VPanel *)panel)->GetParent());
		}
	}
	else
		((vgui2::VPanel *)m_vcontext)->GetAbsPos(panel_x, panel_y);
#endif

	window_x = panel_x + 1;
	window_y = panel_y + 1;

	if (lasthDC != 0)
	{
		::DeleteObject(hBitmap);
		::DeleteDC(hdcMem);
	}

	SetBounds();

	lasthDC = 0;
}

bool HtmlWindow::Show(bool shown)
{
	if (m_webBrowser)
	{
		m_webBrowser->put_Visible(shown);
	}

	return true;
}

void HtmlWindow::SetVisible(bool state)
{
	if (m_webBrowser)
		m_webBrowser->put_Visible(state);

	m_bVisible = state;

	if (m_newpage || !state)
	{
		if (m_bDirectToHWND)
			::ShowWindow(m_oleObjectHWND, state ? SW_SHOWNA : SW_HIDE);
		else
			::ShowWindow(m_oleObjectHWND, SW_HIDE);
	}
}

bool HtmlWindow::StopLoading(void)
{
	HRESULT hret = m_webBrowser->Stop();

	if (SUCCEEDED(hret))
		return true;

	return false;
}

bool HtmlWindow::Refresh(void)
{
	DEBUG("onrefresh");

	OpenURL(m_currentUrl);
	return true;
}

RECT HtmlWindow::SetBounds(void)
{
	RECT posRect;

	if (m_bDirectToHWND)
	{
		posRect.right = w + window_x;
		posRect.bottom = h + window_y;
		posRect.left = window_x;
		posRect.top = window_y;
	}
	else
	{
		posRect.right = w;
		posRect.bottom = h;
		posRect.left = 0;
		posRect.top = 0;
	}

	if (m_oleInPlaceObject)
	{
		int hret = m_oleInPlaceObject->SetObjectRects(&posRect, &posRect);
		ASSERT(SUCCEEDED(hret));
	}

	if (m_webBrowser)
	{
		m_webBrowser->put_Height(h);
		m_webBrowser->put_Width(w);
		ScrollHTML(html_x,html_y);
	}

	if (!m_bDirectToHWND)
		::RedrawWindow(m_ieHWND, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);

	return posRect;
}

void HtmlWindow::OnPaint(HDC hDC)
{
	DEBUG("HtmlWindow::OnPaint repainting html win");

	if (w == 0 || h == 0)
		return;

	if (m_bDirectToHWND)
		return;

	if (!m_viewObject)
	{
		DEBUG("HtmlWindow::OnPaint Not Drawing!");
		return;
	}

	RECT posRect = SetBounds();
	HBITMAP oldbmp = NULL;

	if (lasthDC == 0 || lasthDC != hDC)
	{
		if (lasthDC != 0)
		{
			::DeleteObject(hBitmap);
			::DeleteDC(hdcMem);
		}

		lasthDC = hDC;

		hdcMem = CreateCompatibleDC(hDC);
		hBitmap = CreateCompatibleBitmap(hDC, w, h);
	}

	size_t datalen = sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD);
	LPBITMAPINFO lpbi = (LPBITMAPINFO)(new char[datalen]);

	if (!lpbi)
		return;

	Assert(hBitmap != NULL);

	oldbmp = (HBITMAP)SelectObject(hdcMem, hBitmap);

	Assert(oldbmp != NULL);

	m_webBrowser->put_Height(h);
	m_webBrowser->put_Width(w);

	ScrollHTML(html_x, html_y);

	RECTL *prc = (RECTL *)&posRect;
	m_viewObject->Draw(DVASPECT_CONTENT, -1, NULL, NULL, hDC, hdcMem, prc, NULL, NULL, 0);

	Q_memset(lpbi, 0x0, datalen);
	lpbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

	int chk = GetDIBits(hdcMem, hBitmap, 0L, (DWORD)h, (LPBYTE)NULL, (LPBITMAPINFO)lpbi, (DWORD)DIB_RGB_COLORS);

	if (chk != 0)
	{
		lpbi->bmiHeader.biBitCount = 32;
		lpbi->bmiHeader.biCompression = BI_BITFIELDS;
		lpbi->bmiHeader.biSizeImage = lpbi->bmiHeader.biWidth * lpbi->bmiHeader.biHeight * (lpbi->bmiHeader.biBitCount / 8);
		lpbi->bmiHeader.biHeight *= -1;

		byte *lpvBits = new byte[lpbi->bmiHeader.biSizeImage];

		if (lpvBits)
		{
			chk = GetDIBits(hdcMem, hBitmap, 0L, (DWORD)h, (LPBYTE)lpvBits, (LPBITMAPINFO)lpbi, (DWORD)DIB_RGB_COLORS);

			if (chk != 0)
			{
				byte *bmpBits = lpvBits;

				for (unsigned int i = 0; i < lpbi->bmiHeader.biSizeImage; i += 4)
					bmpBits[i + 3] = 255;

				if (!m_Bitmap)
					m_Bitmap = new vgui2::MemoryBitmap(bmpBits, w, h);
				else
					m_Bitmap->ForceUpload(bmpBits, w, h);
			}

			delete [] lpvBits;
		}
	}

	SelectObject(hdcMem, oldbmp);

	delete [] lpbi;
}

void HtmlWindow::OnMouse(vgui2::MouseCode code, MOUSE_STATE s, int x, int y)
 {
	DEBUG("mouse event");

	UINT msg = 0;
	WPARAM wParam = 1;
	LPARAM lParam = 0;

	lParam = y << 16;
	lParam |= x;
	m_iMousePos = lParam;

	if (code != vgui2::MOUSE_LEFT)
		return;

	if (s == IHTML::UP)
	{
		if (code == vgui2::MOUSE_LEFT)
			msg = WM_LBUTTONUP;
		else if (code == vgui2::MOUSE_RIGHT)
			msg = WM_RBUTTONUP;
	}
	else if (s == IHTML::DOWN)
	{
		if (code == vgui2::MOUSE_LEFT)
			msg = WM_LBUTTONDOWN;
		else if (code == vgui2::MOUSE_RIGHT)
			msg = WM_RBUTTONDOWN;
	}
	else if (s == IHTML::MOVE)
	{
		msg = WM_MOUSEMOVE;
	}
	else
		return;

	if (m_oleInPlaceObject == NULL)
	{
		DEBUG("no oleInPlaceObject");
		return;
	}

	if (m_ieHWND && ::PostMessage(m_ieHWND, msg, wParam, lParam) == 0)
	{
		DEBUG("msg2 not delivered");
		return;
	}

	if (m_ieHWND && ::PostMessage(m_ieHWND, WM_SETFOCUS, reinterpret_cast<uint>(m_oleObjectHWND), lParam) == 0)
	{
		DEBUG("msg2 not delivered");
		return;
	}

	EnumChildWindows(m_ieHWND, EnumChildProc, lParam);

	DEBUG("msg sent");
}

void HtmlWindow::Clear(void)
{
	IDispatch *pDisp;
	m_webBrowser->get_Document(&pDisp);

	if (pDisp != NULL)
	{
		IHTMLDocument2 *pHTMLDocument2;
		HRESULT hr = pDisp->QueryInterface(IID_IHTMLDocument2, (void **)&pHTMLDocument2);

		if (hr == S_OK)
		{
			pHTMLDocument2->close();
			pHTMLDocument2->Release();
		}

		pDisp->Release();
	}
}

void HtmlWindow::AddText(const char *text)
{
	IDispatch *pDisp;
	m_webBrowser->get_Document(&pDisp);

	IHTMLDocument2 *pHTMLDocument2;
	HRESULT hret = pDisp->QueryInterface(IID_IHTMLDocument2, (void **)&pHTMLDocument2);

	if (hret == S_OK)
	{
		wchar_t *tmp = new wchar_t[strlen(text) + 1];

		if (tmp)
		{
			IHTMLElement *pElement;
			::MultiByteToWideChar(CP_ACP, 0, text, strlen(text) + 1, tmp, strlen(text) + 1);
			SysAllocString(tmp);

			HRESULT hret = pHTMLDocument2->get_body(&pElement);

			if (hret == S_OK && pElement)
			{
				wchar_t temp[] = L"beforeEnd";
				BSTR where = temp;
				pElement->insertAdjacentHTML(where,tmp);
				pElement->Release();
			}
		}

		pHTMLDocument2->Release();
	}

	pDisp->Release();
}

void HtmlWindow::OnChar(wchar_t unichar)
{
	UINT msg = 0;
	WPARAM wParam = 1;
	LPARAM lParam = 0;

	wParam = unichar;

	msg = WM_CHAR;

	if (m_oleInPlaceObject == NULL)
	{
		DEBUG("no oleInPlaceObject");
		return;
	}

	if (m_ieHWND && ::PostMessage(m_ieHWND, msg, wParam, lParam) == 0)
	{
		DEBUG("msg not delivered");
		return;
	}

	DEBUG("msg sent");
}

void HtmlWindow::OnKeyDown(vgui2::KeyCode code)
{
	UINT msg = 0;
	WPARAM wParam = 1;
	LPARAM lParam = 0;

	wParam = KeyCode_VGUIToVirtualKey(code);

	msg = WM_KEYDOWN;

	if (m_oleInPlaceObject == NULL)
	{
		DEBUG("no oleInPlaceObject");
		return;
	}

	if (m_ieHWND && ::PostMessage(m_ieHWND, msg, wParam, lParam) == 0)
	{
		DEBUG("msg not delivered");
		return;
	}

	DEBUG("msg sent");
}

bool HtmlWindow::OnStartURL(const char *url, const char *target, bool first)
{
	DEBUG("loading url:");
	DEBUG(url);

	if (m_HtmlEventsAdviseCookie != 0)
		m_connectionPoint->Unadvise(m_HtmlEventsAdviseCookie);

	Q_strncpy(m_currentUrl, url, 512);

	if (m_specificallyOpened)
		m_specificallyOpened = false;

	m_newpage = false;

	if (m_events)
	{
		if (m_events->OnStartURL(url, target, first))
			return true;
		else
			return false;
	}

	return true;
}

void HtmlWindow::OnProgressURL(long current, long maximum)
{
	DEBUG("progress url");

	if (m_events)
		m_events->OnProgressURL(current,maximum);
}

void HtmlWindow::OnSetStatusText(const char *text)
{
	DEBUG("set status text");

	if (m_events)
		m_events->OnSetStatusText(text);
}

void HtmlWindow::OnUpdate(void)
{
	DEBUG("onupdate");

	IDispatch *pDisp;
	m_webBrowser->get_Document(&pDisp);

	if (pDisp)
	{
		IHTMLDocument2 *pHTMLDocument2;
		HRESULT hret = pDisp->QueryInterface(IID_IHTMLDocument2, (void **)&pHTMLDocument2);

		if (hret == S_OK)
		{
			FrameSite *c = reinterpret_cast<FrameSite *>(m_fs);
			IConnectionPointContainer *cpContainer;
			hret = pDisp->QueryInterface(IID_IConnectionPointContainer, (void **)(&cpContainer));
			assert(SUCCEEDED(hret));
			hret = cpContainer->FindConnectionPoint(DIID_HTMLDocumentEvents, &m_connectionPoint);
			assert(SUCCEEDED(hret));

			m_connectionPoint->Advise(c->m_DHTMLDocumentEvents2, &m_HtmlEventsAdviseCookie);
			cpContainer->Release();

			CalculateHTMLSize(pHTMLDocument2);
			pHTMLDocument2->Release();
		}

		pDisp->Release();
	}

	if (m_events)
		m_events->OnUpdate();
}

void HtmlWindow::OnLink(void)
{
	DEBUG("onlink");

	if (m_events)
		m_events->OnLink();
}

void HtmlWindow::OffLink(void)
{
	DEBUG("offlink");

	if (m_events)
		m_events->OffLink();
}

bool HtmlWindow::CheckIsLink(IHTMLElement *el, const char *type)
{
	BSTR bstr;
	bool IsLink = false;
	el->get_tagName(&bstr);
	_bstr_t p = _bstr_t(bstr);

	if (bstr)
	{
		const char *tag = static_cast<char *>(p);

		if (!Q_stricmp(tag,type))
			IsLink = true;

		SysFreeString(bstr);
	}

	return IsLink;
}

void HtmlWindow::CalculateHTMLSize(void *pVoid)
{
	IHTMLDocument2 *pHTMLDocument2 = (IHTMLDocument2 *)pVoid;
	IHTMLBodyElement *piBody = NULL;
	IHTMLTextContainer *piCont = NULL;
	IHTMLElement *piElem = NULL;
	pHTMLDocument2->get_body(&piElem);

	if (!piElem)
		return;

	piElem->QueryInterface(IID_IHTMLBodyElement,(void **)&piBody);

	if (!piBody)
		return;

	piBody->put_scroll(_bstr_t("no"));

	HRESULT hret = piBody->QueryInterface(IID_IHTMLTextContainer,(void **)&piCont);

	if (hret == S_OK && piCont)
	{
		piCont->get_scrollWidth(&html_w);
		piCont->get_scrollHeight(&html_h);

		piCont->Release();
	}

	piBody->Release();
	piElem->Release();
}

void HtmlWindow::ScrollHTML(int x,int y)
{
	IDispatch *pDisp;
	m_webBrowser->get_Document(&pDisp);
	bool bIECSSCompatMode = false;

	if (!pDisp)
		return;

	IHTMLDocument5 *pHTMLDocument5 = NULL;
	HRESULT hret = pDisp->QueryInterface(IID_IHTMLDocument5, (void **)&pHTMLDocument5);

	if (hret == S_OK && pHTMLDocument5)
	{
		BSTR bStr;
		pHTMLDocument5->get_compatMode(&bStr);

		if (!wcscmp(bStr, L"CSS1Compat"))
			bIECSSCompatMode = true;
	}

	if (bIECSSCompatMode)
	{
		IHTMLDocument3 *pHTMLDocument2;
		HRESULT hret = pDisp->QueryInterface(IID_IHTMLDocument3, (void **)&pHTMLDocument2);

		if (hret == S_OK && pHTMLDocument2)
		{
			IHTMLElement *pElement;
			hret = pHTMLDocument2->get_documentElement(&pElement);

			if (hret == S_OK && pElement)
			{
				IHTMLElement2 *piElem2 = NULL;
				hret = pElement->QueryInterface(IID_IHTMLElement2,(void **)&piElem2);

				if (hret == S_OK && piElem2)
				{
					piElem2->put_scrollLeft(x);
					piElem2->put_scrollTop(y);
					piElem2->Release();
				}

				pElement->Release();
			}

			pHTMLDocument2->Release();
		}

		pDisp->Release();
	}
	else
	{
		IHTMLDocument2 *pHTMLDocument2;
		HRESULT hret = pDisp->QueryInterface(IID_IHTMLDocument2, (void **)&pHTMLDocument2);

		if (hret == S_OK && pHTMLDocument2)
		{
			IHTMLElement *pElement = NULL;
			hret = pHTMLDocument2->get_body(&pElement);

			if (hret == S_OK && pElement)
			{
				IHTMLTextContainer *piCont = NULL;
				hret = pElement->QueryInterface(IID_IHTMLTextContainer,(void **)&piCont);

				if (hret == S_OK && piCont)
				{
					piCont->put_scrollLeft(x);
					piCont->put_scrollTop(y);
					piCont->Release();
				}

				pElement->Release();
			}

			pHTMLDocument2->Release();

		}

		pDisp->Release();
	}
}

bool HtmlWindow::OnMouseOver(void)
{
	bool IsLink = false;
	IDispatch *pDisp;
	m_webBrowser->get_Document(&pDisp);

	IHTMLDocument2 *pHTMLDocument2;
	HRESULT hret = pDisp->QueryInterface(IID_IHTMLDocument2, (void **)&pHTMLDocument2);

	if (hret == S_OK)
	{
		IHTMLWindow2 *pParentWindow;
		HRESULT hr = pHTMLDocument2->get_parentWindow(&pParentWindow);

		if (SUCCEEDED(hr))
		{
			IHTMLEventObj *pEvtObj;

			hr = pParentWindow->get_event(&pEvtObj);
			pParentWindow->Release();

			if (SUCCEEDED(hr) && pEvtObj)
			{
				IHTMLElement *el;
				pEvtObj->get_srcElement(&el);

				if (CheckIsLink(el, "A"))
				{
					IsLink = true;
				}
				else
				{
					IHTMLElement *pel = el, *oldpel = NULL;

					while (IsLink == false && SUCCEEDED(pel->get_parentElement(&pel)))
					{
						if (oldpel != NULL)
							oldpel->Release();

						if (pel == NULL || pel == oldpel)
							break;

						if (CheckIsLink(pel, "A"))
							IsLink = true;

						oldpel = pel;
					}

					if (pel != NULL)
						pel->Release();
				}

				el->Release();
			}
		}

		pHTMLDocument2->Release();
	}

	pDisp->Release();

	return IsLink;
}

void HtmlWindow::NewWindow(IDispatch **pIDispatch)
{
	m_webBrowser->get_Application(pIDispatch);
}

void HtmlWindow::OnFinishURL(const char *url)
{
	DEBUG("loaded url:");

	static bool hooked = false;

	m_currentUrl[0] = 0;

	if (url)
		Q_strncpy(m_currentUrl, url, 512);

	m_newpage = true;
	m_cleared = false;

	IDispatch *pDisp;
	m_webBrowser->get_Document(&pDisp);

	if (pDisp)
	{
		IHTMLDocument2 *pHTMLDocument2;
		HRESULT hret = pDisp->QueryInterface(IID_IHTMLDocument2, (void **)&pHTMLDocument2);

		if (hret == S_OK)
		{
			FrameSite *c = reinterpret_cast<FrameSite *>(m_fs);
			IConnectionPointContainer *cpContainer;
			hret = pDisp->QueryInterface(IID_IConnectionPointContainer, (void **)(&cpContainer));
			assert(SUCCEEDED(hret));
			hret = cpContainer->FindConnectionPoint(DIID_HTMLDocumentEvents, &m_connectionPoint);
			assert(SUCCEEDED(hret));

			m_connectionPoint->Advise(c->m_DHTMLDocumentEvents2, &m_HtmlEventsAdviseCookie);
			cpContainer->Release();

			CalculateHTMLSize(pHTMLDocument2);
			pHTMLDocument2->Release();
		}

		pDisp->Release();
	}

	if (m_events)
		m_events->OnFinishURL(url);

	HWND tst = m_oleObjectHWND;
	char name[100];
	memset(name, 0x0, 100);

	while (strcmp(name, "Internet Explorer_Server") && tst)
	{
		tst= ::GetWindow(tst, GW_CHILD);

		if (tst)
			::GetClassName(tst, name, 100 / sizeof(TCHAR));
	}

	if (tst)
		m_ieHWND = tst;

	if (!hooked)
	{
		DWORD tid = ::GetWindowThreadProcessId(m_oleObjectHWND, NULL);
		m_hHook = ::SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, NULL, tid);

		if (m_hHook == NULL)
		{
			DWORD err = ::GetLastError();
			err++;
		}

		hooked = true;
	}

	if (m_bVisible)
	{
		if (m_bDirectToHWND)
			::ShowWindow(m_oleObjectHWND, m_bVisible? SW_SHOWNA : SW_HIDE);
		else
			::ShowWindow(m_oleObjectHWND, SW_HIDE);
	}
}

void HtmlWindow::OpenURL(const char *url)
{
	if (!url || !*url)
		return;

	VARIANTARG navFlag, targetFrame, postData, headers,vUrl;
	navFlag.vt = VT_EMPTY;
	targetFrame.vt = VT_EMPTY;
	postData.vt = VT_EMPTY;
	headers.vt = VT_EMPTY;
	vUrl.vt = VT_BSTR;
	_bstr_t bstr = _T(url);
	vUrl.bstrVal = bstr;

	m_specificallyOpened = true;
	m_webBrowser->Navigate2(&vUrl, &navFlag, &targetFrame, &postData, &headers);
}

FrameSite::FrameSite(HtmlWindow *win, bool AllowJavaScript)
{
	m_cRef = 0;

	m_window = win;
	m_bSupportsWindowlessActivation = true;
	m_bInPlaceLocked = false;
	m_bUIActive = false;
	m_bInPlaceActive = false;
	m_bWindowless = false;

	m_nAmbientLocale = 0;
	m_clrAmbientForeColor = ::GetSysColor(COLOR_WINDOWTEXT);
	m_clrAmbientBackColor = ::GetSysColor(COLOR_WINDOW);
	m_bAmbientUserMode = true;
	m_bAmbientShowHatching = true;
	m_bAmbientShowGrabHandles = true;
	m_bAmbientAppearance = true;

	m_hDCBuffer = NULL;
	m_hWndParent = (HWND)m_window->GetHWND();

	m_IOleInPlaceFrame = new FS_IOleInPlaceFrame(this);
	m_IOleInPlaceSiteWindowless = new FS_IOleInPlaceSiteWindowless(this);
	m_IOleClientSite = new FS_IOleClientSite(this);
	m_IOleControlSite = new FS_IOleControlSite(this);
	m_IOleCommandTarget = new FS_IOleCommandTarget(this);
	m_IOleItemContainer = new FS_IOleItemContainer(this);
	m_IDispatch = new FS_IDispatch(this);
	m_IDispatch->SetAllowJavaScript(AllowJavaScript);
	m_DWebBrowserEvents2 = new FS_DWebBrowserEvents2(this);
	m_DHTMLDocumentEvents2 = new FS_DHTMLDocumentEvents2(this);
	m_IAdviseSink2 = new FS_IAdviseSink2(this);
	m_IAdviseSinkEx = new FS_IAdviseSinkEx(this);
	m_IDocHostUIHandler = new FS_IDocHostUIHandler(this);
}

FrameSite::~FrameSite(void)
{
	delete m_IAdviseSinkEx;
	delete m_IAdviseSink2;
	delete m_DWebBrowserEvents2;
	delete m_DHTMLDocumentEvents2;
	delete m_IDispatch;
	delete m_IOleItemContainer;
	delete m_IOleCommandTarget;
	delete m_IOleControlSite;
	delete m_IOleClientSite;
	delete m_IOleInPlaceSiteWindowless;
	delete m_IOleInPlaceFrame;
	delete m_IDocHostUIHandler;
}

STDMETHODIMP FrameSite::QueryInterface(REFIID riid, void **ppv)
{
	if (ppv == NULL)
		return E_INVALIDARG;

	*ppv = NULL;

	if (riid == IID_IUnknown)
		*ppv = this;
	else if (riid == IID_IOleWindow || riid == IID_IOleInPlaceUIWindow || riid == IID_IOleInPlaceFrame)
		*ppv = m_IOleInPlaceFrame;
	else if (riid == IID_IOleInPlaceSite || riid == IID_IOleInPlaceSiteEx || riid == IID_IOleInPlaceSiteWindowless)
		*ppv = m_IOleInPlaceSiteWindowless;
	else if (riid == IID_IOleClientSite)
		*ppv = m_IOleClientSite;
	else if (riid == IID_IOleControlSite)
		*ppv = m_IOleControlSite;
	else if (riid == IID_IOleCommandTarget)
		*ppv = m_IOleCommandTarget;
	else if (riid == IID_IOleItemContainer || riid == IID_IOleContainer || riid == IID_IParseDisplayName)
		*ppv = m_IOleItemContainer;
	else if (riid == IID_IDispatch)
		*ppv = m_IDispatch;
	else if (riid == DIID_DWebBrowserEvents2)
		*ppv = m_DWebBrowserEvents2;
	else if (riid == DIID_HTMLDocumentEvents)
		*ppv = m_DHTMLDocumentEvents2;
	else if (riid == IID_IAdviseSink || riid == IID_IAdviseSink2)
		*ppv = m_IAdviseSink2;
	else if (riid == IID_IAdviseSinkEx)
		*ppv = m_IAdviseSinkEx;
	else if (riid == IID_IDocHostUIHandler)
		*ppv = m_IDocHostUIHandler;

	if (*ppv == NULL)
		return (HRESULT)E_NOINTERFACE;

	AddRef();
	return S_OK;
}

STDMETHODIMP_(ULONG) FrameSite::AddRef(void)
{
	return ++m_cRef;
}

STDMETHODIMP_(ULONG) FrameSite::Release(void)
{
	if (--m_cRef == 0)
	{
		delete this;
		return 0;
	}

	return m_cRef;
}

HRESULT FS_IDispatch::GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, unsigned int cNames, LCID lcid, DISPID *rgDispId)
{
	DEBUG("IDispatch::GetIDsOfNames");
	return E_NOTIMPL;
}

HRESULT FS_IDispatch::GetTypeInfo(unsigned int iTInfo, LCID lcid, ITypeInfo **ppTInfo)
{
	DEBUG("IDispatch::GetTypeInfo");
	return E_NOTIMPL;
}

HRESULT FS_IDispatch::GetTypeInfoCount(unsigned int *pcTInfo)
{
	DEBUG("IDispatch::GetTypeInfoCount");
	return E_NOTIMPL;
}

HRESULT FS_IDispatch::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, unsigned int *puArgErr)
{
	DEBUG("IDispatch::Invoke");

	if (wFlags & DISPATCH_PROPERTYGET)
	{
		if (pVarResult == NULL)
			return E_INVALIDARG;

		switch (dispIdMember)
		{
			case DISPID_AMBIENT_APPEARANCE:
			{
				pVarResult->vt = VT_BOOL;
				pVarResult->boolVal = m_fs->m_bAmbientAppearance;
				break;
			}

			case DISPID_AMBIENT_FORECOLOR:
			{
				pVarResult->vt = VT_I4;
				pVarResult->lVal = (long) m_fs->m_clrAmbientForeColor;
				break;
			}

			case DISPID_AMBIENT_BACKCOLOR:
			{
				pVarResult->vt = VT_I4;
				pVarResult->lVal = (long) m_fs->m_clrAmbientBackColor;
				break;
			}

			case DISPID_AMBIENT_LOCALEID:
			{
				pVarResult->vt = VT_I4;
				pVarResult->lVal = (long) m_fs->m_nAmbientLocale;
				break;
			}

			case DISPID_AMBIENT_USERMODE:
			{
				pVarResult->vt = VT_BOOL;
				pVarResult->boolVal = m_fs->m_bAmbientUserMode;
				break;
			}

			case DISPID_AMBIENT_SHOWGRABHANDLES:
			{
				pVarResult->vt = VT_BOOL;
				pVarResult->boolVal = m_fs->m_bAmbientShowGrabHandles;
				break;
			}

			case DISPID_AMBIENT_SHOWHATCHING:
			{
				pVarResult->vt = VT_BOOL;
				pVarResult->boolVal = m_fs->m_bAmbientShowHatching;
				break;
			}

			case DISPID_AMBIENT_DLCONTROL:
			{
				pVarResult->vt = VT_I4;
				pVarResult->lVal = (long)(DLCTL_DLIMAGES | DLCTL_VIDEOS | DLCTL_BGSOUNDS);

				if (!m_bAllowJavaScript)
					pVarResult->lVal |= (long)(DLCTL_NO_SCRIPTS);

				break;
			}

			default: return DISP_E_MEMBERNOTFOUND;
		}

		return S_OK;
	}

	switch (dispIdMember)
	{
		case DISPID_BEFORENAVIGATE2:
		{
			VARIANT *vurl = pDispParams->rgvarg[5].pvarVal;
			VARIANT *targetFrame = pDispParams->rgvarg[3].pvarVal;

			if (m_fs->m_window->OnStartURL((char *)_bstr_t(vurl->bstrVal), (char *)_bstr_t(targetFrame->bstrVal), !m_bNewURL))
				*pDispParams->rgvarg->pboolVal = VARIANT_FALSE;
			else
				*pDispParams->rgvarg->pboolVal = VARIANT_TRUE;

			break;
		}

		case DISPID_PROGRESSCHANGE:
		{
			long current = pDispParams->rgvarg[1].lVal;
			long maximum = pDispParams->rgvarg[0].lVal;
			m_fs->m_window->OnProgressURL(current, maximum);
			break;
		}

		case DISPID_DOWNLOADCOMPLETE:
		{
			if (m_bNewURL > 0)
				m_bNewURL--;

			break;
		}

		case DISPID_DOCUMENTCOMPLETE:
		{
			if (m_bNewURL == 0)
			{
				VARIANT *vurl = pDispParams->rgvarg[0].pvarVal;
				m_fs->m_window->OnFinishURL((char *)_bstr_t(vurl->bstrVal));
			}
			else
				m_fs->m_window->OnUpdate();
		}

		case DISPID_NAVIGATECOMPLETE2:
		{
			m_fs->m_window->OnUpdate();
			break;
		}

		case DISPID_DOWNLOADBEGIN:
		{
			m_bNewURL++;

			if (pDispParams->cArgs >= 5 && pDispParams->rgvarg[5].vt == (VT_BYREF | VT_VARIANT))
			{
				char *varURL = pDispParams->rgvarg[5].pcVal;
				Msg("Begining download: %s\n", varURL);
			}
			else
				Msg("Begining download\n");

			break;
		}

		case DISPID_NEWWINDOW2:
		{
			if (!vgui2::surface()->SupportsFeature(vgui2::ISurface::OPENING_NEW_HTML_WINDOWS))
			{
				m_bNewURL++;

				IDispatch *pIDispatch;
				m_fs->m_window->NewWindow(&pIDispatch);
				*(pDispParams->rgvarg[1].ppdispVal) = pIDispatch;
				break;
			}
		}

		case DISPID_HTMLDOCUMENTEVENTS_ONMOUSEOVER:
		{
			if (m_fs->m_window->OnMouseOver())
			{
				m_bOnLink = true;
				m_fs->m_window->OnLink();
			}
			else if (m_bOnLink)
			{
				m_bOnLink = false;
				m_fs->m_window->OffLink();
			}

			break;
		}
	}

	return S_OK;
}

HRESULT FS_IOleInPlaceFrame::GetWindow(HWND *phwnd)
{
	DEBUG("IOleWindow::GetWindow");

	if (phwnd == NULL)
		return E_INVALIDARG;

	(*phwnd) = m_fs->m_hWndParent;
	return S_OK;
}

HRESULT FS_IOleInPlaceFrame::ContextSensitiveHelp(BOOL fEnterMode)
{
	DEBUG("IOleWindow::ContextSensitiveHelp");
	return S_OK;
}

HRESULT FS_IOleInPlaceFrame::GetBorder(LPRECT lprectBorder)
{
	DEBUG("IOleInPlaceUIWindow::GetBorder");

	if (lprectBorder == NULL)
		return E_INVALIDARG;

	return INPLACE_E_NOTOOLSPACE;
}

HRESULT FS_IOleInPlaceFrame::RequestBorderSpace(LPCBORDERWIDTHS pborderwidths)
{
	DEBUG("IOleInPlaceUIWindow::RequestBorderSpace");

	if (pborderwidths == NULL)
		return E_INVALIDARG;

	return INPLACE_E_NOTOOLSPACE;
}

HRESULT FS_IOleInPlaceFrame::SetBorderSpace(LPCBORDERWIDTHS pborderwidths)
{
	DEBUG("IOleInPlaceUIWindow::SetBorderSpace");
	return S_OK;
}

HRESULT FS_IOleInPlaceFrame::SetActiveObject(IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName)
{
	DEBUG("IOleInPlaceUIWindow::SetActiveObject");
	return S_OK;
}

HRESULT FS_IOleInPlaceFrame::InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
	DEBUG("IOleInPlaceFrame::InsertMenus");
	return S_OK;
}

HRESULT FS_IOleInPlaceFrame::SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject)
{
	DEBUG("IOleInPlaceFrame::SetMenu");
	return S_OK;
}

HRESULT FS_IOleInPlaceFrame::RemoveMenus(HMENU hmenuShared)
{
	DEBUG("IOleInPlaceFrame::RemoveMenus");
	return S_OK;
}

HRESULT FS_IOleInPlaceFrame::SetStatusText(LPCOLESTR pszStatusText)
{
	DEBUG("IOleInPlaceFrame::SetStatusText");

	if (m_fs->m_window->GetEvents())
	{
		char tmp[512];
		WideCharToMultiByte(CP_ACP, 0, pszStatusText, -1, tmp,512, NULL, NULL);
		m_fs->m_window->GetEvents()->OnSetStatusText(tmp);
	}

	return S_OK;
}

HRESULT FS_IOleInPlaceFrame::EnableModeless(BOOL fEnable)
{
	DEBUG("IOleInPlaceFrame::EnableModeless");
	return S_OK;
}

HRESULT FS_IOleInPlaceFrame::TranslateAccelerator(LPMSG lpmsg, WORD wID)
{
	DEBUG("IOleInPlaceFrame::TranslateAccelerator");
	return E_NOTIMPL;
}

HRESULT FS_IOleInPlaceSiteWindowless::CanInPlaceActivate(void)
{
	DEBUG("IOleInPlaceSite::CanInPlaceActivate");
	return S_OK;
}

HRESULT FS_IOleInPlaceSiteWindowless::OnInPlaceActivate(void)
{
	DEBUG("**************IOleInPlaceSite::OnInPlaceActivate");
	m_fs->m_bInPlaceActive = true;
	return S_OK;
}

HRESULT FS_IOleInPlaceSiteWindowless::OnUIActivate(void)
{
	DEBUG("*****IOleInPlaceSite::OnUIActivate");
	m_fs->m_bUIActive = true;
	return S_OK;
}

HRESULT FS_IOleInPlaceSiteWindowless::GetWindowContext(IOleInPlaceFrame **ppFrame, IOleInPlaceUIWindow **ppDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
	DEBUG("IOleInPlaceSite::GetWindowContext");

	if (ppFrame == NULL || ppDoc == NULL || lprcPosRect == NULL || lprcClipRect == NULL || lpFrameInfo == NULL)
	{
		if (ppFrame != NULL)
			(*ppFrame) = NULL;

		if (ppDoc != NULL)
			(*ppDoc) = NULL;

		return E_INVALIDARG;
	}

	(*ppDoc) = (*ppFrame) = m_fs->m_IOleInPlaceFrame;
	(*ppDoc)->AddRef();
	(*ppFrame)->AddRef();

	int w, h;
	m_fs->m_window->GetSize(w, h);

	lprcPosRect->left =0;
	lprcPosRect->top = 0;
	lprcPosRect->right = w;
	lprcPosRect->bottom = h;
	lprcClipRect->left = 0;
	lprcClipRect->top = 0;
	lprcClipRect->right = w;
	lprcClipRect->bottom = h;

	lpFrameInfo->fMDIApp = FALSE;
	lpFrameInfo->hwndFrame = m_fs->m_hWndParent;
	lpFrameInfo->haccel = NULL;
	lpFrameInfo->cAccelEntries = 0;

	return S_OK;
}

HRESULT FS_IOleInPlaceSiteWindowless::Scroll(SIZE scrollExtent)
{
	DEBUG("IOleInPlaceSite::Scroll");
	return S_OK;
}

HRESULT FS_IOleInPlaceSiteWindowless::OnUIDeactivate(BOOL fUndoable)
{
	DEBUG("IOleInPlaceSite::OnUIDeactivate");
	m_fs->m_bUIActive = false;
	return S_OK;
}

HRESULT FS_IOleInPlaceSiteWindowless::OnInPlaceDeactivate(void)
{
	DEBUG("IOleInPlaceSite::OnInPlaceDeactivate");
	m_fs->m_bInPlaceActive = false;
	return S_OK;
}

HRESULT FS_IOleInPlaceSiteWindowless::DiscardUndoState(void)
{
	DEBUG("IOleInPlaceSite::DiscardUndoState");
	return S_OK;
}

HRESULT FS_IOleInPlaceSiteWindowless::DeactivateAndUndo(void)
{
	DEBUG("IOleInPlaceSite::DeactivateAndUndo");
	return S_OK;
}

HRESULT FS_IOleInPlaceSiteWindowless::OnPosRectChange(LPCRECT lprcPosRect)
{
	DEBUG("IOleInPlaceSite::OnPosRectChange");
	return S_OK;
}

HRESULT FS_IOleInPlaceSiteWindowless::OnInPlaceActivateEx(BOOL *pfNoRedraw, DWORD dwFlags)
{
	DEBUG("IOleInPlaceSiteEx::OnInPlaceActivateEx");

	if (pfNoRedraw)
		(*pfNoRedraw) = FALSE;

	return S_OK;
}

HRESULT FS_IOleInPlaceSiteWindowless::OnInPlaceDeactivateEx(BOOL fNoRedraw)
{
	DEBUG("************IOleInPlaceSiteEx::OnInPlaceDeactivateEx");
	return S_OK;
}

HRESULT FS_IOleInPlaceSiteWindowless::RequestUIActivate(void)
{
	DEBUG("************IOleInPlaceSiteEx::RequestUIActivate");
	return S_OK;
}

HRESULT FS_IOleInPlaceSiteWindowless::CanWindowlessActivate(void)
{
	DEBUG("************IOleInPlaceSiteWindowless::CanWindowlessActivate");
	return (m_fs->m_bSupportsWindowlessActivation) ? S_OK : S_FALSE;
}

HRESULT FS_IOleInPlaceSiteWindowless::GetCapture(void)
{
	DEBUG("************IOleInPlaceSiteWindowless::GetCapture");
	return S_FALSE;
}

HRESULT FS_IOleInPlaceSiteWindowless::SetCapture(BOOL fCapture)
{
	DEBUG("************IOleInPlaceSiteWindowless::SetCapture");
	return S_FALSE;
}

HRESULT FS_IOleInPlaceSiteWindowless::GetFocus(void)
{
	DEBUG("************IOleInPlaceSiteWindowless::GetFocus");
	return S_OK;
}

HRESULT FS_IOleInPlaceSiteWindowless::SetFocus(BOOL fFocus)
{
	DEBUG("************IOleInPlaceSiteWindowless::SetFocus");
	return S_OK;
}

HRESULT FS_IOleInPlaceSiteWindowless::GetDC(LPCRECT pRect, DWORD grfFlags, HDC *phDC)
{
	DEBUG("************IOleInPlaceSiteWindowless::GetDC");

	if (phDC == NULL)
		return E_INVALIDARG;

	if (grfFlags & OLEDC_NODRAW)
	{
		(*phDC) = m_fs->m_hDCBuffer;
		return S_OK;
	}

	if (m_fs->m_hDCBuffer != NULL)
		return E_UNEXPECTED;

	return E_NOTIMPL;
}

HRESULT FS_IOleInPlaceSiteWindowless::ReleaseDC(HDC hDC)
{
	DEBUG("************IOleInPlaceSiteWindowless::ReleaseDC");
	return E_NOTIMPL;
}

HRESULT FS_IOleInPlaceSiteWindowless::InvalidateRect(LPCRECT pRect, BOOL fErase)
{
	DEBUG("************IOleInPlaceSiteWindowless::InvalidateRect");

	RECT rcI = { 0, 0, 0, 0 };
	RECT posRect=m_fs->m_window->SetBounds();

	if (pRect == NULL)
		rcI = posRect;
	else
		IntersectRect(&rcI, &posRect, pRect);

	::InvalidateRect(m_fs->m_hWndParent, &rcI, fErase);
	return S_OK;
}

HRESULT FS_IOleInPlaceSiteWindowless::InvalidateRgn(HRGN, BOOL)
{
	DEBUG("************IOleInPlaceSiteWindowless::InvalidateRgn");
	return E_NOTIMPL;
}

HRESULT FS_IOleInPlaceSiteWindowless::ScrollRect(INT, INT, LPCRECT, LPCRECT)
{
	DEBUG("************IOleInPlaceSiteWindowless::ScrollRect");
	return E_NOTIMPL;
}

HRESULT FS_IOleInPlaceSiteWindowless::AdjustRect(LPRECT)
{
	DEBUG("************IOleInPlaceSiteWindowless::AdjustRect");
	return E_NOTIMPL;
}

HRESULT FS_IOleInPlaceSiteWindowless::OnDefWindowMessage(UINT, WPARAM, LPARAM, LRESULT *)
{
	DEBUG("************IOleInPlaceSiteWindowless::OnDefWindowMessage");
	return E_NOTIMPL;
}

HRESULT FS_IOleClientSite::SaveObject(void)
{
	DEBUG("IOleClientSite::SaveObject");
	return S_OK;
}

HRESULT FS_IOleClientSite::GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker **ppmk)
{
	DEBUG("IOleClientSite::GetMoniker");
	return E_NOTIMPL;
}

HRESULT FS_IOleClientSite::GetContainer(LPOLECONTAINER *ppContainer)
{
	DEBUG("IOleClientSite::GetContainer");

	if (ppContainer == NULL)
		return E_INVALIDARG;

	this->QueryInterface(IID_IOleContainer, (void **)(ppContainer));
	return S_OK;
}

HRESULT FS_IOleClientSite::ShowObject(void)
{
	DEBUG("IOleClientSite::ShowObject");
	return S_OK;
}

HRESULT FS_IOleClientSite::OnShowWindow(BOOL fShow)
{
	DEBUG("IOleClientSite::OnShowWindow");
	return S_OK;
}

HRESULT FS_IOleClientSite::RequestNewObjectLayout(void)
{
	DEBUG("IOleClientSite::RequestNewObjectLayout");
	return E_NOTIMPL;
}

HRESULT FS_IOleItemContainer::ParseDisplayName(IBindCtx *pbc, LPOLESTR pszDisplayName, ULONG *pchEaten, IMoniker **ppmkOut)
{
	DEBUG("IParseDisplayName::ParseDisplayName");
	return E_NOTIMPL;
}

HRESULT FS_IOleItemContainer::EnumObjects(DWORD grfFlags, IEnumUnknown **ppenum)
{
	DEBUG("IOleContainer::EnumObjects");
	return E_NOTIMPL;
}

HRESULT FS_IOleItemContainer::LockContainer(BOOL fLock)
{
	DEBUG("IOleContainer::LockContainer");
	return S_OK;
}

HRESULT FS_IOleItemContainer::GetObject(LPOLESTR pszItem, DWORD dwSpeedNeeded, IBindCtx *pbc, REFIID riid, void **ppvObject)
{
	DEBUG("IOleItemContainer::GetObject");

	if (pszItem == NULL)
		return E_INVALIDARG;

	if (ppvObject == NULL)
		return E_INVALIDARG;

	*ppvObject = NULL;
	return MK_E_NOOBJECT;
}

HRESULT FS_IOleItemContainer::GetObjectStorage(LPOLESTR pszItem, IBindCtx *pbc, REFIID riid, void **ppvStorage)
{
	DEBUG("IOleItemContainer::GetObjectStorage");

	if (pszItem == NULL)
		return E_INVALIDARG;

	if (ppvStorage == NULL)
		return E_INVALIDARG;

	*ppvStorage = NULL;
	return MK_E_NOOBJECT;
}

HRESULT FS_IOleItemContainer::IsRunning(LPOLESTR pszItem)
{
	DEBUG("IOleItemContainer::IsRunning");

	if (pszItem == NULL)
		return E_INVALIDARG;

	return MK_E_NOOBJECT;
}

HRESULT FS_IOleControlSite::OnControlInfoChanged(void)
{
	DEBUG("IOleControlSite::OnControlInfoChanged");
	return S_OK;
}

HRESULT FS_IOleControlSite::LockInPlaceActive(BOOL fLock)
{
	DEBUG("IOleControlSite::LockInPlaceActive");
	m_fs->m_bInPlaceLocked = (fLock) ? true : false;
	return S_OK;
}

HRESULT FS_IOleControlSite::GetExtendedControl(IDispatch **ppDisp)
{
	DEBUG("IOleControlSite::GetExtendedControl");
	return E_NOTIMPL;
}

HRESULT FS_IOleControlSite::TransformCoords(POINTL *pPtlHimetric, POINTF *pPtfContainer, DWORD dwFlags)
{
	DEBUG("IOleControlSite::TransformCoords");
	HRESULT hr = S_OK;

	if (pPtlHimetric == NULL)
		return E_INVALIDARG;

	if (pPtfContainer == NULL)
		return E_INVALIDARG;

	HDC hdc = ::GetDC(m_fs->m_hWndParent);
	::SetMapMode(hdc, MM_HIMETRIC);

	POINT rgptConvert[2];
	rgptConvert[0].x = 0;
	rgptConvert[0].y = 0;

	if (dwFlags & XFORMCOORDS_HIMETRICTOCONTAINER)
	{
		rgptConvert[1].x = pPtlHimetric->x;
		rgptConvert[1].y = pPtlHimetric->y;
		::LPtoDP(hdc, rgptConvert, 2);

		if (dwFlags & XFORMCOORDS_SIZE)
		{
			pPtfContainer->x = (float)(rgptConvert[1].x - rgptConvert[0].x);
			pPtfContainer->y = (float)(rgptConvert[0].y - rgptConvert[1].y);
		}
		else if (dwFlags & XFORMCOORDS_POSITION)
		{
			pPtfContainer->x = (float)rgptConvert[1].x;
			pPtfContainer->y = (float)rgptConvert[1].y;
		}
		else
			hr = E_INVALIDARG;
	}
	else if (dwFlags & XFORMCOORDS_CONTAINERTOHIMETRIC)
	{
		rgptConvert[1].x = (int)(pPtfContainer->x);
		rgptConvert[1].y = (int)(pPtfContainer->y);
		::DPtoLP(hdc, rgptConvert, 2);

		if (dwFlags & XFORMCOORDS_SIZE)
		{
			pPtlHimetric->x = rgptConvert[1].x - rgptConvert[0].x;
			pPtlHimetric->y = rgptConvert[0].y - rgptConvert[1].y;
		}
		else if (dwFlags & XFORMCOORDS_POSITION)
		{
			pPtlHimetric->x = rgptConvert[1].x;
			pPtlHimetric->y = rgptConvert[1].y;
		}
		else
			hr = E_INVALIDARG;
	}
	else
		hr = E_INVALIDARG;

	::ReleaseDC(m_fs->m_hWndParent, hdc);
	return hr;
}

HRESULT FS_IOleControlSite::TranslateAccelerator(LPMSG pMsg, DWORD grfModifiers)
{
	DEBUG("IOleControlSite::TranslateAccelerator");
	return E_NOTIMPL;
}

HRESULT FS_IOleControlSite::ShowContextMenu(DWORD dwID,POINT *ppt, IUnknown *pcmdtReserved, IDispatch **pdispReserved)
{
	DEBUG("IOleControlSite::ShowContextMenu");
	return S_OK;
}

HRESULT FS_IOleControlSite::OnFocus(BOOL fGotFocus)
{
	DEBUG("IOleControlSite::OnFocus");
	return S_OK;
}

HRESULT FS_IOleControlSite::ShowPropertyFrame(void)
{
	DEBUG("IOleControlSite::ShowPropertyFrame");
	return E_NOTIMPL;
}

HRESULT FS_IOleCommandTarget::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD *prgCmds, OLECMDTEXT *pCmdTet)
{
	DEBUG("IOleCommandTarget::QueryStatus");

	if (prgCmds == NULL)
		return E_INVALIDARG;

	bool bCmdGroupFound = false;

	for (ULONG nCmd = 0; nCmd < cCmds; nCmd++)
		prgCmds[nCmd].cmdf = 0;

	if (!bCmdGroupFound)
		return OLECMDERR_E_UNKNOWNGROUP;

	return S_OK;
}

HRESULT FS_IOleCommandTarget::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pVaIn, VARIANTARG *pVaOut)
{
	DEBUG("IOleCommandTarget::Exec");
	bool bCmdGroupFound = false;

	if (!bCmdGroupFound)
		return OLECMDERR_E_UNKNOWNGROUP;

	return OLECMDERR_E_NOTSUPPORTED;
}

void STDMETHODCALLTYPE FS_IAdviseSink2::OnDataChange(FORMATETC *pFormatEtc, STGMEDIUM *pgStgMed)
{
	DEBUG("IAdviseSink::OnDataChange");
}

void STDMETHODCALLTYPE FS_IAdviseSink2::OnViewChange(DWORD dwAspect, LONG lIndex)
{
	DEBUG("IAdviseSink::OnViewChange");

	m_fs->m_IOleInPlaceSiteWindowless->InvalidateRect(NULL, FALSE);
}

void STDMETHODCALLTYPE FS_IAdviseSink2::OnRename(IMoniker *pmk)
{
	DEBUG("IAdviseSink::OnRename");
}

void STDMETHODCALLTYPE FS_IAdviseSink2::OnSave(void)
{
	DEBUG("IAdviseSink::OnSave");
}

void STDMETHODCALLTYPE FS_IAdviseSink2::OnClose(void)
{
	DEBUG("IAdviseSink::OnClose");
}

void STDMETHODCALLTYPE FS_IAdviseSink2::OnLinkSrcChange(IMoniker *pmk)
{
	DEBUG("IAdviseSink2::OnLinkSrcChange");
}

void STDMETHODCALLTYPE FS_IAdviseSinkEx::OnViewStatusChange(DWORD dwViewStatus)
{
	DEBUG("IAdviseSinkEx::OnViewStatusChange");
}