#ifndef ISURFACE_H
#define ISURFACE_H

#include <interface.h>
#include "vgui/VGUI2.h"
#include "vgui/IHTML.h"
#include "Color.h"

class IHTMLChromeController;
class IHTMLResponses;

namespace vgui2 {

	class IImage;
	class Image;
	class Point;

	// handles
	typedef unsigned long HCursor;
	typedef unsigned long HTexture;
	typedef unsigned long HFont;

	struct VGuiVertex
	{
		VGuiVertex() {}
		VGuiVertex(int x, int y, float u = 0, float v = 0)
			: x(x)
			, y(y)
			, u(u)
			, v(v)
		{
		}
		void Init(int xIn, int yIn, float uIn = 0, float vIn = 0)
		{
			this->x = xIn;
			this->y = yIn;
			this->u = uIn;
			this->v = vIn;
		}

		int x, y;
		float u, v;
	};


	//SRC only defines
	//These types aren't used in GoldSource. - Solokiller
	struct Vertex_t;

	/*
	enum FontDrawType_t
	{
		// Use the "additive" value from the scheme file
		FONT_DRAW_DEFAULT = 0,

		// Overrides
		FONT_DRAW_NONADDITIVE,
		FONT_DRAW_ADDITIVE,

		FONT_DRAW_TYPE_COUNT = 2,
	};
	*/


	// Refactor these two
	/*
	struct CharRenderInfo
	{
		// In:
		FontDrawType_t	drawType;
		wchar_t			ch;

		// Out
		bool			valid;

		// In/Out (true by default)
		bool			shouldclip;
		// Text pos
		int				x, y;
		// Top left and bottom right
		Vertex_t		verts[ 2 ];
		int				textureId;
		int				abcA;
		int				abcB;
		int				abcC;
		int				fontTall;
		HFont			currentFont;
	};
	*/

	/*
	struct IntRect
	{
		int x0;
		int y0;
		int x1;
		int y1;
	};
	*/

	//-----------------------------------------------------------------------------
	// Purpose: Wraps contextless windows system functions
	//-----------------------------------------------------------------------------
	//Inherits from IBaseInterface instead of IAppSystem - Solokiller
	//TODO: provide the IMouseControl interface. - Solokiller
	class ISurface : public IBaseInterface //public IAppSystem
	{
	public:
		// call to Shutdown surface; surface can no longer be used after this is called
		virtual void Shutdown() = 0;

		// frame
		virtual void RunFrame() = 0;

		// hierarchy root
		virtual VPANEL GetEmbeddedPanel() = 0;
		virtual void SetEmbeddedPanel(VPANEL pPanel) = 0;

		// drawing context
		virtual void PushMakeCurrent(VPANEL panel, bool useInsets) = 0;
		virtual void PopMakeCurrent(VPANEL panel) = 0;

		// rendering functions
		virtual void DrawSetColor(int r, int g, int b, int a) = 0;
		virtual void DrawSetColor(Color col) = 0;

		virtual void DrawFilledRect(int x0, int y0, int x1, int y1) = 0;
		//Not used by GoldSource - Solokiller
		//virtual void DrawFilledRectArray( IntRect *pRects, int numRects ) = 0;
		virtual void DrawOutlinedRect(int x0, int y0, int x1, int y1) = 0;

		virtual void DrawLine(int x0, int y0, int x1, int y1) = 0;
		virtual void DrawPolyLine(int *px, int *py, int numPoints) = 0;

		virtual void DrawSetTextFont(HFont font) = 0;
		virtual void DrawSetTextColor(int r, int g, int b, int a) = 0;
		virtual void DrawSetTextColor(Color col) = 0;
		virtual void DrawSetTextPos(int x, int y) = 0;
		virtual void DrawGetTextPos(int& x, int& y) = 0;
		//Font draw types aren't used. - Solokiller
		virtual void DrawPrintText(const wchar_t *text, int textLen/*, FontDrawType_t drawType = FONT_DRAW_DEFAULT*/) = 0;
		virtual void DrawUnicodeChar(wchar_t wch/*, FontDrawType_t drawType = FONT_DRAW_DEFAULT*/) = 0;
		virtual void DrawUnicodeCharAdd(wchar_t wch) = 0;

		virtual void DrawFlushText() = 0;		// flushes any buffered text (for rendering optimizations)
		virtual IHTML *CreateHTMLWindow(vgui2::IHTMLEvents *events, VPANEL context) = 0;
		virtual void PaintHTMLWindow(vgui2::IHTML *htmlwin) = 0;
		virtual void DeleteHTMLWindow(IHTML *htmlwin) = 0;

		//virtual int	 DrawGetTextureId( char const *filename ) = 0;
		//virtual bool DrawGetTextureFile(int id, char *filename, int maxlen ) = 0;
		virtual void DrawSetTextureFile(int id, const char *filename, int hardwareFilter, bool forceReload) = 0;
		virtual void DrawSetTextureRGBA(int id, const unsigned char *rgba, int wide, int tall, int hardwareFilter, bool forceReload) = 0;
		virtual void DrawSetTexture(int id) = 0;
		virtual void DrawGetTextureSize(int id, int &wide, int &tall) = 0;
		virtual void DrawTexturedRect(int x0, int y0, int x1, int y1) = 0;
		virtual bool IsTextureIDValid(int id) = 0;

		virtual int CreateNewTextureID(bool procedural = false) = 0;
#ifdef _XBOX
		virtual void DestroyTextureID(int id) = 0;
		virtual bool IsCachedForRendering(int id, bool bSyncWait) = 0;
		virtual void CopyFrontBufferToBackBuffer() = 0;
		virtual void UncacheUnusedMaterials() = 0;
#endif

		virtual void GetScreenSize(int &wide, int &tall) = 0;
		virtual void SetAsTopMost(VPANEL panel, bool state) = 0;
		virtual void BringToFront(VPANEL panel) = 0;
		virtual void SetForegroundWindow(VPANEL panel) = 0;
		virtual void SetPanelVisible(VPANEL panel, bool state) = 0;
		virtual void SetMinimized(VPANEL panel, bool state) = 0;
		virtual bool IsMinimized(VPANEL panel) = 0;
		virtual void FlashWindow(VPANEL panel, bool state) = 0;
		virtual void SetTitle(VPANEL panel, const wchar_t *title) = 0;
		virtual void SetAsToolBar(VPANEL panel, bool state) = 0;		// removes the window's task bar entry (for context menu's, etc.)

		// windows stuff
		virtual void CreatePopup(VPANEL panel, bool minimised, bool showTaskbarIcon = true, bool disabled = false, bool mouseInput = true, bool kbInput = true) = 0;
		virtual void SwapBuffers(VPANEL panel) = 0;
		virtual void Invalidate(VPANEL panel) = 0;
		virtual void SetCursor(HCursor cursor) = 0;
		virtual bool IsCursorVisible() = 0;
		virtual void ApplyChanges() = 0;
		virtual bool IsWithin(int x, int y) = 0;
		virtual bool HasFocus() = 0;

		// returns true if the surface supports minimize & maximize capabilities
		enum SurfaceFeature_e
		{
			ANTIALIASED_FONTS = 1,
			DROPSHADOW_FONTS = 2,
			ESCAPE_KEY = 3,
			OPENING_NEW_HTML_WINDOWS = 4,
			FRAME_MINIMIZE_MAXIMIZE = 5,
			OUTLINE_FONTS = 6,
			DIRECT_HWND_RENDER = 7,
		};
		virtual bool SupportsFeature(SurfaceFeature_e feature) = 0;

		// restricts what gets drawn to one panel and it's children
		// currently only works in the game
		virtual void RestrictPaintToSinglePanel(VPANEL panel) = 0;

		// these two functions obselete, use IInput::SetAppModalSurface() instead
		virtual void SetModalPanel(VPANEL) = 0;
		virtual VPANEL GetModalPanel() = 0;

		virtual void UnlockCursor() = 0;
		virtual void LockCursor() = 0;
		virtual void SetTranslateExtendedKeys(bool state) = 0;
		virtual VPANEL GetTopmostPopup() = 0;

		// engine-only focus handling (replacing WM_FOCUS windows handling)
		virtual void SetTopLevelFocus(VPANEL panel) = 0;

		// fonts
		// creates an empty handle to a vgui font.  windows fonts can be add to this via AddGlyphSetToFont().
		virtual HFont CreateFont() = 0;

		// adds to the font
		enum EFontFlags
		{
			FONTFLAG_NONE,
			FONTFLAG_ITALIC = 0x001,
			FONTFLAG_UNDERLINE = 0x002,
			FONTFLAG_STRIKEOUT = 0x004,
			FONTFLAG_SYMBOL = 0x008,
			FONTFLAG_ANTIALIAS = 0x010,
			FONTFLAG_GAUSSIANBLUR = 0x020,
			FONTFLAG_ROTARY = 0x040,
			FONTFLAG_DROPSHADOW = 0x080,
			FONTFLAG_ADDITIVE = 0x100,
			FONTFLAG_OUTLINE = 0x200,
			FONTFLAG_CUSTOM = 0x400,		// custom generated font - never fall back to asian compatibility mode
			FONTFLAG_BITMAP = 0x800,		// compiled bitmap font - no fallbacks
		};

		virtual bool AddGlyphSetToFont(HFont font, const char *windowsFontName, int tall, int weight, int blur, int scanlines, int flags, int lowRange, int highRange) = 0;

		// adds a custom font file (only supports true type font files (.ttf) for now)
		virtual bool AddCustomFontFile(const char *fontFileName) = 0;

		// returns the details about the font
		virtual int GetFontTall(HFont font) = 0;
		//virtual bool IsFontAdditive(HFont font) = 0;
		virtual void GetCharABCwide(HFont font, int ch, int &a, int &b, int &c) = 0;
		virtual int GetCharacterWidth(HFont font, int ch) = 0;
		virtual void GetTextSize(HFont font, const wchar_t *text, int &wide, int &tall) = 0;

		// notify icons?!?
		virtual VPANEL GetNotifyPanel() = 0;
		virtual void SetNotifyIcon(VPANEL context, HTexture icon, VPANEL panelToReceiveMessages, const char *text) = 0;

		// plays a sound
		virtual void PlaySound(const char *fileName) = 0;

		//!! these functions should not be accessed directly, but only through other vgui items
		//!! need to move these to seperate interface
		virtual int GetPopupCount() = 0;
		virtual VPANEL GetPopup(int index) = 0;
		virtual bool ShouldPaintChildPanel(VPANEL childPanel) = 0;
		virtual bool RecreateContext(VPANEL panel) = 0;
		virtual void AddPanel(VPANEL panel) = 0;
		virtual void ReleasePanel(VPANEL panel) = 0;
		virtual void MovePopupToFront(VPANEL panel) = 0;
		virtual void MovePopupToBack(VPANEL panel) = 0;

		virtual void SolveTraverse(VPANEL panel, bool forceApplySchemeSettings = false) = 0;
		virtual void PaintTraverse(VPANEL panel) = 0;

		virtual void EnableMouseCapture(VPANEL panel, bool state) = 0;

		// returns the size of the workspace
		virtual void GetWorkspaceBounds(int &x, int &y, int &wide, int &tall) = 0;

		// gets the absolute coordinates of the screen (in windows space)
		virtual void GetAbsoluteWindowBounds(int &x, int &y, int &wide, int &tall) = 0;

		// gets the base resolution used in proportional mode
		virtual void GetProportionalBase(int &width, int &height) = 0;

		virtual void CalculateMouseVisible() = 0;
		virtual bool NeedKBInput() = 0;

		virtual bool HasCursorPosFunctions() = 0;
		virtual void SurfaceGetCursorPos(int &x, int &y) = 0;
		virtual void SurfaceSetCursorPos(int x, int y) = 0;


		// SRC only functions!!!
		//virtual void DrawTexturedLine( const Vertex_t &a, const Vertex_t &b ) = 0;
		//virtual void DrawOutlinedCircle(int x, int y, int radius, int segments) = 0;
		//virtual void DrawTexturedPolyLine( const Vertex_t *p,int n ) = 0; // (Note: this connects the first and last points).
		//virtual void DrawTexturedSubRect( int x0, int y0, int x1, int y1, float texs0, float text0, float texs1, float text1 ) = 0;
		virtual void DrawTexturedPolygon(VGuiVertex *pVertices, int n) = 0;
		virtual int GetFontAscent(HFont font, wchar_t wch) = 0;
		//virtual const wchar_t *GetTitle(VPANEL panel) = 0;
		//virtual bool IsCursorLocked( void ) const = 0;
		//virtual void SetWorkspaceInsets( int left, int top, int right, int bottom ) = 0;

		// Lower level char drawing code, call DrawGet then pass in info to DrawRender
		//virtual bool DrawGetUnicodeCharRenderInfo( wchar_t ch, CharRenderInfo& info ) = 0;
		//virtual void DrawRenderCharFromInfo( const CharRenderInfo& info ) = 0;

		// global alpha setting functions
		// affect all subsequent draw calls - shouldn't normally be used directly, only in Panel::PaintTraverse()
		//virtual void DrawSetAlphaMultiplier( float alpha /* [0..1] */ ) = 0;
		//virtual float DrawGetAlphaMultiplier() = 0;

		// web browser
		virtual void SetAllowHTMLJavaScript(bool state) = 0;

		virtual void SetLanguage(const char* pchLang) = 0;

		virtual const char* GetLanguage() = 0;

		virtual bool DeleteTextureByID(int id) = 0;

		virtual void DrawUpdateRegionTextureBGRA(int nTextureID, int x, int y, const unsigned char *pchData, int wide, int tall) = 0;

		virtual void DrawSetTextureBGRA(int id, const unsigned char *pchData, int wide, int tall) = 0;

		virtual void CreateBrowser(vgui2::VPANEL panel, IHTMLResponses *pBrowser, bool bPopupWindow, const char *pchUserAgentIdentifier) = 0;

		virtual void RemoveBrowser(vgui2::VPANEL panel, IHTMLResponses *pBrowser) = 0;

		virtual IHTMLChromeController *AccessChromeHTMLController() = 0;

		//These *might* work, but will likely cause crashes due to interface compatibility issues. - Solokiller
		/*
		virtual bool setFullscreenMode( int wide, int tall, int bpp ) = 0;
		virtual void setWindowedMode() = 0;

		virtual void SetAsTopMost( bool state ) = 0;
		virtual void SetAsToolBar( bool state ) = 0;

		virtual void PanelRequestFocus( vgui2::VPANEL panel ) = 0;
		virtual void EnableMouseCapture( bool state ) = 0;

		virtual void DrawPrintChar( int x, int y, int wide, int tall, float s0, float t0, float s1, float t1 ) = 0;

		virtual void SetNotifyIcon( vgui2::Image *, vgui2::VPANEL, char const* ) = 0;

		virtual bool SetWatchForComputerUse( bool state ) = 0;
		virtual double GetTimeSinceLastUse() = 0;
		*/

		// video mode changing
		//virtual void OnScreenSizeChanged( int nOldWidth, int nOldHeight ) = 0;
#if !defined( _XBOX )
	//virtual vgui2::HCursor	CreateCursorFromFile( char const *curOrAniFile, char const *pPathID = 0 ) = 0;
#endif
	// create IVguiMatInfo object ( IMaterial wrapper in VguiMatSurface, NULL in CWin32Surface )
	//virtual IVguiMatInfo *DrawGetTextureMatInfoFactory( int id ) = 0;

	//virtual void PaintTraverseEx(VPANEL panel, bool paintPopups = false ) = 0;

	//virtual float GetZPos() const = 0;

	// From the Xbox
	//virtual void SetPanelForInput( VPANEL vpanel ) = 0;
	//virtual void DrawFilledRectFade( int x0, int y0, int x1, int y1, unsigned int alpha0, unsigned int alpha1, bool bHorizontal ) = 0;
	//virtual void DrawSetTextureRGBAEx(int id, const unsigned char *rgba, int wide, int tall, ImageFormat imageFormat ) = 0;
	//virtual void DrawSetTextScale(float sx, float sy) = 0;
	//virtual bool SetBitmapFontGlyphSet(HFont font, const char *windowsFontName, float scalex, float scaley, int flags) = 0;
	//// adds a bitmap font file
	//virtual bool AddBitmapFontFile(const char *fontFileName) = 0;
	//// sets a symbol for the bitmap font
	//virtual void SetBitmapFontName( const char *pName, const char *pFontFilename ) = 0;
	//// gets the bitmap font filename
	//virtual const char *GetBitmapFontName( const char *pName ) = 0;

	//virtual IImage *GetIconImageForFullPath( char const *pFullPath ) = 0;
	//virtual void DrawUnicodeString( const wchar_t *pwString, FontDrawType_t drawType = FONT_DRAW_DEFAULT ) = 0;
	
		//GoldSource doesn't use these, but provide the functions anyway so code is still there. - Solokiller
	public:
	// deadsurface.cpp
	void DrawSetAlphaMultiplier(float alpha);
	float DrawGetAlphaMultiplier(void);
	void DrawFilledRectFade(int x0, int y0, int x1, int y1, unsigned int alpha0, unsigned int alpha1, bool bHorizontal);
	const char *GetResolutionKey(void) const;
	bool DrawGetTextureFile(int id, char *filename, int maxlen);
	int DrawGetTextureId(char const *filename);
	void DrawTexturedPolygon(int n, Vertex_t *pVertices);
	void DrawTexturedSubRect(int x0, int y0, int x1, int y1, float texs0, float text0, float texs1, float text1);
	IImage *GetIconImageForFullPath(char const *pFullPath);
	void SetBlendEnabled(bool state);
	void DumpFontTextures(void);
	int GetCharWidth(HFont font, wchar_t ch);
	void DrawSetTextureRGB(int id, const unsigned char *rgb, int wide, int tall, int hardwareFilter, bool forceReload);
	void DrawSetTextureBGR(int id, const unsigned char *rgba, int wide, int tall, int hardwareFilter, bool forceReload);
	void DrawSetTextureBGRA(int id, const unsigned char *rgba, int wide, int tall, int hardwareFilter, bool forceReload);
	void DrawUpdateRegionTextureRGB(int nTextureID, int x, int y, const unsigned char *pchData, int wide, int tall);
	void DrawUpdateRegionTextureRGBA(int nTextureID, int x, int y, const unsigned char *pchData, int wide, int tall);
	void DrawUpdateRegionTextureBGR(int nTextureID, int x, int y, const unsigned char *pchData, int wide, int tall);
	//void DrawUpdateRegionTextureBGRA(int nTextureID, int x, int y, const unsigned char *pchData, int wide, int tall);
	//void DeleteTextureByID(int id);

	};
}

/**
*	Interface version used by GoldSource.
*/
#define VGUI_SURFACE_INTERFACE_VERSION "VGUI_Surface026"

/*
*	Interface version used by Source 2006.
*/
//#define VGUI_SURFACE_INTERFACE_VERSION "VGUI_Surface030"

#endif // ISURFACE_H
