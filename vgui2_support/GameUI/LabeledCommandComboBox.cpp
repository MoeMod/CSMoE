#include "EngineInterface.h"
#include "LabeledCommandComboBox.h"
#include <KeyValues.h>
#include <vgui/ILocalize.h>

CLabeledCommandComboBox::CLabeledCommandComboBox(vgui2::Panel *parent, const char *panelName) : vgui2::ComboBox(parent, panelName, 6, false)
{
	AddActionSignalTarget(this);
	m_iCurrentSelection = -1;
	m_iStartSelection = -1;
}

CLabeledCommandComboBox::~CLabeledCommandComboBox(void)
{
}

void CLabeledCommandComboBox::DeleteAllItems(void)
{
	BaseClass::DeleteAllItems();
	m_Items.RemoveAll();
}

void CLabeledCommandComboBox::AddItem(char const *text, char const *engineCommand)
{
	int idx = m_Items.AddToTail();
	COMMANDITEM *item = &m_Items[idx];

	item->comboBoxID = BaseClass::AddItem(text, NULL);

	Q_strncpy(item->name, text, sizeof(item->name));

	if (text[0] == '#')
	{
		wchar_t *localized = vgui2::localize()->Find(text);

		if (localized)
			vgui2::localize()->ConvertUnicodeToANSI(localized, item->name, sizeof(item->name));
	}

	Q_strncpy(item->command, engineCommand, sizeof(item->command));
}

void CLabeledCommandComboBox::ActivateItem(int index)
{
	if (index < m_Items.Count())
	{
		int comboBoxID = m_Items[index].comboBoxID;
		BaseClass::ActivateItem(comboBoxID);
		m_iCurrentSelection = index;
	}
}

void CLabeledCommandComboBox::SetInitialItem(int index)
{
	if (index< m_Items.Count())
	{
		m_iStartSelection = index;
		int comboBoxID = m_Items[index].comboBoxID;
		ActivateItem(comboBoxID);
	}
}

void CLabeledCommandComboBox::OnTextChanged(char const *text)
{
	for (int i = 0; i < m_Items.Size(); i++)
	{
		COMMANDITEM *item = &m_Items[i];

		if (!stricmp(item->name, text))
		{
			m_iCurrentSelection = i;
			break;
		}
	}

	if (HasBeenModified())
		PostActionSignal(new KeyValues("ControlModified"));
}

const char *CLabeledCommandComboBox::GetActiveItemCommand(void)
{
	if (m_iCurrentSelection == -1)
		return NULL;

	COMMANDITEM *item = &m_Items[m_iCurrentSelection];
	return item->command;
}

void CLabeledCommandComboBox::ApplyChanges(void)
{
	if (m_iCurrentSelection == -1)
		return;

	if (m_Items.Size() < 1)
		return;

	Assert(m_iCurrentSelection < m_Items.Size());
	COMMANDITEM *item = &m_Items[m_iCurrentSelection];
	engine->pfnClientCmd(item->command);
	m_iStartSelection = m_iCurrentSelection;
}

bool CLabeledCommandComboBox::HasBeenModified(void)
{
	return m_iStartSelection != m_iCurrentSelection;
}

void CLabeledCommandComboBox::Reset(void)
{
	if (m_iStartSelection != -1)
		ActivateItem(m_iStartSelection);
}