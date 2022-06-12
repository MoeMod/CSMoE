//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef SIGNALIMAGEPANEL_H
#define SIGNALIMAGEPANEL_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/ImagePanel.h>

namespace vgui2
{
	class SignalImagePanel : public ImagePanel
	{
		DECLARE_CLASS_SIMPLE(SignalImagePanel, ImagePanel);

	public:
		// You can optionally pass in the panel to send the click message to and the name of the command to send to that panel.
		SignalImagePanel(Panel* parent, const char* panelName, Panel* pActionSignalTarget = NULL, const char* pCmd = NULL);
		~SignalImagePanel();
	private:
		void Init();

	public:
		// Set the command to send when the button is pressed
		// Set the panel to send the command to with AddActionSignalTarget()
		virtual void SetCommand(const char* command);
		// Set the message to send when the button is pressed
		virtual void SetCommand(KeyValues* message);

		virtual KeyValues* GetCommand();

		// Set button to be mouse clickable or not.
		virtual void SetMouseClickEnabled(MouseCode code, bool state);
		// Check if button is mouse clickable
		virtual bool IsMouseClickEnabled(MouseCode code);
		// Message targets that the button has been pressed
		virtual void FireActionSignal(void);

		MESSAGE_FUNC(DoClick, "PressButton");
	protected:
		// Get control settings for editing
		void GetSettings(KeyValues* outResourceData) override;
		void ApplySettings(KeyValues* inResourceData) override;
		const char* GetDescription(void) override;

		void OnMousePressed(MouseCode code) override;
		void OnMouseDoublePressed(MouseCode code) override;
		void OnMouseReleased(MouseCode code) override;

	private:
		KeyValues* _actionMessage;
		int                _mouseClickMask;
	};
};
#endif // SIGNALIMAGEPANEL_H