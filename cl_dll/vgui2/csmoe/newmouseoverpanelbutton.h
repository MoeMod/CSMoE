#ifndef MOUSEOVERPANELBUTTON_NEW_H
#define MOUSEOVERPANELBUTTON_NEW_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/TextImage.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui/ISurface.h>

extern vgui2::Panel *g_lastPanel;
extern vgui2::Button *g_lastButton;

class NewMouseOverPanel : public vgui2::EditablePanel
{
	typedef vgui2::EditablePanel BaseClass;

public:
	NewMouseOverPanel(vgui2::Panel *parent, const char *panelName) : BaseClass(parent, panelName)
	{
	}

public:
	bool RequestInfo(KeyValues *data)
	{
		if (!stricmp(data->GetName(), "ControlFactory"))
		{
			const char *controlName = data->GetString("ControlName");
			Panel *newPanel = CreateControlByName(controlName);

			if (newPanel)
			{
				data->SetPtr("PanelPtr", newPanel);
				return true;
			}
		}

		return BaseClass::RequestInfo(data);
	}

	virtual void ApplySettings(KeyValues *resourceData)
	{
		const char *classesbase = resourceData->GetString("classesbase", "");

		if (classesbase[0])
		{
			LoadControlSettings(classesbase, "GAME");
		}

		BaseClass::ApplySettings(resourceData);
	}

	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme) override
	{
		BaseClass::ApplySchemeSettings(pScheme);
	}

	virtual void PerformLayout(void)
	{
		BaseClass::PerformLayout();
	}
};

class NewMouseOverPanelButton : public MouseOverPanelButton
{
	typedef MouseOverPanelButton BaseClass;

public:
	NewMouseOverPanelButton(vgui2::Panel *parent, const char *panelName, vgui2::EditablePanel *templatePanel) : MouseOverPanelButton(parent, panelName, templatePanel)
	{
		if (m_pPanel)
			delete m_pPanel;

		m_pPanel = new NewMouseOverPanel(parent, NULL);
		m_pPanel ->SetVisible(false);

		int x, y, wide, tall;
		templatePanel->GetBounds(x, y, wide, tall);

		int px, py;
		templatePanel->GetPinOffset(px, py);

		int rx, ry;
		templatePanel->GetResizeOffset(rx, ry);

		m_pPanel->SetBounds(x, y, wide, tall);
		m_pPanel->SetAutoResize(templatePanel->GetPinCorner(), templatePanel->GetAutoResize(), px, py, rx, ry);

		m_pKeyboard = NULL;
		m_pBlankSlot = NULL;
		m_pSelect = NULL;

		m_pFullText = NULL;
		m_pText = NULL;
		m_cHotkey = 0;
		m_iKeyOffset = -1;
		m_iKeySize = -1;
	}

	~NewMouseOverPanelButton(void)
	{
		if (m_pFullText)
			delete [] m_pFullText;
	}

	virtual bool LoadClassPage(void)
	{
		const char *classPage = GetClassPage(GetName());
		KeyValues *resourceData = new KeyValues("classes");

		if (resourceData->LoadFromFile(g_pFullFileSystem, classPage, "GAME"))
		{
			const char *require = resourceData->GetString("require", "");

			if (require[0])
			{
				m_pPanel->LoadControlSettings(require, "GAME");
				for (KeyValues *controlKeys = resourceData->GetFirstSubKey(); controlKeys != NULL; controlKeys = controlKeys->GetNextKey())
				{
					if (controlKeys->GetDataType() != KeyValues::TYPE_NONE)
						continue;

					char const *keyName = controlKeys->GetName();
					Panel *control = m_pPanel->FindChildByName(keyName);

					if (!control)
						continue;

					bool proportional = control->IsProportional();
					control->SetProportional(false);
					control->ApplySettings(controlKeys);
					control->SetProportional(proportional);
				}
			}

			resourceData->deleteThis();
		}

		return BaseClass::LoadClassPage();
	}

	virtual void ApplySettings(KeyValues *resourceData)
	{
		BaseClass::ApplySettings(resourceData);

		m_pKeyboard = vgui2::scheme()->GetImage(resourceData->GetString("image_keyboard", ""), true);
		m_pBlankSlot = vgui2::scheme()->GetImage(resourceData->GetString("image_blankslot", ""), true);
		m_pSelect = vgui2::scheme()->GetImage(resourceData->GetString("image_select", ""), true);
	}

	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme) override
	{
		BaseClass::ApplySchemeSettings(pScheme);

		if (!m_pKeyboard)
			m_pKeyboard = vgui2::scheme()->GetImage("resource/Control/button_ingame/keyboard", true);

		if (!m_pBlankSlot)
			m_pBlankSlot = vgui2::scheme()->GetImage("resource/Control/buybutton/blank_slot", true);

		if (!m_pSelect)
			m_pSelect = vgui2::scheme()->GetImage("resource/Control/buybutton/select", true);

		if (m_iKeyOffset == -1)
		{
			m_iKeyOffset = 3;

			if (IsProportional())
				m_iKeyOffset = vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), m_iKeyOffset);
		}

		if (m_iKeySize == -1)
		{
			m_iKeySize = 18;

			if (IsProportional())
				m_iKeySize = vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), m_iKeySize);
		}

		if (m_pFullText)
		{
			if (m_pKeyboard)
			{
				SetText(m_pText);
				SetHotkey(m_cHotkey);
			}
		}

		//SetSelectedInset(0, 0);
	}

	virtual void PerformLayout(void)
	{
		BaseClass::PerformLayout();
	}

	void DrawBox(int x, int y, int wide, int tall, Color color, float normalizedAlpha, bool hollow)
	{
		if (!ShouldPaint())
			return;

		if (m_pBlankSlot && m_pSelect)
		{
			if (IsArmed())
			{
				m_pSelect->SetSize(wide, tall);
				m_pSelect->Paint();
			}
			else
			{
				m_pBlankSlot->SetSize(wide, tall);
				m_pBlankSlot->Paint();
			}
		}

		if (m_pKeyboard)
		{
			wchar_t key = m_cHotkey;

			if (key)
			{
				if (m_iKeyOffset > 0 && m_iKeySize > 0)
				{
					int keyWide = m_iKeySize;
					int keyTall = m_iKeySize;

					int keyX = m_iKeyOffset;
					int keyY = (tall - keyTall) / 2;

					m_pKeyboard->SetPos(keyX, keyY);
					m_pKeyboard->SetSize(keyWide, keyTall);
					m_pKeyboard->Paint();

					SetTextInset(keyX + keyWide + keyX, 0);
				}
			}
		}
	}

	virtual void Paint(void)
	{
		BaseClass::Paint();

		if (m_pKeyboard)
		{
			int wide, tall;
			GetSize(wide, tall);

			wchar_t key = m_cHotkey;

			if (key)
			{
				if (m_iKeyOffset > 0 && m_iKeySize > 0)
				{
					int keyWide = m_iKeySize;
					int keyTall = m_iKeySize;

					int keyX = m_iKeyOffset;
					int keyY = (tall - keyTall) / 2;

					vgui2::HFont font = GetFont();

					int charWide, charTall;
					charTall = vgui2::surface()->GetFontTall(font);

					int a, b, c;
					vgui2::surface()->GetCharABCwide(font, key, a, b, c);
					charWide = a + b + c;

					vgui2::surface()->DrawSetTextColor({ 255,255,255,255 });
					vgui2::surface()->DrawSetTextFont(font);
					vgui2::surface()->DrawSetTextPos(keyX + (keyWide - charWide) / 2, keyY + (keyTall - charTall) / 2);
					vgui2::surface()->DrawUnicodeChar(key);
					vgui2::surface()->DrawFlushText();
				}
			}
		}
	}

	virtual void PaintBackground(void)
	{
		//BaseClass::PaintBackground();
		int x, y, w, h;
		GetBounds(x, y, w, h);
		DrawBox(x, y, w, h, { 255,255,255,255 }, 1.0, true);
	}
	virtual void OnCursorEntered(void)
	{
		BaseClass::OnCursorEntered();
		PostActionSignal(new KeyValues("UpdateClass", "name", this->GetName()));

	}

	void GetText(char *textOut, int bufferLen)
	{
		vgui2::localize()->ConvertUnicodeToANSI(m_pFullText, textOut, bufferLen);
	}

	void GetText(wchar_t *textOut, int bufLenInBytes)
	{
		wcsncpy(textOut, m_pFullText, bufLenInBytes / sizeof(wchar_t));
	}

	void SetHotkey(wchar_t ch)
	{
		m_cHotkey = ch;
		BaseClass::SetHotkey(towlower(ch));
	}

	void SetText(const wchar_t *wtext)
	{
		BaseClass::SetText(wtext);
	}

	void SetText(const char *text)
	{
		const wchar_t *wtext;

		if (text[0] == '#')
		{
			wtext = vgui2::localize()->Find(text);
		}
		else
		{
			wchar_t unicodeVar[256];
            vgui2::localize()->ConvertANSIToUnicode(text, unicodeVar, sizeof(unicodeVar));
			wtext = unicodeVar;
		}

		if (!wtext)
		{
			BaseClass::SetText("");
			return;
		}

		wchar_t hotkey = 0;
		const wchar_t *saveptr = wtext;

		for (const wchar_t *ch = wtext; *ch != 0; ch++)
		{
			if (*ch == '&')
			{
				ch++;

				if (*ch == '&')
				{
					continue;
				}
				else if (*ch == 0)
				{
					break;
				}
				else if (iswalnum(*ch))
				{
					hotkey = *ch;

					if (ch <= (wtext + 1))
					{
						ch++;

						while (*ch == ' ')
							ch++;

						wtext = ch;
						break;
					}
				}
			}
		}

		if (m_pFullText)
			delete [] m_pFullText;

		m_pFullText = CloneWString(saveptr);
		m_pText = m_pFullText + (wtext - saveptr);
		m_cHotkey = hotkey;

		if (m_pKeyboard)
		{
			SetText(wtext);
			SetHotkey(hotkey);
		}
		else
		{
			BaseClass::SetText(text);
		}
	}

protected:
	vgui2::IImage *m_pKeyboard;
	vgui2::IImage *m_pBlankSlot;
	vgui2::IImage *m_pSelect;

	wchar_t *m_pFullText;
	wchar_t *m_pText;
	wchar_t m_cHotkey;
	int m_iKeyOffset;
	int m_iKeySize;
};

#endif