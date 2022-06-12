#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui/IVGui.h>

#include <tier1/KeyValues.h>

#include "RoundPanel.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>


using namespace vgui2;
//
//vgui2::Panel* RoundPanel_Factory()
//{
//	return new RoundPanel(NULL, NULL, RoundPanel::TOOLTIP_LIST);
//}
//
//DECLARE_BUILD_FACTORY_CUSTOM(RoundPanel, RoundPanel_Factory);

namespace vgui2
{

RoundPanel::RoundPanel(Panel *parent, const char *panelName, ImageType type) : Panel(parent, panelName)
{
	SetImage(type);
}

RoundPanel::~RoundPanel(void)
{
}

void RoundPanel::SetImage(ImageType type)
{
	const char *szImage[NUM_TYPES][3][3] = 
	{
		{
			{ NULL, NULL, NULL },
			{ NULL, NULL, NULL },
			{ NULL, NULL, NULL }
		},
		{
			{ "resource/control/selected/Selected_top_left", "resource/control/selected/Selected_top_center", "resource/control/selected/Selected_top_right" },
			{ "resource/control/selected/Selected_center_left", "resource/control/selected/Selected_center_center", "resource/control/selected/Selected_center_right" },
			{ "resource/control/selected/Selected_bottom_left", "resource/control/selected/Selected_bottom_center", "resource/control/selected/Selected_bottom_right" }
		},
		{
			{ "resource/control/Frame_Dehilight/window_default_top_left", "resource/control/Frame_Dehilight/window_default_top_center", "resource/control/Frame_Dehilight/window_default_top_right" },
			{ "resource/control/Frame_Dehilight/window_default_center_left", "resource/control/Frame_Dehilight/window_default_center_center", "resource/control/Frame_Dehilight/window_default_center_right" },
			{ "resource/control/Frame_Dehilight/window_default_bottom_left", "resource/control/Frame_Dehilight/window_default_bottom_center", "resource/control/Frame_Dehilight/window_default_bottom_right" }
		},
		{
			{ "resource/control/Frame_Dehilight/window_default_top_left_noround", "resource/control/Frame_Dehilight/window_default_top_center", "resource/control/Frame_Dehilight/window_default_top_right" },
			{ "resource/control/Frame_Dehilight/window_default_center_left", "resource/control/Frame_Dehilight/window_default_center_center", "resource/control/Frame_Dehilight/window_default_center_right" },
			{ "resource/control/Frame_Dehilight/window_default_bottom_left", "resource/control/Frame_Dehilight/window_default_bottom_center", "resource/control/Frame_Dehilight/window_default_bottom_right" }
		},
		{
			{ "resource/control/TabButton/verticaltab_on_top_left", "resource/control/TabButton/verticaltab_on_top_center", NULL },
			{ "resource/control/TabButton/verticaltab_on_center_left", NULL, NULL },
			{ "resource/control/TabButton/verticaltab_on_bottom_left", "resource/control/TabButton/verticaltab_on_bottom_center", NULL }
		},
		{
			{ "resource/control/selected/Room_Selected_top_left", "resource/control/selected/Room_Selected_top_center", "resource/control/selected/Room_Selected_top_right" },
			{ "resource/control/selected/Room_Selected_center_left", "resource/control/selected/Room_Selected_center_center", "resource/control/selected/Room_Selected_center_right" },
			{ "resource/control/selected/Room_Selected_bottom_left", "resource/control/selected/Room_Selected_bottom_center", "resource/control/selected/Room_Selected_bottom_right" }
		},
		{
			{ "resource/control/floatingmenu/pulldown_top_left@n", "resource/control/floatingmenu/pulldown_top_center@n", "resource/control/floatingmenu/pulldown_top_right@n" },
			{ "resource/control/floatingmenu/pulldown_center_left@n", "resource/control/floatingmenu/pulldown_center_center@n", "resource/control/floatingmenu/pulldown_center_right@n" },
			{ "resource/control/floatingmenu/pulldown_bottom_left@n", "resource/control/floatingmenu/pulldown_bottom_center@n", "resource/control/floatingmenu/pulldown_bottom_right@n" }
		},
		{
			{ "resource/control/floatingmenu/window_balloon_top_left", "resource/control/floatingmenu/window_balloon_top_center", "resource/control/floatingmenu/window_balloon_top_right" },
			{ "resource/control/floatingmenu/window_balloon_center_left", "resource/control/floatingmenu/window_balloon_center_center", "resource/control/floatingmenu/window_balloon_center_left" },
			{ "resource/control/floatingmenu/window_balloon_bottom_left", "resource/control/floatingmenu/window_balloon_top_center", "resource/control/floatingmenu/window_balloon_bottom_Right" }
		},
		{
			{ "resource/control/floatingmenu/window_buff_top_left", "resource/control/floatingmenu/window_buff_top_center", "resource/control/floatingmenu/window_buff_top_right" },
			{ "resource/control/floatingmenu/window_buff_center_left", "resource/control/floatingmenu/window_buff_center_center", "resource/control/floatingmenu/window_buff_center_right" },
			{ "resource/control/floatingmenu/window_buff_bottom_left", "resource/control/floatingmenu/window_buff_bottom_center", "resource/control/floatingmenu/window_buff_bottom_right" }
		},
		{
			{ "resource/control/TextEntry/Output_default_top_left@n", "resource/control/TextEntry/Output_default_top_center@n", "resource/control/TextEntry/Output_default_top_right@n" },
			{ "resource/control/TextEntry/Output_default_center_left@n", "resource/control/TextEntry/Output_default_center_center@n", "resource/control/TextEntry/Output_default_center_right@n" },
			{ "resource/control/TextEntry/Output_default_bottom_left@n", "resource/control/TextEntry/Output_default_bottom_center@n", "resource/control/TextEntry/Output_default_bottom_right@n" },
		},
		{
			{ "resource/Control/Selected/Over_top_left", "resource/Control/Selected/Over_top_center", "resource/Control/Selected/Over_top_right" },
			{ "resource/Control/Selected/Over_center_left", "resource/Control/Selected/Over_center_center", "resource/Control/Selected/Over_center_right" },
			{ "resource/Control/Selected/Over_bottom_left", "resource/Control/Selected/Over_bottom_center", "resource/Control/Selected/Over_bottom_right" }
		},
		{
			{ "resource/Control/Selected/Disable_top_left", "resource/Control/Selected/Disable_top_center", "resource/Control/Selected/Disable_top_right" },
			{ "resource/Control/Selected/Disable_center_left", "resource/Control/Selected/Disable_center_center", "resource/Control/Selected/Disable_center_right" },
			{ "resource/Control/Selected/Disable_bottom_left", "resource/Control/Selected/Disable_bottom_center", "resource/Control/Selected/Disable_bottom_right" }
		},
		{
			{ "resource/control/TextEntry/Tooltip_default_top_left@n", "resource/control/TextEntry/Tooltip_default_top_center@n", "resource/control/TextEntry/Tooltip_default_top_right@n" },
			{ "resource/control/TextEntry/Tooltip_default_center_left@n", "resource/control/TextEntry/Tooltip_default_center_center@n", "resource/control/TextEntry/Tooltip_default_center_right@n" },
			{ "resource/control/TextEntry/Tooltip_default_bottom_left@n", "resource/control/TextEntry/Tooltip_default_bottom_center@n", "resource/control/TextEntry/Tooltip_default_bottom_right@n" }
		},
		{
			{ "resource/zombieux/Disable_top_left", "resource/zombieux/Disable_top_center", "resource/zombieux/Disable_top_right" },
			{ "resource/zombieux/Disable_center_left", "resource/zombieux/Disable_center_center", "resource/zombieux/Disable_center_right" },
			{ "resource/zombieux/Disable_bottom_left", "resource/zombieux/Disable_bottom_center", "resource/zombieux/Disable_bottom_right" }
		},
		{
			{ "resource/Control/Selected/Room_Selected_top_left_red", "resource/Control/Selected/Room_Selected_top_center_red", "resource/Control/Selected/Room_Selected_top_right_red" },
			{ "resource/Control/Selected/Room_Selected_center_left_red", "resource/Control/Selected/Room_Selected_center_center_red", "resource/Control/Selected/Room_Selected_center_right_red" },
			{ "resource/Control/Selected/Room_Selected_bottom_left_red", "resource/Control/Selected/Room_Selected_bottom_center_red", "resource/Control/Selected/Room_Selected_bottom_right_red" }
		},
		{
			{ "resource/Control/Selected/Room_Selected_top_left_green", "resource/Control/Selected/Room_Selected_top_center_green", "resource/Control/Selected/Room_Selected_top_right_green" },
			{ "resource/Control/Selected/Room_Selected_center_left_green", "resource/Control/Selected/Room_Selected_center_center_green", "resource/Control/Selected/Room_Selected_center_right_green" },
			{ "resource/Control/Selected/Room_Selected_bottom_left_green", "resource/Control/Selected/Room_Selected_bottom_center_green", "resource/Control/Selected/Room_Selected_bottom_right_green" }
		},
		{
			{ "resource/Control/Selected/Room_Selected_top_left_blue", "resource/Control/Selected/Room_Selected_top_center_blue", "resource/Control/Selected/Room_Selected_top_right_blue" },
			{ "resource/Control/Selected/Room_Selected_center_left_blue", "resource/Control/Selected/Room_Selected_center_center_blue", "resource/Control/Selected/Room_Selected_center_right_blue" },
			{ "resource/Control/Selected/Room_Selected_bottom_left_blue", "resource/Control/Selected/Room_Selected_bottom_center_blue", "resource/Control/Selected/Room_Selected_bottom_right_blue" }
		},
		{
			{ "resource/Control/Button_List/9/Tootip_List_top_left@n", "resource/Control/Button_List/9/Tootip_List_top_center@n", "resource/Control/Button_List/9/Tootip_List_top_right@n" },
			{ "resource/Control/Button_List/9/Tootip_List_center_left@n", "resource/Control/Button_List/9/Tootip_List_center_center@n", "resource/Control/Button_List/9/Tootip_List_center_right@n" },
			{ "resource/Control/Button_List/9/Tootip_List_bottom_left@n", "resource/Control/Button_List/9/Tootip_List_bottom_center@n", "resource/Control/Button_List/9/Tootip_List_bottom_right@n" }
		}
	};

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if (!szImage[type][i][j])
			{
				m_pPics[i][j] = NULL;

				continue;
			}

			m_pPics[i][j] = scheme()->GetImage(szImage[type][i][j], true);
		}
	}
}

void RoundPanel::Paint(void)
{
	if (!m_pPics[0][0])
		return;

	if (!m_pPics[1][0])
		return;

	if (!m_pPics[2][0])
		return;

	int wide, tall;
	int left, right;
	int top, bottom;
	int y;

	GetSize(wide, tall);
	
	m_pPics[0][0]->GetSize(left, top);

	if (m_pPics[0][2])
		m_pPics[0][2]->GetSize(right, top);
	else
		right = 0;

	m_pPics[0][0]->SetColor(Color(255, 255, 255, 255));
	m_pPics[0][0]->SetPos(0, 0);
	m_pPics[0][0]->Paint();

	if (m_pPics[0][1])
	{
		m_pPics[0][1]->SetColor(Color(255, 255, 255, 255));
		m_pPics[0][1]->SetPos(left, 0);
		m_pPics[0][1]->SetSize(wide - left - right, top);
		m_pPics[0][1]->Paint();
	}
	
	if (m_pPics[0][2])
	{
		m_pPics[0][2]->SetColor(Color(255, 255, 255, 255));
		m_pPics[0][2]->SetPos(wide - right, 0);
		m_pPics[0][2]->Paint();
	}

	y = top;

	m_pPics[1][0]->GetSize(left, top);

	if (m_pPics[1][2])
		m_pPics[1][2]->GetSize(right, top);
	else
		right = 0;

	m_pPics[2][0]->GetSize(bottom, bottom);

	m_pPics[1][0]->SetColor(Color(255, 255, 255, 255));
	m_pPics[1][0]->SetPos(0, y);
	m_pPics[1][0]->SetSize(left, tall - y - bottom);
	m_pPics[1][0]->Paint();

	if (m_pPics[1][1])
	{
		m_pPics[1][1]->SetColor(Color(255, 255, 255, 255));
		m_pPics[1][1]->SetPos(left, y);
		m_pPics[1][1]->SetSize(wide - left - right, tall - y - bottom);
		m_pPics[1][1]->Paint();
	}
	
	if (m_pPics[1][2])
	{
		m_pPics[1][2]->SetColor(Color(255, 255, 255, 255));
		m_pPics[1][2]->SetPos(wide - right, y);
		m_pPics[1][2]->SetSize(right, tall - y - bottom);
		m_pPics[1][2]->Paint();
	}

	m_pPics[2][0]->GetSize(left, bottom);

	if (m_pPics[2][2])
		m_pPics[2][2]->GetSize(right, bottom);
	else
		right = 0;

	y = tall - bottom;

	m_pPics[2][0]->SetColor(Color(255, 255, 255, 255));
	m_pPics[2][0]->SetPos(0, y);
	m_pPics[2][0]->Paint();

	if (m_pPics[2][1])
	{
		m_pPics[2][1]->SetColor(Color(255, 255, 255, 255));
		m_pPics[2][1]->SetPos(left, y);
		m_pPics[2][1]->SetSize(wide - left - right, bottom);
		m_pPics[2][1]->Paint();
	}

	if (m_pPics[2][2])
	{
		m_pPics[2][2]->SetColor(Color(255, 255, 255, 255));
		m_pPics[2][2]->SetPos(wide - right, y);
		m_pPics[2][2]->Paint();
	}
}
}