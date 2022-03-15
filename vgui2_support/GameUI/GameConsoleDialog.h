//========= Copyright ?1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef GAMECONSOLEDIALOG_H
#define GAMECONSOLEDIALOG_H
#ifdef _WIN32
#pragma once
#endif

//#include "TaskFrame.h"
#include "EngineInterface.h"
//#include "vgui_controls/consoledialog.h"
#include <Color.h>
#include "UtlVector.h"
#include "vgui_controls/Frame.h"
#include "cvardef.h"

// Things the user typed in and hit submit/return with
class CHistoryItem
{
public:
	CHistoryItem( void );
	CHistoryItem( const char *text, const char *extra = NULL );
	CHistoryItem( const CHistoryItem& src );
	~CHistoryItem( void );

	const char *GetText() const;
	const char *GetExtra() const;
	void SetText( const char *text, const char *extra );
	bool HasExtra() { return m_bHasExtra; }
private:

	char		*m_text;
	char		*m_extraText;
	bool		m_bHasExtra;
};

class TabCatchingTextEntry;
class CNoKeyboardInputRichText;

//-----------------------------------------------------------------------------
// Purpose: Game/dev console dialog
//-----------------------------------------------------------------------------
class CGameConsoleDialog : public vgui2::Frame
{
	DECLARE_CLASS_SIMPLE(CGameConsoleDialog, vgui2::Frame);

public:
	CGameConsoleDialog();
	~CGameConsoleDialog();

	// brings dialog to the fore
	MESSAGE_FUNC( Activate, "Activate" );

	// normal text print
	void Print(const char *msg);

	// debug text print
	void DPrint(const char *msg);
	
	// clears the console
	void Clear();

	void Hide();
	void DumpConsoleTextToFile();

private:
	enum
	{
		MAX_HISTORY_ITEMS = 100,
	};

	// methods
	void OnAutoComplete(bool reverse);
	MESSAGE_FUNC_PTR( OnTextChanged, "TextChanged", panel );
	void RebuildCompletionList(const char *partialText);
	void UpdateCompletionListPosition();
	MESSAGE_FUNC( CloseCompletionList, "CloseCompletionList" );
	MESSAGE_FUNC_CHARPTR( OnMenuItemSelected, "CompletionCommand", command );
	void AddToHistory( const char *commandText, const char *extraText );

	// vgui overrides
	virtual void PerformLayout();
	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme);
	virtual void OnCommand(const char *command);
	virtual void OnKeyCodeTyped(enum vgui2::KeyCode code);

	CNoKeyboardInputRichText *m_pHistory;
	TabCatchingTextEntry *m_pEntry;
	vgui2::Button *m_pSubmit;
	vgui2::Menu *m_pCompletionList;
	Color m_PrintColor;
	Color m_DPrintColor;

	bool m_bAutoCompleteMode;	// true if the user is currently tabbing through completion options
	int m_iNextCompletion;		// the completion that we'll next go to
	char m_szPartialText[256];

	struct cmdnode_t
	{
		// pointer to commands (only one will be non-null)
		void* cmd;
		struct cvar_s *cvar;
	};

	class CompletionItem
	{
	public:
		CompletionItem( void )
		{
			iscommand = true;
			cmd.cmd = 0;
			cmd.cvar = NULL;

			m_text = NULL;
		}

		CompletionItem( const CompletionItem& src )
		{
			iscommand = src.iscommand;
			cmd = src.cmd;
			if ( src.m_text )
			{
				m_text = new CHistoryItem( (const CHistoryItem& )src.m_text );
			}
			else
			{
				m_text = NULL;
			}
		}
		
		CompletionItem& operator =( const CompletionItem& src )
		{
			if ( this == &src )
				return *this;

			iscommand = src.iscommand;
			cmd = src.cmd;
			if ( src.m_text )
			{
				m_text = new CHistoryItem( (const CHistoryItem& )*src.m_text );
			}
			else
			{
				m_text = NULL;
			}

			return *this;
		}

		~CompletionItem( void )
		{
			delete m_text;
		}

		char const *GetItemText( void )
		{
			static char text[256];
			text[0] = 0;
			if ( m_text )
			{
				if ( m_text->HasExtra() )
				{
					_snprintf( text, sizeof( text ), "%s %s", m_text->GetText(), m_text->GetExtra() );
				}
				else
				{
					strcpy( text, m_text->GetText() );
				}
			}
			return text;
		}	

		const char *GetCommand( void )
		{
			static char text[256];
			text[0] = 0;
			if ( m_text )
			{
				strcpy( text, m_text->GetText() );
			}
			return text;
		}

		bool			iscommand;
		cmdnode_t cmd;
		CHistoryItem	*m_text;
	};


	CUtlVector<CompletionItem> m_CompletionList;
	CUtlVector<CHistoryItem>	m_CommandHistory;
};


#endif // GAMECONSOLEDIALOG_H
