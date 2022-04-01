#include "EngineInterface.h"
#include "OptionsSubMultiplayer.h"
#include "MultiplayerAdvancedDialog.h"
#include <stdio.h>

#include <vgui_controls/Button.h>
#include <vgui_controls/CheckButton.h>
#include "tier1/KeyValues.h"
#include <vgui_controls/Label.h>
#include <vgui/ISystem.h>
#include <vgui/ISurface.h>
#include <vgui/Cursor.h>
#include <vgui_controls/RadioButton.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/FileOpenDialog.h>
#include <vgui_controls/MessageBox.h>
#include <vgui/IVgui.h>
#include <vgui/ILocalize.h>
#include <vgui/IPanel.h>
#include <vgui_controls/MessageBox.h>

#include "CvarTextEntry.h"
#include "CvarToggleCheckButton.h"
#include "CvarSlider.h"
#include "LabeledCommandComboBox.h"
#include "FileSystem.h"
#include "BitmapImagePanel.h"
#include "UtlBuffer.h"
#include "ModInfo.h"
#include "LogoFile.h"
#include "BMPUtils.h"

#include <memory>

struct ColorItem_t
{
	const char *name;
	int r, g, b;
};

static ColorItem_t itemlist[] =
{
	{ "#Valve_Orange", 255, 120, 24 },
	{ "#Valve_Yellow", 225, 180, 24 },
	{ "#Valve_Blue", 0, 60, 255 },
	{ "#Valve_Ltblue", 0, 167, 255 },
	{ "#Valve_Green", 0, 167, 0 },
	{ "#Valve_Red", 255, 43, 0 },
	{ "#Valve_Brown", 123, 73, 0 },
	{ "#Valve_Ltgray", 100, 100, 100 },
	{ "#Valve_Dkgray", 36, 36, 36 },
};

static ColorItem_t s_crosshairColors[] =
{
	{ "#Valve_Green", 50, 250, 50 },
	{ "#Valve_Red", 250, 50, 50 },
	{ "#Valve_Blue", 50, 50, 250 },
	{ "#Valve_Yellow", 250, 250, 50 },
	{ "#Valve_Ltblue", 50, 250, 250 }
};

static const int NumCrosshairColors = sizeof(s_crosshairColors) / sizeof(s_crosshairColors[0]);

class CrosshairImagePanel : public vgui2::ImagePanel
{
	typedef ImagePanel BaseClass;

public:
	CrosshairImagePanel(Panel *parent, const char *name, CCvarToggleCheckButton *pAdditive, CLabeledCommandComboBox *pCrosshairType);

public:
	virtual void Paint(void);

public:
	void UpdateCrosshair(int r, int g, int b, int size);

protected:
	int m_R, m_G, m_B;
	int m_barSize;
	int m_barGap;
	CCvarToggleCheckButton *m_pAdditive;
	CLabeledCommandComboBox *m_pCrosshairType;
};

CrosshairImagePanel::CrosshairImagePanel(Panel *parent, const char *name, CCvarToggleCheckButton *pAdditive, CLabeledCommandComboBox *pCrosshairType) : ImagePanel(parent, name)
{
	m_pAdditive = pAdditive;
	m_pCrosshairType = pCrosshairType;

	UpdateCrosshair(50, 250, 50, 0);
}

void CrosshairImagePanel::UpdateCrosshair(int r, int g, int b, int size)
{
	m_R = r;
	m_G = g;
	m_B = b;

	int screenWide, screenTall;
	vgui2::surface()->GetScreenSize(screenWide, screenTall);

	if (size == 0)
	{
		if (screenWide <= 640)
			size = 1;
		else if (screenWide < 1024)
			size = 2;
		else
			size = 3;
	}

	int scaleBase = 1024;

	switch (size)
	{
		case 3: scaleBase = 640; break;
		case 2: scaleBase = 800; break;
		default: scaleBase = 1024; break;
	}

	m_barSize = (int)9 * screenWide / scaleBase;
	m_barGap = (int)5 * screenWide / scaleBase;
}

static void DrawCrosshairRect(int x, int y, int w, int h, bool additive)
{
	vgui2::surface()->DrawFilledRect(x, y, x + w, y + h);
}

void CrosshairImagePanel::Paint(void)
{
	BaseClass::Paint();

	if (!m_pAdditive)
		return;

	int x = 0, y = 0, wide, tall;
    GetSize(wide, tall);
    LocalToScreen(x, y);

	int type = m_pCrosshairType->GetActiveItem();

	bool bDrawPoint = false;
	bool bDrawCircle = false;
	bool bDrawCross = false;

	switch (type)
	{
		case 1:
		{
			bDrawPoint = true;
			bDrawCross = true;
			break;
		}

		case 2:
		{
			bDrawPoint = true;
			bDrawCircle = true;
			break;
		}

		case 3:
		{
			bDrawPoint = true;
			bDrawCircle = true;
			bDrawCross = true;
			break;
		}

		case 4:
		{
			bDrawPoint = true;
			break;
		}

		default:
		{
			bDrawCross = true;
			break;
		}
	}

	bool additive = m_pAdditive->IsSelected();
	/*
	if (additive)
		vgui2::surface()->DrawSetColor(m_R, m_G, m_B, surface()->DrawGetAlphaMultiplier() * 200);
	else
		vgui2::surface()->DrawSetColor(m_R, m_G, m_B, surface()->DrawGetAlphaMultiplier() * 255);

	if (bDrawCircle)
	{
		int radius = m_barGap + (m_barSize / 2);
		int count = radius * 6;

		for (int i = 0; i < count; i++)
			DrawCrosshairRect((wide / 2) + radius * cos(2 * M_PI / count * i), (tall / 2) + radius * sin(2 * M_PI / count * i), 1, 1, additive);
	}

	if (bDrawPoint)
		DrawCrosshairRect((wide / 2) - 1, (tall / 2) - 1, 3, 3, additive);

	if (bDrawCross)
	{
		DrawCrosshairRect((wide / 2 - m_barGap - m_barSize) + 1, tall / 2, m_barSize, 1, additive);
		DrawCrosshairRect(wide / 2 + m_barGap, tall / 2, m_barSize, 1, additive);
		DrawCrosshairRect(wide / 2, tall / 2 - m_barGap - m_barSize, 1, m_barSize, additive);
		DrawCrosshairRect(wide / 2, tall / 2 + m_barGap, 1, m_barSize, additive);
	}
	*/

	if (bDrawCircle)
	{
		int radius = m_barGap + (m_barSize / 2);
		int count = radius * 6;

		if (additive)
		{
			for (int i = 0; i < count; i++)
				engine->pfnFillRGBA(x + (wide / 2) + radius * cos(2 * M_PI / count * i), y + (tall / 2) + radius * sin(2 * M_PI / count * i), 1, 1, m_R, m_G, m_B, 255);
		}
		else
		{
			for (int i = 0; i < count; i++)
				engine->pfnFillRGBABlend(x + (wide / 2) + radius * cos(2 * M_PI / count * i), y + (tall / 2) + radius * sin(2 * M_PI / count * i), 1, 1, m_R, m_G, m_B, 255);
		}
	}

	if (bDrawPoint)
	{
		if (additive)
		{
			engine->pfnFillRGBA(x + (wide / 2) - 1, y + (tall / 2) - 1, 3, 3, m_R, m_G, m_B, 255);
		}
		else
		{
			engine->pfnFillRGBABlend(x + (wide / 2) - 1, y + (tall / 2) - 1, 3, 3, m_R, m_G, m_B, 255);
		}
	}

	if (bDrawCross)
	{
		if (additive)
		{
			engine->pfnFillRGBA(x + (wide / 2 - m_barGap - m_barSize) + 1, y + tall / 2, m_barSize, 1, m_R, m_G, m_B, 255);
			engine->pfnFillRGBA(x + wide / 2 + m_barGap, y + tall / 2, m_barSize, 1, m_R, m_G, m_B, 255);
			engine->pfnFillRGBA(x + wide / 2, y + tall / 2 - m_barGap - m_barSize + 1, 1, m_barSize, m_R, m_G, m_B, 255);
			engine->pfnFillRGBA(x + wide / 2, y + tall / 2 + m_barGap, 1, m_barSize, m_R, m_G, m_B, 255);
		}
		else
		{
			engine->pfnFillRGBABlend(x + (wide / 2 - m_barGap - m_barSize) + 1, y + tall / 2, m_barSize, 1, m_R, m_G, m_B, 255);
			engine->pfnFillRGBABlend(x + wide / 2 + m_barGap, y + tall / 2, m_barSize, 1, m_R, m_G, m_B, 255);
			engine->pfnFillRGBABlend(x + wide / 2, y + tall / 2 - m_barGap - m_barSize + 1, 1, m_barSize, m_R, m_G, m_B, 255);
			engine->pfnFillRGBABlend(x + wide / 2, y + tall / 2 + m_barGap, 1, m_barSize, m_R, m_G, m_B, 255);
		}
	}
	
}

COptionsSubMultiplayer::COptionsSubMultiplayer(vgui2::Panel *parent) : vgui2::PropertyPage(parent, "OptionsSubMultiplayer")
{
	vgui2::Button *cancel = new vgui2::Button(this, "Cancel", "#GameUI_Cancel");
	cancel->SetCommand("Close");

	vgui2::Button *ok = new vgui2::Button(this, "OK", "#GameUI_OK");
	ok->SetCommand("Ok");

	vgui2::Button *apply = new vgui2::Button(this, "Apply", "#GameUI_Apply");
	apply->SetCommand("Apply");

	vgui2::Button *advanced = new vgui2::Button(this, "Advanced", "#GameUI_AdvancedEllipsis");
	advanced->SetCommand("Advanced");

	m_pNameTextEntry = new CCvarTextEntry(this, "NameEntry", "name");
	m_pHighQualityModelCheckBox = new CCvarToggleCheckButton(this, "High Quality Models", "#GameUI_HighModels", "cl_himodels");

	m_pLogoList = new CLabeledCommandComboBox(this, "SpraypaintList");
	m_pColorList = new CLabeledCommandComboBox(this, "SpraypaintColor");
	m_LogoName[0] = 0;

	InitLogoColorEntries();
	InitLogoList(m_pLogoList);

	m_pLogoImage = new CBitmapImagePanel(this, "LogoImage");
	m_pLogoImage->AddActionSignalTarget(this);

	m_nLogoR = 255;
	m_nLogoG = 255;
	m_nLogoB = 255;

	m_pCrosshairColorComboBox = new CLabeledCommandComboBox(this, "CrosshairColorComboBox");
	m_pCrosshairSize = new CLabeledCommandComboBox(this, "CrosshairSizeComboBox");
	m_pCrosshairType = new CLabeledCommandComboBox(this, "CrosshairTypeComboBox");
	m_pCrosshairTranslucencyCheckbox = new CCvarToggleCheckButton(this, "CrosshairTranslucencyCheckbox", "#GameUI_Translucent", "cl_crosshair_translucent");
	m_pCrosshairImage = new CrosshairImagePanel(this, "CrosshairImage", m_pCrosshairTranslucencyCheckbox, m_pCrosshairType);

	LoadControlSettings("Resource/OptionsSubMultiplayer.res");

	InitCrosshairColorEntries();
	InitCrosshairSizeList(m_pCrosshairSize);
	InitCrosshairTypeList(m_pCrosshairType);

	RedrawCrosshairImage();
}

COptionsSubMultiplayer::~COptionsSubMultiplayer(void)
{
}

void COptionsSubMultiplayer::OnCommand(const char *command)
{
	if (!stricmp(command, "Advanced"))
	{
		if (!m_hMultiplayerAdvancedDialog.Get())
			m_hMultiplayerAdvancedDialog = new CMultiplayerAdvancedDialog(this);

		m_hMultiplayerAdvancedDialog->Activate();
	}

	BaseClass::OnCommand(command);
}

void COptionsSubMultiplayer::InitLogoList(CLabeledCommandComboBox *cb)
{
	FileFindHandle_t fh;
	char directory[512];

	vgui2::filesystem()->RemoveFile("logos/remapped.bmp", NULL);

	const char *logofile = engine->pfnGetCvarString("cl_logofile");
	sprintf(directory, "logos/*.bmp");
	const char *fn = vgui2::filesystem()->FindFirst(directory, &fh);
	int i = 0, initialItem = 0;

	cb->DeleteAllItems();

	while (fn)
	{
		if (stricmp(fn, "remapped.bmp"))
		{
			if (fn[0] && fn[0] != '.')
			{
				char filename[512];
				strcpy(filename, fn);

				if (strlen(filename) >= 4)
					filename[strlen(filename) - 4] = 0;

				if (!stricmp(filename, logofile))
					initialItem = i;

				cb->AddItem(filename, "");

				if (m_LogoName[0] == 0)
					strcpy(m_LogoName, filename);
			}

			i++;
		}

		fn = vgui2::filesystem()->FindNext(fh);
	}

	vgui2::filesystem()->FindClose(fh);
	cb->SetInitialItem(initialItem);
}

void COptionsSubMultiplayer::InitLogoColorEntries(void)
{
	char const *currentcolor = engine->pfnGetCvarString("cl_logocolor");
	int count = sizeof(itemlist) / sizeof(itemlist[0]);
	int selected = 0;

	for (int i = 0; i < count; i++)
	{
		if (currentcolor && !stricmp(currentcolor, itemlist[i].name))
			selected = i;

		char command[256];
		sprintf(command, "cl_logocolor %s\n", itemlist[i].name);
		m_pColorList->AddItem(itemlist[i].name, command);
	}

	m_pColorList->SetInitialItem(selected);
	m_pColorList->AddActionSignalTarget(this);
}

void COptionsSubMultiplayer::InitCrosshairColorEntries(void)
{
	if (m_pCrosshairColorComboBox == NULL)
		return;

	int selected = 0;
	int cr = 0, cg = 0, cb = 0;
	const char *color = engine->pfnGetCvarString("cl_crosshair_color");

	if (color)
		sscanf(color, "%d %d %d", &cr, &cg, &cb);

	for (int i = 0; i < NumCrosshairColors; i++)
	{
		if (s_crosshairColors[i].r == cr && s_crosshairColors[i].g == cg && s_crosshairColors[i].b == cb)
			selected = i;

		char command[256];
		sprintf(command, "cl_crosshair_color \"%d %d %d\"\n", s_crosshairColors[i].r, s_crosshairColors[i].g, s_crosshairColors[i].b);
		m_pCrosshairColorComboBox->AddItem(s_crosshairColors[i].name, command);
	}

	m_pCrosshairColorComboBox->SetInitialItem(selected);
	m_pColorList->AddActionSignalTarget(this);
}

void COptionsSubMultiplayer::RedrawCrosshairImage(void)
{
	if (m_pCrosshairColorComboBox == NULL)
		return;

	if (m_pCrosshairImage && m_pCrosshairSize)
	{
		int size = m_pCrosshairSize->GetActiveItem();
		int selectedVal = m_pCrosshairColorComboBox->GetActiveItem();

		m_pCrosshairImage->UpdateCrosshair(s_crosshairColors[selectedVal].r, s_crosshairColors[selectedVal].g, s_crosshairColors[selectedVal].b, size);
	}
}

void COptionsSubMultiplayer::InitCrosshairSizeList(CLabeledCommandComboBox *cb)
{
	if (cb == NULL)
		return;

	cb->Reset();

	cb->AddItem("#GameUI_Auto", "cl_crosshair_size auto");
	cb->AddItem("#GameUI_Small", "cl_crosshair_size small");
	cb->AddItem("#GameUI_Medium", "cl_crosshair_size medium");
	cb->AddItem("#GameUI_Large", "cl_crosshair_size large");

	auto value = engine->pfnGetCvarString("cl_crosshair_size");

	if (!value)
		return;

	int initialScale = atoi(value);

	switch (tolower(value[0]))
	{
		case 's': initialScale = 1; break;
		case 'm': initialScale = 2; break;
		case 'l': initialScale = 3; break;
	}

	cb->SetInitialItem(initialScale);
}

void COptionsSubMultiplayer::InitCrosshairTypeList(CLabeledCommandComboBox *cb)
{
	if (cb == NULL)
		return;

	cb->Reset();

	cb->AddItem("#CSO_CrossHairType_0", "cl_crosshair_type 0");
	cb->AddItem("#CSO_CrossHairType_1", "cl_crosshair_type 1");
	cb->AddItem("#CSO_CrossHairType_2", "cl_crosshair_type 2");
	cb->AddItem("#CSO_CrossHairType_3", "cl_crosshair_type 3");
	cb->AddItem("#CSO_CrossHairType_4", "cl_crosshair_type 4");

	auto value = engine->pfnGetCvarString("cl_crosshair_type");

	if (!value)
		return;

	int initialType = atoi(value);

	cb->SetInitialItem(initialType);
}

void COptionsSubMultiplayer::RemapLogo(void)
{
	char logoname[256];
	m_pLogoList->GetText(logoname, sizeof(logoname));

	if (!logoname[0])
		return;

	int r, g, b;
	const char *colorname = m_pColorList->GetActiveItemCommand();

	if (!colorname || !colorname[0])
		return;

	colorname += strlen("cl_logocolor ");

	ColorForName(colorname, r, g, b);
	RemapLogoPalette(logoname, r, g, b);

	m_pLogoImage->setTexture("logos/remapped", true);
}

void COptionsSubMultiplayer::OnTextChanged(vgui2::Panel *panel)
{
	if (panel == m_pNameTextEntry)
		return;

	if (panel == m_pLogoList || panel == m_pColorList)
		RemapLogo();

	if (panel == m_pCrosshairSize || panel == m_pCrosshairType || panel == m_pCrosshairTranslucencyCheckbox || panel == m_pCrosshairColorComboBox)
		RedrawCrosshairImage();
}

void COptionsSubMultiplayer::OnSliderMoved(KeyValues *data)
{
}

void COptionsSubMultiplayer::OnApplyButtonEnable(void)
{
	PostMessage(GetParent(), new KeyValues("ApplyButtonEnable"));
	InvalidateLayout();
}

void COptionsSubMultiplayer::ColorForName(char const *pszColorName, int &r, int &g, int &b)
{
	r = g = b = 0;

	int count = sizeof(itemlist) / sizeof(itemlist[0]);

	for (int i = 0; i < count; i++)
	{
		if (!strnicmp(pszColorName, itemlist[i].name, strlen(itemlist[i].name)))
		{
			r = itemlist[i].r;
			g = itemlist[i].g;
			b = itemlist[i].b;
			return;
		}
	}
}

void COptionsSubMultiplayer::RemapLogoPalette(char *filename, int r, int g, int b)
{
	char infile[256];
	char outfile[256];

	sprintf(infile, "logos/%s.bmp", filename);
	sprintf(outfile, "logos/remapped.bmp");

    auto bmpFile = std::unique_ptr<ui::CBMP>(ui::CBMP::LoadFile( filename ));
    if(!bmpFile)
        return;
    bmpFile->RemapLogo( r, g, b );

	vgui2::filesystem()->RemoveFile(outfile, NULL);

	vgui2::filesystem()->CreateDirHierarchy("logos", NULL);
	auto file = vgui2::filesystem()->Open(outfile, "wb");

	if (file != FILESYSTEM_INVALID_HANDLE)
	{
		vgui2::filesystem()->Write(bmpFile->GetBitmap(), bmpFile->GetBitmapHdr()->fileSize, file);
		vgui2::filesystem()->Close(file);
	}
}

void COptionsSubMultiplayer::OnPageShow(void)
{
}

void COptionsSubMultiplayer::OnResetData(void)
{
	m_pNameTextEntry->Reset();
	m_pNameTextEntry->GotoTextEnd();
	m_pLogoList->Reset();
	m_pColorList->Reset();
	m_pHighQualityModelCheckBox->Reset();
	m_pCrosshairSize->Reset();
	m_pCrosshairType->Reset();
	m_pCrosshairColorComboBox->Reset();
	m_pCrosshairTranslucencyCheckbox->Reset();
}

void COptionsSubMultiplayer::OnApplyChanges(void)
{
	m_pLogoList->ApplyChanges();
	m_pLogoList->GetText(m_LogoName, sizeof(m_LogoName));
	m_pColorList->ApplyChanges();
	m_pHighQualityModelCheckBox->ApplyChanges();
	m_pNameTextEntry->ApplyChanges();

	for (int i = 0; i < m_cvarToggleCheckButtons.GetCount(); ++i)
	{
		CCvarToggleCheckButton *toggleButton = m_cvarToggleCheckButtons[i];

		if (toggleButton->IsVisible() && toggleButton->IsEnabled())
			toggleButton->ApplyChanges();
	}

	if (1/*!ModInfo().NoCrosshair()*/)
	{
		if (m_pCrosshairSize != NULL)
			m_pCrosshairSize->ApplyChanges();

		if (m_pCrosshairType != NULL)
			m_pCrosshairType->ApplyChanges();

		if (m_pCrosshairTranslucencyCheckbox != NULL)
			m_pCrosshairTranslucencyCheckbox->ApplyChanges();

		ApplyCrosshairColorChanges();
	}
/*
	const char *colorname = m_pColorList->GetActiveItemCommand();

	if (colorname && colorname[0])
	{
		colorname += strlen("cl_logocolor ");

		char cmd[512];
		_snprintf(cmd, sizeof(cmd) - 1, "cl_logofile %s\n", m_LogoName);
		engine->pfnClientCmd(cmd);

		int r, g, b;
		ColorForName(colorname, r, g, b);

		char infile[256];
		sprintf(infile, "logos/remapped.bmp");
		FileHandle_t file = vgui2::filesystem()->Open(infile, "rb");

		if (file != FILESYSTEM_INVALID_HANDLE)
		{
			BITMAPFILEHEADER bmfHeader;
			DWORD dwBitsSize, dwFileSize;

			dwFileSize = vgui2::filesystem()->Size(file);
			vgui2::filesystem()->Read(&bmfHeader, sizeof(bmfHeader), file);

			if (bmfHeader.bfType == DIB_HEADER_MARKER)
			{
				dwBitsSize = dwFileSize - sizeof(bmfHeader);
				HGLOBAL hDIB = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, dwBitsSize);

				char *pDIB = (LPSTR)GlobalLock((HGLOBAL)hDIB);
				vgui2::filesystem()->Read(pDIB, dwBitsSize, file);
				GlobalUnlock((HGLOBAL)hDIB);
				UpdateLogoWAD((void *)hDIB, r, g, b);
				GlobalFree((HGLOBAL)hDIB);
			}

			vgui2::filesystem()->Close(file);
		}
	}
 */
}

void COptionsSubMultiplayer::ApplyCrosshairColorChanges(void)
{
	char cmd[256];
	cmd[0] = 0;

	if (m_pCrosshairColorComboBox != NULL)
	{
		int i = m_pCrosshairColorComboBox->GetActiveItem();
		Q_snprintf(cmd, sizeof(cmd), "cl_crosshair_color \"%d %d %d\"\n", s_crosshairColors[i].r, s_crosshairColors[i].g, s_crosshairColors[i].b);
		engine->pfnClientCmd(cmd);
	}
}

vgui2::Panel *COptionsSubMultiplayer::CreateControlByName(const char *controlName)
{
	if (!Q_stricmp("CCvarToggleCheckButton", controlName))
	{
		CCvarToggleCheckButton *newButton = new CCvarToggleCheckButton(this, controlName, "", "");
		m_cvarToggleCheckButtons.AddElement(newButton);
		return newButton;
	}
	else
		return BaseClass::CreateControlByName(controlName);
}