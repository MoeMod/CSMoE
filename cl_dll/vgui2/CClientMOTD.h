#ifndef GAME_CLIENT_UI_VGUI2_CCLIENTMOTD_H
#define GAME_CLIENT_UI_VGUI2_CCLIENTMOTD_H

#include <vgui_controls/Frame.h>
#include "IViewportPanel.h"

class IViewport;

namespace vgui2
{
class HTML;
class Label;
class RichText;
}

class CClientMOTD : public vgui2::Frame, public IViewportPanel
{
public:
	DECLARE_CLASS_SIMPLE( CClientMOTD, Frame );

	static const size_t MAX_HTML_FILENAME_LENGTH = 4096;
	
public:
	CClientMOTD( IViewport* pParent );
	virtual ~CClientMOTD();

	void SetLabelText( const char* textEntryName, const wchar_t* text );

	virtual bool IsURL( const char* str );

	void PerformLayout() override;
	void OnKeyCodeTyped( vgui2::KeyCode key ) override;
	void OnCommand( const char* command ) override;

	void Close() override;

	virtual void Activate( const char* title, const char* msg );
	virtual void ActivateHtml( const char* title, const char* msg );
	//virtual void Activate( const wchar_t* title, const wchar_t* msg );

	//IViewportPanel overrides
	const char *GetName() override
	{
		return "ClientMOTD";
	}

	void SetData( KeyValues *data ) override {}

	void Reset() override;

	void Update() override {}

	bool NeedsUpdate() override
	{
		return false;
	}

	bool HasInputElements() override
	{
		return true;
	}

	void ShowPanel( bool state ) override;

	// VGUI functions:
	vgui2::VPANEL GetVPanel() override final
	{
		return BaseClass::GetVPanel();
	}

	bool IsVisible() override final
	{
		return BaseClass::IsVisible();
	}

	void SetParent( vgui2::VPANEL parent ) override final
	{
		BaseClass::SetParent( parent );
	}

	void Init() override final
	{
		SetVisible(false);
	}

	void VidInit() override final
	{
		SetVisible(false);
	}

private:
	void RemoveTempFile();

private:
	IViewport* m_pViewport;

	vgui2::RichText* m_pMessage;
	vgui2::HTML* m_pMessageHtml;
	vgui2::Label* m_pServerName;
	bool m_bFileWritten;
	char m_szTempFileName[ MAX_HTML_FILENAME_LENGTH ];
	int m_iScoreBoardKey;
};

#endif //GAME_CLIENT_UI_VGUI2_CCLIENTMOTD_H
