#include <vector>
#include <string.h>

#include "vgui_api.h"
#include "cdll_int.h"

#include "BaseUISurface.h"
#include "vgui/IPanel.h"
#include "vgui/IInput.h"
#include "vgui/IInputInternal.h"
#include "vgui_controls/controls.h"

#include "filesystem.h"
#include "LoadBMP.h"
#include "LoadTGA.h"

extern vguiapi_t *g_api;
extern cl_enginefunc_t gEngfuncs;

bool BaseUISurface::m_bTranslateExtendedKeys;

static wrect_t g_ScissorRect;
qboolean g_bScissor;
static int g_iCurrentTexture;

// TODO: implement vgui2::Dar
std::vector<vgui2::VPANEL> staticPopupList;

BaseUISurface::BaseUISurface() {
	_restrictedPanel = 0;
	m_hCurrentFont = 0;
	_cursorLocked = 0;
	m_iCurrentTexture = 0;
	m_pChromeController = NULL;
	_needMouse = true;
	_needKB = true;
	m_bVGUI2MouseControl = false;
	m_bFullScreen = false;
	m_deltay = 0;
	m_deltax = 0;
	m_bTranslateExtendedKeys = true;
	_drawColor[0] = _drawColor[1] = _drawColor[2] = _drawColor[3] = 255;
	_drawTextColor[0] = _drawTextColor[1] = _drawTextColor[2] = _drawTextColor[3] = 255;
	m_iSurfaceBounds[0] = m_iSurfaceBounds[1] = m_iSurfaceBounds[2] = m_iSurfaceBounds[3] = 0;
	_drawTextPos[0] = _drawTextPos[1] = 0;
	_translateX = _translateY = 0;
}

BaseUISurface::~BaseUISurface() {
	m_bAllowJavaScript = false;
}

void BaseUISurface::Init(vgui2::VPANEL embeddedPanel, IHTMLChromeController *pChromeController) {
	SCREENINFO screeninfo;
	screeninfo.iSize = sizeof(screeninfo);
	gEngfuncs.pfnGetScreenInfo(&screeninfo);
	SetScreenInfo(&screeninfo);
	SetScreenBounds(0, 0, screeninfo.iWidth, screeninfo.iHeight);

	SetEmbeddedPanel(embeddedPanel);
	m_pChromeController = pChromeController;
	
	if (pChromeController) {
		m_pChromeController->Init("htmlcache", "htmlcookies");
		m_pChromeController->SetCefThreadTargetFrameRate(60);
	}
}

void BaseUISurface::Shutdown() {
	if (m_pChromeController) {
		m_pChromeController->Shutdown();
	}
}

void BaseUISurface::RunFrame() {
	if (m_pChromeController) {
		m_pChromeController->RunFrame();
	}
}

vgui2::VPANEL BaseUISurface::GetEmbeddedPanel() {
	return _embeddedPanel;
}

void BaseUISurface::SetEmbeddedPanel(vgui2::VPANEL panel) {
	_embeddedPanel = panel;
}

void SetScissorRect(int left, int top, int right, int bottom) {
	g_ScissorRect.left = left;
	g_ScissorRect.top = top;
	g_ScissorRect.right = right;
	g_ScissorRect.bottom = bottom;
}

void BaseUISurface::SetupPaintState(const paintState_t &paintState) {
	_translateX = paintState.iTranslateX;
	_translateY = paintState.iTranslateY;
	SetScissorRect(paintState.iScissorLeft, paintState.iScissorTop, paintState.iScissorRight, paintState.iScissorBottom);
}

void BaseUISurface::PushMakeCurrent(vgui2::VPANEL panel, bool useInsets) {
	int inSets[4] = { 0, 0, 0, 0 };
	int absExtents[4];
	int clipRect[4];
	int wide, tall;

	if (useInsets) {
		vgui2::ipanel()->GetInset(panel, inSets[0], inSets[1], inSets[2], inSets[3]);
	}

	vgui2::ipanel()->GetAbsPos(panel, absExtents[0], absExtents[1]);
	vgui2::ipanel()->GetSize(panel, wide, tall);
	absExtents[2] = absExtents[0] + wide;
	absExtents[3] = absExtents[1] + tall;
	vgui2::ipanel()->GetClipRect(panel, clipRect[0], clipRect[1], clipRect[2], clipRect[3]);

	_paintStack.emplace_back();
	paintState_t &paintState = _paintStack.back();
	paintState.vPanel = panel;

	paintState.iTranslateX = inSets[0] + absExtents[0] - m_iSurfaceBounds[0];
	paintState.iTranslateY = inSets[1] + absExtents[1] - m_iSurfaceBounds[1];

	paintState.iScissorLeft = clipRect[0] - m_iSurfaceBounds[0];
	paintState.iScissorTop = clipRect[1] - m_iSurfaceBounds[1];
	paintState.iScissorRight = clipRect[2] - m_iSurfaceBounds[0];
	paintState.iScissorBottom = clipRect[3] - m_iSurfaceBounds[1];

	SetupPaintState(paintState);
}

void BaseUISurface::PopMakeCurrent(vgui2::VPANEL panel) {
	if (!_paintStack.size()) {
		return;
	}

	_paintStack.pop_back();

	if (_paintStack.size()) {
		SetupPaintState(_paintStack.back());
	}
}

void BaseUISurface::DrawSetColor(int r, int g, int b, int a) {
	_drawColor[0] = r;
	_drawColor[1] = g;
	_drawColor[2] = b;
	_drawColor[3] = 255 - a;
}

void BaseUISurface::DrawSetColor(Color col) {
	col.GetColor(_drawColor[0], _drawColor[1], _drawColor[2], _drawColor[3]);
	_drawColor[3] = 255 - _drawColor[3];
}

void BaseUISurface::InitVertex(vpoint_t &vertex, int x, int y, float u, float v) {
	vertex.point[0] = x + _translateX;
	vertex.point[1] = y + _translateY;
	vertex.coord[0] = u;
	vertex.coord[1] = v;
}

inline float InterpTCoord(float val, float mins, float maxs, float tMin, float tMax) {
	float	flPercent;

	if (mins != maxs) {
		flPercent = (float)(val - mins) / (maxs - mins);
	} else {
		flPercent = 0.5f;
	}

	return tMin + (tMax - tMin) * flPercent;
}

qboolean ClipRect(const vpoint_t &inUL, const vpoint_t &inLR, vpoint_t *pOutUL, vpoint_t *pOutLR) {
	if (g_bScissor) {
		if (g_ScissorRect.left > inUL.point[0]) {
			pOutUL->point[0] = g_ScissorRect.left;
		} else {
			pOutUL->point[0] = inUL.point[0];
		}

		if (g_ScissorRect.right <= inLR.point[0]) {
			pOutLR->point[0] = g_ScissorRect.right;
		} else {
			pOutLR->point[0] = inLR.point[0];
		}

		if (g_ScissorRect.top > inUL.point[1]) {
			pOutUL->point[1] = g_ScissorRect.top;
		} else {
			pOutUL->point[1] = inUL.point[1];
		}

		if (g_ScissorRect.bottom <= inLR.point[1]) {
			pOutLR->point[1] = g_ScissorRect.bottom;
		} else {
			pOutLR->point[1] = inLR.point[1];
		}

		if ((pOutUL->point[0] > pOutLR->point[0]) || (pOutUL->point[1] > pOutLR->point[1])) {
			return false;
		}

		pOutUL->coord[0] = InterpTCoord(pOutUL->point[0], inUL.point[0], inLR.point[0], inUL.coord[0], inLR.coord[0]);
		pOutLR->coord[0] = InterpTCoord(pOutLR->point[0], inUL.point[0], inLR.point[0], inUL.coord[0], inLR.coord[0]);
		pOutUL->coord[1] = InterpTCoord(pOutUL->point[1], inUL.point[1], inLR.point[1], inUL.coord[1], inLR.coord[1]);
		pOutLR->coord[1] = InterpTCoord(pOutLR->point[1], inUL.point[1], inLR.point[1], inUL.coord[1], inLR.coord[1]);
	} else {
		*pOutUL = inUL;
		*pOutLR = inLR;
	}

	return true;
}

void BaseUISurface::DrawFilledRect(int x0, int y0, int x1, int y1) {
	vpoint_t rect[2];
	vpoint_t clippedRect[2];

	if (_drawColor[3] >= 255) {
		return;
	}

	InitVertex(rect[0], x0, y0, 0, 0);
	InitVertex(rect[1], x1, y1, 0, 0);

	if (!ClipRect(rect[0], rect[1], &clippedRect[0], &clippedRect[1])) {
		return;
	}

	g_api->SetupDrawingRect(_drawColor);
	g_api->EnableTexture(false);
	g_api->DrawQuad(&clippedRect[0], &clippedRect[1]);
	g_api->EnableTexture(true);
}

void BaseUISurface::DrawOutlinedRect(int x0, int y0, int x1, int y1) {
	if (_drawColor[3] >= 255) {
		return;
	}

	DrawFilledRect(x0, y0, x1, y0 + 1);
	DrawFilledRect(x0, y1 - 1, x1, y1);
	DrawFilledRect(x0, y0 + 1, x0 + 1, y1 - 1);
	DrawFilledRect(x1 - 1, y0 + 1, x1, y1 - 1);
}

void BaseUISurface::DrawLine(int, int, int, int) {
	gEngfuncs.Con_DPrintf((char *)"DrawLine ");
}

void BaseUISurface::DrawPolyLine(int *, int *, int) {
	gEngfuncs.Con_DPrintf((char *)"DrawPolyLine ");
}

void BaseUISurface::DrawSetTextFont(vgui2::HFont font) {
	m_hCurrentFont = font;
}

void BaseUISurface::DrawSetTextColor(int r, int g, int b, int a) {
	_drawTextColor[0] = r;
	_drawTextColor[1] = g;
	_drawTextColor[2] = b;
	_drawTextColor[3] = a;
}

void BaseUISurface::DrawSetTextColor(Color col) {
	col.GetColor(_drawTextColor[0], _drawTextColor[1], _drawTextColor[2], _drawTextColor[3]);
}

void BaseUISurface::DrawSetTextPos(int x, int y) {
	_drawTextPos[0] = x;
	_drawTextPos[1] = y;
}

void BaseUISurface::DrawGetTextPos(int &x, int &y) {
	x = _drawTextPos[0];
	y = _drawTextPos[1];
}

void BaseUISurface::DrawPrintText(const wchar_t *text, int textlen) {
	if (!text || textlen <= 0) {
		return;
	}

	for (int i = 0; i < textlen; ++i) {
		DrawUnicodeChar(text[i]);
	}
}

void BaseUISurface::DrawUnicodeChar(wchar_t ch) {
	// TODO: support custom fonts
	gEngfuncs.pfnVGUI2DrawCharacterAdditive(_drawTextPos[0] + _translateX, _drawTextPos[1] + _translateY, ch, _drawTextColor[0], _drawTextColor[1], _drawTextColor[2], m_hCurrentFont);
}

void BaseUISurface::DrawUnicodeCharAdd(wchar_t ch) {
	// TODO: support custom fonts
	gEngfuncs.pfnVGUI2DrawCharacterAdditive(_drawTextPos[0] + _translateX, _drawTextPos[1] + _translateY, ch, _drawTextColor[0], _drawTextColor[1], _drawTextColor[2], m_hCurrentFont);
}

void BaseUISurface::DrawFlushText() {
	// TODO: support custom fonts
}

class vgui2::IHTML *BaseUISurface::CreateHTMLWindow(class vgui2::IHTMLEvents *, vgui2::VPANEL) {
	return NULL;
}

void BaseUISurface::PaintHTMLWindow(class vgui2::IHTML *) {
	// TODO
}

void BaseUISurface::DeleteHTMLWindow(class vgui2::IHTML *) {
	// TODO
}

void BaseUISurface::DrawSetTextureFile(int id, const char  * filename, int hardwareFilter, bool forceReload) {
	// TODO
	//gEngfuncs.Con_DPrintf("DrawSetTextureFile ");
	//

	char name[512];
	snprintf(name, sizeof(name), "%s.tga", filename);
	int width, height;
	static byte buffer[1024 * 1024 * 4];

	if (!LoadTGA(name, buffer, sizeof(buffer), &width, &height))
	{
		snprintf(name, sizeof(name), "%s.bmp", filename);

		if (!LoadBMP(name, buffer, sizeof(buffer), &width, &height))
		{
			DrawSetTexture(id);
			return;
		}
	}

	DrawSetTextureRGBA(id, buffer, width, height, hardwareFilter, forceReload);
}

void BaseUISurface::DrawSetTextureRGBA(int id, const unsigned char *rgba, int wide, int tall, int hardwareFilter, bool forceReload) {
	g_api->UploadTexture(id, (const char *)rgba, wide, tall);
	DrawSetTexture(id);
}

void BaseUISurface::DrawSetTexture(int id) {
	g_iCurrentTexture = id;
	g_api->BindTexture(id);
}

void BaseUISurface::DrawGetTextureSize(int id, int &wide, int &tall) {
	int width, height;
	g_api->BindTexture(id);
	g_api->GetTextureSizes(&width, &height);
	g_api->BindTexture(g_iCurrentTexture);
	wide = width;
	tall = height;
}

void BaseUISurface::DrawTexturedRect(int x0, int y0, int x1, int y1) {
	vpoint_t rect[2];
	vpoint_t clippedRect[2];

	InitVertex(rect[0], x0, y0, 0, 0);
	InitVertex(rect[1], x1, y1, 1, 1);

	if (!ClipRect(rect[0], rect[1], &clippedRect[0], &clippedRect[1])) {
		return;
	}

	g_api->SetupDrawingImage(_drawColor);
	g_api->DrawQuad(&clippedRect[0], &clippedRect[1]);
}

bool BaseUISurface::IsTextureIDValid(int) {
	return true;
}

int BaseUISurface::CreateNewTextureID(bool procedural) {
	return g_api->GenerateTexture();
}

void BaseUISurface::GetScreenSize(int &wide, int &tall) {
	wide = _screeninfo.iWidth;
	tall = _screeninfo.iHeight;
}

void BaseUISurface::SetAsTopMost(vgui2::VPANEL, bool) {
	// 
}

void BaseUISurface::BringToFront(vgui2::VPANEL panel) {
	vgui2::ipanel()->MoveToFront(panel);
	
	if (vgui2::ipanel()->IsPopup(panel)) {
		MovePopupToFront(panel);
	}
}

void BaseUISurface::SetForegroundWindow(vgui2::VPANEL panel) {
	BringToFront(panel);
}

void BaseUISurface::SetPanelVisible(vgui2::VPANEL, bool) {
	//
}

void BaseUISurface::SetMinimized(vgui2::VPANEL panel, bool state) {
	if (state) {
		vgui2::ipanel()->SetPlat(panel, VPANEL_MINIMIZED);
		vgui2::ipanel()->SetVisible(panel, false);
	} else {
		vgui2::ipanel()->SetPlat(panel, VPANEL_NORMAL);
	}
}

bool BaseUISurface::IsMinimized(vgui2::VPANEL panel) {
	return vgui2::ipanel()->Plat(panel) == VPANEL_MINIMIZED;
}

void BaseUISurface::FlashWindow(vgui2::VPANEL, bool) {
	//
}

void BaseUISurface::SetTitle(vgui2::VPANEL, const wchar_t  *) {
	//
}

void BaseUISurface::SetAsToolBar(vgui2::VPANEL, bool) {
	//
}

void BaseUISurface::CreatePopup(vgui2::VPANEL panel, bool minimised, bool showTaskbarIcon, bool disabled, bool mouseInput, bool kbInput) {
	if (!vgui2::ipanel()->GetParent(panel)) {
		vgui2::ipanel()->SetParent(panel, GetEmbeddedPanel());
	}

	vgui2::ipanel()->SetPopup(panel, true);
	vgui2::ipanel()->SetKeyBoardInputEnabled(panel, kbInput);
	vgui2::ipanel()->SetMouseInputEnabled(panel, mouseInput);

	// TODO: implement vgui2::Dar
	staticPopupList.push_back(panel);
}

void BaseUISurface::SwapBuffers(vgui2::VPANEL) {
	//
}

void BaseUISurface::Invalidate(vgui2::VPANEL) {
	//
}

void BaseUISurface::SetCursor(vgui2::HCursor cursor) {
	_currentCursor = cursor;
	g_api->CursorSelect((VGUI_DefaultCursor)cursor);
}

bool BaseUISurface::IsCursorVisible() {
	return _currentCursor != dc_none;
}

void BaseUISurface::ApplyChanges() {
	//
}

bool BaseUISurface::IsWithin(int, int) {
	return true;
}

bool BaseUISurface::HasFocus() {
	return true;
}

bool BaseUISurface::SupportsFeature(SurfaceFeature_e feature) {
	return feature < ESCAPE_KEY;
}

void BaseUISurface::RestrictPaintToSinglePanel(vgui2::VPANEL panel) {
	_restrictedPanel = panel;
	vgui2::input()->SetAppModalSurface(panel);
}

void BaseUISurface::SetModalPanel(vgui2::VPANEL) {
	//
}

vgui2::VPANEL BaseUISurface::GetModalPanel() {
	return _restrictedPanel;
}

void BaseUISurface::UnlockCursor() {
	_cursorLocked = false;
}

void BaseUISurface::LockCursor() {
	_cursorLocked = true;
}

void BaseUISurface::SetTranslateExtendedKeys(bool state) {
	m_bTranslateExtendedKeys = state;
}

vgui2::VPANEL BaseUISurface::GetTopmostPopup() {
	if (staticPopupList.size() > 0) {
		return staticPopupList.back();
	}

	return 0;
}

void BaseUISurface::SetTopLevelFocus(vgui2::VPANEL subFocus) {
	while (subFocus) {
		if (vgui2::ipanel()->IsPopup(subFocus)) {
			BringToFront(subFocus);
			break;
		}

		subFocus = vgui2::ipanel()->GetParent(subFocus);
	}
}

vgui2::HFont BaseUISurface::CreateFont() {
	gEngfuncs.Con_DPrintf((char *)"CreateFont ");
	return 1;
}

bool BaseUISurface::AddGlyphSetToFont(vgui2::HFont, const char  *, int, int, int, int, int, int, int) {
	gEngfuncs.Con_DPrintf((char *)"AddGlyphSetToFont ");
	return true;
}

bool BaseUISurface::AddCustomFontFile(const char  *) {
	gEngfuncs.Con_DPrintf((char *)"AddCustomFontFile ");
	return true;
}

int BaseUISurface::GetFontTall(vgui2::HFont font) {
	// TODO: support custom fonts
	return _screeninfo.iCharHeight;
}

void BaseUISurface::GetCharABCwide(vgui2::HFont font, int ch, int &a, int &b, int &c) {
	// TODO: support custom fonts
	a = _screeninfo.charWidths[static_cast<char>(g_api->ProcessUtfChar(ch))];
	b = c = 0;
}

int BaseUISurface::GetCharacterWidth(vgui2::HFont font, int ch) {
	// TODO: support custom fonts
	return _screeninfo.charWidths[static_cast<char>(g_api->ProcessUtfChar(ch))];
}

void BaseUISurface::GetTextSize(vgui2::HFont, const wchar_t *text, int &wide, int &tall) {
	// TODO: support custom fonts
	wide = 0;
	tall = _screeninfo.iCharHeight;

	for (; *text != 0; ++text) {
		wide += _screeninfo.charWidths[static_cast<char>(g_api->ProcessUtfChar(*text))];
	}
}

vgui2::VPANEL BaseUISurface::GetNotifyPanel() {
	return 0;
}

void BaseUISurface::SetNotifyIcon(vgui2::VPANEL, vgui2::HTexture, vgui2::VPANEL, const char  *) {
	//
}

void BaseUISurface::PlaySound(const char *fileName) {
	gEngfuncs.pfnPlaySoundByName((char *)fileName, VOL_NORM);
}

int BaseUISurface::GetPopupCount() {
	// TODO: implement vgui2::Dar
	return staticPopupList.size();
}

vgui2::VPANEL BaseUISurface::GetPopup(int index) {
	if (index >= 0 && index < GetPopupCount()) {
		// TODO: implement vgui2::Dar
		return staticPopupList[index];
	}

	return 0;
}

bool BaseUISurface::ShouldPaintChildPanel(vgui2::VPANEL panel) {
	if (_restrictedPanel && !vgui2::ipanel()->HasParent(panel, _restrictedPanel)) {
		return false;
	}

	bool isPopup = vgui2::ipanel()->IsPopup(panel);

	if (isPopup) {
		for (int i = 0; i < GetPopupCount(); ++i) {
			if (panel == GetPopup(i)) {
				vgui2::ipanel()->Render_SetPopupVisible(panel, true);
				break;
			}
		}
	}

	return !isPopup;
}

bool BaseUISurface::RecreateContext(vgui2::VPANEL) {
	return true;
}

void BaseUISurface::AddPanel(vgui2::VPANEL panel) {
	if (vgui2::ipanel()->IsPopup(panel)) {
		CreatePopup(panel, false, false, false, true, true);
	}
}

void BaseUISurface::ReleasePanel(vgui2::VPANEL panel) {
	// TODO: implement vgui2::Dar
	std::vector<vgui2::VPANEL>::iterator it = std::find(staticPopupList.begin(), staticPopupList.end(), panel);

	if (it != staticPopupList.end()) {
		staticPopupList.erase(it);
	}

	if (_restrictedPanel == panel) {
		_restrictedPanel = 0;
	}
}

void BaseUISurface::MovePopupToFront(vgui2::VPANEL panel) {
	// TODO: implement vgui2::Dar
	std::vector<vgui2::VPANEL>::iterator it = std::find(staticPopupList.begin(), staticPopupList.end(), panel);

	if (it != staticPopupList.end()) {
		staticPopupList.erase(it);
	}

	staticPopupList.push_back(panel);
}

void BaseUISurface::MovePopupToBack(vgui2::VPANEL panel) {
	// TODO: implement vgui2::Dar
	std::vector<vgui2::VPANEL>::iterator it = std::find(staticPopupList.begin(), staticPopupList.end(), panel);

	if (it != staticPopupList.end()) {
		staticPopupList.erase(it);
	}

	staticPopupList.insert(staticPopupList.begin(), panel);
}

void BaseUISurface::SolveTraverse(vgui2::VPANEL panel, bool forceApplySchemeSettings) {
	InternalSchemeSettingsTraverse(panel, forceApplySchemeSettings);
	InternalThinkTraverse(panel);
	InternalSolveTraverse(panel);
}

void BaseUISurface::PaintTraverse(vgui2::VPANEL panel) {
	if (!vgui2::ipanel()->IsVisible(panel)) {
		return;
	}

	if (panel != GetEmbeddedPanel()) {
		vgui2::ipanel()->PaintTraverse(panel, true, true);
		return;
	}

	if (_restrictedPanel) {
		panel = _restrictedPanel;
	}

	for (int i = 0; i < GetPopupCount(); ++i) {
		vgui2::ipanel()->Render_SetPopupVisible(GetPopup(i), false);
	}

	vgui2::ipanel()->PaintTraverse(panel, true, true);

	for (int i = 0; i < GetPopupCount(); ++i) {
		vgui2::VPANEL pop = GetPopup(i);

		if (vgui2::ipanel()->Render_GetPopupVisible(pop)) {
			vgui2::ipanel()->PaintTraverse(pop, true, true);
		}
	}
}

void BaseUISurface::EnableMouseCapture(vgui2::VPANEL, bool) {
	//
}

void BaseUISurface::GetWorkspaceBounds(int &x, int &y, int &wide, int &tall) {
	x = 0;
	y = 0;
	vgui2::ipanel()->GetSize(_embeddedPanel, wide, tall);
}

void BaseUISurface::GetAbsoluteWindowBounds(int &x, int &y, int &wide, int &tall) {
	x = m_iSurfaceBounds[0];
	y = m_iSurfaceBounds[1];
	wide = m_iSurfaceBounds[2];
	tall = m_iSurfaceBounds[3];
}

void BaseUISurface::GetProportionalBase(int &width, int &height) {
	width = BASE_WIDTH;
	height = BASE_HEIGHT;
}

void BaseUISurface::CalculateMouseVisible() {
	if (m_bIgnoreMouseVisCalc) {
		return;
	}

	_needMouse = false;
	_needKB = false;

	for (int i = 0; i < GetPopupCount(); i++) {
		vgui2::VPANEL pop = GetPopup(i);

		if (!pop) {
			continue;
		}

		bool isVisible = vgui2::ipanel()->IsVisible(pop);
		vgui2::VPANEL p = vgui2::ipanel()->GetParent(pop);

		while (p && isVisible) {
			if (vgui2::ipanel()->IsVisible(p) == false) {
				isVisible = false;
				break;
			}

			p = vgui2::ipanel()->GetParent(p);
		}

		if (isVisible) {
			_needMouse = _needMouse || vgui2::ipanel()->IsMouseInputEnabled(pop);
			_needKB = _needKB || vgui2::ipanel()->IsKeyBoardInputEnabled(pop);
		}
	}

	UnlockCursor();

	if (_needMouse) {
		SetCursor(dc_arrow);
	} else {
		SetCursor(dc_none);
		LockCursor();
	}
}

bool BaseUISurface::NeedKBInput() {
	return _needKB;
}

bool BaseUISurface::HasCursorPosFunctions() {
	return true;
}

void BaseUISurface::SurfaceGetCursorPos(int &x, int &y) {
	int _x, _y;
	g_api->GetCursorPos(&_x, &_y);
	x = _x;
	y = _y;
}

void BaseUISurface::SurfaceSetCursorPos(int x, int y) {
	gEngfuncs.Con_DPrintf((char *)"SurfaceSetCursorPos ");
}

bool BaseUISurface::VGUI2MouseControl() {
	return m_bVGUI2MouseControl;
}

void BaseUISurface::SetVGUI2MouseControl(bool state) {
	m_bVGUI2MouseControl = state;
}

void BaseUISurface::DrawTexturedPolygon(class vgui2::VGuiVertex *, int) {
	gEngfuncs.Con_DPrintf((char *)"DrawTexturedPolygon ");
}

int BaseUISurface::GetFontAscent(vgui2::HFont, wchar_t) {
	gEngfuncs.Con_DPrintf((char *)"GetFontAscent ");
	return 0;
}

void BaseUISurface::SetAllowHTMLJavaScript(bool state) {
	m_bAllowJavaScript = state;
}

void BaseUISurface::SetLanguage(const char *pchLang) {
	if (pchLang) {
		strncpy_s(m_szLanguage, pchLang, sizeof(m_szLanguage));
	} else {
		strcpy_s(m_szLanguage, "english");
	}
}

const char *BaseUISurface::GetLanguage() {
	return m_szLanguage;
}

bool BaseUISurface::DeleteTextureByID(int) {
	return false;
}

void BaseUISurface::DrawUpdateRegionTextureBGRA(int, int, int, const unsigned char  *, int, int) {
	gEngfuncs.Con_DPrintf((char *)"DrawUpdateRegionTextureBGRA ");
}

void BaseUISurface::DrawSetTextureBGRA(int id, const unsigned char *bgra, int wide, int tall) {
	unsigned char *rgba = (unsigned char *)calloc(wide * tall, sizeof(unsigned char) * 4);

	for (int i = 0; i < tall; ++i) {
		for (int j = 0; j < wide * 4; j += 4) {
			*(rgba + wide * i * 4 + j) = *(bgra + wide * i * 4 + j + 2);
			*(rgba + wide * i * 4 + j + 1) = *(bgra + wide * i * 4 + j + 1);
			*(rgba + wide * i * 4 + j + 2) = *(bgra + wide * i * 4 + j);
			*(rgba + wide * i * 4 + j + 3) = *(bgra + wide * i * 4 + j + 3);
		}
	}

	DrawSetTextureRGBA(id, (const unsigned char *)rgba, wide, tall, false, false);
	free(rgba);
}

void BaseUISurface::CreateBrowser(vgui2::VPANEL panel, class IHTMLResponses *pBrowser, bool bPopupWindow, const char *pchUserAgentIdentifier) {
	if (m_pChromeController) {
		m_pChromeController->CreateBrowser(pBrowser, bPopupWindow, pchUserAgentIdentifier);
	}
}

void BaseUISurface::RemoveBrowser(vgui2::VPANEL panel, class IHTMLResponses *pBrowser) {
	if (m_pChromeController) {
		m_pChromeController->RemoveBrowser(pBrowser);
	}
}

void BaseUISurface::IgnoreMouseVisibility(bool state) {
	m_bIgnoreMouseVisCalc = state;
}

class IHTMLChromeController *BaseUISurface::AccessChromeHTMLController() {
	return m_pChromeController;
}

void BaseUISurface::SetScreenBounds(int x, int y, int wide, int tall) {
	m_iSurfaceBounds[0] = x;
	m_iSurfaceBounds[1] = y;
	m_iSurfaceBounds[2] = wide;
	m_iSurfaceBounds[3] = tall;
}

void BaseUISurface::InternalSchemeSettingsTraverse(vgui2::VPANEL panel, bool forceApplySchemeSettings) {
	for (int i = 0; i < vgui2::ipanel()->GetChildCount(panel); ++i) {
		vgui2::VPANEL child = vgui2::ipanel()->GetChild(panel, i);

		if (vgui2::ipanel()->IsVisible(child) || forceApplySchemeSettings) {
			InternalSchemeSettingsTraverse(child, forceApplySchemeSettings);
		}
	}

	vgui2::ipanel()->PerformApplySchemeSettings(panel);
}

void BaseUISurface::SetScreenInfo(SCREENINFO *scrinfo) {
	memcpy(&_screeninfo, scrinfo, scrinfo->iSize);
}

void BaseUISurface::InternalThinkTraverse(vgui2::VPANEL panel) {
	vgui2::ipanel()->Think(panel);

	for (int i = 0; i < vgui2::ipanel()->GetChildCount(panel); ++i) {
		vgui2::VPANEL child = vgui2::ipanel()->GetChild(panel, i);

		if (vgui2::ipanel()->IsVisible(child)) {
			InternalThinkTraverse(child);
		}
	}
}

void BaseUISurface::InternalSolveTraverse(vgui2::VPANEL panel) {
	vgui2::ipanel()->Solve(panel);

	for (int i = 0; i < vgui2::ipanel()->GetChildCount(panel); ++i) {
		vgui2::VPANEL child = vgui2::ipanel()->GetChild(panel, i);

		if (vgui2::ipanel()->IsVisible(child)) {
			InternalSolveTraverse(child);
		}
	}
}

void BaseUISurface::DrawSetTextureRGBAWithAlphaChannel(int id, const byte* rgba, int wide, int tall, int hardwareFilter)
{
	DrawSetTextureRGBA(id, rgba, wide, tall, hardwareFilter, true);
}

void BaseUISurface::DrawSetSubTextureRGBA(int textureID, int drawX, int drawY, const byte* rgba, int subTextureWide, int subTextureTall)
{
	//DrawSetSubTextureRGBA(textureID, drawX, drawY, rgba, subTextureWide, subTextureTall);

	/*
	qglTexSubImage2D(
		GL_TEXTURE_2D,
		0,
		drawX, drawY,
		subTextureWide, subTextureTall,
		GL_RGBA, GL_UNSIGNED_BYTE,
		rgba
	);
	*/
	return g_api->UploadTextureBlock(textureID, drawX, drawY, rgba, subTextureWide, subTextureTall);
}

EXPOSE_SINGLE_INTERFACE(BaseUISurface, ISurface, VGUI_SURFACE_INTERFACE_VERSION);
