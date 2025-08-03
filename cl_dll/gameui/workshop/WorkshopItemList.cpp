//========= Copyright (c) 2022 Zombie Panic! Team, All rights reserved. ============//

#include <tier1/interface.h>
#include <vgui/MouseCode.h>
#include <vgui/IInput.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>

#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/ScrollBar.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Controls.h>
#include "WorkshopItemList.h"
#include "../../client_vgui.h"
#include <steamworks/steam_api.h>

#include <KeyValues.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui2;

DECLARE_BUILD_FACTORY(WorkshopItemList);

char* vgui2::VarArgs(char* format, ...)
{
	va_list argptr;
	static char string[1024];

	va_start(argptr, format);
	vsprintf(string, format, argptr);
	va_end(argptr);

	return string;
}

void vgui2::STDReplaceString(std::string& path, std::string search, std::string replace)
{
	size_t pos = 0;
	while ((pos = path.find(search, pos)) != std::string::npos) {
		path.replace(pos, search.length(), replace);
		pos += replace.length();
	}
}

bool vgui2::STDContains(std::string path, std::string search)
{
	return (path.rfind(search, 0) == 0);
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
WorkshopItemList::WorkshopItemList(vgui2::Panel* parent, char const* panelName)
	: EditablePanel(parent, panelName)
{
	SetBounds(0, 0, 100, 100);

	m_vbar = new vgui2::ScrollBar(this, "PanelListPanelVScroll", true);
	m_vbar->SetVisible(false);
	m_vbar->AddActionSignalTarget(this);

	m_pPanelEmbedded = new EditablePanel(this, "PanelListEmbedded");
	m_pPanelEmbedded->SetBounds(0, 0, 43, 20);
	m_pPanelEmbedded->SetPaintBackgroundEnabled(false);
	m_pPanelEmbedded->SetPaintBorderEnabled(false);

	m_iFirstColumnWidth = 100; // default width
	m_iNumColumns = 1; // 1 column by default

	if (IsProportional())
	{
		m_iDefaultHeight = vgui2::scheme()->GetProportionalScaledValue(GetScheme());
		m_iPanelBuffer = vgui2::scheme()->GetProportionalScaledValue(GetScheme());
	}
	else
	{
		m_iDefaultHeight = DEFAULT_HEIGHT;
		m_iPanelBuffer = PANELBUFFER;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
WorkshopItemList::~WorkshopItemList()
{
	// free data from table
	DeleteAllItems();
}

void WorkshopItemList::SetVerticalBufferPixels(int buffer)
{
	m_iPanelBuffer = buffer;
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: counts the total vertical pixels
//-----------------------------------------------------------------------------
int	WorkshopItemList::ComputeVPixelsNeeded()
{
	int iCurrentItem = 0;
	int iLargestH = 0;

	int pixels = 0;
	for (int i = 0; i < m_SortedItems.Count(); i++)
	{
		vgui2::Panel* panel = m_DataItems[m_SortedItems[i]].texture;
		if (!panel)
			continue;

		if (panel->IsLayoutInvalid())
		{
			panel->InvalidateLayout(true);
		}

		int iCurrentColumn = iCurrentItem % m_iNumColumns;

		int w, h;
		panel->GetSize(w, h);

		if (iLargestH < h)
			iLargestH = h;

		if (iCurrentColumn == 0)
			pixels += m_iPanelBuffer; // add in buffer. between rows.

		if (iCurrentColumn >= m_iNumColumns - 1)
		{
			pixels += iLargestH;
			iLargestH = 0;
		}

		iCurrentItem++;
	}

	// Add in remaining largest height
	pixels += iLargestH;

	pixels += m_iPanelBuffer; // add in buffer below last item

	return pixels;
}

//-----------------------------------------------------------------------------
// Purpose: Returns the panel to use to render a cell
//-----------------------------------------------------------------------------
vgui2::Panel* WorkshopItemList::GetCellRenderer(int row)
{
	if (!m_SortedItems.IsValidIndex(row))
		return NULL;

	vgui2::Panel* panel = m_DataItems[m_SortedItems[row]].texture;
	return panel;
}

//-----------------------------------------------------------------------------
// Purpose: adds an item to the view
//			data->GetName() is used to uniquely identify an item
//			data sub items are matched against column header name to be used in the table
//-----------------------------------------------------------------------------
int WorkshopItemList::AddItem(vgui2::Panel* Texture, vgui2::Panel* Author, vgui2::Panel* Title, vgui2::Panel* Description, vgui2::Panel* Activated, vgui2::Panel* Error_msg, uint64 nWorkshopID, bool bCanEdit)
{
	if (Author)
		Author->SetParent(m_pPanelEmbedded);

	if (Title)
		Title->SetParent(m_pPanelEmbedded);

	if (Description)
		Description->SetParent(m_pPanelEmbedded);

	if (Activated)
		Activated->SetParent(m_pPanelEmbedded);

	if (Error_msg)
		Error_msg->SetParent(m_pPanelEmbedded);

	Texture->SetParent(m_pPanelEmbedded);

	int itemID = m_DataItems.AddToTail();
	DATAITEM& newitem = m_DataItems[itemID];
	newitem.author = Author;
	newitem.title = Title;
	newitem.texture = Texture;
	newitem.desc = Description;
	newitem.activated = Activated;
	newitem.error_msg = Error_msg;

	// Fonts
	vgui2::HFont hTextFont;
	vgui2::IScheme* pScheme = vgui2::scheme()->GetIScheme(
		vgui2::scheme()->LoadSchemeFromFile(VGUI2_ROOT_DIR "resource/ClientSourceScheme.res", "ClientSourceScheme")
	);

	// Create the button here instead.
	vgui2::Button* pWorkshopButton = new vgui2::Button(
		m_pPanelEmbedded,
		"WorkshopButton",
		"#Phoenix_Workshop_Link",
		this,
		VarArgs(
			"WorkshopURL_%llu",
			nWorkshopID
		)
	);
	hTextFont = pScheme->GetFont("AchievementItemDescription");
	if (hTextFont != vgui2::INVALID_FONT)
		pWorkshopButton->SetFont(hTextFont);

	newitem.button = pWorkshopButton;

	if (bCanEdit)
	{
		vgui2::Button* pEditButton = new vgui2::Button(
			m_pPanelEmbedded,
			"EditButton",
			"#Phoenix_Workshop_Edit",
			this,
			VarArgs(
				"WorkshopEdit_%llu",
				nWorkshopID
			)
		);
		hTextFont = pScheme->GetFont("AchievementItemDescription");
		if (hTextFont != vgui2::INVALID_FONT)
			pEditButton->SetFont(hTextFont);
		newitem.button_edit = pEditButton;
	}
	else
		newitem.button_edit = nullptr;

	newitem.workshopid = nWorkshopID;
	m_SortedItems.AddToTail(itemID);

	InvalidateLayout();
	return itemID;
}

//-----------------------------------------------------------------------------
// Purpose: iteration accessor
//-----------------------------------------------------------------------------
int	WorkshopItemList::GetItemCount() const
{
	return m_DataItems.Count();
}

int WorkshopItemList::GetItemIDFromRow(int nRow) const
{
	if (nRow < 0 || nRow >= GetItemCount())
		return m_DataItems.InvalidIndex();
	return m_SortedItems[nRow];
}


//-----------------------------------------------------------------------------
// Iteration. Use these until they return InvalidItemID to iterate all the items.
//-----------------------------------------------------------------------------
int WorkshopItemList::FirstItem() const
{
	return m_DataItems.Head();
}

int WorkshopItemList::NextItem(int nItemID) const
{
	return m_DataItems.Next(nItemID);
}

int WorkshopItemList::InvalidItemID() const
{
	return m_DataItems.InvalidIndex();
}


//-----------------------------------------------------------------------------
// Purpose: returns label panel for this itemID
//-----------------------------------------------------------------------------
vgui2::Panel* WorkshopItemList::GetItemLabel(int itemID)
{
	if (!m_DataItems.IsValidIndex(itemID))
		return NULL;

	return m_DataItems[itemID].title;
}

//-----------------------------------------------------------------------------
// Purpose: returns label panel for this itemID
//-----------------------------------------------------------------------------
vgui2::Panel* WorkshopItemList::GetItemPanel(int itemID)
{
	if (!m_DataItems.IsValidIndex(itemID))
		return NULL;

	return m_DataItems[itemID].texture;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void WorkshopItemList::RemoveItem(int itemID)
{
	if (!m_DataItems.IsValidIndex(itemID))
		return;

	DATAITEM& item = m_DataItems[itemID];

	if (item.texture)
		item.texture->MarkForDeletion();

	if (item.author)
		item.author->MarkForDeletion();

	if (item.title)
		item.title->MarkForDeletion();

	if (item.desc)
		item.desc->MarkForDeletion();

	if (item.activated)
		item.activated->MarkForDeletion();

	if (item.error_msg)
		item.error_msg->MarkForDeletion();

	if (item.button)
		item.button->MarkForDeletion();

	if (item.button_edit)
		item.button_edit->MarkForDeletion();

	m_DataItems.Remove(itemID);
	m_SortedItems.FindAndRemove(itemID);

	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: clears and deletes all the memory used by the data items
//-----------------------------------------------------------------------------
void WorkshopItemList::DeleteAllItems()
{
	FOR_EACH_LL(m_DataItems, i)
	{
		if (m_DataItems[i].texture)
		{
			m_DataItems[i].texture->MarkForDeletion();
			m_DataItems[i].texture = NULL;
		}
		if (m_DataItems[i].activated)
		{
			m_DataItems[i].activated->MarkForDeletion();
			m_DataItems[i].activated = NULL;
		}
		if (m_DataItems[i].error_msg)
		{
			m_DataItems[i].error_msg->MarkForDeletion();
			m_DataItems[i].error_msg = NULL;
		}
		if (m_DataItems[i].author)
		{
			m_DataItems[i].author->MarkForDeletion();
			m_DataItems[i].author = NULL;
		}
		if (m_DataItems[i].title)
		{
			m_DataItems[i].title->MarkForDeletion();
			m_DataItems[i].title = NULL;
		}
		if (m_DataItems[i].desc)
		{
			m_DataItems[i].desc->MarkForDeletion();
			m_DataItems[i].desc = NULL;
		}
		if (m_DataItems[i].button)
		{
			m_DataItems[i].button->MarkForDeletion();
			m_DataItems[i].button = NULL;
		}
		if (m_DataItems[i].button_edit)
		{
			m_DataItems[i].button_edit->MarkForDeletion();
			m_DataItems[i].button_edit = NULL;
		}
	}

	m_DataItems.RemoveAll();
	m_SortedItems.RemoveAll();

	// move the scrollbar to the top of the list
	m_vbar->SetValue(0);
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: clears and deletes all the memory used by the data items
//-----------------------------------------------------------------------------
void WorkshopItemList::RemoveAll()
{
	m_DataItems.RemoveAll();
	m_SortedItems.RemoveAll();

	// move the scrollbar to the top of the list
	m_vbar->SetValue(0);
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void WorkshopItemList::OnSizeChanged(int wide, int tall)
{
	BaseClass::OnSizeChanged(wide, tall);
	InvalidateLayout();
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: relayouts out the panel after any internal changes
//-----------------------------------------------------------------------------
void WorkshopItemList::PerformLayout()
{
	int wide, tall;
	GetSize(wide, tall);

	int vpixels = ComputeVPixelsNeeded();

	m_vbar->SetRange(0, vpixels);
	m_vbar->SetRangeWindow(tall);
	m_vbar->SetButtonPressedScrollValue(tall / 4); // standard height of labels/buttons etc.

	m_vbar->SetPos(wide - m_vbar->GetWide() - 2, 0);
	m_vbar->SetSize(m_vbar->GetWide(), tall - 2);

	int top = m_vbar->GetValue();

	if (vpixels < 306)
		vpixels = 306;

	m_pPanelEmbedded->SetPos(0, -top);
	m_pPanelEmbedded->SetSize(wide - m_vbar->GetWide(), vpixels);	// scrollbar will sit on top (zpos set explicitly)

	bool bScrollbarVisible = true;
	// If we're supposed to automatically hide the scrollbar when unnecessary, check it now
	if (m_bAutoHideScrollbar)
		bScrollbarVisible = (m_pPanelEmbedded->GetTall() > tall);

	m_vbar->SetVisible(bScrollbarVisible);

	// Now lay out the controls on the embedded panel
	int y = 0;
	int h = 73;

	int xpos = m_iFirstColumnWidth + m_iPanelBuffer;

	for (int i = 0; i < m_SortedItems.Count(); i++)
	{
		int iCurrentColumn = i % m_iNumColumns;

		// add in a little buffer between panels
		if (iCurrentColumn == 0)
			y += m_iPanelBuffer;

		DATAITEM& item = m_DataItems[m_SortedItems[i]];

		// Override the texture height
		item.texture->SetTall(73);

		if (h < item.texture->GetTall())
			h = item.texture->GetTall();

		int imgw = 150;
		item.texture->SetBounds(5, y, imgw, item.texture->GetTall());

		int textx = 5 + imgw;

		if (item.title)
			item.title->SetBounds(textx + 5, y - 2, 400, 20);

		if (item.desc)
			item.desc->SetBounds(textx + 8, y + 5, 490, 35);

		if (item.author)
			item.author->SetBounds(textx + 8, y + 20, 490, 35);

		if (item.activated)
		{
			item.activated->SetBounds(textx + 5, y + 38, 150, 35);
			item.activated->SetEnabled(true);
			item.activated->MoveToFront();
		}

		if (item.error_msg)
		{
			item.error_msg->SetBounds(wide - textx - 200, y + 40, 200, 35);
			item.error_msg->SetFgColor(Color(255, 0, 0, 255));
		}

		if (item.button)
			item.button->SetBounds(wide - 130, y - 2, 100, 24);

		if (item.button_edit)
			item.button_edit->SetBounds(wide - 130, y + 32, 100, 24);

		if (iCurrentColumn >= m_iNumColumns - 1)
		{
			y += h;
			h = 0;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: scheme settings
//-----------------------------------------------------------------------------
void WorkshopItemList::ApplySchemeSettings(vgui2::IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetBorder(pScheme->GetBorder("ButtonDepressedBorder"));
	SetBgColor(GetSchemeColor("ListPanel.BgColor", GetBgColor(), pScheme));
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void WorkshopItemList::OnSliderMoved(int position)
{
	InvalidateLayout();
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void WorkshopItemList::MoveScrollBarToTop()
{
	m_vbar->SetValue(0);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void WorkshopItemList::SetFirstColumnWidth(int width)
{
	m_iFirstColumnWidth = width;
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
int WorkshopItemList::GetFirstColumnWidth()
{
	return m_iFirstColumnWidth;
}

void WorkshopItemList::SetNumColumns(int iNumColumns)
{
	m_iNumColumns = iNumColumns;
}

int WorkshopItemList::GetNumColumns(void)
{
	return m_iNumColumns;
}

//-----------------------------------------------------------------------------
// Purpose: moves the scrollbar with the mousewheel
//-----------------------------------------------------------------------------
void WorkshopItemList::OnMouseWheeled(int delta)
{
	int val = m_vbar->GetValue();
	val -= (delta * DEFAULT_HEIGHT);
	m_vbar->SetValue(val);
}

void vgui2::WorkshopItemList::OnCommand(const char* pcCommand)
{
	if (vgui2::STDContains(pcCommand, "WorkshopURL_"))
	{
		if (!SteamAPI_IsSteamRunning()) return;
		if (!SteamFriends()) return;
		std::string szCommand(pcCommand);
		vgui2::STDReplaceString(szCommand, "WorkshopURL_", "");
		SteamFriends()->ActivateGameOverlayToWebPage(
			vgui2::VarArgs(
				"https://steamcommunity.com/workshop/filedetails/?id=%s",
				szCommand.c_str()
			)
		);
	}
	else if (vgui2::STDContains(pcCommand, "WorkshopEdit_"))
	{
		if (!SteamAPI_IsSteamRunning()) return;
		if (!SteamFriends()) return;
		std::string szCommand(pcCommand);
		vgui2::STDReplaceString(szCommand, "WorkshopEdit_", "");
		uint64 nWorkshopID = std::strtoull(szCommand.c_str(), NULL, 0);
		KeyValues* kv = new KeyValues("WorkshopEdit");
		kv->SetUint64("workshopID", nWorkshopID);
		PostActionSignal(kv);
	}
	else
	{
		BaseClass::OnCommand(pcCommand);
	}
}

//-----------------------------------------------------------------------------
// Purpose: selection handler
//-----------------------------------------------------------------------------
void WorkshopItemList::SetSelectedPanel(Panel* panel)
{
	if (panel != m_hSelectedItem)
	{
		// notify the panels of the selection change
		if (m_hSelectedItem)
		{
			PostMessage(m_hSelectedItem.Get(), new KeyValues("PanelSelected", "state", 0));
		}
		if (panel)
		{
			PostMessage(panel, new KeyValues("PanelSelected", "state", 1));
		}
		m_hSelectedItem = panel;
	}
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
vgui2::Panel* WorkshopItemList::GetSelectedPanel()
{
	return m_hSelectedItem;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void WorkshopItemList::ScrollToItem(int itemNumber)
{
	if (!m_vbar->IsVisible())
	{
		return;
	}

	DATAITEM& item = m_DataItems[m_SortedItems[itemNumber]];
	if (!item.texture)
		return;

	int x, y;
	item.texture->GetPos(x, y);
	int lx, ly;
	lx = x;
	ly = y;
	m_pPanelEmbedded->LocalToScreen(lx, ly);
	ScreenToLocal(lx, ly);

	int h = item.texture->GetTall();

	if (ly >= 0 && ly + h < GetTall())
		return;

	m_vbar->SetValue(y);
	InvalidateLayout();
}