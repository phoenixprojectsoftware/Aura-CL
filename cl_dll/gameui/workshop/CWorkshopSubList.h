// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef CWORKSHOP_SUB_LIST_H
#define CWORKSHOP_SUB_LIST_H
#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/ComboBox.h>
#include "WorkshopItemList.h"

class CWorkshopSubList : public vgui2::PropertyPage
{
	DECLARE_CLASS_SIMPLE(CWorkshopSubList, vgui2::PropertyPage);

public:
	CWorkshopSubList(vgui2::Panel* parent);
	~CWorkshopSubList();
	virtual void OnTick();
	virtual void ApplySchemeSettings(vgui2::IScheme* pScheme);
	virtual void PerformLayout();
	virtual void OnCommand(const char* pcCommand);

	virtual void UpdateItems();

	virtual bool HasFilterFlag(int iFilters);

private:
	vgui2::ComboBox* pComboList;
	vgui2::WorkshopItemList* pList;
	int m_iCurrentFilter;
};

#endif
