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

#include "CustomGameComposer.h"
#include <FileSystem.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <KeyValues.h>
#include "../../engineclientcmd.h"
#include <tier0/dbg.h>

using namespace vgui2;

#define GMLIST_TALL 400

//
// CCustomGameComposer
//
CCustomGameComposer::CCustomGameComposer(Panel* pParent) : Frame(pParent, "CustomGameComposer")
{
	SetTitle("#Phoenix_CustomGameTitle", true);
	SetSize(800, 800);
	SetMoveable(true);
	SetSizeable(false);
	SetDeleteSelfOnClose(true);
	MoveToCenterOfScreen();

	m_pMapList = new CMapListPanel(this);
	m_pMapList->SetBounds(10, 40, 250, 500);
	m_pMapList->LoadMaps();

	m_pGamemodeList = new CGamemodeListPanel(this);
	m_pGamemodeList->SetBounds(270, 40, 200, GMLIST_TALL);
	m_pGamemodeList->LoadGamemodesFromDir();

	m_pOptionsPanel = new CComposerOptionsPanel(this);
	m_pOptionsPanel->SetBounds(480, 40, 300, 700);

	m_pStartButton = new Button(this, "StartButton", "Start Game", this, "startgame");
	m_pStartButton->SetBounds(480, 750, 140, 30);

	m_pCancelButton = new Button(this, "CancelButton", "Cancel", this, "cancel");
	m_pCancelButton->SetBounds(640, 750, 140, 30);
}

void CCustomGameComposer::OnCommand(const char* command)
{
	if (!Q_stricmp(command, "startgame"))
	{
		const char* map = m_pMapList->GetSelectedMap();
		const char* gamemode = m_pGamemodeList->GetSelectedGamemode();

		char config[1024];
		m_pOptionsPanel->GetConfig(config, sizeof(config));

		char mapCmd[128];
		Q_snprintf(mapCmd, sizeof(mapCmd), "map %s\n", map);

		char gmCmd[128];
		Q_snprintf(gmCmd, sizeof(gmCmd), "sv_aura_gamemode %s\n", gamemode);

		VGUI2_Cmd(config);
		VGUI2_Cmd(mapCmd);
		VGUI2_Cmd(gmCmd);

		// debug output
		Msg("[Composer] Applied server config: %s\n", config);
		Msg("[Composer] Starting map: %s on %s\n", map, gamemode);

		Close();
	}
	else if (!Q_stricmp(command, "cancel"))
		Close();
	else
		BaseClass::OnCommand(command);
}

void CMapListPanel::AddMap(const char* mapName)
{
	KeyValues* kv = new KeyValues("Map");
	kv->SetString("name", mapName);
	m_pList->AddItem(kv, 0, false, false);
	kv->deleteThis();
}

void CMapListPanel::LoadMaps()
{
	m_pList->RemoveAll();

	FileFindHandle_t findHandle;
	const char* pszFilename = g_pFullFileSystem->FindFirst("maps/*.bsp", &findHandle);  // map codename

	CUtlVector<CUtlString> addedMaps; // track added map names to avoid duplicates

	while (pszFilename)
	{
		if (!pszFilename || !*pszFilename)
		{
			pszFilename = g_pFullFileSystem->FindNext(findHandle);
			continue;
		}
		// strip extension
		char mapName[MAX_PATH]; // clean version of map name
		Q_StripExtension(pszFilename, mapName, sizeof(mapName));

		// check for duplicates
		bool bDuplicate = false;
		for (int i = 0; i < addedMaps.Count(); i++)
		{
			if (!Q_stricmp(addedMaps[i].Get(), mapName))
			{
				bDuplicate = true;
				break;
			}
		}
		if (bDuplicate)
		{
#ifdef _DEBUG
			Msg("LoadMaps() skipping duplicate map: %s\n", mapName);
#endif
			pszFilename = g_pFullFileSystem->FindNext(findHandle);
			continue;
		}

		addedMaps.AddToTail(mapName); // remember we added this map

#ifdef _DEBUG
		Msg("LoadMaps() found map bsp: %s -> %s\n",pszFilename,  mapName);
#endif

		KeyValues* kvMap = new KeyValues("Map");
		kvMap->SetString("name", mapName);
		m_pList->AddItem(kvMap, 0, false, false);
		kvMap->deleteThis();

		pszFilename = g_pFullFileSystem->FindNext(findHandle);
	}
	g_pFullFileSystem->FindClose(findHandle);

	// refresh UI
	m_pList->InvalidateLayout();
	m_pList->Repaint();

	if (m_pList->GetItemCount() > 0)
	{
		m_pList->SetSingleSelectedItem(0);
		OnItemSelected();
	}
}

//
// CMapListPanel
//
CMapListPanel::CMapListPanel(Panel* pParent) : Panel(pParent, "MapListPanel")
{
	m_pList = new ListPanel(this, "MapList");
	m_pList->SetBounds(0, 0, 250, 400);
	m_pList->AddColumnHeader(0, "name", "Map", 128);
	m_pList->SetColumnVisible(0, true);
	m_pList->SetMultiselectEnabled(false);
	m_pList->SetSelectIndividualCells(true); // not sure about keeping this
	m_pList->AddActionSignalTarget(this);

	m_pThumbnail = new ImagePanel(this, "MapThumbnail");
	m_pThumbnail->SetBounds(0, 410, 250, 150);
	m_pThumbnail->SetImage("ui/gfx/vgui/nomap.tga"); // fallback
	m_pThumbnail->SetShouldScaleImage(true);
	m_pThumbnail->SetShouldCenterImage(true);
	m_pThumbnail->SetScaleAmount(1.0f);
	m_pThumbnail->SetPos(4, 4);
}

void CMapListPanel::SetMapThumbnail(const char* mapName)
{
	char path[MAX_PATH];
	Q_snprintf(path, sizeof(path), "maps/thumbs/%s.tga", mapName);

	if (g_pFullFileSystem->FileExists(path))
		m_pThumbnail->SetImage(path);
	else
		m_pThumbnail->SetImage("ui/gfx/vgui/nomap.tga"); // fallback
}

void CMapListPanel::OnItemSelected()
{
	int itemID = m_pList->GetSelectedItem(0);
	if (itemID == -1)
	{
		m_pThumbnail->SetImage("ui/gfx/vgui/nomap.tga"); // fallback
		return;
	}

	KeyValues* kv = m_pList->GetItem(itemID);
	if (kv)
		SetMapThumbnail(kv->GetString("name", "unknown"));
}

void CMapListPanel::OnCommand(const char* command)
{
	if (!Q_stricmp(command, "ItemSelected"))
	{
		OnItemSelected();
	}
	else
		BaseClass::OnCommand(command);
}

const char* CMapListPanel::GetSelectedMap()
{
	int itemID = m_pList->GetSelectedItem(0);
	if (itemID == -1)
		return "Crossfire"; // fallback

	KeyValues* kv = m_pList->GetItem(itemID);
	return kv ? kv->GetString("name", "Crossfire") : "Crossfire"; // fallback to Crossfire if something goes wrong
}

//
// CGamemodeListPanel
//
CGamemodeListPanel::CGamemodeListPanel(Panel* pParent) : Panel(pParent, "GamemodeListPanel")
{
	m_pList = new ListPanel(this, "GamemodeList");
	m_pList->SetBounds(0, 0, 200, GMLIST_TALL);
	m_pList->AddColumnHeader(0, "name", "Gamemode", 128);
	m_pList->SetColumnVisible(0, true);
	m_pList->SetMultiselectEnabled(false);
}

void CGamemodeListPanel::LoadGamemodesFromDir()
{
#ifdef _DEBUG
	Msg("LoadGamemodesFromDir() called\n");
#endif

	m_pList->RemoveAll();

	FileFindHandle_t findHandle;
	const char* pszFilename = g_pFullFileSystem->FindFirst("gamemodes/*.cfg", &findHandle);

	CUtlVector<CUtlString> addedModes; // track added gamemodes to avoid duplicates

	while (pszFilename)
	{
		if (!pszFilename || !*pszFilename)
		{
			pszFilename = g_pFullFileSystem->FindNext(findHandle);
			continue;
		}

		// strip extension
		char gmInternal[MAX_PATH];
		Q_StripExtension(pszFilename, gmInternal, sizeof(gmInternal));

		char fullPath[MAX_PATH];
		Q_snprintf(fullPath, sizeof(fullPath), "gamemodes/%s", pszFilename);

		// check for duplicates
		bool bDuplicate = false;
		for (int i = 0; i < addedModes.Count(); i++)
					{
			if (!Q_stricmp(addedModes[i].Get(), gmInternal))
			{
				bDuplicate = true;
				break;
			}
		}
		if (bDuplicate)
		{
			#ifdef _DEBUG
			Msg("LoadGamemodesFromDir() skipping duplicate gamemode: %s\n", gmInternal);
#endif
			pszFilename = g_pFullFileSystem->FindNext(findHandle);
			continue;
		}

		addedModes.AddToTail(gmInternal); // remember we added this gamemode

		// open file to read first two lines
		FileHandle_t file = g_pFullFileSystem->Open(fullPath, "r");
		char displayName[128] = {0};
		char description[256] = {0};

		if (file)
		{
			char line[256];

			// first line: display name
			if (g_pFullFileSystem->ReadLine(line, sizeof(line), file))
			{
				// first line of a gamemode starts with "//", so skip those two chars
				if (Q_strnicmp(line, "//", 2) == 0)
				{
					Q_strncpy(displayName, line + 2, sizeof(displayName));
					Q_StripPrecedingAndTrailingWhitespace(displayName);
				}
			}

			// second line: description
			if (g_pFullFileSystem->ReadLine(line, sizeof(line), file))
			{
				// second line of a gamemode starts with "//", so skip those two chars
				if (Q_strnicmp(line, "//", 2) == 0)
				{
					Q_strncpy(description, line + 2, sizeof(description));
					Q_StripPrecedingAndTrailingWhitespace(description);
				}
			}

			g_pFullFileSystem->Close(file);
		}

		if (!*displayName)
			Q_strncpy(displayName, gmInternal, sizeof(displayName));

		if (!*description)
			Q_strncpy(description, "No description available.", sizeof(description));

#ifdef _DEBUG
		Msg("Adding gamemode: internal='%s', display='%s'\n", gmInternal, displayName);
#endif

		// Add to list
		KeyValues* kv = new KeyValues("Gamemode");
		kv->SetString("name", displayName);  // visible name
		kv->SetString("internal", gmInternal); // internal name. this gets fed to sv_aura_gamemode
		kv->SetString("description", description); // tooltip text
		m_pList->AddItem(kv, 0, false, false);
		kv->deleteThis();

		pszFilename = g_pFullFileSystem->FindNext(findHandle);
	}

	g_pFullFileSystem->FindClose(findHandle);

	// refresh UI
	m_pList->InvalidateLayout();
	m_pList->Repaint();
}

const char* CGamemodeListPanel::GetTooltipTextForItem(int itemID)
{
	KeyValues* kv = m_pList->GetItem(itemID);
	if (kv)
		return kv->GetString("description", "No description available.");
	return nullptr;
}

const char* CGamemodeListPanel::GetSelectedGamemode()
{
	int itemID = m_pList->GetSelectedItem(0);
	if (itemID == -1)
		return "ffa"; // fallback

	KeyValues* kv = m_pList->GetItem(itemID);
	return kv->GetString("internal", "ffa");
}

#define R_BOUNDARY SetBounds(10, y, 200, 20) // sick of fucking repeatin myself man
//
// CComposeOptionsPanel
//
CComposerOptionsPanel::CComposerOptionsPanel(Panel* pParent) : Panel(pParent, "ComposerOptionsPanel")
{
	int y = 0;

	// HOSTNAME
	m_lServerName = new Label(this, "ServerLabel", "Hostname");
	m_lServerName->SetBounds(10, y, 200, 20);
	y += 30;

	m_pServerName = new TextEntry(this, "ServerName");
	m_pServerName->R_BOUNDARY;
	m_pServerName->SetText("Cross Product Server");
	y += 30;

	// MAXPLAYERS
	m_lMaxplayers = new Label(this, "PlayersLabel", "Max Players");
	m_lMaxplayers->R_BOUNDARY;
	y += 30;

	m_pMaxplayers = new TextEntry(this, "MaxPeepee");
	m_pMaxplayers->SetBounds(10, y, 200, 20);
	m_pMaxplayers->SetText("12");
	y += 30;

	// CHECK BUTTONS
	m_pLAN = new CheckButton(this, "LAN", "Local server");
	m_pLAN->SetBounds(5, y, 200, 20); y += 25;

	m_pRealisticFall = new CheckButton(this, "Fall", "Realistic fall damage");
	m_pRealisticFall->SetBounds(5, y, 200, 20); y += 25;

	m_pAutoGamemode = new CheckButton(this, "AutoGM", "Auto gamemode switch for CTF maps");
	m_pAutoGamemode->SetBounds(5, y, 250, 20); y += 25;

	m_pFriendlyFire = new CheckButton(this, "FF", "Friendly Fire");
	m_pFriendlyFire->SetBounds(5, y, 200, 20); y += 25;

	m_pWeaponsStay = new CheckButton(this, "WS", "Weapons Stay");
	m_pWeaponsStay->SetBounds(5, y, 200, 20); y += 25;

	m_pForceRespawn = new CheckButton(this, "FR", "Force Respawn");
	m_pForceRespawn->SetBounds(5, y, 200, 20); y += 25;

	m_pAllowCheats = new CheckButton(this, "Cheats", "Allow Cheats");
	m_pAllowCheats->SetBounds(5, y, 200, 20); y += 25;

	// SPAWN SYSTEM
	m_lSpawnSystem = new Label(this, "SpawnLabel", "Spawn System");
	m_lSpawnSystem->R_BOUNDARY;
	y += 30;

	m_pSpawnSystem = new ComboBox(this, "SpawnSystem", 3, false);
	m_pSpawnSystem->AddItem("Sequential", nullptr);
	m_pSpawnSystem->AddItem("Random", nullptr);
	m_pSpawnSystem->AddItem("Far", nullptr);
	m_pSpawnSystem->ActivateItem(0);
	m_pSpawnSystem->SetBounds(10, y, 200, 20); y += 30;

	// INFINITE AMMO
	m_lInfiniteAmmo = new Label(this, "InfiniteLabel", "Infinite Ammo Mode");
	m_lInfiniteAmmo->R_BOUNDARY;
	y += 30;

	m_pInfiniteAmmo = new ComboBox(this, "InfiniteAmmo", 3, false);
	m_pInfiniteAmmo->AddItem("Off", nullptr);
	m_pInfiniteAmmo->AddItem("Clip", nullptr);
	m_pInfiniteAmmo->AddItem("Full", nullptr);
	m_pInfiniteAmmo->ActivateItem(0);
	m_pInfiniteAmmo->SetBounds(10, y, 200, 20); y += 30;

	// MAPCYCLE
	m_lMapcycle = new Label(this, "CycleLabel", "Mapcycle");
	m_lMapcycle->R_BOUNDARY;
	y += 30;

	m_pMapcycle = new ComboBox(this, "Mapcycle", 8, false);
	m_pMapcycle->AddItem("default.mc", nullptr); // TODO: enumerate
	m_pMapcycle->ActivateItem(0);
	m_pMapcycle->SetBounds(10, y, 200, 20); y += 30;

	// FRAG LIMIT
	m_lFragLimit = new Label(this, "FragLabel", "Frag Limit");
	m_lFragLimit->R_BOUNDARY;
	y += 30;

	m_pFragLimit = new TextEntry(this, "FragLimit");
	m_pFragLimit->SetBounds(10, y, 200, 20);
	m_pFragLimit->SetText("50"); y += 30;

	// TIME LIMIT
	m_lTimeLimit = new Label(this, "TimeLabel", "Time Limit");
	m_lTimeLimit->R_BOUNDARY;
	y += 30;

	m_pTimeLimit = new TextEntry(this, "TimeLimit");
	m_pTimeLimit->SetBounds(10, y, 200, 20);
	m_pTimeLimit->SetText("15"); y += 30;
}

void CComposerOptionsPanel::GetConfig(char* buffer, size_t bufSize)
{
	Q_strncpy(buffer, "", bufSize);

	char tmp[256];
	char line[256];

	m_pServerName->GetText(tmp, sizeof(tmp));
	Q_snprintf(line, sizeof(line), "deathmatch 1 \nhostname \"%s\"\n", tmp);
	Q_strncat(buffer, line, bufSize);

	m_pMaxplayers->GetText(tmp, sizeof(tmp));
	Q_snprintf(line, sizeof(line), "maxplayers %s\n", tmp);
	Q_strncat(buffer, line, bufSize);

	// Frag / Time limits
	m_pFragLimit->GetText(tmp, sizeof(tmp));
	Q_snprintf(line, sizeof(line), "mp_fraglimit %s\n", tmp);
	Q_strncat(buffer, line, bufSize);

	m_pTimeLimit->GetText(tmp, sizeof(tmp));
	Q_snprintf(line, sizeof(line), "mp_timelimit %s\n", tmp);
	Q_strncat(buffer, line, bufSize);

	if (m_pLAN->IsSelected())
		Q_strncat(buffer, "sv_lan 1\n", bufSize);
	else
		Q_strncat(buffer, "sv_lan 0; sv_use_steam_networking 1\n", bufSize);

	if (m_pRealisticFall->IsSelected())
		Q_strncat(buffer, "mp_falldamage 2\n", bufSize);

	if (m_pAutoGamemode->IsSelected())
		Q_strncat(buffer, "sv_aura_gamemode_auto 1\n", bufSize);
	else
		Q_strncat(buffer, "sv_aura_gamemode_auto 0\n", bufSize);

	if (m_pFriendlyFire->IsSelected())
		Q_strncat(buffer, "mp_friendlyfire 1\n", bufSize);

	if (m_pWeaponsStay->IsSelected())
		Q_strncat(buffer, "mp_weaponstay 1\n", bufSize);

	if (m_pForceRespawn->IsSelected())
		Q_strncat(buffer, "mp_forcerespawn 1\n", bufSize);

	if (m_pAllowCheats->IsSelected())
		Q_strncat(buffer, "sv_cheats 1\n", bufSize);
	else
		Q_strncat(buffer, "sv_cheats 0\n", bufSize);

	int spawnType = m_pSpawnSystem->GetActiveItem();
	switch (spawnType)
	{
	case 0: // Sequential
		Q_strncat(buffer, "ag_spawn_system 0\n", bufSize);
		break;
	case 1: // Random
		Q_strncat(buffer, "ag_spawn_system 1\n", bufSize);
		break;
	case 2: // Far
		Q_strncat(buffer, "ag_spawn_system 2\n", bufSize);
		break;
	default:
		Q_strncat(buffer, "ag_spawn_system 0\n", bufSize);
		break;
	}

	int ammoType = m_pInfiniteAmmo->GetActiveItem();
	switch (ammoType)
	{
	case 0: // Off
		Q_strncat(buffer, "sv_aura_infinite_ammo 0\n", bufSize);
		break;
	case 1: // Clip
		Q_strncat(buffer, "sv_aura_infinite_ammo 1\n", bufSize);
		break;
	case 2: // Full
		Q_strncat(buffer, "sv_aura_infinite_ammo 2\n", bufSize);
		break;
	default:
		Q_strncat(buffer, "sv_aura_infinite_ammo 0\n", bufSize);
		break;
	}

	m_pMapcycle->GetText(tmp, sizeof(tmp));
	Q_snprintf(line, sizeof(line), "mapcyclefile %s\n", tmp);
	Q_strncat(buffer, line, bufSize);
}