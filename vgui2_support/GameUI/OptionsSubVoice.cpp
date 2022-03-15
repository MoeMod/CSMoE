
#include "EngineInterface.h"
#include "OptionsSubVoice.h"
#include "CvarSlider.h"
#include <vgui/IVGui.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/Slider.h>
#include "ivoicetweak.h"
#include "CvarToggleCheckButton.h"
#include "tier1/KeyValues.h"

COptionsSubVoice::COptionsSubVoice(vgui2::Panel *parent) : PropertyPage(parent, NULL)
{
	
	m_pVoiceTweak = engine->pVoiceTweak;

	m_pMicMeter = new vgui2::ImagePanel(this, "MicMeter");
	m_pMicMeter2 = new vgui2::ImagePanel(this, "MicMeter2");

	m_pReceiveSliderLabel = new vgui2::Label(this, "ReceiveLabel", "#GameUI_VoiceReceiveVolume");
	m_pReceiveVolume = new CCvarSlider(this, "VoiceReceive", "#GameUI_ReceiveVolume", 0.0f, 1.0f, "voice_scale");

	m_pMicrophoneSliderLabel = new vgui2::Label(this, "MicrophoneLabel", "#GameUI_VoiceTransmitVolume");
	m_pMicrophoneVolume = new vgui2::Slider(this, "#GameUI_MicrophoneVolume");
	m_pMicrophoneVolume->SetRange(0, 100);
	m_pMicrophoneVolume->AddActionSignalTarget(this);

	m_pVoiceEnableCheckButton = new CCvarToggleCheckButton(this, "voice_modenable", "#GameUI_EnableVoice", "voice_modenable");

	m_pMicBoost = new vgui2::CheckButton(this, "MicBoost", "#GameUI_BoostMicrophone");
	m_pMicBoost->AddActionSignalTarget(this);
	m_pTestMicrophoneButton = new vgui2::Button(this, "TestMicrophone", "#GameUI_TestMicrophone");

	LoadControlSettings("Resource/OptionsSubVoice.res");

	m_bVoiceOn = false;
	m_pMicMeter2->SetVisible(false);

	if (!m_pVoiceTweak)
	{
		m_pReceiveVolume->SetEnabled(false);
		m_pMicrophoneVolume->SetEnabled(false);
		m_pVoiceEnableCheckButton->SetEnabled(false);
		m_pMicBoost->SetEnabled(false);
		m_pTestMicrophoneButton->SetEnabled(false);
	}
}

COptionsSubVoice::~COptionsSubVoice(void)
{
	if (m_bVoiceOn)
		EndTestMicrophone();
}

void COptionsSubVoice::OnPageShow(void)
{
}

void COptionsSubVoice::OnResetData(void)
{
	if (!m_pVoiceTweak)
		return;

	m_pVoiceTweak->SetControlFloat(OtherSpeakerScale, engine->pfnGetCvarFloat("voice_scale"));

	float micVolume = m_pVoiceTweak->GetControlFloat(MicrophoneVolume);
	m_pMicrophoneVolume->SetValue((int)(100.0f * micVolume));
	m_nMicVolumeValue = m_pMicrophoneVolume->GetValue();

	float fMicBoost = m_pVoiceTweak->GetControlFloat(MicBoost);
	m_pMicBoost->SetSelected(fMicBoost != 0.0f);
	m_bMicBoostSelected = m_pMicBoost->IsSelected();

	m_pReceiveVolume->Reset();
	m_fReceiveVolume = m_pReceiveVolume->GetSliderValue();

	m_pVoiceEnableCheckButton->Reset();
}

void COptionsSubVoice::OnSliderMoved(int position)
{
	if (m_pVoiceTweak)
	{
		if (m_pMicrophoneVolume->GetValue() != m_nMicVolumeValue)
			PostActionSignal(new KeyValues("ApplyButtonEnable"));
	}
}

void COptionsSubVoice::OnCheckButtonChecked(int state)
{
	if (m_pVoiceTweak)
	{
		if (m_pMicBoost->IsSelected() != m_bMicBoostSelected)
			PostActionSignal(new KeyValues("ApplyButtonEnable"));
	}
}

void COptionsSubVoice::OnApplyChanges(void)
{
	if (!m_pVoiceTweak)
		return;

	m_nMicVolumeValue = m_pMicrophoneVolume->GetValue();
	float fMicVolume = (float) m_nMicVolumeValue / 100.0f;
	m_pVoiceTweak->SetControlFloat(MicrophoneVolume, fMicVolume);

	m_bMicBoostSelected = m_pMicBoost->IsSelected();
	m_pVoiceTweak->SetControlFloat(MicBoost, m_bMicBoostSelected ? 1.0f : 0.0f);

	m_pReceiveVolume->ApplyChanges();
	m_fReceiveVolume = m_pReceiveVolume->GetSliderValue();

	m_pVoiceEnableCheckButton->ApplyChanges();
}

void COptionsSubVoice::StartTestMicrophone(void)
{
	if (!m_pVoiceTweak || m_bVoiceOn)
		return;

	m_bVoiceOn = true;

	UseCurrentVoiceParameters();

	if (m_pVoiceTweak->StartVoiceTweakMode())
	{
		m_pTestMicrophoneButton->SetText("#GameUI_StopTestMicrophone");

		m_pReceiveVolume->SetEnabled(false);
		m_pMicrophoneVolume->SetEnabled(false);
		m_pVoiceEnableCheckButton->SetEnabled(false);
		m_pMicBoost->SetEnabled(false);
		m_pMicrophoneSliderLabel->SetEnabled(false);
		m_pReceiveSliderLabel->SetEnabled(false);

		m_pMicMeter2->SetVisible(true);
	}
	else
	{
		ResetVoiceParameters();
		m_bVoiceOn = false;
		return;
	}
}

void COptionsSubVoice::UseCurrentVoiceParameters(void)
{
	int nVal = m_pMicrophoneVolume->GetValue();
	float val = (float)nVal / 100.0f;
	m_pVoiceTweak->SetControlFloat(MicrophoneVolume, val);

	bool bSelected = m_pMicBoost->IsSelected();
	val = bSelected ? 1.0f : 0.0f;
	m_pVoiceTweak->SetControlFloat(MicBoost, val);

	m_nReceiveSliderValue = m_pReceiveVolume->GetValue();
	m_pReceiveVolume->ApplyChanges();
}

void COptionsSubVoice::ResetVoiceParameters(void)
{
	float fMicVolume = (float)m_nMicVolumeValue / 100.0f;
	m_pVoiceTweak->SetControlFloat(MicrophoneVolume, fMicVolume);
	m_pVoiceTweak->SetControlFloat(MicBoost, m_bMicBoostSelected ? 1.0f : 0.0f);

	engine->Cvar_SetValue("voice_scale", m_fReceiveVolume);

	m_pReceiveVolume->Reset();
	m_pReceiveVolume->SetValue(m_nReceiveSliderValue);
}

void COptionsSubVoice::EndTestMicrophone(void)
{
	if (!m_pVoiceTweak || !m_bVoiceOn)
		return;

	m_pVoiceTweak->EndVoiceTweakMode();
	ResetVoiceParameters();
	m_pTestMicrophoneButton->SetText("#GameUI_TestMicrophone");
	m_bVoiceOn = false;

	m_pReceiveVolume->SetEnabled(true);
	m_pMicrophoneVolume->SetEnabled(true);
	m_pVoiceEnableCheckButton->SetEnabled(true);
	m_pMicBoost->SetEnabled(true);
	m_pMicrophoneSliderLabel->SetEnabled(true);
	m_pReceiveSliderLabel->SetEnabled(true);
	m_pMicMeter2->SetVisible(false);
}

void COptionsSubVoice::OnCommand(const char *command)
{
	if (!stricmp(command, "TestMicrophone"))
	{
		if (!m_bVoiceOn)
			StartTestMicrophone();
		else
			EndTestMicrophone();
	}
	else
		BaseClass::OnCommand(command);
}

void COptionsSubVoice::OnPageHide(void)
{
	if (m_bVoiceOn)
		EndTestMicrophone();

	BaseClass::OnPageHide();
}

void COptionsSubVoice::OnControlModified(void)
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
}

#define BAR_WIDTH 160
#define BAR_INCREMENT 8

void COptionsSubVoice::OnThink(void)
{
	BaseClass::OnThink();

	if (m_bVoiceOn)
	{
		float nValue = m_pVoiceTweak->GetSpeakingVolume();

		int width = (BAR_WIDTH * nValue) / 32768;
		width = ((width + (BAR_INCREMENT - 1)) / BAR_INCREMENT) * BAR_INCREMENT;

		int wide, tall;
		m_pMicMeter2->GetSize(wide, tall);
		m_pMicMeter2->SetSize(width, tall);
		m_pMicMeter2->Repaint();
	}
}