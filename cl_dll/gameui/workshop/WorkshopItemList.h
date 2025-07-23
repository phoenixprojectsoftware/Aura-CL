//========= Copyright (c) 2022 Zombie Panic! Team, All rights reserved. ============//

#ifndef WORKSHOP_ITEM_LIST_H
#define WORKSHOP_ITEM_LIST_H

#ifdef _WIN32
#pragma once
#endif

#include <string>
#include <utllinkedlist.h>
#include <utlvector.h>
#include <vgui_controls/EditablePanel.h>

class KeyValues;

namespace vgui2
{
	enum
	{
		FILTER_NONE = 0,
		FILTER_MAP = (1 << 0),
		FILTER_WEAPONS = (1 << 1),
		FILTER_SOUNDS = (1 << 2),
		FILTER_PLAYERMODEL = (1 << 3),
		FILTER_HUD = (1 << 4),
		FILTER_RESOURCES = (1 << 5),
		FILTER_SPRAYS = (1 << 6),
		FILTER_MUSIC = (1 << 7),
		FILTER_MAPCYCLE = (1 << 8),
		FILTER_TRACKERSCHEME = (1 << 9),
		FILTER_LOC = (1 << 10),
		FILTER_CTF = (1 << 11),
		FILTER_DOM = (1 << 12),

		MAX_CATEGORY_FILTER
	};

	struct WorkshopItem
	{
		char szName[52];
		char szDesc[52];
		char szAuthor[52];
		uint64 uWorkshopID = 0;
		int iFilterFlag = 0;
		bool bMounted = false;
		bool bIsWorkshopDownload = false;
		bool bFoundConflictingFiles = false;
	};

	char* VarArgs(char* format, ...);
	void STDReplaceString(std::string& path, std::string search, std::string replace);
	bool STDContains(std::string path, std::string search);

	inline bool FStrEq(const char* sz1, const char* sz2)
	{
		return (sz1 == sz2 || V_stricmp(sz1, sz2) == 0);
	}

	//-----------------------------------------------------------------------------
	// Purpose: A list of variable height child panels
	// each list item consists of a label-panel pair. Height of the item is
	// determined from the label.
	//-----------------------------------------------------------------------------
	class WorkshopItemList : public vgui2::EditablePanel
	{
		DECLARE_CLASS_SIMPLE(WorkshopItemList, Panel);

	public:
		WorkshopItemList(vgui2::Panel* parent, char const* panelName);
		~WorkshopItemList();

		// DATA & ROW HANDLING
		// The list now owns the panel
		virtual int AddItem(vgui2::Panel* texture, vgui2::Panel* author, vgui2::Panel* title, vgui2::Panel* desc, vgui2::Panel* activated, vgui2::Panel* error_msg, uint64 nWorkshopID, bool bCanEdit = false);
		int	GetItemCount() const;
		int GetItemIDFromRow(int nRow) const;

		// Iteration. Use these until they return InvalidItemID to iterate all the items.
		int FirstItem() const;
		int NextItem(int nItemID) const;
		int InvalidItemID() const;

		virtual vgui2::Panel* GetItemLabel(int itemID);
		virtual vgui2::Panel* GetItemPanel(int itemID);

		vgui2::ScrollBar* GetScrollbar() { return m_vbar; }

		virtual void RemoveItem(int itemID); // removes an item from the table (changing the indices of all following items)
		virtual void DeleteAllItems(); // clears and deletes all the memory used by the data items
		void RemoveAll();

		// painting
		virtual vgui2::Panel* GetCellRenderer(int row);

		// layout
		void SetFirstColumnWidth(int width);
		int GetFirstColumnWidth();
		void SetNumColumns(int iNumColumns);
		int GetNumColumns(void);
		void MoveScrollBarToTop();

		// selection
		void SetSelectedPanel(vgui2::Panel* panel);
		vgui2::Panel* GetSelectedPanel();
		/*
		On a panel being selected, a message gets sent to it
		"PanelSelected"		int "state"
		where state is 1 on selection, 0 on deselection
		*/

		void		SetVerticalBufferPixels(int buffer);

		void		ScrollToItem(int itemNumber);

		CUtlVector< int >* GetSortedVector(void)
		{
			return &m_SortedItems;
		}

		int	ComputeVPixelsNeeded();

	protected:
		// overrides
		virtual void OnSizeChanged(int wide, int tall);
		MESSAGE_FUNC_INT(OnSliderMoved, "ScrollBarSliderMoved", position);
		virtual void PerformLayout();
		virtual void ApplySchemeSettings(vgui2::IScheme* pScheme);
		virtual void OnMouseWheeled(int delta);
		virtual void OnCommand(const char* pcCommand);

	private:


		enum { DEFAULT_HEIGHT = 24, PANELBUFFER = 15 };

		typedef struct dataitem_s
		{
			// Always store a panel pointer
			vgui2::Panel* texture;
			vgui2::Panel* author;
			vgui2::Panel* title;
			vgui2::Panel* desc;
			vgui2::Panel* activated;
			vgui2::Panel* error_msg;
			vgui2::Panel* button;
			vgui2::Panel* button_edit;
			uint64 workshopid;
		} DATAITEM;

		// list of the column headers

		CUtlLinkedList<DATAITEM, int>		m_DataItems;
		CUtlVector<int>						m_SortedItems;

		vgui2::ScrollBar* m_vbar;
		vgui2::Panel* m_pPanelEmbedded;

		vgui2::PHandle					m_hSelectedItem;
		int						m_iFirstColumnWidth;
		int						m_iNumColumns;
		int						m_iDefaultHeight;
		int						m_iPanelBuffer;

		CPanelAnimationVar(bool, m_bAutoHideScrollbar, "autohide_scrollbar", "0");
	};

}
#endif // ACHIVLIST_H
