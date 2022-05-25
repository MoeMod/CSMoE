#ifndef BASEUISURFACE_H
#define BASEUISURFACE_H

#include "tier0/platform.h"
#include "IHTMLChromeController.h"
#include "vgui/ISurface.h"
#include "IMouseControl.h"
#include "Color.h"

#include "vguimatsurface/FontTextureCache.h"

#include <vector>
#include <tier1/utldict.h>

#include "winsani_in.h"
#include "xash3d_types.h"
#include "cdll_int.h"
#include "vgui_api.h"
#include "winsani_out.h"

class BaseUISurface : public vgui2::ISurface, public IMouseControl {
	enum {
		BASE_HEIGHT = 480,
		BASE_WIDTH = 640,
	};

	struct PaintState_t {
		vgui2::VPANEL vPanel;
		int	iTranslateX;
		int	iTranslateY;
		int	iScissorLeft;
		int	iScissorRight;
		int	iScissorTop;
		int	iScissorBottom;
	};

    struct CharRenderInfo
    {
        int drawType;
        uchar32 ch;

        bool valid;
        bool additive;
        bool shouldclip;

        int x, y;

        vpoint_t verts[2];
        int textureId;
        int abcA;
        int abcB;
        int abcC;
        int fontTall;
        vgui2::HFont currentFont;
    };

    struct CBatchedCharInfo
    {
        vpoint_t verts[2];
        int textureId;
    };

public:
	BaseUISurface();
	~BaseUISurface();
	void Init(vgui2::VPANEL, IHTMLChromeController *);
	void Shutdown();	
	void AppHandler(void *, void *);	
	bool IsMinimized(vgui2::VPANEL);
	void RestrictPaintToSinglePanel(vgui2::VPANEL);
	vgui2::VPANEL GetEmbeddedPanel();
	void SetEmbeddedPanel(vgui2::VPANEL);
	void RunFrame();	
	void GetScreenSize(int &, int &);	
	void SetAsTopMost(vgui2::VPANEL, bool);
	void SetTitle(vgui2::VPANEL, const wchar_t *);
	void SetAsToolBar(vgui2::VPANEL, bool);
	vgui2::VPANEL GetNotifyPanel();
	void SetNotifyIcon(vgui2::VPANEL, vgui2::HTexture, vgui2::VPANEL, const char *);
	bool RecreateContext(vgui2::VPANEL);
	void SetTopLevelFocus(vgui2::VPANEL);
	int GetPopupCount();	
	vgui2::VPANEL GetPopup(int);
	void SolveTraverse(vgui2::VPANEL, bool);
	void PaintTraverse(vgui2::VPANEL);
	void SetCursor(vgui2::HCursor);
	bool IsCursorVisible();	
	void LockCursor();	
	void UnlockCursor();
	void PushMakeCurrent(vgui2::VPANEL, bool);
	void PopMakeCurrent(vgui2::VPANEL);
	bool setFullscreenMode(int, int, int);	
	void setWindowedMode();	
	void SetAsTopMost(bool);	
	void SetAsToolBar(bool);	
	bool SupportsFeature(enum SurfaceFeature_e);	
	bool HasFocus();	
	bool IsWithin(int, int);	
	void CreatePopup(vgui2::VPANEL, bool, bool, bool, bool, bool);
	void Invalidate(vgui2::VPANEL);
#if defined( _MSC_VER ) || defined( WIN32 )
	void DrawPrintText(const wchar_t *, int);
#endif
	void DrawPrintText(const uchar32 *, int);
	void DrawUnicodeChar(uchar32);
	void DrawUnicodeCharAdd(uchar32);
	void DrawSetTextFont(vgui2::HFont);
	void AddPanel(vgui2::VPANEL);
	void ReleasePanel(vgui2::VPANEL);
	void MovePopupToFront(vgui2::VPANEL);
	void MovePopupToBack(vgui2::VPANEL);
	bool ShouldPaintChildPanel(vgui2::VPANEL);
	void PanelRequestFocus(vgui2::VPANEL);
	void SetPanelVisible(vgui2::VPANEL, bool);
	void EnableMouseCapture(bool);	
	void ApplyChanges();	
	void DrawFilledRect(int, int, int, int);	
	void DrawOutlinedRect(int, int, int, int);	
	void DrawLine(int, int, int, int);	
	void DrawPolyLine(int *, int *, int);	
	void DrawTexturedPolygon(vgui2::Vertex_t *, int) override;
	void DrawSetTextureRGBA(int, const unsigned char *, int, int, int, bool);	
	void DrawSetTexture(int);	
	void DrawTexturedRect(int, int, int, int);	
	int CreateNewTextureID(bool);	
	bool IsTextureIDValid(int);	
	void DrawSetColor(int, int, int, int);	
	void DrawSetColor(Color);	
	void DrawSetTextColor(int, int, int, int);	
	void DrawSetTextColor(Color);
	void DrawSetTextPos(int, int);	
	void DrawGetTextPos(int &, int &);	
	void DrawPrintChar(int, int, int, int, float, float, float, float);	
	void DrawFlushText();	
	void CreateBrowser(vgui2::VPANEL, class IHTMLResponses *, bool, const char *);
	void RemoveBrowser(vgui2::VPANEL, class IHTMLResponses *);
	void DrawSetTextureFile(int, const char *, int, bool);	
	void DrawGetTextureSize(int, int &, int &);	
	void EnableMouseCapture(vgui2::VPANEL, bool);
	void SwapBuffers(vgui2::VPANEL);
	void PlaySound(const char *);	
	void SetNotifyIcon(class Image *, vgui2::VPANEL, const char *);
	void SetMinimized(vgui2::VPANEL, bool);
	void FlashWindow(vgui2::VPANEL, bool);
	void BringToFront(vgui2::VPANEL);
	void SetForegroundWindow(vgui2::VPANEL);
	void SetModalPanel(vgui2::VPANEL);
	vgui2::VPANEL GetModalPanel();
	bool SetWatchForComputerUse(bool);	
	double GetTimeSinceLastUse();	
	void GetWorkspaceBounds(int &, int &, int &, int &);	
	vgui2::HFont CreateFont();
	bool AddGlyphSetToFont(vgui2::HFont, const char *, int, int, int, int, int, int, int);
	int GetFontTall(vgui2::HFont);
	int GetFontAscent(vgui2::HFont, uchar32);
	void GetCharABCwide(vgui2::HFont, int, int &, int &, int &);
	int GetCharacterWidth(vgui2::HFont, int);
#if defined( _MSC_VER ) || defined( WIN32 )
	void GetTextSize(vgui2::HFont, const wchar_t *, int &, int &);
#endif
    void GetTextSize(vgui2::HFont, const uchar32 *, int &, int &);
	bool AddCustomFontFile(const char *);	
	void DrawSetSubTextureRGBA(int, int, int, const unsigned char *, int, int);	
	void SetScreenBounds(int, int, int, int);	
	void GetAbsoluteWindowBounds(int &, int &, int &, int &);	
	vgui2::VPANEL GetTopmostPopup();
	void SetTranslateExtendedKeys(bool);	
	void GetProportionalBase(int &, int &);	
	void CalculateMouseVisible();	
	bool NeedKBInput();	
	bool HasCursorPosFunctions();	
	void SurfaceGetCursorPos(int &, int &);	
	void SurfaceSetCursorPos(int, int);	
	bool VGUI2MouseControl();	
	void SetVGUI2MouseControl(bool);	
	void SetAllowHTMLJavaScript(bool);	
	void ResetCurrentTexture();	
	void ResetViewPort();	
	void DrawSetTextureRGBAWithAlphaChannel(int, const unsigned char *, int, int, int, bool);	
	void SetLanguage(const char *);	
	const char *GetLanguage();	
	bool BIsFullScreen();	
	void GetMouseDelta(int *, int *);	
	void IgnoreMouseVisibility(bool);	
	IHTMLChromeController *AccessChromeHTMLController();	
	bool DeleteTextureByID(int);	
	void DrawUpdateRegionTextureBGRA(int, int, int, const unsigned char *, int, int);	
	void DrawSetTextureBGRA(int, const unsigned char *, int, int);	
	class vgui2::IHTML *CreateHTMLWindow(class vgui2::IHTMLEvents *, vgui2::VPANEL);
	void PaintHTMLWindow(class vgui2::IHTML *);
	void DeleteHTMLWindow(class vgui2::IHTML *);
	void InternalSolveTraverse(vgui2::VPANEL);
	void InternalThinkTraverse(vgui2::VPANEL);
	void InternalSchemeSettingsTraverse(vgui2::VPANEL, bool);
	void DrawSetTextureRGBAWithAlphaChannel(int id, const byte* rgba, int wide, int tall, int hardwareFilter);
#ifndef DISABLE_MOE_VGUI2_EXT
    bool IsEmojiChar(uchar32 ch);
#endif

protected:
    bool DrawGetUnicodeCharRenderInfo(uchar32 ch, CharRenderInfo &info);
    void DrawRenderCharInternal(const CharRenderInfo &info);
    void DrawRenderCharFromInfo(const CharRenderInfo &info);


    void DrawQuad(vpoint_t ul, vpoint_t lr, int *pColor);
    void DrawQuadBlend(vpoint_t ul, vpoint_t lr, int *pColor);
    void DrawQuadArray(int quadCount, vpoint_t *pVerts, int *pColor);
#if defined(LINUX) || defined(OSX) || defined(_WIN32)
    struct font_entry
    {
        void *data;
        int size;
    };

    static CUtlDict< font_entry, unsigned short > m_FontData;

    static void *FontDataHelper( const char *pchFontName, int &size, const char *fontFileName );
#endif

private:
	void SetupPaintState(const PaintState_t &paintState);
	void InitVertex(vpoint_t &vertex, int x, int y, float u, float v);

	vgui2::VPANEL _embeddedPanel;
	vgui2::HCursor _currentCursor;
	vgui2::VPANEL _restrictedPanel;
	vgui2::HFont m_hCurrentFont;
	int m_iCurrentTexture;
	int m_iSurfaceBounds[4];
    CUtlVector<CUtlSymbol> m_CustomFontFileNames;
	IHTMLChromeController *m_pChromeController;
	bool _cursorLocked;
	static bool m_bTranslateExtendedKeys;
	bool _needMouse;
	bool _needKB;
	bool m_bVGUI2MouseControl;
	bool m_bAllowJavaScript;
	char m_szLanguage[32];
	bool m_bFullScreen;
	int m_deltax;
	int m_deltay;
	bool m_bIgnoreMouseVisCalc;
	long unsigned int m_MouseButtonDownTimeStamp;
	int m_MouseButtonDownX;
	int m_MouseButtonDownY;
	bool m_bGotMouseButtonDown;
	int _drawColor[4];
	int _drawTextColor[4];
	int _drawTextPos[2];
	int	_translateX;
	int	_translateY;
	std::vector<PaintState_t> _paintStack;

    CFontTextureCache m_FontTextureCache;
    std::vector<CBatchedCharInfo> m_BatchedCharInfos;
    static constexpr auto MAX_BATCHED_CHAR_VERTS = 4096;
};

#endif // BASEUISURFACE_H
