#ifndef _HTMLWINDOW_H_
#define _HTMLWINDOW_H_
#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI2.h>
#include <vgui/IHTML.h>
#include <vgui/IImage.h>

#include "VPanel.h"

#include <exdispid.h>
#include <olectl.h>
#include <exdisp.h>

#include "Memorybitmap.h"

struct IHTMLElement;

class HtmlWindow : public vgui2::IHTML, vgui2::IHTMLEvents
{
public:
	HtmlWindow(vgui2::IHTMLEvents *events, vgui2::VPANEL c, HWND parent, bool AllowJavaScript, bool DirectToHWND);
	virtual ~HtmlWindow(void);

public:
	virtual void OpenURL(const char *);
	virtual bool StopLoading(void);
	virtual bool Refresh(void);
	virtual void SetVisible(bool state);
	virtual bool Show(bool state);

public:
	void CreateBrowser(bool AllowJavaScript);
	HWND GetHWND(void) { return m_parent; }
	HWND GetIEHWND(void) { return m_ieHWND; }
	HWND GetObjectHWND(void) { return m_oleObjectHWND; }
	vgui2::IHTMLEvents *GetEvents(void) { return m_events; }

public:
	virtual void OnPaint(HDC hDC);
	virtual vgui2::IImage *GetBitmap(void) { return m_Bitmap; }
	virtual void OnSize(int x, int y, int w, int h);
	virtual void ScrollHTML(int x,int y);
	virtual void OnMouse(vgui2::MouseCode code, MOUSE_STATE s, int x, int y);
	virtual void OnChar(wchar_t unichar);
	virtual void OnKeyDown(vgui2::KeyCode code);
	virtual void AddText(const char *text);
	virtual void Clear(void);
	virtual bool OnMouseOver(void);
	void GetSize(int &wide, int &tall) { wide = w; tall = h; }
	virtual void GetHTMLSize(int &wide, int &tall) { wide = html_w; tall = html_h; }
	virtual bool OnStartURL(const char *url, const char *target, bool first);
	virtual void OnFinishURL(const char *url);
	virtual void OnProgressURL(long current, long maximum);
	virtual void OnSetStatusText(const char *text);
	virtual void OnUpdate(void);
	virtual void OnLink(void);
	virtual void OffLink(void);

public:
	char *GetOpenedPage(void) { return m_currentUrl; }
	RECT SetBounds(void);
	void NewWindow(IDispatch **pIDispatch);
	void CalculateHTMLSize(void *pVoid);
	HHOOK GetHook(void) { return m_hHook; }
	void *GetIEWndProc(void) { return (void *)m_hIEWndProc; }
	UINT GetMousePos(void) { return m_iMousePos;}
	UINT GetMouseMessage(void) { return mouse_msg;}
	bool IsVisible(void) { return m_bVisible; }

public:
	int textureID;

private:
	bool CheckIsLink(IHTMLElement *el, const char *type);

private:
	char m_currentUrl[512];
	bool m_specificallyOpened;
	long w, h;
	long window_x, window_y;
	long html_w, html_h;
	long html_x, html_y;
	IConnectionPoint *m_connectionPoint;
	IWebBrowser2 *m_webBrowser;
	IOleObject *m_oleObject;
	IOleInPlaceObject *m_oleInPlaceObject;
	IViewObject *m_viewObject;
	HWND m_parent;
	HWND m_oleObjectHWND;
	HWND m_ieHWND;
	vgui2::VPANEL m_vcontext;
	vgui2::IHTMLEvents *m_events;
	HDC hdcMem;
	HDC lasthDC;
	HBITMAP hBitmap;
	vgui2::MemoryBitmap *m_Bitmap;
	bool m_cleared, m_newpage;
	bool m_bDirectToHWND;
	bool m_bParentCreated;
	void *m_fs;
	DWORD m_adviseCookie;
	DWORD m_HtmlEventsAdviseCookie;
	HHOOK m_hHook;
	bool m_bHooked;
	LPARAM m_iMousePos;
	UINT mouse_msg;
	bool m_bVisible;
	long m_hIEWndProc;
	HDC m_hMemDC;
	HBITMAP m_hBitmap;
	byte *m_pDibData;
	bool m_bSetClientSiteFirst;
	bool m_bVisibleAtRuntime;
};

#endif