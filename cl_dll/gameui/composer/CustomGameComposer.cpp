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

using namespace vgui2;

//
// CCustomGameComposer
//
CCustomGameComposer::CCustomGameComposer(Panel* pParent) : Frame(pParent, "CustomGameComposer")
{
	SetTitle("#Phoenix_CustomGameTitle", true);
	SetSize(800, 600);
	SetMoveable(true);
	SetSizeable(false);
	SetDeleteSelfOnClose(true);

	m_pMapList = new CMapListPanel(this);
	m_pMapList->SetBounds(10, 40, 250, 500);

	m_pGamemodeList = new CGamemodeListPanel(this);
	m_pGamemodeList->SetBounds(270, 40, 200, 500);

	m_pOptionsPanel = new CComposerOptionsPanel(this);
	m_pOptionsPanel->SetBounds(480, 40, 300, 500);

	m_pStartButton = new Button(this, "StartButton", "Start Game", this, "startgame");
	m_pStartButton->SetBounds(480, 550, 140, 30);

	m_pCancelButton = new Button(this, "CancelButton", "Cancel", this, "cancel");
	m_pCancelButton->SetBounds(640, 550, 140, 30);

	LoadMaps();
}

void CCustomGameComposer::OnCommand(const char* command)
{
	if (!Q_stricmp(command, "startgame"))
	{
		const char* map = m_pMapList->GetSelectedMap();
		const char* gamemode = m_pGamemodeList->GetSelectedGamemode();

		char config[1024];
		m_pOptionsPanel->GetConfig(config, sizeof(config));

		// build command string (placeholder, will expand)
		char cmd[128];
		Q_snprintf(cmd, sizeof(cmd), "map %s\n", map);

		VGUI2_Cmd(config);
		VGUI2_Cmd(cmd);

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

void CCustomGameComposer::LoadMaps()
{
	// enumerate maps directory for BSP files
	FileFindHandle_t findHandle;
	const char* pszFile = g_pFullFileSystem->FindFirst("maps/*.bsp", &findHandle);
	while (pszFile)
	{
		char mapName[MAX_PATH];
		Q_StripExtension(pszFile, mapName, sizeof(mapName));

		KeyValues* kv = new KeyValues("Map");
		kv->SetString("name", mapName);
		m_pMapList->AddMap(mapName);
		kv->deleteThis();

		pszFile = g_pFullFileSystem->FindNext(findHandle);
	}
	g_pFullFileSystem->FindClose(findHandle);
}

void CCustomGameComposer::LoadGamemodesForMap(const char* mapName)
{
	m_pGamemodeList->LoadGamemodesFromTag(mapName);
}

//
// CMapListPanel
//
CMapListPanel::CMapListPanel(Panel* pParent) : Panel(pParent, "MapListPanel")
{
	m_pList = new ListPanel(this, "MapList");
	m_pList->SetBounds(0, 0, 250, 400);

	m_pThumbnail = new ImagePanel(this, "MapThumbnail");
	m_pThumbnail->SetBounds(0, 410, 250, 90);
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

const char* CMapListPanel::GetSelectedMap()
{
	int itemID = m_pList->GetSelectedItem(0);
	if (itemID == -1)
		return "Crossfire"; // fallback

	KeyValues* kv = m_pList->GetItem(itemID);
	return kv->GetString("name", "Crossfire");
}

//
// CGamemodeListPanel
//
CGamemodeListPanel::CGamemodeListPanel(Panel* pParent) : Panel(pParent, "GamemodeListPanel")
{
	m_pList = new ListPanel(this, "GamemodeList");
	m_pList->SetBounds(0, 0, 200, 500);
}

void CGamemodeListPanel::LoadGamemodesFromTag(const char* mapName)
{
	m_pList->RemoveAll();

	char path[MAX_PATH];
	Q_snprintf(path, sizeof(path), "maps/%s.tag", mapName);

	if (!g_pFullFileSystem->FileExists(path))
		return;

	FileHandle_t fh = g_pFullFileSystem->Open(path, "r");
	if (!fh)
		return;

	char line[128];
	while (g_pFullFileSystem->ReadLine(line, sizeof(line), fh))
	{
		Q_StripPrecedingAndTrailingWhitespace(line);
		if (line[0] == '\0')
			continue;

		KeyValues* kv = new KeyValues("Gamemode");
		kv->SetString("name", line);
		m_pList->AddItem(kv, 0, false, false);
		kv->deleteThis();
	}
	g_pFullFileSystem->Close(fh);
}

const char* CGamemodeListPanel::GetSelectedGamemode()
{
	int itemID = m_pList->GetSelectedItem(0);
	if (itemID == -1)
		return "arcade";

	KeyValues* kv = m_pList->GetItem(itemID);
	return kv->GetString("name", "arcade");
}

//
// CComposeOptionsPanel
//
CComposerOptionsPanel::CComposerOptionsPanel(Panel* pParent) : Panel(pParent, "ComposerOptionsPanel")
{
	int y = 0;

	m_pServerName = new TextEntry(this, "ServerName");
	m_pServerName->SetBounds(10, y, 200, 20);
	m_pServerName->SetText("Cross Product Server");
	y += 30;

	m_pLAN = new CheckButton(this, "LAN", "Local server");
	m_pLAN->SetBounds(10, y, 200, 20); y += 25;

	m_pRealisticFall = new CheckButton(this, "Fall", "Realistic fall damage");
	m_pRealisticFall->SetBounds(10, y, 200, 20); y += 25;

	m_pAutoGamemode = new CheckButton(this, "AutoGM", "Auto gamemode switch for CTF maps");
	m_pAutoGamemode->SetBounds(10, y, 200, 20); y += 25;

	m_pFriendlyFire = new CheckButton(this, "FF", "Friendly Fire");
	m_pFriendlyFire->SetBounds(10, y, 200, 20); y += 25;

	m_pWeaponsStay = new CheckButton(this, "WS", "Weapons Stay");
	m_pWeaponsStay->SetBounds(10, y, 200, 20); y += 25;

	m_pForceRespawn = new CheckButton(this, "FR", "Force Respawn");
	m_pForceRespawn->SetBounds(10, y, 200, 20); y += 25;

	m_pAllowCheats = new CheckButton(this, "Cheats", "Allow Cheats");
	m_pAllowCheats->SetBounds(10, y, 200, 20); y += 25;

	m_pSpawnSystem = new ComboBox(this, "SpawnSystem", 3, false);
	m_pSpawnSystem->AddItem("Sequential", nullptr);
	m_pSpawnSystem->AddItem("Random", nullptr);
	m_pSpawnSystem->AddItem("Far", nullptr);
	m_pSpawnSystem->SetBounds(10, y, 200, 20); y += 30;

	m_pInfiniteAmmo = new ComboBox(this, "InfiniteAmmo", 3, false);
	m_pInfiniteAmmo->AddItem("Off", nullptr);
	m_pInfiniteAmmo->AddItem("Clip", nullptr);
	m_pInfiniteAmmo->AddItem("Full", nullptr);
	m_pInfiniteAmmo->SetBounds(10, y, 200, 20); y += 30;

	m_pMapcycle = new ComboBox(this, "Mapcycle", 8, false);
	m_pMapcycle->AddItem("default.mc", nullptr); // TODO: enumerate
	m_pMapcycle->SetBounds(10, y, 200, 20); y += 30;

	m_pFragLimit = new TextEntry(this, "FragLimit");
	m_pFragLimit->SetBounds(10, y, 80, 20);
	m_pFragLimit->SetText("50"); y += 30;

	m_pTimeLimit = new TextEntry(this, "TimeLimit");
	m_pTimeLimit->SetBounds(10, y, 80, 20);
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

	if (m_pLAN->IsSelected())
		Q_strncat(buffer, "sv_lan 1\n", bufSize);
	else
		Q_strncat(buffer, "sv_lan 0; sv_use_steam_networking 1\n", bufSize);

	if (m_pRealisticFall->IsSelected())
		Q_strncat(buffer, "mp_falldamage 2\n", bufSize);

	if (m_pFriendlyFire->IsSelected())
		Q_strncat(buffer, "mp_friendlyfire 1\n", bufSize);

	if (m_pWeaponsStay->IsSelected())
		Q_strncat(buffer, "mp_weaponstay 1\n", bufSize);

	if (m_pForceRespawn->IsSelected())
		Q_strncat(buffer, "mp_forcerespawn 1\n", bufSize);

	if (m_pAllowCheats->IsSelected())
		Q_strncat(buffer, "sv_cheats 1\n", bufSize);

	// Frag / Time limits
	m_pFragLimit->GetText(tmp, sizeof(tmp));
	Q_snprintf(line, sizeof(line), "mp_fraglimit %s\n", tmp);
	Q_strncat(buffer, line, bufSize);

	m_pTimeLimit->GetText(tmp, sizeof(tmp));
	Q_snprintf(line, sizeof(line), "mp_timelimit %s\n", tmp);
	Q_strncat(buffer, line, bufSize);
}