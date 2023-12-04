/***********************************************
	CWeaponListRightPanel
	Build in 2023/11
************************************************/

#include "GameUI/InventoryMenu_V2/WeaponEnchant/CWeaponEnchant.h"
#include "CWeaponListRightPanel.h"
#include "GameUI/InventoryMenu_V2/Util/Util.h"

void CWeaponListRightPanel::OnPageTabActivated(Panel* pageTab)
{
	((CWeaponEnchant*)(GetParent()->GetParent()))->OnPageTabActivated(this);
}