#pragma once


#include <vgui_controls/TextImage.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Button.h>
#include <vgui/ISurface.h>

class IGButton : public vgui2::Button
{
	typedef vgui2::Button BaseClass;

public:
	IGButton(vgui2::Panel* parent, const char* panelName) : Button(parent, panelName, "")
	{
		m_pKeyboard = NULL;
		m_pBlankSlot = NULL;
		m_pSelect = NULL;

		m_pFullText = NULL;
		m_pText = NULL;
		m_cHotkey = 0;
		m_iKeyOffset = -1;
		m_iKeySize = -1;
	}

	~IGButton(void)
	{
		if (m_pFullText)
			delete[] m_pFullText;
	}

	virtual void ApplySettings(KeyValues* resourceData)
	{
		BaseClass::ApplySettings(resourceData);

		m_pKeyboard = vgui2::scheme()->GetImage(resourceData->GetString("image_keyboard", ""), true);
		m_pBlankSlot = vgui2::scheme()->GetImage(resourceData->GetString("image_blankslot", ""), true);
		m_pSelect = vgui2::scheme()->GetImage(resourceData->GetString("image_select", ""), true);
	}

	virtual void ApplySchemeSettings(vgui2::IScheme* pScheme) override
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

	void GetText(char* textOut, int bufferLen)
	{
		vgui2::localize()->ConvertUnicodeToANSI(m_pFullText, textOut, bufferLen);
	}

	void GetText(wchar_t* textOut, int bufLenInBytes)
	{
		wcsncpy(textOut, m_pFullText, bufLenInBytes / sizeof(wchar_t));
	}

	void SetHotkey(wchar_t ch)
	{
		m_cHotkey = ch;
		BaseClass::SetHotkey(m_cHotkey);
	}

	void SetText(const wchar_t* wtext)
	{
		BaseClass::SetText(wtext);
		BaseClass::SetHotkey(m_cHotkey);
	}

	void SetText(const char* text)
	{
		const wchar_t* wtext;

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
		const wchar_t* saveptr = wtext;

		for (const wchar_t* ch = wtext; *ch != 0; ch++)
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

						while (iswspace(*ch))
							ch++;

						wtext = ch;
						break;
					}
				}
			}
		}

		if (m_pFullText)
			delete[] m_pFullText;

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
	vgui2::IImage* m_pKeyboard;
	vgui2::IImage* m_pBlankSlot;
	vgui2::IImage* m_pSelect;

	wchar_t* m_pFullText;
	wchar_t* m_pText;
	wchar_t m_cHotkey;
	int m_iKeyOffset;
	int m_iKeySize;
};