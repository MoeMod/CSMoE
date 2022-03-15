#if !defined (PANELLISTPANEL_H)
#define PANELLISTPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Panel.h>

class KeyValues;

class CPanelListPanel : public vgui2::Panel
{
	DECLARE_CLASS_SIMPLE(CPanelListPanel, vgui2::Panel);

public:
	typedef struct dataitem_s
	{
		vgui2::Panel *panel;
	}
	DATAITEM;

public:
	CPanelListPanel(vgui2::Panel *parent, char const *panelName, bool inverseButtons = false);
	~CPanelListPanel(void);

public:
	virtual int computeVPixelsNeeded(void);
	virtual int AddItem(vgui2::Panel *panel);
	virtual int GetItemCount(void);
	virtual vgui2::Panel *GetItem(int itemIndex);
	virtual void RemoveItem(int itemIndex);
	virtual void DeleteAllItems(void);
	virtual vgui2::Panel *GetCellRenderer(int row);
	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme);

public:
	MESSAGE_FUNC_INT(OnSliderMoved, "ScrollBarSliderMoved", position);

public:
	void SetSliderYOffset(int pixels);

	vgui2::Panel *GetEmbedded(void)
	{
		return _embedded;
	}

protected:
	DATAITEM *GetDataItem(int itemIndex);

protected:
	virtual void PerformLayout(void);
	virtual void PaintBackground(void);
	virtual void OnMouseWheeled(int delta);

private:
	vgui2::Dar<DATAITEM *> _dataItems;
	vgui2::ScrollBar *_vbar;
	vgui2::Panel *_embedded;

	int _tableStartX;
	int _tableStartY;
	int _sliderYOffset;
};

#endif