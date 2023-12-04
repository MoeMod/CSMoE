/***********************************************
	CSearchBar
	Build in 2023/11
************************************************/

#include "CSearchBar.h"
#include "GameUI/InventoryMenu_V2/Util/Util.h"

void CSearchBar::OnMenuItemSelected()
{
	wchar_t temp[64];
	GetItemText(GetActiveItem(), temp, 63);
	SetText(temp);
	PostActionSignal(new KeyValues("TextChanged"));

	BaseClass::OnMenuItemSelected();
	HideMenu();
}