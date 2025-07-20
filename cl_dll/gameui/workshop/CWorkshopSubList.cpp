// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include <string>
#include <vector>
#include <FileSystem.h>
#include <tier1/interface.h>
#include <tier1/strtools.h>
#include <vgui/IVGui.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/URLLabel.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/CheckButton.h>
#include <KeyValues.h>
#include <current_version.h>
// #include <bhl_urls.h>
#include "../../client_vgui.h"
#include "../gameui_viewport.h"
#include "CWorkshopSubList.h"

CWorkshopSubList::CWorkshopSubList(vgui2::Panel* parent)
	: BaseClass(parent, "WorkshopSubList")
{
	SetSize(100, 100); // Silence "parent not sized yet" warning

	// Our Item List
	pList = new vgui2::WorkshopItemList(this, "listpanel");
	pList->SetPos(15, 100);
	pList->SetSize(600, 302);

	// KeyValues
	KeyValues* kv = new KeyValues("Categories", "Key", "Value");

	// Category filter
	pComboList = new vgui2::ComboBox(this, "category", vgui2::MAX_CATEGORY_FILTER, false);
	pComboList->SetPos(25, 410);
	pComboList->SetSize(235, 24);
	pComboList->AddItem("#ZP_UI_Workshop_CategoryFilter_None", kv);
	pComboList->AddItem("#ZP_UI_Workshop_CategoryFilter_Map", kv);
	pComboList->AddItem("#ZP_UI_Workshop_CategoryFilter_Weapons", kv);
	pComboList->AddItem("#ZP_UI_Workshop_CategoryFilter_Sounds", kv);
	pComboList->AddItem("#ZP_UI_Workshop_CategoryFilter_Survivor", kv);
	pComboList->AddItem("#ZP_UI_Workshop_CategoryFilter_Zombie", kv);
	pComboList->AddItem("#ZP_UI_Workshop_CategoryFilter_Background", kv);
	pComboList->AddItem("#ZP_UI_Workshop_CategoryFilter_Sprays", kv);
	pComboList->AddItem("#ZP_UI_Workshop_CategoryFilter_Music", kv);
	// Auto select "Show All Achievements"
	pComboList->ActivateItem(0);

	// Load this last, so we can move our items around.
	LoadControlSettings(VGUI2_ROOT_DIR "resource/workshop/mainlist.res");

	// Never allow it
	pComboList->SetEditable(false);

	m_iCurrentFilter = -1;

	vgui2::ivgui()->AddTickSignal(GetVPanel(), 25);
}

CWorkshopSubList::~CWorkshopSubList()
{
}

void CWorkshopSubList::ApplySchemeSettings(vgui2::IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

void CWorkshopSubList::UpdateItems()
{
	pList->DeleteAllItems();

	// Fonts
	vgui2::HFont hTextFont;
	vgui2::IScheme* pScheme = vgui2::scheme()->GetIScheme(
		vgui2::scheme()->LoadSchemeFromFile(VGUI2_ROOT_DIR "resource/ClientSourceScheme.res", "ClientSourceScheme")
	);

	// Grab the items from our GameUI instance
	std::vector<vgui2::WorkshopItem> m_Items;
	CGameUIViewport::Get()->GetCurrentItems(m_Items);

	for (int i = 0; i < m_Items.size(); i++)
	{
		vgui2::WorkshopItem& item = m_Items[i];
		if (!HasFilterFlag(item.iFilterFlag)) continue;

		// Create Image
		vgui2::ImagePanel* pIcon = new vgui2::ImagePanel(this, "Icon");

		char buffer[158];
		Q_snprintf(buffer, sizeof(buffer), "%llu/thumb", item.uWorkshopID);

		pIcon->SetImage(vgui2::scheme()->GetImage(buffer, false));
		pIcon->SetSize(56, 56);
		pIcon->SetPos(4, 4);
		pIcon->SetShouldScaleImage(true);

		// Font Text
		vgui2::Label* pTitle = new vgui2::Label(this, "Title", "");
		pTitle->SetSize(400, 20);
		pTitle->SetPos(70, 5);
		pTitle->SetPaintBackgroundEnabled(false);
		hTextFont = pScheme->GetFont("AchievementItemTitle");
		if (hTextFont != vgui2::INVALID_FONT)
			pTitle->SetFont(hTextFont);
		pTitle->SetColorCodedText(item.szName);

		vgui2::Label* pAuthor = new vgui2::Label(this, "Author", "");
		pAuthor->SetSize(400, 20);
		pAuthor->SetPos(70, 35);
		pAuthor->SetPaintBackgroundEnabled(false);
		hTextFont = pScheme->GetFont("AchievementItemDescription");
		if (hTextFont != vgui2::INVALID_FONT)
			pAuthor->SetFont(hTextFont);
		pAuthor->SetColorCodedText(item.szAuthor);

		vgui2::Label* pDesc = new vgui2::Label(this, "Description", "");
		pDesc->SetSize(400, 20);
		pDesc->SetPos(70, 15);
		pDesc->SetPaintBackgroundEnabled(false);
		hTextFont = pScheme->GetFont("AchievementItemDescription");
		if (hTextFont != vgui2::INVALID_FONT)
			pDesc->SetFont(hTextFont);
		pDesc->SetColorCodedText(item.szDesc);

		vgui2::CheckButton* pActivated = new vgui2::CheckButton(this, "CheckButton", "#ZP_UI_Workshop_ActivateAddon");
		pActivated->SetSize(400, 20);
		pActivated->SetPos(70, 50);
		pActivated->SetPaintBackgroundEnabled(false);
		pActivated->SetSelected(item.bMounted);
		Q_snprintf(buffer, sizeof(buffer), "MountAddon_%llu", item.uWorkshopID);
		pActivated->SetCommand(buffer);
		pActivated->AddActionSignalTarget(this);

		vgui2::Label* pError = new vgui2::Label(this, "Error", "");
		pError->SetSize(400, 20);
		pError->SetPos(4, 65);
		pError->SetPaintBackgroundEnabled(false);

		wchar_t* pStr = g_pVGuiLocalize->Find("ZP_UI_Workshop_AddonError");
		if (pStr)
			g_pVGuiLocalize->ConvertUnicodeToANSI(pStr, buffer, sizeof(buffer));
		else
			Q_snprintf(buffer, sizeof(buffer), "^1This addon have conflicting files.");
		pError->SetColorCodedText(buffer);
		pError->SetVisible(CGameUIViewport::Get()->HasConflictingFiles(item));

		pList->AddItem(
			pIcon, pAuthor,
			pTitle, pDesc,
			pActivated, pError,
			item.uWorkshopID
		);
	}
}

bool CWorkshopSubList::HasFilterFlag(int iFilters)
{
	int iFlag = 0;
	switch (pComboList->GetActiveItem())
	{
	case 0: iFlag = vgui2::FILTER_NONE; break;
	case 1: iFlag = vgui2::FILTER_MAP; break;
	case 2: iFlag = vgui2::FILTER_WEAPONS; break;
	case 3: iFlag = vgui2::FILTER_SOUNDS; break;
	case 4: iFlag = vgui2::FILTER_SURVIVOR; break;
	case 5: iFlag = vgui2::FILTER_ZOMBIE; break;
	case 6: iFlag = vgui2::FILTER_BACKGROUND; break;
	case 7: iFlag = vgui2::FILTER_SPRAYS; break;
	case 8: iFlag = vgui2::FILTER_MUSIC; break;
	}
	if (iFlag == 0) return true;
	return (iFilters & iFlag) != 0;
}

void CWorkshopSubList::OnTick()
{
	BaseClass::OnTick();

	if (pComboList->GetActiveItem() != m_iCurrentFilter)
	{
		m_iCurrentFilter = pComboList->GetActiveItem();
		UpdateItems();
	}
}

void CWorkshopSubList::PerformLayout()
{
	BaseClass::PerformLayout();
}

void CWorkshopSubList::OnCommand(const char* pcCommand)
{
	if (vgui2::STDContains(pcCommand, "MountAddon_"))
	{
		std::string szCommand(pcCommand);
		vgui2::STDReplaceString(szCommand, "MountAddon_", "");
		uint64 nWorkshopID = std::strtoull(szCommand.c_str(), NULL, 0);
		vgui2::WorkshopItem item = CGameUIViewport::Get()->GetWorkshopItem(nWorkshopID);

		CGameUIViewport::Get()->ShowWorkshopInfoBox(item.szName, item.bMounted ? WorkshopInfoBoxState::State_Dismounting : WorkshopInfoBoxState::State_Mounting);
		CGameUIViewport::Get()->ShowWorkshopInfoBox(item.szName, WorkshopInfoBoxState::State_Done);
		CGameUIViewport::Get()->MountWorkshopItem(item, nullptr, nullptr);
	}
	else
	{
		BaseClass::OnCommand(pcCommand);
	}
}