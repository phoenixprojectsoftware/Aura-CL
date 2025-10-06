/****
*
* Copyright (c) 2022 Zombie Panic! Team. All Rights Reserved. Modified under Open Source License by The Phoenix Project Software.
*
* AURA
*
* Achievement Manager
*
*
****/

#include <tier1/interface.h>
#include "vgui/MouseCode.h"
#include "vgui/IInput.h"
#include "vgui/IScheme.h"
#include "vgui/ISurface.h"

#include "vgui_controls/EditablePanel.h"
#include "vgui_controls/ScrollBar.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/Controls.h"
#include "AchievementList.h"

#include "KeyValues.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui2;

DECLARE_BUILD_FACTORY(AchievementList);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
AchievementList::AchievementList(vgui2::Panel* parent, char const* panelName) : EditablePanel(parent, panelName)
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
AchievementList::~AchievementList()
{
	// free data from table
	DeleteAllItems();
}

void AchievementList::SetVerticalBufferPixels(int buffer)
{
	m_iPanelBuffer = buffer;
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: counts the total vertical pixels
//-----------------------------------------------------------------------------
int	AchievementList::ComputeVPixelsNeeded()
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
vgui2::Panel* AchievementList::GetCellRenderer(int row)
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
int AchievementList::AddItem(vgui2::Panel* Texture, vgui2::Panel* Title, vgui2::Panel* Description, vgui2::Panel* Progress_NUM, vgui2::Panel* Progress, vgui2::Panel* Progress_BG, int progress_achv, int progress_achv_max, vgui2::Panel* AchievedBG)
{
	if (AchievedBG)
		AchievedBG->SetParent(m_pPanelEmbedded);

	if (Progress_BG)
		Progress_BG->SetParent(m_pPanelEmbedded);

	if (Progress)
		Progress->SetParent(m_pPanelEmbedded);

	if (Title)
		Title->SetParent(m_pPanelEmbedded);

	if (Description)
		Description->SetParent(m_pPanelEmbedded);

	if (Progress_NUM)
		Progress_NUM->SetParent(m_pPanelEmbedded);

	Texture->SetParent(m_pPanelEmbedded);

	int itemID = m_DataItems.AddToTail();
	DATAITEM& newitem = m_DataItems[itemID];
	newitem.title = Title;
	newitem.texture = Texture;
	newitem.desc = Description;
	newitem.progress_num = Progress_NUM;
	newitem.progress = Progress;
	newitem.progress_bg = Progress_BG;
	newitem.achv_progress = progress_achv;
	newitem.achv_progress_max = progress_achv_max;
	newitem.texture_obtained = AchievedBG;
	m_SortedItems.AddToTail(itemID);

	InvalidateLayout();
	return itemID;
}

//-----------------------------------------------------------------------------
// Purpose: iteration accessor
//-----------------------------------------------------------------------------
int	AchievementList::GetItemCount() const
{
	return m_DataItems.Count();
}

int AchievementList::GetItemIDFromRow(int nRow) const
{
	if (nRow < 0 || nRow >= GetItemCount())
		return m_DataItems.InvalidIndex();
	return m_SortedItems[nRow];
}


//-----------------------------------------------------------------------------
// Iteration. Use these until they return InvalidItemID to iterate all the items.
//-----------------------------------------------------------------------------
int AchievementList::FirstItem() const
{
	return m_DataItems.Head();
}

int AchievementList::NextItem(int nItemID) const
{
	return m_DataItems.Next(nItemID);
}

int AchievementList::InvalidItemID() const
{
	return m_DataItems.InvalidIndex();
}


//-----------------------------------------------------------------------------
// Purpose: returns label panel for this itemID
//-----------------------------------------------------------------------------
vgui2::Panel* AchievementList::GetItemLabel(int itemID)
{
	if (!m_DataItems.IsValidIndex(itemID))
		return NULL;

	return m_DataItems[itemID].title;
}

//-----------------------------------------------------------------------------
// Purpose: returns label panel for this itemID
//-----------------------------------------------------------------------------
vgui2::Panel* AchievementList::GetItemPanel(int itemID)
{
	if (!m_DataItems.IsValidIndex(itemID))
		return NULL;

	return m_DataItems[itemID].texture;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void AchievementList::RemoveItem(int itemID)
{
	if (!m_DataItems.IsValidIndex(itemID))
		return;

	DATAITEM& item = m_DataItems[itemID];

	if (item.texture)
		item.texture->MarkForDeletion();

	if (item.title)
		item.title->MarkForDeletion();

	if (item.desc)
		item.desc->MarkForDeletion();

	if (item.progress_num)
		item.progress_num->MarkForDeletion();

	if (item.progress)
		item.progress->MarkForDeletion();

	if (item.progress_bg)
		item.progress_bg->MarkForDeletion();

	if (item.texture_obtained)
		item.texture_obtained->MarkForDeletion();

	m_DataItems.Remove(itemID);
	m_SortedItems.FindAndRemove(itemID);

	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: clears and deletes all the memory used by the data items
//-----------------------------------------------------------------------------
void AchievementList::DeleteAllItems()
{
	FOR_EACH_LL(m_DataItems, i)
	{
		if (m_DataItems[i].texture)
		{
			m_DataItems[i].texture->MarkForDeletion();
			m_DataItems[i].texture = NULL;
		}
		if (m_DataItems[i].progress)
		{
			m_DataItems[i].progress->MarkForDeletion();
			m_DataItems[i].progress = NULL;
		}
		if (m_DataItems[i].progress_bg)
		{
			m_DataItems[i].progress_bg->MarkForDeletion();
			m_DataItems[i].progress_bg = NULL;
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
		if (m_DataItems[i].progress_num)
		{
			m_DataItems[i].progress_num->MarkForDeletion();
			m_DataItems[i].progress_num = NULL;
		}
		if (m_DataItems[i].texture_obtained)
		{
			m_DataItems[i].texture_obtained->MarkForDeletion();
			m_DataItems[i].texture_obtained = NULL;
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
void AchievementList::RemoveAll()
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
void AchievementList::OnSizeChanged(int wide, int tall)
{
	BaseClass::OnSizeChanged(wide, tall);
	InvalidateLayout();
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: relayouts out the panel after any internal changes
//-----------------------------------------------------------------------------
void AchievementList::PerformLayout()
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
	int h = 10;
	int totalh = 0;

	int xpos = m_iFirstColumnWidth + m_iPanelBuffer;
	int iColumnWidth = (wide - xpos - m_vbar->GetWide() - 12) / m_iNumColumns;

	for (int i = 0; i < m_SortedItems.Count(); i++)
	{
		int iCurrentColumn = i % m_iNumColumns;

		// add in a little buffer between panels
		if (iCurrentColumn == 0)
			y += m_iPanelBuffer;

		DATAITEM& item = m_DataItems[m_SortedItems[i]];

		// Override the texture height
		item.texture->SetTall(70);

		if (h < item.texture->GetTall())
			h = item.texture->GetTall();

		item.texture->SetBounds(5, y, iColumnWidth, item.texture->GetTall());

		if (item.title)
			item.title->SetBounds(xpos + iCurrentColumn - 25, y - 2, 400, 20);

		if (item.desc)
			item.desc->SetBounds(xpos + iCurrentColumn - 22, y + 5, 490, 35);

		if (item.progress_num)
			item.progress_num->SetBounds(xpos + iCurrentColumn + 200, y + 30, 150, 12);

		item.texture_obtained->SetBounds(5, y - 2, iColumnWidth + 50, item.texture->GetTall() - 2);

		// Lets calculate the achievement bar
		int barWidth_cur = item.achv_progress;
		int barWidth_max = item.achv_progress_max;
		int barWidth_pos = 0;

		// Get propper ratio of the bar
		float ratio = barWidth_cur / (float)barWidth_max;
		int   realpos = ratio * 475;

		// Make our position bigger!
		for (int i_pos = 0; i_pos < realpos; i_pos++)
			barWidth_pos = i_pos;

		if (barWidth_pos < 475)
			barWidth_pos = barWidth_pos;
		else
			barWidth_pos = 475;

		if (item.progress)
			item.progress->SetBounds(xpos + iCurrentColumn - 25, y + 30, barWidth_pos, 12);

		if (item.progress_bg)
			item.progress_bg->SetBounds(xpos + iCurrentColumn - 25, y + 30, 475, 12);

		if (iCurrentColumn >= m_iNumColumns - 1)
		{
			y += h;
			totalh += h;

			h = 0;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: scheme settings
//-----------------------------------------------------------------------------
void AchievementList::ApplySchemeSettings(vgui2::IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetBorder(pScheme->GetBorder("ButtonDepressedBorder"));
	SetBgColor(GetSchemeColor("ListPanel.BgColor", GetBgColor(), pScheme));
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void AchievementList::OnSliderMoved(int position)
{
	InvalidateLayout();
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void AchievementList::MoveScrollBarToTop()
{
	m_vbar->SetValue(0);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void AchievementList::SetFirstColumnWidth(int width)
{
	m_iFirstColumnWidth = width;
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
int AchievementList::GetFirstColumnWidth()
{
	return m_iFirstColumnWidth;
}

void AchievementList::SetNumColumns(int iNumColumns)
{
	m_iNumColumns = iNumColumns;
}

int AchievementList::GetNumColumns(void)
{
	return m_iNumColumns;
}

//-----------------------------------------------------------------------------
// Purpose: moves the scrollbar with the mousewheel
//-----------------------------------------------------------------------------
void AchievementList::OnMouseWheeled(int delta)
{
	int val = m_vbar->GetValue();
	val -= (delta * DEFAULT_HEIGHT);
	m_vbar->SetValue(val);
}

//-----------------------------------------------------------------------------
// Purpose: selection handler
//-----------------------------------------------------------------------------
void AchievementList::SetSelectedPanel(Panel* panel)
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
vgui2::Panel* AchievementList::GetSelectedPanel()
{
	return m_hSelectedItem;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void AchievementList::ScrollToItem(int itemNumber)
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