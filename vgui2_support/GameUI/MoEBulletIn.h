#pragma once
#include <memory>
#include <vector>
#include <string>

#include <boost/asio.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/ssl.hpp>

#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/ScrollBar.h>
#include <vgui_controls/RichText.h>

#include <vgui_controls/Button.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/PanelListPanel.h>
#include <vgui_controls/TextImage.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/MessageBox.h>

#define BULLETIN_MAX_PAGES 32
#define BULLETIN_MAX_IMAGE 32

class CCSBTEBulletinFrameImage : public vgui2::Panel
{
	DECLARE_CLASS_SIMPLE(CCSBTEBulletinFrameImage, Panel);

public:
	CCSBTEBulletinFrameImage(vgui2::Panel* parent, const char* panelName);

protected:
	virtual void ApplySchemeSettings(vgui2::IScheme* pScheme);
	virtual void PaintBackground(void);

	bool m_bImageBackground;
	vgui2::IImage* m_pTopBackground[3];
	vgui2::IImage* m_pCenterBackground[3];
	vgui2::IImage* m_pBottomBackground[3];
	bool m_bFrameTitleButtonLeft;
};

class CCSBTEBulletinColumn : public vgui2::Panel
{
	DECLARE_CLASS_SIMPLE(CCSBTEBulletinColumn, Panel);

public:
	CCSBTEBulletinColumn(vgui2::Panel* parent, const char* panelName, Panel* pActionSignalTarget = NULL, const char* text = NULL, const char* pCmd = NULL);
	CCSBTEBulletinColumn(vgui2::Panel* parent, const char* panelName, Panel* pActionSignalTarget = NULL, const wchar_t* text = NULL, const char* pCmd = NULL);
	~CCSBTEBulletinColumn(void);

private:
	void Initialize(void);

public:
	enum Column_Flag
	{
		COLUMN_NO_FLAG = 0,
		COLUMN_NO_FOCUS_EFFECT = 0x01,
		COLUMN_NO_PRESS_EFFECT = 0x02,
		COLUMN_NO_ACTIVE_EFFECT = 0x03,
	};

	enum DisplayType
	{
		COLUMN_DISPLAY_DEFAULT = 0,
		COLUMN_DISPLAY_CHAT = 0x01
	};

	virtual void SetWide(int wide, bool resize = true);
	virtual void SetSize(int wide, int tall, bool resize = true);
	virtual void SetBounds(int x, int y, int wide, int tall, bool resize = true);

	virtual int AddColumn(const char* text, const char* cmd);
	virtual int AddColumn(const wchar_t* text, const char* cmd);
	virtual int AddColumn(const char* text, KeyValues* msg);
	virtual int AddColumn(const wchar_t* text, KeyValues* msg);

	virtual int AddTexturedColumn(const char* texture[4], const char* cmd);
	virtual int AddTexturedColumn(const char* texture[4], KeyValues* msg);
	virtual int AddTexturedColumn(const char* cmd, const char* texdefault, const char* texfocus = NULL, const char* texpress = NULL, const char* texdisabled = NULL);
	virtual int AddTexturedColumn(KeyValues* msg, const char* texdefault, const char* texfocus = NULL, const char* texpress = NULL, const char* texdisabled = NULL);
	virtual int AddTexturedColumn(vgui2::IImage* pImage[4], const char* cmd);
	virtual int AddTexturedColumn(vgui2::IImage* pImage[4], KeyValues* msg);

	virtual bool IsTexturedColumn(int columnID);

	virtual int FindColumn(const char* key, bool bCmd = false); // find column contain "text or command" at first
	virtual int FindColumn(const wchar_t* text); // find column contain "text" at first

	virtual int FindTexturedColumn(char* texture, int type = -1); // find column contain Texture Image at first

	virtual bool DestroyColumn(int iPos);
	virtual bool DestroyColumn(char* text); // destory column contain "text" at first
	virtual bool DestroyColumn(wchar_t* text); // destory column contain "text" at first

	virtual int GetColumnSize(void);

	virtual void SetText(int index, const char* szText);
	virtual void SetText(int index, const wchar_t* unicodeString, bool bClearUnlocalizedSymbol = false);

	virtual void GetText(int index, char* textOut, int bufferLen);
	virtual void GetText(int index, wchar_t* textOut, int bufLenInBytes);

	virtual void SetColumnFlag(int flag);
	virtual void UnSetColumnFlag(int flag);
	virtual int GetColumnFlag(void);

	// Content alignment
	// Get the size of the content within the label
	virtual void GetContentSize(int index, int& wide, int& tall);

	// Set how the content aligns itself within the label
	// alignment code, used to determine how the images are layed out within the Label
	enum Alignment
	{
		a_northwest = 0,
		a_north,
		a_northeast,
		a_west,
		a_center,
		a_east,
		a_southwest,
		a_south,
		a_southeast,
	};

	virtual void SetContentAlignment(Alignment alignment);
	virtual void SetTextInset(int xInset, int yInset);

	virtual void SetFont(vgui2::HFont font);
	virtual vgui2::HFont GetFont();

	virtual Panel* HasHotkey(wchar_t key);
	virtual void SetHotkey(wchar_t key);

	virtual vgui2::TextImage* GetTextImage(int index);

	virtual void SetOnFocus(int index, bool state);
	virtual bool IsOnFocus(void);

	virtual void SetClicked(int index, bool bTrue);
	virtual bool IsClicked(void);

	virtual void SetActive(int index, bool bTrue);
	virtual bool IsActive(void);

	virtual int CalcCursor(void);

	MESSAGE_FUNC(DoClick, "PressButton");
	MESSAGE_FUNC(OnHotkey, "Hotkey")
	{
		DoClick();
	}

	MESSAGE_FUNC_INT_INT(OnCursorMoved, "OnCursorMoved", x, y);
	virtual void OnCursorEntered();
	virtual void OnCursorExited();

	// Set button to be mouse clickable or not.
	virtual void SetMouseClickEnabled(vgui2::MouseCode code, bool state);
	// Check if button is mouse clickable
	virtual bool IsMouseClickEnabled(vgui2::MouseCode code);

	// Set default button colors.
	virtual void SetDefaultColor(Color fgColor);
	// Set armed button colors
	virtual void SetOnFocusColor(Color fgColor);
	// Set depressed button colors
	virtual void SetClickedColor(Color fgColor);

	void SetDisabledFgColor1(Color color);
	void SetDisabledFgColor2(Color color);

	Color GetDisabledFgColor1();
	Color GetDisabledFgColor2();

	virtual Color GetColumnFgColor(int index);

	// Text colors
	virtual void SetFgColor(int index, Color color);

	virtual void SetImageBorderByName(vgui2::IScheme* pScheme, char* szName);
	virtual void SetImageBorderByString(vgui2::IScheme* pScheme, char* szBorder);

	// Set the command to send when the button is pressed
	// Set the panel to send the command to with AddActionSignalTarget()
	virtual void SetCommand(int index, const char* command);
	// Set the message to send when the button is pressed
	virtual void SetCommand(int index, KeyValues* message);

	void SetClickSound(const char* sound);

	void SetDisplayType(DisplayType type);

	inline char* va(const char* format, ...)
	{
		va_list		argptr;
		static char	string[256][1024], * s;
		static int	stringindex = 0;

		s = string[stringindex];
		stringindex = (stringindex + 1) & 255;
		va_start(argptr, format);
		Q_vsnprintf(s, sizeof(string[0]), format, argptr);
		va_end(argptr);

		return s;
	}

protected:
	virtual void SetColumnSize(int size);

	virtual void FireActionSignal(void);

	virtual void PerformLayout();

	virtual wchar_t CalculateHotkey(const char* text);
	virtual wchar_t CalculateHotkey(const wchar_t* text);
	virtual void ComputeAlignment(int index, int& tx0, int& ty0, int& tx1, int& ty1);

	virtual void Paint(void);
	virtual void PaintBackground(void);

	virtual void ApplySchemeSettings(vgui2::IScheme* pScheme);

	virtual void OnMousePressed(vgui2::MouseCode code);
	virtual void OnMouseReleased(vgui2::MouseCode code);
	virtual void OnMouseDoublePressed(vgui2::MouseCode code);
	virtual void OnKeyCodePressed(vgui2::KeyCode code);

	KeyValues* GetActionMessage(int index);

private:
	wchar_t	   _hotkey;

	enum Column_State
	{
		STATE_DEFAULT = 0,
		STATE_ONFOCUS = 0x0001,
		STATE_CLICKED = 0x0002
	};

	int                _mouseClickMask;

	unsigned short	   m_sClickSoundName;

	void SetWrap(bool bWrap);
	bool	m_bWrap;

	struct Column_element
	{
		bool isImage;
		vgui2::IImage* pImage[4];
		vgui2::TextImage* textImage;
		KeyValues* actionMessage;
	};

	Alignment  _contentAlignment;
	int		   _textInset[2];
	Color      _disabledFgColor1;
	Color      _disabledFgColor2;

	CUtlVector<Column_element> _columnList;

	Color			   _defaultFgColor;
	Color			   _focusFgColor;
	Color              _clickedFgColor;

	int _pointed;
	int _pressing;

	int _size;
	int _state;
	int _flag;

	bool _imageborder;
	vgui2::IImage* _defaultImage[3];
	vgui2::IImage* _focusImage[3];
	vgui2::IImage* _clickedImage[3];

	DisplayType _displaytype;
};

class CCSBTEBulletinImage : public vgui2::ImagePanel
{
	DECLARE_CLASS_SIMPLE(CCSBTEBulletinImage, ImagePanel);

public:
	CCSBTEBulletinImage(Panel* parent, const char* name);

	virtual void OnMousePressed(vgui2::MouseCode code);
	virtual void OnCursorEntered();
	virtual void OnCursorExited();

	virtual void ApplySchemeSettings(vgui2::IScheme* pScheme);

	void SetBulletinText(char* szTitle, char* szDate);
	void SetBulletinText(wchar_t* szTitle, wchar_t* szDate);

	void SetId(int id) { m_iId = id; };
	void SetActive(bool bActive) { m_pActiveImage->SetVisible(bActive); };
	void SetCanUse(bool bUse) { m_bCanUse = bUse; };

private:
	CCSBTEBulletinFrameImage* m_pActiveImage;
	ImagePanel* m_pMouseImage;

	vgui2::Label* m_pTitle;
	vgui2::Label* m_pDate;

	int m_iId;
	bool m_bCanUse;
};

class CCSBTEBulletinScroll : public vgui2::ScrollBar
{
	DECLARE_CLASS_SIMPLE(CCSBTEBulletinScroll, ScrollBar);

public:
	CCSBTEBulletinScroll(Panel* parent, const char* panelName, bool vertical);

	MESSAGE_FUNC(OnScrollBarSliderMoved, "ScrollBarSliderMoved");

	void SetContextCount(int size);
	int GetContextCount() { return m_iSize; };

	void SetUnusedScrollbarInvisible(bool bInvis) { m_bUnusedScrollbarInvis = bInvis; }
	void SwitchToLine(int line);

	inline char* va(const char* format, ...)
	{
		va_list		argptr;
		static char	string[256][1024], * s;
		static int	stringindex = 0;

		s = string[stringindex];
		stringindex = (stringindex + 1) & 255;
		va_start(argptr, format);
		Q_vsnprintf(s, sizeof(string[0]), format, argptr);
		va_end(argptr);

		return s;
	}

protected:
	bool m_bUnusedScrollbarInvis;

private:
	int m_iSize;
};

class CCSBTEBulletinText : public vgui2::RichText
{
	DECLARE_CLASS_SIMPLE(CCSBTEBulletinText, RichText);

public:
	CCSBTEBulletinText(Panel* parent, const char* panelName);
	~CCSBTEBulletinText();

	MESSAGE_FUNC(OnSliderMoved, "ScrollBarSliderMoved");

	struct bulletin_imgdata
	{
		int line;
		char szImage[64];
		int x;
		int y;
		int w;
		int h;
		bool frame;
		bool solid = false;
	};

	struct bulletin_elem
	{
		wchar_t szTitle[256];
		wchar_t szDate[128];

		std::vector<std::string> szContext;
		std::vector<bulletin_imgdata> vecImage;
		std::vector<std::pair<int, Color>> vecColor; //line, Color
	};

	void ClearImageData(void)
	{
		for (int i = 0; i < BULLETIN_MAX_IMAGE; i++)
		{
			if (m_Image[i])
			{
				m_Image[i]->SetVisible(false);
				m_ImageBackground[i]->SetDrawColor(Color(0, 0, 0, 0));
				memset(&m_ImageData[i], 0, sizeof(bulletin_imgdata));
			}
		}
	};
	void SetImage(int pos, bulletin_imgdata Data);
	void LoadContext(bulletin_elem Data);

	unsigned long m_Font;
private:
	vgui2::ImagePanel* m_Image[BULLETIN_MAX_IMAGE];
	vgui2::ImagePanel* m_ImageBackground[BULLETIN_MAX_IMAGE];
	bulletin_imgdata m_ImageData[BULLETIN_MAX_IMAGE];
};

class CCSBTEBulletin : public vgui2::Frame
{
	DECLARE_CLASS_SIMPLE(CCSBTEBulletin, Frame);

public:
	boost::asio::io_context s_bulletin_ioc;

	boost::asio::awaitable<void> UpdateModel_AsyncRefresh();
	void UpdateModel_Refresh();
	void Update_Initiate();
	void Update_OnGui();

	CCSBTEBulletin(Panel* parent, const char* panelName = "BulletinDlg", bool showTaskbarIcon = true);

	enum Bulletin_Type
	{
		BULLETIN_BULLETIN = 0,
		BULLETIN_SERVERUPDATE,
		BULLETIN_EVENTPROGRESS
	};

	struct bulletin_column_elem
	{
		vgui2::Label* pLabel;
		char szCommand[64];
	};

	using bulletin_imgdata = CCSBTEBulletinText::bulletin_imgdata;
	using bulletin_elem = CCSBTEBulletinText::bulletin_elem;

	void SetType(Bulletin_Type Type, bool bFirst = false);
	bool LoadContext(int page);
	bool IsPageValid(Bulletin_Type type, int page)
	{
		if (page < (int)m_vecPageBulletin[(int)type].size() && page >= 0)
			return true;

		return false;
	};
	void ReadCustomizeFile(void);
	void ReadServerFile(void);

	virtual void OnMouseWheeled(int delta);
	virtual void ApplySchemeSettings(vgui2::IScheme* pScheme);
	virtual void OnThink(void);

	void SwitchToLine(int line);

	void ReplaceTokenAll(char* szString, const char* szToken, const char* szSwitch)
	{
		if (!szString || !szToken || !strlen(szToken))
			return;

		if (!szSwitch)
			szSwitch = "";

		if (!strcmp(szToken, szSwitch))
			return;

		char* p = strstr(szString, szToken);
		while (p)
		{
			int dist = p - szString;

			int size = dist + 1;
			char* q = new char[size];
			strncpy(q, szString, size);
			q[size - 1] = NULL;

			size = strlen(szString) - dist - strlen(szToken) + 1;
			char* r = new char[size];
			strncpy(r, szString + dist + strlen(szToken), size);
			r[size - 1] = NULL;

			sprintf(szString, "%s%s%s", q, szSwitch, r);

			delete[] q;
			delete[] r;

			p = strstr(szString, szToken);
		}
	}
	void ReplaceTokenFirst(char* szString, const char* szToken, const char* szSwitch)
	{
		if (!szString || !szToken || !strlen(szToken))
			return;

		if (!szSwitch)
			szSwitch = "";

		if (!strcmp(szToken, szSwitch))
			return;

		char* p = strstr(szString, szToken);
		if (p)
		{
			int dist = p - szString;

			int size = dist + 1;
			char* q = new char[size];
			strncpy(q, szString, size);
			q[size - 1] = NULL;

			size = strlen(szString) - dist - strlen(szToken) + 1;
			char* r = new char[size];
			strncpy(r, szString + dist + strlen(szToken), size);
			r[size - 1] = NULL;

			sprintf(szString, "%s%s%s", q, szSwitch, r);

			delete[] q;
			delete[] r;
		}
	}
	inline char* va(const char* format, ...)
	{
		va_list		argptr;
		static char	string[256][1024], * s;
		static int	stringindex = 0;

		s = string[stringindex];
		stringindex = (stringindex + 1) & 255;
		va_start(argptr, format);
		Q_vsnprintf(s, sizeof(string[0]), format, argptr);
		va_end(argptr);

		return s;
	}

protected:
	virtual void OnCommand(const char* command) override;
	virtual vgui2::Panel* CreateControlByName(const char* controlName) override;

private:
	int ReadContextFile(bulletin_elem* pElement, char* szFile, int iType);
	int ReadServerContextFile(bulletin_elem* pElement, std::string szFile, int iType);

	void LoadCreator(void);

	struct impl_t;
	std::unique_ptr<impl_t> pimpl;

	Bulletin_Type m_Type;
	int m_iCurrentPage;

	std::vector<bulletin_elem> m_vecPageBulletin[3];
	std::vector<bulletin_column_elem> m_vecColumn;

	vgui2::MessageBox* pMessageBox;

	int _pressed;
};