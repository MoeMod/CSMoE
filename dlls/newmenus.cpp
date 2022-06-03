// vim: set ts=4 sw=4 tw=99 noet:
//
// AMX Mod X, based on AMX Mod by Aleksander Naszko ("OLO").
// Copyright (C) The AMX Mod X Development Team.
//
// This software is licensed under the GNU General Public License, version 3 or higher.
// Additional exceptions apply. For full license details, see LICENSE.txt or visit:
//     https://alliedmods.net/amxmodx-license

#include "newmenus.h"
#include "util.h"
#include <string>
#include <vector>
#include <stack>

namespace sv {

/* warning - don't pass here const string */
void UTIL_ShowMenu(CBasePlayer* pPlayer, int slots, int time, char* menu, int mlen)
{
	char* n = menu;
	char c = 0;
	int a;

	if (!gmsgShowMenu)
		return;			// some games don't support ShowMenu (Firearms)

	do
	{
		a = mlen;
		if (a > 175) a = 175;
		mlen -= a;
		c = *(n += a);
		*n = 0;

		MESSAGE_BEGIN(MSG_ONE, gmsgShowMenu, NULL, pPlayer->pev);
		WRITE_SHORT(slots);
		WRITE_CHAR(time);
		WRITE_BYTE(c ? TRUE : FALSE);
		WRITE_STRING(menu);
		MESSAGE_END();
		*n = c;
		menu = n;
	} while (*n);
}

size_t UTIL_Format(char* buffer, size_t maxlength, const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	size_t len = vsnprintf(buffer, maxlength, fmt, ap);
	va_end(ap);

	if (len >= maxlength)
	{
		buffer[maxlength - 1] = '\0';
		return (maxlength - 1);
	}
	else
	{
		return len;
	}
}

PlayerMenu::PlayerMenu(const char* title, const std::function<void(CBasePlayer*, PlayerMenu*, int)>& handler) : m_Title(title), m_ItemColor("\\r"),
m_NeverExit(false), m_AutoColors(true), thisId(0), func(handler),
isDestroying(false), items_per_page(7)
{
	m_OptNames[abs(MENU_BACK)].assign("返回");
	m_OptNames[abs(MENU_MORE)].assign("更多");
	m_OptNames[abs(MENU_EXIT)].assign("退出");
}

PlayerMenu::~PlayerMenu()
{
	for (size_t i = 0; i < m_Items.size(); i++)
	{
		delete m_Items[i];
	}

	m_Items.clear();
}

menuitem* PlayerMenu::AddItem(std::string name, std::string cmd)
{
	/*if (!this->items_per_page && this->GetItemCount() >= 10)
	{
		LogError(amx, AMX_ERR_NATIVE, "Non-paginated menus are limited to 10 items.");
		return nullptr;
	}*/

	menuitem* pItem = new menuitem;

	pItem->name.assign(name);
	pItem->cmd.assign(cmd);
	pItem->id = m_Items.size();
	pItem->isBlank = false;

	m_Items.push_back(pItem);

	return pItem;
}

void PlayerMenu::AddBlank()
{
	menuitem* pItem = AddItem("");
	pItem->isBlank = true;
}

menuitem* PlayerMenu::GetMenuItem(item_t item)
{
	if (item >= m_Items.size())
		return NULL;

	return m_Items[item];
}

size_t PlayerMenu::GetItemCount()
{
	return m_Items.size();
}

size_t PlayerMenu::GetPageCount()
{
	size_t items = GetItemCount();
	if (items_per_page == 0)
	{
		return 1;
	}

	return ((items / items_per_page) + ((items % items_per_page) ? 1 : 0));
}

int PlayerMenu::PagekeyToItem(page_t page, item_t key)
{
	size_t start = page * items_per_page;
	size_t num_pages = GetPageCount();

	if (num_pages == 1 || !items_per_page)
	{
		if (key > m_Items.size())
		{
			return MENU_EXIT;
		}
		else {
			return key - 1;
		}
	}
	else {
		//first page
		if (page == 0)
		{
			/* The algorithm for spaces here is same as a middle page. */
			item_t new_key = key;
			for (size_t i = start; i < (start + key - 1) && i < m_Items.size(); i++)
			{
				for (size_t j = 0; j < m_Items[i]->blanks.size(); j++)
				{
					if (m_Items[i]->blanks[j].EatNumber())
					{
						if (!new_key)
						{
							break;
						}
						new_key--;
					}
					if (!new_key)
					{
						break;
					}
				}
			}
			key = new_key;
			if (key == items_per_page + 2)
			{
				return MENU_MORE;
			}
			else if (key == items_per_page + 3) {
				return MENU_EXIT;
			}
			else {
				return (start + key - 1);
			}
		}
		else if (page == num_pages - 1) {
			//last page
			item_t item_tracker = 0; //  tracks how many valid items we have passed so far.
			size_t remaining = m_Items.size() - start;
			item_t new_key = key;

			// For every item that takes up a slot (item or padded blank)
			// we subtract one from new key.
			// For every item (not blanks), we increase item_tracker.
			// When new_key equals 0, item_tracker will then be set to
			// whatever valid item was selected.
			for (size_t i = m_Items.size() - remaining; i < m_Items.size(); i++)
			{
				item_tracker++;

				if (new_key <= 1) // If new_key is 0, or will be 0 after the next decrease
				{
					new_key = 0;
					break;
				}

				new_key--;

				for (size_t j = 0; j < m_Items[i]->blanks.size(); j++)
				{
					if (m_Items[i]->blanks[j].EatNumber())
					{
						new_key--;
					}
					if (!new_key)
					{
						break;
					}
				}
			}
			// If new_key doesn't equal zero, then a back/exit button was pressed.
			if (new_key != 0)
			{
				if (key == items_per_page + 1)
				{
					return MENU_BACK;
				}
				else if (key == items_per_page + 3)
				{
					return MENU_EXIT;
				}
				// MENU_MORE should never happen here.
			}
			// otherwise our item is now start + item_tracker - 1
			return (start + item_tracker - 1);
		}
		else {
			/* The algorithm for spaces here is a bit harder.  We have to subtract
			 * one from the key for each space we find along the way.
			 */
			item_t new_key = key;
			for (size_t i = start; i < (start + items_per_page - 1) && i < m_Items.size(); i++)
			{
				for (size_t j = 0; j < m_Items[i]->blanks.size(); j++)
				{
					if (m_Items[i]->blanks[j].EatNumber())
					{
						if (!new_key)
						{
							break;
						}
						new_key--;
					}
					if (!new_key)
					{
						break;
					}
				}
			}
			key = new_key;
			if (key > items_per_page && (key - items_per_page <= 3))
			{
				unsigned int num = key - items_per_page - 1;
				static int map[] = { MENU_BACK, MENU_MORE, MENU_EXIT };
				return map[num];
			}
			else {
				return (start + key - 1);
			}
		}
	}
}

bool PlayerMenu::Display(CBasePlayer* pPlayer, page_t page, duration_t menuexpire)
{
	int keys = 0;
	const char* str = GetTextString(pPlayer, page, keys);

	if (!str)
		return false;

	static char buffer[2048];
	int len = UTIL_Format(buffer, sizeof(buffer) - 1, "%s", str);

	pPlayer->m_iMenu = Menu_OFF;
	pPlayer->ClientCommand("menuselect", "10");

	pPlayer->m_iMenu = Menu_CustomMenu;
	pPlayer->m_iNewMenuKeys = keys;
	pPlayer->m_iNewMenuID = thisId;
	pPlayer->m_iNewMenuPage = (int)page;
	if (menuexpire == 0s)
		pPlayer->m_iNewMenuExpire = invalid_time_point;
	else
		pPlayer->m_iNewMenuExpire = gpGlobals->time + menuexpire;

	UTIL_ShowMenu(pPlayer, keys, -1, buffer, len);

	return true;
}

void PlayerMenu::Close(CBasePlayer* pPlayer)
{
	int status;
	if (gpGlobals->time > pPlayer->m_iNewMenuExpire)
		status = MENU_TIMEOUT;
	else
		status = MENU_EXIT;

	pPlayer->m_iMenu = Menu_OFF;
	pPlayer->m_iNewMenuKeys = 0;
	pPlayer->m_iNewMenuID = -1;

	func(pPlayer, this, status);
	Destroy();
}

void PlayerMenu::Destroy()
{
	if (this->isDestroying)
	{
		return;	//prevent infinite recursion
	}

	this->isDestroying = true;
	int id = this->thisId;
	g_MenuMgr.m_NewMenus[id] = NULL;
	delete this;
	g_MenuMgr.m_MenuFreeStack.push(id);
}

const char* PlayerMenu::GetTextString(CBasePlayer* pPlayer, page_t page, int& keys)
{
	page_t pages = GetPageCount();
	item_t numItems = GetItemCount();

	if (page >= pages)
		return NULL;

	m_Text.clear();

	char buffer[255];
	if (items_per_page && (pages != 1))
	{
		if (m_AutoColors)
			UTIL_Format(buffer, sizeof(buffer) - 1, "\\y%s %d/%d\n\\w\n", m_Title.c_str(), page + 1, pages);
		else
			UTIL_Format(buffer, sizeof(buffer) - 1, "%s %d/%d\n\n", m_Title.c_str(), page + 1, pages);
	}
	else {
		if (m_AutoColors)
			UTIL_Format(buffer, sizeof(buffer) - 1, "\\y%s\n\\w\n", m_Title.c_str());
		else
			UTIL_Format(buffer, sizeof(buffer) - 1, "%s\n\n", m_Title.c_str());
	}

	m_Text.append(buffer);

	enum
	{
		Display_Back = (1 << 0),
		Display_Next = (1 << 1),
	};

	int flags = Display_Back | Display_Next;

	item_t start = page * items_per_page;
	item_t end = 0;
	if (items_per_page)
	{
		if (start + items_per_page >= numItems)
		{
			end = numItems;
			flags &= ~Display_Next;
		}
		else {
			end = start + items_per_page;
		}
	}
	else {
		end = numItems;
		if (end > 10)
		{
			end = 10;
		}
	}

	if (page == 0)
	{
		flags &= ~Display_Back;
	}

	menuitem* pItem = NULL;

	int option = 0;
	keys = 0;
	bool enabled = true;
	int ret = 0;
	int slots = 0;
	int option_display = 0;

	for (item_t i = start; i < end; i++)
	{
		// reset enabled
		enabled = true;
		pItem = m_Items[i];

		if (pItem->isBlank)
		{
			enabled = false;
		}

		if (enabled)
		{
			keys |= (1 << option);
		}

		option_display = ++option;
		if (option_display == 10)
		{
			option_display = 0;
		}

		if (pItem->isBlank)
		{
			UTIL_Format(buffer, sizeof(buffer) - 1, "%s\n", pItem->name.c_str());
		}
		else if (enabled)
		{
			if (m_AutoColors)
			{
				UTIL_Format(buffer, sizeof(buffer) - 1, "%s%d.\\w %s\n", m_ItemColor.c_str(), option_display, pItem->name.c_str());
			}
			else {
				UTIL_Format(buffer, sizeof(buffer) - 1, "%d. %s\n", option_display, pItem->name.c_str());
			}
		}
		else {
			if (m_AutoColors)
			{
				UTIL_Format(buffer, sizeof(buffer) - 1, "\\d%d. %s\n\\w", option_display, pItem->name.c_str());
			}
			else {
				UTIL_Format(buffer, sizeof(buffer) - 1, "#. %s\n", pItem->name.c_str());
			}
		}
		slots++;

		m_Text.append(buffer);

		//attach blanks
		if (pItem->blanks.size())
		{
			for (size_t j = 0; j < pItem->blanks.size(); j++)
			{
				if (pItem->blanks[j].EatNumber())
				{
					option++;
				}
				m_Text.append(pItem->blanks[j].GetDisplay());
				m_Text.append("\n");
				slots++;
			}
		}
	}

	if (items_per_page)
	{
		/* Pad spaces until we reach the end of the max possible items */
		for (unsigned int i = (unsigned)slots; i < items_per_page; i++)
		{
			m_Text.append("\n");
			option++;
		}
		/* Make sure there is at least one visual pad */
		m_Text.append("\n");

		/* Don't bother if there is only one page */
		if (pages > 1)
		{
			if (flags & Display_Back)
			{
				keys |= (1 << option++);
				if (m_AutoColors)
				{
					UTIL_Format(buffer,
						sizeof(buffer) - 1,
						"%s%d. \\w%s\n",
						m_ItemColor.c_str(),
						option == 10 ? 0 : option,
						m_OptNames[abs(MENU_BACK)].c_str());
				}
				else {
					UTIL_Format(buffer,
						sizeof(buffer) - 1,
						"%d. %s\n",
						option == 10 ? 0 : option,
						m_OptNames[abs(MENU_BACK)].c_str());
				}
			}
			else {
				option++;
				if (m_AutoColors)
				{
					UTIL_Format(buffer,
						sizeof(buffer) - 1,
						"\\d%d. %s\n\\w",
						option == 10 ? 0 : option,
						m_OptNames[abs(MENU_BACK)].c_str());
				}
				else {
					UTIL_Format(buffer, sizeof(buffer) - 1, "#. %s\n", m_OptNames[abs(MENU_BACK)].c_str());
				}
			}
			m_Text.append(buffer);

			if (flags & Display_Next)
			{
				keys |= (1 << option++);
				if (m_AutoColors)
				{
					UTIL_Format(buffer,
						sizeof(buffer) - 1,
						"%s%d. \\w%s\n",
						m_ItemColor.c_str(),
						option == 10 ? 0 : option,
						m_OptNames[abs(MENU_MORE)].c_str());
				}
				else {
					UTIL_Format(buffer,
						sizeof(buffer) - 1,
						"%d. %s\n",
						option == 10 ? 0 : option,
						m_OptNames[abs(MENU_MORE)].c_str());
				}
			}
			else {
				option++;
				if (m_AutoColors)
				{
					UTIL_Format(buffer,
						sizeof(buffer) - 1,
						"\\d%d. %s\n\\w",
						option == 10 ? 0 : option,
						m_OptNames[abs(MENU_MORE)].c_str());
				}
				else {
					UTIL_Format(buffer, sizeof(buffer) - 1, "#. %s\n", m_OptNames[abs(MENU_MORE)].c_str());
				}
			}
			m_Text.append(buffer);
		}
		else {
			/* Keep padding */
			option += 2;
		}
	}

	if ((items_per_page && !m_NeverExit) || (m_ForceExit && numItems < 10))
	{
		/* Visual pad has not been added yet */
		if (!items_per_page)
			m_Text.append("\n");

		keys |= (1 << option++);
		if (m_AutoColors)
		{
			UTIL_Format(buffer,
				sizeof(buffer) - 1,
				"%s%d. \\w%s\n",
				m_ItemColor.c_str(),
				option == 10 ? 0 : option,
				m_OptNames[abs(MENU_EXIT)].c_str());
		}
		else {
			UTIL_Format(buffer,
				sizeof(buffer) - 1,
				"%d. %s\n",
				option == 10 ? 0 : option,
				m_OptNames[abs(MENU_EXIT)].c_str());
		}
		m_Text.append(buffer);
	}

	return m_Text.c_str();
}

// *****************************************************
// class MenuMngr
// *****************************************************

#define GETMENU_R(p) PlayerMenu *pMenu = g_MenuMgr.get_menu_by_id(p); \
	if (pMenu == NULL) { \
	UTIL_LogPrintf("Invalid menu id %d(%d)\n", p, g_MenuMgr.m_NewMenus.size()); \
	return false; }

MenuMngr::MenuMngr()
{
}

MenuMngr::~MenuMngr()
{
	clear();
}

void MenuMngr::clear()
{
	for (size_t i = 0; i < m_NewMenus.size(); i++)
	{
		delete m_NewMenus[i];
	}

	m_NewMenus.clear();
	while (!m_MenuFreeStack.empty())
	{
		m_MenuFreeStack.pop();
	}
}

PlayerMenu* MenuMngr::get_menu_by_id(int id)
{
	if (id < 0 || size_t(id) >= m_NewMenus.size() || !m_NewMenus[id])
		return NULL;

	return m_NewMenus[id];
}

PlayerMenu* MenuMngr::menu_create(const char* title, const std::function<void(CBasePlayer*, PlayerMenu*, int)> &handler)
{
	PlayerMenu* pMenu = new PlayerMenu(title, handler);

	if (m_MenuFreeStack.empty())
	{
		m_NewMenus.push_back(pMenu);
		pMenu->thisId = (int)m_NewMenus.size() - 1;
	}
	else {
		int pos = m_MenuFreeStack.top();
		m_MenuFreeStack.pop();
		m_NewMenus[pos] = pMenu;
		pMenu->thisId = pos;
	}

	return pMenu;
}

bool MenuMngr::menu_destroy(int id)
{
	GETMENU_R(id);
	menu_destroy(pMenu);
	return true;
}

bool MenuMngr::menu_destroy(PlayerMenu* pMenu)
{
	if (pMenu->isDestroying)
	{
		return false;	//prevent infinite recursion
	}
	
	pMenu->isDestroying = true;

	for (int i = 1; i <= gpGlobals->maxClients; ++i)
	{
		CBasePlayer* pPlayer = static_cast<CBasePlayer*>(UTIL_PlayerByIndex(i));
		if (pPlayer && pPlayer->m_iNewMenuID == pMenu->thisId)
		{
			pMenu->Close(pPlayer);
			return true; // menu is unique for each one;
		}
	}
	pMenu->Destroy();
	return true;
}

}