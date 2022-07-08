#include <time.h>

#include "MultiplayerAdvancedDialog.h"

#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui_controls/ListPanel.h>
#include <KeyValues.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/MessageBox.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/TextEntry.h>
#include "PanelListPanel.h"
#include <vgui/IInput.h>
#include <vgui/IInputInternal.h>

#include "FileSystem.h"
#include "GameConsole.h"

#define OPTIONS_FILE "user.scr"

namespace vgui2 {
	class ComboBox;
}

CMultiplayerAdvancedDialog::CMultiplayerAdvancedDialog(vgui2::Panel *parent) : BaseClass(NULL, "MultiplayerAdvancedDialog")
{
	SetBounds(0, 0, 372, 160);
	SetSizeable(false);

	SetTitle("#GameUI_MultiplayerAdvanced", true);

	vgui2::Button *cancel = new vgui2::Button(this, "Cancel", "#GameUI_Cancel");
	cancel->SetCommand("Close");

	vgui2::Button *ok = new vgui2::Button(this, "OK", "#GameUI_OK");
	ok->SetCommand("Ok");

	m_pListPanel = new CPanelListPanel(this, "PanelListPanel");

	m_pList = NULL;

	m_pDescription = new CInfoDescription(m_pListPanel);
	m_pDescription->InitFromFile(OPTIONS_FILE);
	m_pDescription->TransferCurrentValues(NULL);

	LoadControlSettings("Resource/MultiplayerAdvancedDialog.res");
	CreateControls();

	MoveToCenterOfScreen();
	SetSizeable(false);
	SetDeleteSelfOnClose(true);
}

CMultiplayerAdvancedDialog::~CMultiplayerAdvancedDialog(void)
{
	delete m_pDescription;
}

void CMultiplayerAdvancedDialog::Activate(void)
{
	BaseClass::Activate();

	vgui2::input()->SetAppModalSurface(GetVPanel());
}

void CMultiplayerAdvancedDialog::OnClose(void)
{
	BaseClass::OnClose();

	vgui2::input()->SetAppModalSurface(NULL);
}

void CMultiplayerAdvancedDialog::OnCommand(const char *command)
{
	if (!stricmp(command, "Ok"))
	{
		SaveValues();
		OnClose();
		return;
	}

	BaseClass::OnCommand(command);
}

void CMultiplayerAdvancedDialog::OnKeyCodeTyped(vgui2::KeyCode code)
{
	if (code == vgui2::KEY_ESCAPE)
	{
		SetAlpha(0);
		Close();
	}
	else
		BaseClass::OnKeyCodeTyped(code);
}

void CMultiplayerAdvancedDialog::GatherCurrentValues(void)
{
	if (!m_pDescription)
		return;

	vgui2::CheckButton *pBox;
	vgui2::TextEntry *pEdit;
	vgui2::ComboBox *pCombo;
	CScriptObject *pObj;
	CScriptListItem *pItem;

	char szValue[256];
	char strValue[256];
	mpcontrol_t *pList = m_pList;

	while (pList)
	{
		pObj = pList->pScrObj;

		if (!pList->pControl)
		{
			pObj->SetCurValue(pObj->defValue);
			pList = pList->next;
			continue;
		}

		switch (pObj->type)
		{
			case O_BOOL:
			{
				pBox = (vgui2::CheckButton *)pList->pControl;
				sprintf(szValue, "%s", pBox->IsSelected() ? "1" : "0");
				break;
			}

			case O_NUMBER:
			{
				pEdit = (vgui2::TextEntry *)pList->pControl;
				pEdit->GetText(strValue, sizeof(strValue));
				sprintf(szValue, "%s", strValue);
				break;
			}

			case O_STRING:
			{
				pEdit = (vgui2::TextEntry *)pList->pControl;
				pEdit->GetText(strValue, sizeof(strValue));
				sprintf(szValue, "%s", strValue);
				break;
			}

			case O_LIST:
			{
				pCombo = (vgui2::ComboBox *)pList->pControl;
				int activeItem = pCombo->GetActiveItem();
				pItem = pObj->pListItems;
				int n = (int)pObj->fdefValue;

				while (pItem)
				{
					if (!activeItem--)
						break;

					pItem = pItem->pNext;
				}

				if (pItem)
				{
					sprintf(szValue, "%s", pItem->szValue);
				}
				else
				{
					assert(!("Couldn't find string in list, using default value"));
					sprintf(szValue, "%s", pObj->defValue);
				}

				break;
			}
		}

		UTIL_StripInvalidCharacters(szValue, sizeof(szValue));
		strcpy(strValue, szValue);
		pObj->SetCurValue(strValue);
		pList = pList->next;
	}
}

void CMultiplayerAdvancedDialog::CreateControls(void)
{
	DestroyControls();

	CScriptObject *pObj = m_pDescription->pObjList;
	mpcontrol_t *pCtrl;
	vgui2::CheckButton *pBox;
	vgui2::TextEntry *pEdit;
	vgui2::ComboBox *pCombo;
	CScriptListItem *pListItem;

	Panel *objParent = m_pListPanel;

	while (pObj)
	{
		pCtrl = new mpcontrol_t(objParent, "mpcontrol_t");
		pCtrl->type = pObj->type;

		switch (pCtrl->type)
		{
			case O_BOOL:
			{
				pBox = new vgui2::CheckButton(pCtrl, "DescCheckButton", pObj->prompt);
				pBox->SetSelected(pObj->fdefValue != 0.0f ? true : false);
				pCtrl->pControl = (Panel *)pBox;
				break;
			}

			case O_STRING:
			case O_NUMBER:
			{
				pEdit = new vgui2::TextEntry(pCtrl, "DescTextEntry");
				pEdit->InsertString(pObj->defValue);
				pCtrl->pControl = (Panel *)pEdit;
				break;
			}

			case O_LIST:
			{
				pCombo = new vgui2::ComboBox(pCtrl, "DescComboBox", 5, false);
				pListItem = pObj->pListItems;

				while (pListItem)
				{
					pCombo->AddItem(pListItem->szItemText, NULL);
					pListItem = pListItem->pNext;
				}

				pCombo->ActivateItemByRow((int)pObj->fdefValue);
				pCtrl->pControl = (Panel *)pCombo;
				break;
			}

			default: break;
		}

		if (pCtrl->type != O_BOOL)
		{
			pCtrl->pPrompt = new vgui2::Label(pCtrl, "DescLabel", "");
			pCtrl->pPrompt->SetContentAlignment(vgui2::Label::a_west);
			pCtrl->pPrompt->SetTextInset(5, 0);
			pCtrl->pPrompt->SetText(pObj->prompt);
		}

		pCtrl->pScrObj = pObj;
		pCtrl->SetSize(100, 28);
		m_pListPanel->AddItem(pCtrl);

		if (!m_pList)
		{
			m_pList = pCtrl;
			pCtrl->next = NULL;
		}
		else
		{
			mpcontrol_t *p = m_pList;

			while (p)
			{
				if (!p->next)
				{
					p->next = pCtrl;
					pCtrl->next = NULL;
					break;
				}

				p = p->next;
			}
		}

		pObj = pObj->pNext;
	}
}

void CMultiplayerAdvancedDialog::DestroyControls(void)
{
	mpcontrol_t *p, *n;
	p = m_pList;

	while (p)
	{
		n = p->next;
		delete p->pControl;
		delete p->pPrompt;
		delete p;
		p = n;
	}

	m_pList = NULL;
}

void CMultiplayerAdvancedDialog::SaveValues(void)
{
	GatherCurrentValues();

	if (m_pDescription)
	{
		m_pDescription->WriteToConfig();

		FileHandle_t fp = vgui2::filesystem()->Open("user.scr", "wb");

		if (fp)
		{
			m_pDescription->WriteToScriptFile(fp);
			vgui2::filesystem()->Close(fp);
		}
	}
}

void CInfoDescription::WriteScriptHeader(void *fp)
{
	char am_pm[] = "AM";
	time_t timer = time(NULL);
	tm *tblock = localtime(&timer);

	vgui2::filesystem()->FPrintf(fp, (char *)getHint());
	vgui2::filesystem()->FPrintf(fp, "// Half-Life User Info Configuration Layout Script (stores last settings chosen, too)\r\n");
	vgui2::filesystem()->FPrintf(fp, "// File generated:  %.19s %s\r\n", asctime(tblock), am_pm);
	vgui2::filesystem()->FPrintf(fp, "//\r\n//\r\n// Cvar\t-\tSetting\r\n\r\n");
	vgui2::filesystem()->FPrintf(fp, "VERSION %.1f\r\n\r\n", SCRIPT_VERSION);
	vgui2::filesystem()->FPrintf(fp, "DESCRIPTION INFO_OPTIONS\r\n{\r\n");
}

void CInfoDescription::WriteFileHeader(void *fp)
{
	char am_pm[] = "AM";
	time_t timer = time(NULL);
	tm *tblock = localtime(&timer);

	vgui2::filesystem()->FPrintf(fp, "// Half-Life User Info Configuration Settings\r\n");
	vgui2::filesystem()->FPrintf(fp, "// DO NOT EDIT, GENERATED BY HALF-LIFE\r\n");
	vgui2::filesystem()->FPrintf(fp, "// File generated:  %.19s %s\r\n", asctime(tblock), am_pm);
	vgui2::filesystem()->FPrintf(fp, "//\r\n//\r\n// Cvar\t-\tSetting\r\n\r\n");
}

CInfoDescription::CInfoDescription(CPanelListPanel *panel) : CDescription(panel)
{
	setHint("// NOTE:  THIS FILE IS AUTOMATICALLY REGENERATED, \r\n//DO NOT EDIT THIS HEADER, YOUR COMMENTS WILL BE LOST IF YOU DO\r\n// User options script\r\n//\r\n// Format:\r\n//  Version [float]\r\n//  Options description followed by \r\n//  Options defaults\r\n//\r\n// Option description syntax:\r\n//\r\n//  \"cvar\" { \"Prompt\" { type [ type info ] } { default } }\r\n//\r\n//  type = \r\n//   BOOL   (a yes/no toggle)\r\n//   STRING\r\n//   NUMBER\r\n//   LIST\r\n//\r\n// type info:\r\n// BOOL                 no type info\r\n// NUMBER       min max range, use -1 -1 for no limits\r\n// STRING       no type info\r\n// LIST          delimited list of options value pairs\r\n//\r\n//\r\n// default depends on type\r\n// BOOL is \"0\" or \"1\"\r\n// NUMBER is \"value\"\r\n// STRING is \"value\"\r\n// LIST is \"index\", where index \"0\" is the first element of the list\r\n\r\n\r\n");
	setDescription("INFO_OPTIONS");
}