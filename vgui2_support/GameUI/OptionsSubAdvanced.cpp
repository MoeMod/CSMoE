
#include "OptionsSubAdvanced.h"
#include "CvarSlider.h"
#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/Slider.h>
#include "ivoicetweak.h"
#include "CvarToggleCheckButton.h"
#include "tier1/KeyValues.h"
#include "tier1/UtlVector.h"

CUtlVector<char *> g_vLanguageList;

const char *g_LanguageList[] =
{
	"english",
	"schinese",
	"tchinese",
	"macedonian"
};

COptionsSubAdvanced::COptionsSubAdvanced(vgui2::Panel *parent) : PropertyPage(parent, NULL)
{

	//vgui2::localize()->AddFile("Resource/language_%language%.txt");

	LoadControlSettings("Resource/OptionsSubAdvanced.res");
}

COptionsSubAdvanced::~COptionsSubAdvanced(void)
{
	for (int i = 0; i < g_vLanguageList.Size(); i++)
		free(g_vLanguageList[i]);

	g_vLanguageList.RemoveAll();
}

void COptionsSubAdvanced::OnPageShow(void)
{
}

void COptionsSubAdvanced::OnResetData(void)
{
}

void COptionsSubAdvanced::OnSliderMoved(int position)
{
}

void COptionsSubAdvanced::OnCheckButtonChecked(int state)
{
}

void COptionsSubAdvanced::OnThink(void)
{
	BaseClass::OnThink();
}

void COptionsSubAdvanced::OnApplyChanges(void)
{
	
}

void COptionsSubAdvanced::OnCommand(const char *command)
{
	BaseClass::OnCommand(command);
}

void COptionsSubAdvanced::OnPageHide(void)
{
	BaseClass::OnPageHide();
}

void COptionsSubAdvanced::OnControlModified(void)
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
}

void COptionsSubAdvanced::OnTextChanged(char const *text)
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
}