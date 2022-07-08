// vim: set ts=4 sw=4 tw=99 noet:
//
// AMX Mod X, based on AMX Mod by Aleksander Naszko ("OLO").
// Copyright (C) The AMX Mod X Development Team.
//
// This software is licensed under the GNU General Public License, version 3 or higher.
// Additional exceptions apply. For full license details, see LICENSE.txt or visit:
//     https://alliedmods.net/amxmodx-license

#ifndef _INCLUDE_NEWMENUS_H
#define _INCLUDE_NEWMENUS_H
#ifdef _WIN32
#pragma once
#endif

namespace sv {

#define MENU_TIMEOUT	-4
#define	MENU_EXIT		-3
#define	MENU_BACK		-2
#define	MENU_MORE		-1

class BlankItem
{
private:
	char* m_text;
	bool m_num;
public:
	BlankItem() : m_text(NULL), m_num(false) { }
	BlankItem(BlankItem& src) { this->copyFrom(src); }
	~BlankItem() { free(m_text); }

	void copyFrom(BlankItem& src)
	{
		m_text = src.m_text;
		m_num = src.m_num;
		src.m_text = NULL; // stop the src from freeing the buffer
	}
	BlankItem& operator = (const BlankItem& src) { this->copyFrom(const_cast<BlankItem&>(src)); return *this; }

	/* is this text instead of a blank */
	bool IsText() { return m_text != NULL; }

	/* is this a blank instead of text */
	bool IsBlank() { return m_text == NULL; }

	/* does this item take up a number */
	bool EatNumber() { return m_num; }

	/* the text this item is to display */
	const char* GetDisplay() { return m_text == NULL ? "" : m_text; }

	/* sets this item to use a blank */
	void SetBlank() { free(m_text); m_text = NULL; }

	/* sets this item to display text */
	void SetText(const char* text) { free(m_text); m_text = strdup(text); }

	/* sets whether or not this item takes up a line */
	void SetEatNumber(bool val) { m_num = val; }

};
struct menuitem
{
	std::string name;
	std::string cmd;
	bool isBlank;
	size_t id;
	std::vector<BlankItem> blanks;
};

typedef unsigned int menu_t;
typedef unsigned int item_t;
typedef unsigned int page_t;

class PlayerMenu
{
public:
	PlayerMenu(const char* title, const std::function<void(CBasePlayer*, PlayerMenu*, int)> &handler);
	~PlayerMenu();

	menuitem* GetMenuItem(item_t item);
	size_t GetPageCount();
	size_t GetItemCount();
	menuitem* AddItem(std::string name, std::string cmd = "");
	void AddBlank();

	const char* GetTextString(CBasePlayer* pPlayer, page_t page, int& keys);
	bool Display(CBasePlayer* pPlayer, page_t page = 0, duration_t menuexpire = 0s);
	void Close(CBasePlayer* pPlayer);
	void Destroy();

	int PagekeyToItem(page_t page, item_t key);
public:
	std::vector<menuitem* > m_Items;
	std::string m_Title;
	std::string m_Text;

	std::string m_OptNames[4];

	std::string m_ItemColor;
	bool m_NeverExit;
	bool m_ForceExit;
	bool m_AutoColors;

	int thisId;
	std::function<void(CBasePlayer*, PlayerMenu*, int)> func;
	bool isDestroying;
public:
	unsigned int items_per_page;
};

// *****************************************************
// class MenuMngr
// *****************************************************

class MenuMngr
{
public:
	MenuMngr();
	~MenuMngr();

	// Interface
	void clear();
	PlayerMenu* get_menu_by_id(int id);

	PlayerMenu* menu_create(const char* title, const std::function<void(CBasePlayer*, PlayerMenu*, int)>& handler);
	bool menu_destroy(PlayerMenu* pMenu);
	bool menu_destroy(int id);

public:
	std::vector<PlayerMenu*> m_NewMenus;
	std::stack<int> m_MenuFreeStack;
};

extern MenuMngr g_MenuMgr;

}
#endif //_INCLUDE_NEWMENUS_H
