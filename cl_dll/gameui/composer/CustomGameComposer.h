/****
*
* Copyright (c) 2021-2025 The Phoenix Project Software. Some Rights Reserved.
*
* AURA
*
* Custom Game Composer
*
*
****/

#pragma once

#include <vgui_controls/Frame.h>
#include <vgui_controls/ListPanel.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Slider.h>
#include <vgui_controls/Button.h>

using namespace vgui2;

class CMapListPanel;
class CGamemodeListPanel;
class CComposerOptionsPanel;

// main composer window
class CCustomGameComposer : public Frame
{
	DECLARE_CLASS_SIMPLE(CCustomGameComposer, Frame);

public:
	CCustomGameComposer(Panel* pParent);
	void OnCommand(const char* command) override;

	void LoadGamemodesForMap(const char* mapName);

private:
	CMapListPanel* m_pMapList;
	CGamemodeListPanel* m_pGamemodeList;
	CComposerOptionsPanel* m_pOptionsPanel;
	Button* m_pStartButton;
	Button* m_pCancelButton;
};

// map list + thumbnail
class CMapListPanel : public Panel
{
	DECLARE_CLASS_SIMPLE(CMapListPanel, Panel);

public:
	CMapListPanel(Panel* pParent);
	void LoadMaps();
	void AddMap(const char* mapName);
	void OnItemSelected();
	void OnCommand(const char* command) override;
	void SetMapThumbnail(const char* mapName);
	const char* GetSelectedMap();

private:
	ListPanel* m_pList;
	ImagePanel* m_pThumbnail;
};

// gamemode list
class CGamemodeListPanel : public Panel
{
	DECLARE_CLASS_SIMPLE(CGamemodeListPanel, Panel);

public:
	CGamemodeListPanel(Panel* pParent);
	void LoadGamemodesFromDir();
	const char* GetTooltipTextForItem(int itemID);
	const char* GetSelectedGamemode();

private:
	ListPanel* m_pList;
};

// options panel
class CComposerOptionsPanel : public Panel
{
	DECLARE_CLASS_SIMPLE(CComposerOptionsPanel, Panel);

public:
	CComposerOptionsPanel(Panel* pParent);

	void LoadMapcycles();

	const char* GetFragLimit();
	const char* GetTimeLimit();
	const char* GetInfiniteAmmo();
	const char* GetSpawnSystem();
	const char* GetAutoGM();
	const char* GetFlashlight();
	const char* GetFallDamage();
	const char* GetFriendlyFire();
	const char* GetWeaponsStay();
	const char* GetForceRespawn();
	const char* GetAllowCheats();
	void GetConfig(char* buffer, size_t bufSize);

private:
	Label* m_lServerName;
	TextEntry* m_pServerName;

	Label* m_lMaxplayers;
	TextEntry* m_pMaxplayers;

	CheckButton* m_pUseSteamNetwork;
	CheckButton* m_pLAN;
	CheckButton* m_pRealisticFall;
	CheckButton* m_pAutoGamemode;
	CheckButton* m_pFlashlight;
	CheckButton* m_pFriendlyFire;
	CheckButton* m_pWeaponsStay;
	CheckButton* m_pForceRespawn;
	CheckButton* m_pAllowCheats;

	Label* m_lSpawnSystem;
	ComboBox* m_pSpawnSystem;

	Label* m_lInfiniteAmmo;
	ComboBox* m_pInfiniteAmmo;
	
	Label* m_lMapcycle;
	ComboBox* m_pMapcycle;

	Label* m_lFragLimit;
	TextEntry* m_pFragLimit;

	Label* m_lTimeLimit;
	TextEntry* m_pTimeLimit;
};
