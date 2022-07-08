#ifndef LABELEDCOMMANDCOMBOBOX_H
#define LABELEDCOMMANDCOMBOBOX_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/ComboBox.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Panel.h>
#include "UtlVector.h"

class CLabeledCommandComboBox : public vgui2::ComboBox
{
	DECLARE_CLASS_SIMPLE(CLabeledCommandComboBox, vgui2::ComboBox);

public:
	CLabeledCommandComboBox(vgui2::Panel *parent, const char *panelName);
	~CLabeledCommandComboBox(void);

public:
	virtual void DeleteAllItems(void);
	virtual void AddItem(char const *text, char const *engineCommand);
	virtual void ActivateItem(int itemIndex);

public:
	const char *GetActiveItemCommand(void);
	void SetInitialItem(int itemIndex);
	void ApplyChanges(void);
	void Reset(void);
	bool HasBeenModified(void);

public:
	enum
	{
		MAX_NAME_LEN = 256,
		MAX_COMMAND_LEN = 256
	};

private:
	MESSAGE_FUNC_CHARPTR(OnTextChanged, "TextChanged", text);

private:
	struct COMMANDITEM
	{
		char name[MAX_NAME_LEN];
		char command[MAX_COMMAND_LEN];
		int comboBoxID;
	};

private:
	CUtlVector<COMMANDITEM> m_Items;
	int m_iCurrentSelection;
	int m_iStartSelection;
};

#endif