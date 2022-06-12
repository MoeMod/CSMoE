#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui/IVGui.h>
#include <vgui/IInputInternal.h>

#include <tier1/KeyValues.h>

#include "SignalImagePanel.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>


namespace vgui2
{

	//-----------------------------------------------------------------------------
	// Purpose: Constructor
	//-----------------------------------------------------------------------------
	SignalImagePanel::SignalImagePanel(Panel* parent, const char* panelName, Panel* pActionSignalTarget, const char* pCmd) : ImagePanel(parent, panelName)
	{
		Init();
		if (pActionSignalTarget && pCmd)
		{
			AddActionSignalTarget(pActionSignalTarget);
			SetCommand(pCmd);
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void SignalImagePanel::Init()
	{
		_mouseClickMask = 0;
		_actionMessage = NULL;
		SetMouseClickEnabled(MOUSE_LEFT, true);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Destructor
	//-----------------------------------------------------------------------------
	SignalImagePanel::~SignalImagePanel()
	{
		if (_actionMessage)
		{
			_actionMessage->deleteThis();
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Set button to be mouse clickable or not.
	//-----------------------------------------------------------------------------
	void SignalImagePanel::SetMouseClickEnabled(MouseCode code, bool state)
	{
		if (state)
		{
			//set bit to 1
			_mouseClickMask |= 1 << ((int)(code + 1));
		}
		else
		{
			//set bit to 0
			_mouseClickMask &= ~(1 << ((int)(code + 1)));
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Check if button is mouse clickable
	//-----------------------------------------------------------------------------
	bool SignalImagePanel::IsMouseClickEnabled(MouseCode code)
	{
		if (_mouseClickMask & (1 << ((int)(code + 1))))
		{
			return true;
		}
		return false;
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void SignalImagePanel::OnMousePressed(MouseCode code)
	{
		if (!IsEnabled())
			return;

		if (!IsMouseClickEnabled(code))
			return;

		{
			/*if (IsKeyBoardInputEnabled())
			{
				RequestFocus();
			}*/
			Repaint();
		}

		// lock mouse input to going to this button
		input()->SetMouseCapture(GetVPanel());
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void SignalImagePanel::OnMouseDoublePressed(MouseCode code)
	{
		OnMousePressed(code);
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void SignalImagePanel::OnMouseReleased(MouseCode code)
	{
		input()->SetMouseCapture(NULL);

		if (!IsMouseClickEnabled(code))
			return;

		// it has to be both enabled and (mouse over the button or using a key) to fire
		if (IsEnabled() && (GetVPanel() == input()->GetMouseOver()))
		{
			DoClick();
		}

		// make sure the button gets unselected
		Repaint();
	}

	//-----------------------------------------------------------------------------
	// Purpose:	Activate a button click.
	//-----------------------------------------------------------------------------
	void SignalImagePanel::DoClick()
	{
		FireActionSignal();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Message targets that the button has been pressed
	//-----------------------------------------------------------------------------
	void SignalImagePanel::FireActionSignal()
	{
		// message-based action signal
		if (_actionMessage)
		{
			PostActionSignal(_actionMessage->MakeCopy());
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: sets the command to send when the button is pressed
	//-----------------------------------------------------------------------------
	void SignalImagePanel::SetCommand(const char* command)
	{
		SetCommand(new KeyValues("Command", "command", command));
	}

	//-----------------------------------------------------------------------------
	// Purpose: sets the message to send when the button is pressed
	//-----------------------------------------------------------------------------
	void SignalImagePanel::SetCommand(KeyValues* message)
	{
		// delete the old message
		if (_actionMessage)
		{
			_actionMessage->deleteThis();
		}

		_actionMessage = message;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Peeks at the message to send when button is pressed
	// Input  :  - 
	// Output : KeyValues
	//-----------------------------------------------------------------------------
	KeyValues* SignalImagePanel::GetCommand()
	{
		return _actionMessage;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Get control settings for editing
	//-----------------------------------------------------------------------------
	void SignalImagePanel::GetSettings(KeyValues* outResourceData)
	{
		BaseClass::GetSettings(outResourceData);

		if (_actionMessage)
		{
			outResourceData->SetString("command", _actionMessage->GetString("command", ""));
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void SignalImagePanel::ApplySettings(KeyValues* inResourceData)
	{
		BaseClass::ApplySettings(inResourceData);

		const char* cmd = inResourceData->GetString("command", "");
		if (*cmd)
		{
			// add in the command
			SetCommand(cmd);
		}
	}


	//-----------------------------------------------------------------------------
	// Purpose: Describes editing details
	//-----------------------------------------------------------------------------
	const char* SignalImagePanel::GetDescription(void)
	{
		static char buf[1024];
		Q_snprintf(buf, sizeof(buf), "%s, string command", BaseClass::GetDescription());
		return buf;
	}
};