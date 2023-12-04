/***********************************************
	CTexturedMixButton(From ...)
	Build in 2023/11
************************************************/

#include "CTexturedMixButton.h"
#include "GameUI/InventoryMenu_V2/Util/Util.h"

using namespace vgui2;

static const char* g_ButtonPicElement[3] = { "Left", "Center", "Right" };
static const char* g_ButtonPicState[3] = { "N", "O", "C" };

#define PIC_DEFAULT	0
#define PIC_ARMED	1
#define PIC_DEPRESSED 2


const char* CTexturedMixButton::GetButtonResourceName(const char* buttonControlName, int i, int j, IScheme* pScheme)
{
	static char resourceName[32];

	if (!pScheme)
		return NULL;

	if (i >= 0 && j >= 0 && i < 3 && j < 3)
	{
		if (!buttonControlName)
			_snprintf(resourceName, 32, "Button/%s%s", g_ButtonPicElement[i], g_ButtonPicState[j]);
		else
			_snprintf(resourceName, 32, "%s/%s%s", buttonControlName, g_ButtonPicElement[i], g_ButtonPicState[j]);
	}
	const char* value = pScheme->GetResourceString(resourceName);

	return *value ? value : NULL;
}


void CTexturedMixButton::GetNinePics(const char* buttonControlName, IImage* pics[3][3], vgui2::IScheme* pScheme)
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			const char* name = GetButtonResourceName(buttonControlName, i, j, pScheme);

			if (name)
				pics[j][i] = scheme()->GetImage(name, true);
			else
				pics[j][i] = NULL;
		}
	}
}

void CTexturedMixButton::GetNinePics(const char* path, IImage* pics[3][3], const char identifier)
{
	static char fullpath[MAX_PATH];

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if (identifier)
				_snprintf(fullpath, MAX_PATH, "%s_%s%c%s", path, g_ButtonPicElement[i], identifier, g_ButtonPicState[j]);
			else
				_snprintf(fullpath, MAX_PATH, "%s_%s%s", path, g_ButtonPicElement[i], g_ButtonPicState[j]);

			pics[j][i] = scheme()->GetImage(fullpath, true);
		}
	}
}

void CTexturedMixButton::SetTgaGroupPath(const char* path, const char identifier)
{
	_picType = BUTTON_PIC_THREE;

	if (path)
	{
		GetNinePics(path, _pic, identifier);
	}
	else
	{
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				_pic[i][j] = NULL;
			}
		}
	}
}

void CTexturedMixButton::SetArmedPic(const char* path)
{
	if (path)
	{
		_picType = BUTTON_PIC_SINGLE;

		_pic[PIC_ARMED][0] = scheme()->GetImage(path, true);
	}
	else
	{
		_picType = BUTTON_PIC_THREE;

		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				_pic[i][j] = NULL;
			}
		}
	}
}

void CTexturedMixButton::SetDepressedPic(const char* path)
{
	if (path)
	{
		_picType = BUTTON_PIC_SINGLE;

		_pic[PIC_DEPRESSED][0] = scheme()->GetImage(path, true);
	}
	else
	{
		_picType = BUTTON_PIC_THREE;

		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				_pic[i][j] = NULL;
			}
		}
	}
}

void CTexturedMixButton::SetDefaultPic(const char* path)
{
	if (path)
	{
		_picType = BUTTON_PIC_SINGLE;

		_pic[PIC_DEFAULT][0] = scheme()->GetImage(path, true);
	}
	else
	{
		_picType = BUTTON_PIC_THREE;

		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				_pic[i][j] = NULL;
			}
		}
	}
}

void CTexturedMixButton::SetArmedPicGroup(const char* path, const char identifier)
{
	_picType = BUTTON_PIC_THREE;

	if (path)
	{
		static char fullpath[MAX_PATH];

		for (int i = 0; i < 3; i++)
		{
			_snprintf(fullpath, MAX_PATH, "%s_%s%c%s", path, g_ButtonPicElement[i], identifier, g_ButtonPicState[PIC_ARMED]);

			_pic[PIC_ARMED][i] = scheme()->GetImage(fullpath, true);
		}
	}
	else
	{
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				_pic[i][j] = NULL;
			}
		}
	}
}

void CTexturedMixButton::SetDepressedPicGroup(const char* path, const char identifier)
{
	_picType = BUTTON_PIC_THREE;

	if (path)
	{
		static char fullpath[MAX_PATH];

		for (int i = 0; i < 3; i++)
		{
			_snprintf(fullpath, MAX_PATH, "%s_%s%c%s", path, g_ButtonPicElement[i], identifier, g_ButtonPicState[PIC_DEPRESSED]);

			_pic[PIC_DEPRESSED][i] = scheme()->GetImage(fullpath, true);
		}
	}
	else
	{
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				_pic[i][j] = NULL;
			}
		}
	}
}

void CTexturedMixButton::SetDefaultPicGroup(const char* path, const char identifier)
{
	_picType = BUTTON_PIC_THREE;

	if (path)
	{
		static char fullpath[MAX_PATH];

		for (int i = 0; i < 3; i++)
		{
			_snprintf(fullpath, MAX_PATH, "%s_%s%c%s", path, g_ButtonPicElement[i], identifier, g_ButtonPicState[PIC_DEFAULT]);

			_pic[PIC_DEFAULT][i] = scheme()->GetImage(fullpath, true);
		}
	}
	else
	{
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				_pic[i][j] = NULL;
			}
		}
	}
}