//========= Copyright ?1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "EngineInterface.h"
#include "GameConsoleDialog.h"
#include "IBaseUI.h"

#include <vgui/IInput.h>
#include <vgui/IInputInternal.h>
#include <vgui/IScheme.h>
#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <KeyValues.h>

#include <vgui_controls/Button.h>
#include <vgui/KeyCode.h>
#include <vgui_controls/Menu.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/RichText.h>

#include "igameuifuncs.h"
#include "keydefs.h"
#include "FileSystem.h"
#include "LoadingDialog.h"
#include "menu_int.h"
#include "keydest.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

extern IBaseUI *baseuifuncs;

namespace ui{
	extern ui_enginefuncs_t engfuncs;
}

//#include "Taskbar.h"
//extern CTaskbar *g_pTaskbar;

//-----------------------------------------------------------------------------
// Purpose: forwards tab key presses up from the text entry so we can do autocomplete
//-----------------------------------------------------------------------------
class TabCatchingTextEntry : public vgui2::TextEntry
{
public:
	TabCatchingTextEntry(Panel *parent, const char *name, vgui2::VPANEL comp) : TextEntry(parent, name), m_pCompletionList( comp )
	{
		SetAllowNonAsciiCharacters( true );
	}

	virtual void OnKeyCodeTyped(KeyCode code)
	{
		if ( code == KeyCode::KEY_PAD_ENTER )
		{
			code = KeyCode::KEY_ENTER;
		}

		if ( code != KeyCode::KEY_DOWN && code != KeyCode::KEY_UP && code != KeyCode::KEY_ENTER )
		{
			GetParent()->OnKeyCodeTyped(code);
		}

		TextEntry::OnKeyCodeTyped(code);
	}

	virtual void OnKeyTyped(uchar32 key)
	{
		if ( !m_bIgnoreKeyTyped )
			TextEntry::OnKeyTyped( key );

		m_bIgnoreKeyTyped = false;
	}

	void IgnoreNextTextInput( bool bIgnore )
	{
		m_bIgnoreKeyTyped = bIgnore;
	}

	virtual void OnKillFocus()
	{
		if (vgui2::input()->GetFocus() != m_pCompletionList ) // if its not the completion window trying to steal our focus
		{
			PostMessage(GetParent(), new KeyValues("CloseCompletionList"));
		}
	}

private:
	vgui2::VPANEL m_pCompletionList;
	bool m_bIgnoreKeyTyped;
};

class CNoKeyboardInputRichText : public vgui2::RichText
{
public:
	CNoKeyboardInputRichText(Panel *parent, const char *name, Panel *pFocusPanel) : RichText(parent, name)
	{
		m_pFocusPanel = pFocusPanel;
	}

	virtual void OnRequestFocus(vgui2::VPANEL subFocus, vgui2::VPANEL defaultPanel)
	{
		m_pFocusPanel->RequestFocus();
	}

	virtual void OnKeyCodeTyped(KeyCode code)
	{
		RichText::OnKeyCodeTyped( code );
	}
	
	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme)
	{
		RichText::ApplySchemeSettings( pScheme );

		if (engine->pfnGetCvarFloat("con_mono") > 0)
		{
			SetFont( pScheme->GetFont( "GameConsole_Mono", IsProportional() ) );
		}
	}

private:
	Panel *m_pFocusPanel;
};

// Things the user typed in and hit submit/return with
CHistoryItem::CHistoryItem( void )
{
	m_text = NULL;
	m_extraText = NULL;
	m_bHasExtra = false;
}

CHistoryItem::CHistoryItem( const char *text, const char *extra)
{
	Assert( text );
	m_text = NULL;
	m_extraText = NULL;
	m_bHasExtra = false;
	SetText( text , extra );
}

CHistoryItem::CHistoryItem( const CHistoryItem& src )
{
	m_text = NULL;
	m_extraText = NULL;
	m_bHasExtra = false;
	SetText( src.GetText(), src.GetExtra() );
}

CHistoryItem::~CHistoryItem( void )
{
	delete[] m_text;
	delete[] m_extraText;
	m_text = NULL;
}

const char *CHistoryItem::GetText() const
{
	if ( m_text )
	{
		return m_text;
	}
	else
	{
		return "";
	}
}

const char *CHistoryItem::GetExtra() const
{
	if ( m_extraText )
	{
		return m_extraText;
	}
	else
	{
		return NULL;
	}
}

void CHistoryItem::SetText( const char *text, const char *extra )
{
	delete[] m_text;
	m_text = new char[ strlen( text ) + 1 ];
	memset( m_text, 0x0, strlen( text ) + 1);
	strcpy( m_text, text );

	if ( extra )
	{
		m_bHasExtra = true;
		delete[] m_extraText;
		m_extraText = new char[ strlen( extra ) + 1 ];
		memset( m_extraText, 0x0, strlen( extra ) + 1);
		strcpy( m_extraText, extra );
	}
	else
	{
		m_bHasExtra = false;
	}
}



//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CGameConsoleDialog::CGameConsoleDialog() : BaseClass(NULL, "GameConsole", false)
{
	// initialize dialog
	MakePopup();
	SetVisible(false);
	SetMinimumSize(100,100);

	//g_pTaskbar->AddTask(GetVPanel());

	SetTitle("#GameUI_Console", true);

	// create controls
	m_pSubmit = new vgui2::Button(this, "ConsoleSubmit", "#GameUI_Submit");
	m_pSubmit->SetCommand("submit");
	m_pCompletionList = new vgui2::Menu(this, "CompletionList");
	m_pCompletionList->MakePopup();
	m_pCompletionList->SetVisible(false);

	m_pEntry = new TabCatchingTextEntry(this, "ConsoleEntry", m_pCompletionList->GetVPanel() );
	m_pEntry->AddActionSignalTarget(this);
	m_pEntry->SendNewLine(true);
	m_pEntry->SetTabPosition(1);

	m_pHistory = new CNoKeyboardInputRichText(this, "ConsoleHistory", m_pEntry);
	m_pHistory->SetVerticalScrollbar(true);

	m_bAutoCompleteMode = false;
	m_szPartialText[0] = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CGameConsoleDialog::~CGameConsoleDialog()
{
}

//-----------------------------------------------------------------------------
// Purpose: brings dialog to the fore
//-----------------------------------------------------------------------------
void CGameConsoleDialog::Activate()
{
	BaseClass::Activate();
	m_pEntry->RequestFocus();
	m_pEntry->IgnoreNextTextInput( false );
	m_pEntry->InvalidateLayout( false, true );
}

//-----------------------------------------------------------------------------
// Purpose: Clears the console
//-----------------------------------------------------------------------------
void CGameConsoleDialog::Clear()
{
	m_pHistory->SetText("");
}

//-----------------------------------------------------------------------------
// Purpose: normal text print
//-----------------------------------------------------------------------------
void CGameConsoleDialog::Print(const char *msg)
{
	if (strstr(msg, "connect") || strstr(msg, "Connect"))
		return;
	if (strstr(msg, "retry") || strstr(msg, "Retry"))
		return;

	m_pHistory->InsertColorChange(m_PrintColor);
	m_pHistory->InsertString(msg);
}

//-----------------------------------------------------------------------------
// Purpose: debug text print
//-----------------------------------------------------------------------------
void CGameConsoleDialog::DPrint(const char *msg)
{
	m_pHistory->InsertColorChange(m_DPrintColor);
	m_pHistory->InsertString(msg);
}

//-----------------------------------------------------------------------------
// Purpose: rebuilds the list of possible completions from the current entered text
//-----------------------------------------------------------------------------
void CGameConsoleDialog::RebuildCompletionList(const char *text)
{
	// clear any old completion list
	m_CompletionList.RemoveAll();

	// we need the length of the text for the partial string compares
	int len = strlen(text);
	if (len < 1)
	{
		// Fill the completion list with history instead
		for ( int i = 0 ; i < m_CommandHistory.Count(); i++ )
		{
			CHistoryItem *item = &m_CommandHistory[ i ];
			CompletionItem *comp = &m_CompletionList[ m_CompletionList.AddToTail() ];
			comp->iscommand = false;
			comp->m_text = new CHistoryItem( *item );
		}
		return;
	}

	if ( strchr( text, ' ' ) )
		return;

	// look through the command list for all matches
	auto cmd = engine->pfnGetFirstCmdFunctionHandle();
	while (cmd)
	{
		if (!strnicmp(text, engine->pfnGetCmdFunctionName(cmd), len))
		{
			// match found, add to list
			int node = m_CompletionList.AddToTail();
			CompletionItem *item = &m_CompletionList[node];
			item->iscommand = true;
			item->cmd.cmd = cmd;
			item->cmd.cvar = NULL;
			item->m_text = new CHistoryItem( engine->pfnGetCmdFunctionName(cmd) );
		}

		cmd = engine->pfnGetNextCmdFunctionHandle(cmd);
	}

	// walk the cvarlist looking for all the matches
	cvar_t *cvar = engine->pfnGetFirstCvarPtr();
	while (cvar)
	{
		if (!_strnicmp(text, cvar->name, len))
		{
			// match found, add to list
			int node = m_CompletionList.AddToTail();
			CompletionItem *item = &m_CompletionList[node];
			item->iscommand = true;
			item->cmd.cmd = 0;
			item->cmd.cvar = cvar;
			item->m_text = new CHistoryItem( cvar->name, cvar->string );
		}

		cvar = cvar->next;
	}
}

//-----------------------------------------------------------------------------
// Purpose: auto completes current text
//-----------------------------------------------------------------------------
void CGameConsoleDialog::OnAutoComplete(bool reverse)
{
	if (!m_bAutoCompleteMode)
	{
		// we're not in auto-complete mode, Start
		m_iNextCompletion = 0;
		m_bAutoCompleteMode = true;
	}

	// if we're in reverse, move back to before the current
	if (reverse)
	{
		m_iNextCompletion -= 2;
		if (m_iNextCompletion < 0)
		{
			// loop around in reverse
			m_iNextCompletion = m_CompletionList.Size() - 1;
		}
	}

	// get the next completion
	if (!m_CompletionList.IsValidIndex(m_iNextCompletion))
	{
		// loop completion list
		m_iNextCompletion = 0;
	}

	// make sure everything is still valid
	if (!m_CompletionList.IsValidIndex(m_iNextCompletion))
		return;

	// match found, set text
	char completedText[256];
	CompletionItem *item = &m_CompletionList[m_iNextCompletion];
	Assert( item );

	if ( item->iscommand )
	{
		strncpy(completedText, item->GetCommand(), sizeof(completedText) - 2 );
	}
	else
	{
		strncpy(completedText, item->GetItemText(), sizeof(completedText) - 2 );
	}
	if ( !Q_strstr( completedText, " " ) )
	{
		strcat(completedText, " ");
	}

	m_pEntry->SetText(completedText);
	m_pEntry->SelectNone();
	m_pEntry->GotoTextEnd();

	m_iNextCompletion++;
}

//-----------------------------------------------------------------------------
// Purpose: Called whenever the user types text
//-----------------------------------------------------------------------------
void CGameConsoleDialog::OnTextChanged(Panel *panel)
{
	if (panel != m_pEntry)
		return;

	// get the partial text the user type
	m_pEntry->GetText(m_szPartialText, sizeof(m_szPartialText));

	// see if they've hit the tilde key (which opens & closes the console)
	int len = strlen(m_szPartialText);

	if (!m_bAutoCompleteMode)
	{
		// check to see if the user just typed a space, so we can auto complete
		if (len > 0 && m_szPartialText[len - 1] == ' ')
		{
			m_iNextCompletion = 0;
			OnAutoComplete(false);
		}
	}
	// clear auto-complete state since the user has typed
	m_bAutoCompleteMode = false;

	RebuildCompletionList(m_szPartialText);

	// build the menu
	if (/*len < 1 ||*/ m_CompletionList.Size() < 1)
	{
		m_pCompletionList->SetVisible(false);
	}
	else
	{
		m_pCompletionList->SetVisible(true);
		m_pCompletionList->DeleteAllItems();
		const int MAX_MENU_ITEMS = 7;

		// add the first ten items to the list
		for (int i = 0; i < m_CompletionList.Size() && i < MAX_MENU_ITEMS; i++)
		{
			char text[256];
			text[0] = 0;
			if (i == MAX_MENU_ITEMS - 1)
			{
				strcpy(text, "...");
			}
			else
			{
				strcpy(text, m_CompletionList[i].GetItemText() );
			}
			text[sizeof(text) - 1] = 0;
			KeyValues *kv = new KeyValues("CompletionCommand");
			kv->SetString("command",text);
			int index = m_pCompletionList->AddMenuItem(text, kv, this);
		}

		UpdateCompletionListPosition();
	}
	
	RequestFocus();
	m_pEntry->RequestFocus();

}

//-----------------------------------------------------------------------------
// Purpose: generic vgui command handler
//-----------------------------------------------------------------------------
void CGameConsoleDialog::OnCommand(const char *command)
{
	if (!stricmp(command, "Submit"))
	{
		// submit the entry as a console commmand
		char szCommand[256];
		m_pEntry->GetText(szCommand, sizeof(szCommand));
		engine->pfnClientCmd(szCommand);

		// add to the history
		Print("] ");
		Print(szCommand);
		Print("\n");

		// clear the field
		m_pEntry->SetText("");

		// clear the completion state
		OnTextChanged(m_pEntry);

		// always go the end of the buffer when the user has typed something
		m_pHistory->GotoTextEnd();
		// Add the command to the history
		char *extra = strchr(szCommand, ' ');
		if ( extra )
		{
			*extra = '\0';
			extra++;
		}

		AddToHistory( szCommand, extra );
		m_pCompletionList->SetVisible(false);

	}
	else
	{
		if (!stricmp(command, "Close"))
		{
			ui::engfuncs.pfnSetKeyDest(key_menu);
		}
		BaseClass::OnCommand(command);
	}
}

//-----------------------------------------------------------------------------
// Purpose: swallows tab key pressed
//-----------------------------------------------------------------------------
void CGameConsoleDialog::OnKeyCodeTyped(KeyCode code)
{
	BaseClass::OnKeyCodeTyped(code);

	KeyCode consolecode = gameuifuncs->GetVGUI2KeyCodeForBind( "toggleconsole" );
	bool bShiftPressed = vgui2::input()->IsKeyDown(vgui2::KEY_LSHIFT ) || vgui2::input()->IsKeyDown(vgui2::KEY_RSHIFT );

	// check for processing
	if (vgui2::input()->GetFocus() == m_pEntry->GetVPanel())
	{
		if (code == vgui2::KEY_TAB)
		{
			bool reverse = false;
			if (vgui2::input()->IsKeyDown(vgui2::KEY_LSHIFT) || vgui2::input()->IsKeyDown(vgui2::KEY_RSHIFT))
			{
				reverse = true;
			}

			// attempt auto-completion
			OnAutoComplete(reverse);
			m_pEntry->RequestFocus();
		}
		else if ( code == vgui2::KEY_ENTER )
		{
            PostMessage( this, new KeyValues( "Command", "command", "submit" ) );
		}
		else if (code == vgui2::KEY_DOWN)
		{
			OnAutoComplete(false);
		//	UpdateCompletionListPosition();
		//	m_pCompletionList->SetVisible(true);

			m_pEntry->RequestFocus();
		}
		else if (code == vgui2::KEY_UP)
		{
			OnAutoComplete(true);
			m_pEntry->RequestFocus();
		}
	}

	m_pEntry->GetText( m_szPartialText, sizeof( m_szPartialText ) );

	bool bCtrlDown = vgui2::input()->IsKeyDown( vgui2::KEY_LCONTROL ) || vgui2::input()->IsKeyDown( vgui2::KEY_RCONTROL );
	bool bWinDown = vgui2::input()->IsKeyDown( vgui2::KEY_LWIN ) || vgui2::input()->IsKeyDown( vgui2::KEY_RWIN );

	if ( strlen( m_szPartialText ) == 0 && ( bCtrlDown/* || ( bWinDown && IsOSX() )*/ ) )
	{
		if ( code == vgui2::KEY_X || code == KeyCode::KEY_C )
		{
			m_pHistory->CopySelected();
		}

		if ( code == vgui2::KEY_A )
		{
			m_pHistory->SelectAllText();
		}
	}

	if ( !bShiftPressed && code == consolecode )
	{
		if ( consolecode != KeyCode::KEY_NONE )
		{
			m_pEntry->SetText( "" );
			m_pEntry->IgnoreNextTextInput( true );
			/*PostMessage( this, new KeyValues( "Close" ) );

			if ( baseuifuncs )
			{
				if ( LoadingDialog() )
				{
					vgui2::surface()->RestrictPaintToSinglePanel( LoadingDialog()->GetVPanel() );
				}
				else
				{
					baseuifuncs->HideGameUI();
				}
			}*/
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: lays out controls
//-----------------------------------------------------------------------------
void CGameConsoleDialog::PerformLayout()
{
	BaseClass::PerformLayout();

	// setup tab ordering
	GetFocusNavGroup().SetDefaultButton(m_pSubmit);

	vgui2::IScheme *pScheme = vgui2::scheme()->GetIScheme( GetScheme() );
	m_pEntry->SetBorder(pScheme->GetBorder("DepressedButtonBorder"));
	m_pHistory->SetBorder(pScheme->GetBorder("DepressedButtonBorder"));

	// layout controls
	int wide, tall;
	GetSize(wide, tall);

	const int inset = 8;
	const int entryHeight = 24;
	const int topHeight = 28;
	const int entryInset = 4;
	const int submitWide = 64;

	m_pHistory->SetPos(inset, inset + topHeight); 
	m_pHistory->SetSize(wide - (inset * 2), tall - (entryInset * 2 + inset * 2 + topHeight + entryHeight));

	m_pEntry->SetPos(inset, tall - (entryInset * 2 + entryHeight));
	m_pEntry->SetSize(wide - (inset * 3 + submitWide), entryHeight);

	m_pSubmit->SetPos(wide - (inset + submitWide), tall - (entryInset * 2 + entryHeight));
	m_pSubmit->SetSize(submitWide, entryHeight);

	UpdateCompletionListPosition();
}

//-----------------------------------------------------------------------------
// Purpose: Sets the position of the completion list popup
//-----------------------------------------------------------------------------
void CGameConsoleDialog::UpdateCompletionListPosition()
{
	int ex, ey, x, y;
	GetPos(x, y);
	m_pEntry->GetPos(ex, ey);

	m_pCompletionList->SetPos(x + ex, y + ey + 32);
	m_pEntry->RequestFocus();
	MoveToFront();
}

//-----------------------------------------------------------------------------
// Purpose: Closes the completion list
//-----------------------------------------------------------------------------
void CGameConsoleDialog::CloseCompletionList()
{
	m_pCompletionList->SetVisible(false);
}

//-----------------------------------------------------------------------------
// Purpose: sets up colors
//-----------------------------------------------------------------------------
void CGameConsoleDialog::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_PrintColor = GetFgColor();
	m_DPrintColor = GetSchemeColor("BrightControlText", pScheme);
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Handles autocompletion menu input
//-----------------------------------------------------------------------------
void CGameConsoleDialog::OnMenuItemSelected(const char *command)
{
	if ( strstr( command, "..." ) )
	{
		m_pCompletionList->SetVisible( true );
		return;
	}

	m_pEntry->SetText(command);
	m_pEntry->GotoTextEnd();
	m_pEntry->InsertChar(' ');
	m_pEntry->GotoTextEnd();

}

void CGameConsoleDialog::Hide()
{
	BaseClass::Close();
	m_iNextCompletion = 0;
}

void CGameConsoleDialog::AddToHistory( const char *commandText, const char *extraText )
{
	// Newest at end, oldest at head
	while ( m_CommandHistory.Count() >= MAX_HISTORY_ITEMS )
	{
		// Remove from head until size is reasonable
		m_CommandHistory.Remove( 0 );
	}

	// strip the space off the end of the command before adding it to the history
	char *command = static_cast<char *>( _alloca( (strlen( commandText ) + 1 ) * sizeof( char ) ));
	if ( command )
	{
		memset( command, 0x0, strlen( commandText ) + 1 );
		strncpy( command, commandText, strlen( commandText ));
		if ( command[ strlen( command ) -1 ] == ' ' )
		{
			 command[ strlen( command ) -1 ] = '\0';
		}
	}

	// strip the quotes off the extra text
	char *extra = NULL;

	if ( extraText )
	{
		extra = static_cast<char *>( malloc( (strlen( extraText ) + 1 ) * sizeof( char ) ));
		if ( extra )
		{
			memset( extra, 0x0, strlen( extraText ) + 1 );
			strncpy( extra, extraText, strlen( extraText )); // +1 to dodge the starting quote
			
			// Strip trailing spaces
			int i = strlen( extra ) - 1; 
			while ( i >= 0 &&  // Check I before referencing i == -1 into the extra array!
				extra[ i ] == ' ' )
			{
				extra[ i ] = '\0';
				i--;
			}
		}
	}

	// If it's already there, then remove since we'll add it to the end instead
	CHistoryItem *item = NULL;
	for ( int i = m_CommandHistory.Count() - 1; i >= 0; i-- )
	{
		item = &m_CommandHistory[ i ];
		if ( !item )
			continue;

		if ( stricmp( item->GetText(), command ) )
			continue;

		if ( extra || item->GetExtra() )
		{
			if ( !extra || !item->GetExtra() )
				continue;

			// stricmp so two commands with the same starting text get added
			if ( stricmp( item->GetExtra(), extra ) )	
				continue;
		}
		m_CommandHistory.Remove( i );
	}

	item = &m_CommandHistory[ m_CommandHistory.AddToTail() ];
	Assert( item );
	item->SetText( command, extra );

	m_iNextCompletion = 0;
	RebuildCompletionList( m_szPartialText );

	free( extra );
}

//-----------------------------------------------------------------------------
// Purpose: writes out console to disk
//-----------------------------------------------------------------------------
void CGameConsoleDialog::DumpConsoleTextToFile()
{
	const int CONDUMP_FILES_MAX_NUM = 1000;

	FileHandle_t handle;
	bool found = false;
	char szfile[ 512 ];

	// we don't want to overwrite other condump.txt files
	for ( int i = 0 ; i < CONDUMP_FILES_MAX_NUM ; ++i )
	{
		_snprintf( szfile, sizeof(szfile), "condump%03d.txt", i );
		if ( !vgui2::filesystem()->FileExists(szfile) )
		{
			found = true;
			break;
		}
	}

	if ( !found )
	{
		Print( "Can't condump! Too many existing condump output files in the gamedir!\n" );
		return;
	}

	handle = vgui2::filesystem()->Open(szfile, "wb");
	if ( handle != FILESYSTEM_INVALID_HANDLE )
	{
		int pos = 0;
		while (1)
		{
			wchar_t buf[512];
			m_pHistory->GetText(pos, buf, sizeof(buf));
			pos += sizeof(buf) / sizeof(wchar_t);

			// don't continue if none left
			if (buf[0] == 0)
				break;

			// convert to ansi
			char ansi[512];
			vgui2::localize()->ConvertUnicodeToANSI(buf, ansi, sizeof(ansi));

			// write to disk
			int len = strlen(ansi);
			for (int i = 0; i < len; i++)
			{
				// preceed newlines with a return
				if (ansi[i] == '\n')
				{
					char ret = '\r';
					vgui2::filesystem()->Write( &ret, 1, handle );
				}

				vgui2::filesystem()->Write( ansi + i, 1, handle );
			}
		}

		vgui2::filesystem()->Close( handle );

		Print( "console dumped to " );
		Print( szfile );
		Print( "\n" );
	}
	else
	{
		Print( "Unable to condump to " );
		Print( szfile );
		Print( "\n" );
	}
}
