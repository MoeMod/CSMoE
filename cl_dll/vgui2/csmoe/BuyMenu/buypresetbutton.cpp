#include <stdio.h>
#include <wchar.h>
#include <UtlSymbol.h>

#include <vgui/IBorder.h>
#include <vgui/IInput.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui/IVGui.h>
#include <vgui/ILocalize.h>
#include <vgui/MouseCode.h>
#include <vgui/KeyCode.h>
#include <KeyValues.h>

#include "buypresetbutton.h"
#include <vgui_controls/TextImage.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/EditablePanel.h>

#include "WeaponImagePanel.h"

#include "shared_util.h"

#include <string>

using namespace vgui2;

class BuyPresetImagePanel : public EditablePanel
{
	typedef EditablePanel BaseClass;

public:
	BuyPresetImagePanel(Panel *parent, const char *panelName) : BaseClass(parent, panelName)
	{
		SetMouseInputEnabled(false);
		SetKeyBoardInputEnabled(false);

		m_pPrimaryWeapon = new WeaponImagePanel(this, "primary");
		m_pSecondaryWeapon = new WeaponImagePanel(this, "secondary");
		m_pKnifeWeapon = new WeaponImagePanel(this, "knife");

		m_pPrimaryWeapon->SetShouldScaleImage(true);
		m_pPrimaryWeapon->SetShouldCenterImage(true);
		m_pSecondaryWeapon->SetShouldScaleImage(true);
		m_pSecondaryWeapon->SetShouldCenterImage(true);
		m_pKnifeWeapon->SetShouldScaleImage(true);
		m_pKnifeWeapon->SetShouldCenterImage(true);
	}

	void ClearWeapons(void)
	{
		m_pPrimaryWeapon->SetWeapon(nullptr);
		m_pSecondaryWeapon->SetWeapon(nullptr);
		m_pKnifeWeapon->SetWeapon(nullptr);
	}

	void SetPrimaryWeapon(const char *name)
	{
		m_pPrimaryWeapon->SetWeapon(name);
	}

	void SetSecondaryWeapon(const char *name)
	{
		m_pSecondaryWeapon->SetWeapon(name);
	}

	void SetKnifeWeapon(const char *name)
	{
		m_pKnifeWeapon->SetWeapon(name);
	}

	virtual void PerformLayout() override
	{
		int w, h;
		GetSize(w, h);
		float scale = w / 175.0;
		m_pPrimaryWeapon->SetBounds(25 * scale, 0 * scale, 100 * scale, 38 * scale);
		m_pSecondaryWeapon->SetBounds(125 * scale, 0 * scale, 50 * scale, 19 * scale);
		m_pKnifeWeapon->SetBounds(125 * scale, 19 * scale, 50 * scale, 19 * scale);
	}

private:
	WeaponImagePanel *m_pPrimaryWeapon;
	WeaponImagePanel *m_pSecondaryWeapon;
	WeaponImagePanel *m_pKnifeWeapon;
};

BuyPresetButton::BuyPresetButton(Panel *parent, const char *panelName) : BaseClass(parent, panelName, "")
{
	m_pImagePanel = new BuyPresetImagePanel(this, "ImagePanel");

	m_pKeyboard = NULL;
	m_pBlankSlot = NULL;
	m_pArmed = NULL;
	m_pSelect = NULL;

	m_pFullText = NULL;
	m_pText = NULL;
	m_cHotkey = 0;
	m_iKeyOffset = -1;
	m_iKeySize = -1;

	m_pSetCommand = NULL;
	m_iMousePressed = (MouseCode)-1;
}

BuyPresetButton::~BuyPresetButton(void)
{
	delete m_pImagePanel;

	if (m_pFullText)
		delete [] m_pFullText;

	if (m_pSetCommand)
		delete [] m_pSetCommand;
}

void BuyPresetButton::ApplySettings(KeyValues *resourceData)
{
	BaseClass::ApplySettings(resourceData);

	m_pKeyboard = scheme()->GetImage(resourceData->GetString("image_keyboard", ""), true);
	m_pBlankSlot = scheme()->GetImage(resourceData->GetString("image_blankslot", ""), true);
	m_pSelect = scheme()->GetImage(resourceData->GetString("image_select", ""), true);

	m_pSetCommand = CloneString(resourceData->GetString("command_set", ""));

	if (m_pSetCommand[0])
		SetMouseClickEnabled(MOUSE_RIGHT, true);
}

void BuyPresetButton::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	if (!m_pKeyboard)
		m_pKeyboard = scheme()->GetImage("resource/Control/button_ingame/keyboard", true);

	if (!m_pBlankSlot)
		m_pBlankSlot = scheme()->GetImage("resource/Control/button_ingame/favoritebg", true);

	if(!m_pArmed)
		m_pArmed = scheme()->GetImage("resource/Control/buybutton/select", true);

	if (!m_pSelect)
		m_pSelect = scheme()->GetImage("resource/Control/buybutton/select", true);

	if (m_iKeyOffset == -1)
	{
		m_iKeyOffset = 6;

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
}

void BuyPresetButton::PerformLayout(void)
{
	BaseClass::PerformLayout();
	int w, h;
	GetSize(w, h);
	m_pImagePanel->SetSize(w, h);
}

void BuyPresetButton::DrawBox(int x, int y, int wide, int tall, Color color, float normalizedAlpha, bool hollow)
{
	if (!ShouldPaint())
		return;

	if (m_pBlankSlot && m_pSelect)
	{
		if (IsDepressed())
		{
			m_pSelect->SetSize(wide, tall);
			m_pSelect->Paint();
		}
		else if (IsArmed())
		{
			m_pArmed->SetSize(wide, tall);
			m_pArmed->Paint();
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

void BuyPresetButton::Paint(void)
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

				HFont font = GetFont();

				int charWide, charTall;
				charTall = surface()->GetFontTall(font);

				int a, b, c;
				surface()->GetCharABCwide(font, key, a, b, c);
				charWide = a + b + c;

				surface()->DrawSetTextColor(GetFgColor());
				surface()->DrawSetTextFont(font);
				surface()->DrawSetTextPos(keyX + (keyWide - charWide) / 2, keyY + (keyTall - charTall) / 2);
				surface()->DrawUnicodeChar(key);
				surface()->DrawFlushText();
			}
		}
	}
}

void BuyPresetButton::PaintBackground(void)
{
	//BaseClass::PaintBackground();
	int x, y, w, h;
	GetBounds(x, y, w, h);
	DrawBox(x, y, w, h, { 255,255,255,255 }, 1.0, true);
}

void BuyPresetButton::FireActionSignal(void)
{
	if (m_iMousePressed == MOUSE_RIGHT)
	{
		PostActionSignal(new KeyValues("Command", "command", m_pSetCommand));
		return;
	}

	BaseClass::FireActionSignal();
}

void BuyPresetButton::OnMousePressed(MouseCode code)
{
	BaseClass::OnMousePressed(code);

	m_iMousePressed = code;
}

void BuyPresetButton::OnMouseReleased(MouseCode code)
{
	BaseClass::OnMouseReleased(code);

	m_iMousePressed = (MouseCode)-1;
}

void BuyPresetButton::GetText(char *textOut, int bufferLen)
{
	vgui2::localize()->ConvertUnicodeToANSI(m_pFullText, textOut, bufferLen);
}

void BuyPresetButton::GetText(wchar_t *textOut, int bufLenInBytes)
{
	wcsncpy(textOut, m_pFullText, bufLenInBytes / sizeof(wchar_t));
}

void BuyPresetButton::SetHotkey(wchar_t ch)
{
	m_cHotkey = ch;
	BaseClass::SetHotkey(towlower(ch));
}

void BuyPresetButton::SetText(const wchar_t *wtext)
{
	BaseClass::SetText(wtext);
}

void BuyPresetButton::SetText(const char *text)
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

void BuyPresetButton::ClearWeapons(void)
{
	m_pImagePanel->ClearWeapons();
}

void BuyPresetButton::SetPrimaryWeapon(const char *name)
{
	return m_pImagePanel->SetPrimaryWeapon(name);
}
void BuyPresetButton::SetSecondaryWeapon(const char *name)
{
	return m_pImagePanel->SetSecondaryWeapon(name);
}
void BuyPresetButton::SetKnifeWeapon(const char *name)
{
	return m_pImagePanel->SetKnifeWeapon(name);
}