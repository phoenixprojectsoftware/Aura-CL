#include <tier1/interface.h>
#include <IBaseUI.h>
#include <IEngineVGui.h>
#include <vgui/ISurface.h>
#include <tier2/tier2.h>
#include <vgui/ILocalize.h>
#include "../hud.h"
#include "../client_vgui.h"
#include "gameui_viewport.h"
#include "gameui_test_panel.h"
#include "achievements/C_AchievementDialog.h"
#include "composer/CustomGameComposer.h"
#ifndef _HALO
#include "leaderboard/LeaderboardPanel.h"
#endif
#include "serverbrowser/CServerBrowser.h"
// TODO: ADVANCED OPTIONS
// TODO: ACHIEVEMENT MENU
#include "workshop/CWorkshopDialog.h"
#include "CImageMenuButton.h"
#include <FileSystem.h>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <convar.h>
#include <KeyValues.h>
#include "../console.h"

CON_COMMAND(gameui_opentest, "Opens test")
{
	CGameUIViewport::Get()->OpenTestPanel();
}

CON_COMMAND(gameui_composer, "Composer")
{
	CGameUIViewport::Get()->OpenComposer();
}

#ifndef _HALO
CON_COMMAND(gameui_leaderboard, "Leaderboard")
{
	CGameUIViewport::Get()->OpenLeaderboard();
}
#endif

CGameUIViewport::CGameUIViewport() : BaseClass(nullptr, "ClientGameUIViewport"), m_steamcallback_OnDownloadItemResult(this, &CGameUIViewport::OnDownloadItemResult)
{
	Assert(!m_sInstance);
	m_sInstance = this;
	vgui2::VPANEL parent = g_pEngineVGui->GetPanel(PANEL_GAMEUIDLL);
	SetParent(parent);

	SetScheme(vgui2::scheme()->LoadSchemeFromFile(VGUI2_ROOT_DIR "resource/ClientSourceScheme.res", "ClientSourceScheme"));
	SetProportional(false);
	SetSize(0, 0);
	SetVisible(true);

	m_bDownloadedItemsReady = false;
	m_bPrepareForQueryDownload = false;
	m_hWorkshopInfoBox = nullptr;
	SetQueryWait(1.0f);

	LoadWorkshopItems(false);
	LoadWorkshop();
}

CGameUIViewport::~CGameUIViewport()
{
	Assert(m_sInstance);
	m_sInstance = this;
}

void CGameUIViewport::PreventEscapeToShow(bool state)
{
	if (state)
	{
		m_bPreventEscape = true;
		m_bDelayedPreventEscape = false;
	}
	else
	{
		// PreventEscapeToShow(false) may be called the same frame that esc was pressed
		// and CGameUIViewport::OnThink wont hide gameui
		// so the change is delayed by 1 frame
		m_bPreventEscape = false;
		m_bDelayedPreventEscape = true;
	}
}

void CGameUIViewport::OpenTestPanel()
{
	GetDialog(m_hTestPanel)->Activate();
}

void CGameUIViewport::OpenComposer()
{
	CCustomGameComposer* pComposer = new CCustomGameComposer(this);
	pComposer->MakePopup();
	pComposer->SetVisible(true);
	pComposer->MoveToFront();
}

#ifndef _HALO
CLeaderboardPanel* g_pLeaderboardPanel = nullptr;

void CGameUIViewport::OpenLeaderboard()
{
	if (!g_pLeaderboardPanel)
		g_pLeaderboardPanel = new CLeaderboardPanel(GetVPanel());

	g_pLeaderboardPanel->SetVisible(true);
	g_pLeaderboardPanel->Activate();
}
#endif

void CGameUIViewport::OnThink()
{
	BaseClass::OnThink();

	if (m_bPreventEscape || m_bDelayedPreventEscape)
	{
		g_pBaseUI->HideGameUI();

		// hiding gameui doesnt update the mouse cursor
		g_pVGuiSurface->CalculateMouseVisible();

		// PreventEscapeToShow(false) may be called the same frame that ESC was pressed
		// and CGameUIViewport::OnThink won't hide GameUI
		// So the change is delayed by one frame
		if (m_bDelayedPreventEscape)
			m_bDelayedPreventEscape = false;
	}

	// create the dialog immediately
	if (!m_hImageButton && !SteamUtils()->IsSteamRunningOnSteamDeck())
	{
		int wx, wy, ww, wt;
		vgui2::surface()->GetWorkspaceBounds(wx, wy, ww, wt);
		m_hImageButton = new CImageMenuButton(this, VGUI2_ROOT_DIR "gfx/vgui/discord", "https://discord.gg/mGr94ZqDWU");
		m_hImageButton->MakePopup(false, false);
		m_hImageButton->SetContent((ww - 256) - 50, (wt - 128) - 384, 256, 128); // Right-center so it doesn't cover the ver watermark
	}

	if (m_bPrepareForQueryDownload)
	{
		// We are currently downloading, check for progress.
		if (m_CurrentQueryItem.WorkshopID > 0)
		{
			uint32 eState = SteamUGC()->GetItemState(m_CurrentQueryItem.WorkshopID);
			bool bCompleted = ((eState & k_EItemStateInstalled) != 0);

			uint64 progress[2];
			SteamUGC()->GetItemDownloadInfo(m_CurrentQueryItem.WorkshopID, &progress[0], &progress[1]);
			float flCurrent = (float)progress[0];
			float flTotal = (float)progress[1];
			float flProgress = flCurrent / flTotal;
			SetWorkshopInfoBoxProgress(flProgress);
			if (flProgress == 1.0f || bCompleted)
			{
				m_flQueryWait = 2.0f;
				ShowWorkshopInfoBox(m_CurrentQueryItem.Title, WorkshopInfoBoxState::State_Done);
				m_CurrentQueryItem.WorkshopID = 0;
			}
			return;
		}

		if (m_flQueryWait > 0.0f)
		{
			m_flQueryWait -= 0.25f;
			return;
		}

		// Prepare the next query.
		if (PrepareForQueryDownload())
			return;

		// Clear it
		m_QueryRequests.clear();

		if (!m_bDownloadedItemsReady) return;

		// Returned false? then stop the query download.
		m_bPrepareForQueryDownload = false;

		// Now check the client workshop content
		LoadWorkshopItems(true);
	}
	else
		CheckWorkshopSubscriptions();
}

C_AchievementDialog* CGameUIViewport::GetAchievementDialog()
{
	return GetDialog(m_hAchDialog);
}

CServerBrowser* CGameUIViewport::GetServerBrowser()
{
	return GetDialog(m_hServerBrowser);
}

//TODO: ADV OPTIONS

// TODO: ACH

CWorkshopDialog* CGameUIViewport::GetWorkshopDialog()
{
	return GetDialog(m_hWorkshopDialog);
}

bool CGameUIViewport::IsVACBanned() const
{
	return SteamApps()->BIsVACBanned(); // have you been a naughty boy?
}

void CGameUIViewport::OnDownloadItemResult(DownloadItemResult_t* pCallback)
{
	// Item either downloaded properly or not.
	// Let's continue, and try reading the directory after 1 second delay.
	SetQueryWait(1.0f);
	m_bDownloadedItemsReady = true;
	m_bPrepareForQueryDownload = true;

	Color clr = Color(255, 22, 22, 255);
	const char* szMsg = nullptr;
	switch (pCallback->m_eResult)
	{
	case k_EResultOK: szMsg = "Successfully downloaded Workshop Item"; clr = Color(0, 255, 255, 255); break;
	case k_EResultFail: szMsg = "Failed to download Workshop Item"; break;
	case k_EResultNoConnection: szMsg = "Failed to download: No Internet Connection"; break;
	case k_EResultInvalidPassword: szMsg = "Failed to download: Invalid Password"; break;
	case k_EResultLoggedInElsewhere: szMsg = "Failed to download: Already logged in elsewhere"; break;
	case k_EResultInvalidProtocolVer: szMsg = "Failed to download: Invalid Protocol Version"; break;
	case k_EResultInvalidParam: szMsg = "Failed to download: Invalid parameter"; break;
	case k_EResultFileNotFound: szMsg = "Failed to download: File Not Found"; break;
	case k_EResultAccessDenied: szMsg = "Failed to download: Access Denied"; break;
	case k_EResultTimeout: szMsg = "Failed to download: Connection Timed Out"; break;
	case k_EResultAccountNotFound: szMsg = "Failed to download: Account Not Found"; break;
	case k_EResultBanned: szMsg = "Failed to download: VAC Banned"; break;
	case k_EResultInvalidSteamID: szMsg = "Failed to download: Invalid SteamID"; break;
	case k_EResultNotLoggedOn: szMsg = "Failed to download: Not Logged On"; break;
	case k_EResultServiceUnavailable: szMsg = "Failed to download: Service Unavailable"; break;
	default: szMsg = vgui2::VarArgs("Returned result ID [%i]", pCallback->m_eResult); break;
	}

	ConPrintf(clr, "[Workshop] %s [%llu]\n", szMsg, pCallback->m_nPublishedFileId);
}

void CGameUIViewport::GetCurrentItems(std::vector<vgui2::WorkshopItem>& items)
{
	items = m_Items;
}

void CGameUIViewport::CheckWorkshopSubscriptions()
{
	if (m_flQueryWait > 0.0f)
	{
		m_flQueryWait -= 1.0f;
		return;
	}
	SetQueryWait(1.55f);
	const int MAX_WORKSHOP_ITEMS = 100;
	PublishedFileId_t vWorkshopItems[MAX_WORKSHOP_ITEMS];
	uint32 nItems = SteamUGC()->GetSubscribedItems(vWorkshopItems, MAX_WORKSHOP_ITEMS);
	for (size_t i = 0; i < nItems; i++)
	{
		if (!HasSubscribedToItem(vWorkshopItems[i]))
			DownloadWorkshopAddon(vWorkshopItems[i]);
	}
}

bool CGameUIViewport::HasSubscribedToItem(PublishedFileId_t nWorkshopID)
{
	for (size_t i = 0; i < m_SubscribedItems.size(); i++)
	{
		if (m_SubscribedItems[i] == nWorkshopID)
			return true;
	}
	return false;
}

void CGameUIViewport::UpdateAddonList()
{
	KeyValues* pAddonList = new KeyValues("AddonList");
	KeyValuesAD autodel(pAddonList);
	if (pAddonList->LoadFromFile(g_pFullFileSystem, "addonlist.txt", "WORKSHOP"))
	{
		for (int iID = 0; iID < m_Items.size(); iID++)
		{
			vgui2::WorkshopItem& WorkshopAddon = m_Items[iID];
			std::string strWorkshopID(std::to_string(WorkshopAddon.uWorkshopID));
			pAddonList->SetBool(strWorkshopID.c_str(), WorkshopAddon.bMounted);
		}
	}

	// save the file
	pAddonList->SaveToFile(g_pFullFileSystem, "addonlist.txt", "WORKSHOP");
}

// ===================================
// Purpose: Load Workshop Content
// ===================================
void CGameUIViewport::LoadWorkshop()
{
	if (!SteamAPI_IsSteamRunning()) return;
	if (SteamUGC() && SteamUser())
	{
		handle = SteamUGC()->CreateQueryUserUGCRequest(
			SteamUser()->GetSteamID().GetAccountID(),
			k_EUserUGCList_Subscribed,
			k_EUGCMatchingUGCType_Items_ReadyToUse,
			k_EUserUGCListSortOrder_LastUpdatedDesc,
			(AppId_t)3416640, (AppId_t)3416640, 1 // AURA_APPID
		);
		SteamUGC()->SetReturnChildren(handle, true);
		SteamAPICall_t apiCall = SteamUGC()->SendQueryUGCRequest(handle);
		m_SteamCallResultOnSendQueryUGCRequest.Set(apiCall, this, &CGameUIViewport::OnSendQueryUGCRequest);
	}
}

bool CGameUIViewport::HasLoadedItem(PublishedFileId_t nWorkshopID)
{
	for (int iID = 0; iID < m_Items.size(); iID++)
	{
		vgui2::WorkshopItem& WorkshopAddon = m_Items[iID];
		if (WorkshopAddon.uWorkshopID == nWorkshopID)
			return true;
	}
	return false;
}

void CGameUIViewport::LoadWorkshopItems(bool bWorkshopFolder)
{
	// Load our data from zamnhlmp_workshop
	FileFindHandle_t fh;
	char const* fn = g_pFullFileSystem->FindFirst("*.*", &fh, bWorkshopFolder ? "WORKSTOPDL" : "WORKSHOP");
	if (!fn) return;
	do
	{
		// Setup the path string, and lowercase it, so we don't need to search for both uppercase, and lowercase files.
		char strFile[4028];
		bool isSameDir = false;
		strFile[0] = 0;
		V_strcpy_safe(strFile, fn);
		Q_strlower(strFile);

		// Ignore the same folder
		// And ignore content folder, unless we are loading workshop content trough it.
		if(vgui2::FStrEq(strFile, ".") || vgui2::FStrEq(strFile, ".."))
			isSameDir = true;

		// Folder found!
		if (g_pFullFileSystem->FindIsDirectory(fh) && !isSameDir)
		{
			// Setup the string
			std::string strAddonInfo = strFile;
			strAddonInfo += "/addoninfo.txt";

			// The Workshop file item.
			unsigned long long nFileItem = std::strtoull(strFile, NULL, 0);

			// Check if the file exist
			if (g_pFullFileSystem->FileExists(strAddonInfo.c_str()) && !HasLoadedItem(nFileItem))
			{

				vgui2::WorkshopItem MountAddon;
				MountAddon.iFilterFlag = 0;
				MountAddon.uWorkshopID = nFileItem;
				MountAddon.bMounted = false;
				MountAddon.bIsWorkshopDownload = bWorkshopFolder;

				// Default title
				Q_snprintf(MountAddon.szName, sizeof(MountAddon.szName), "%s", strFile);
				Q_snprintf(MountAddon.szDesc, sizeof(MountAddon.szDesc), "Unknown 3rd party addon");
				Q_snprintf(MountAddon.szAuthor, sizeof(MountAddon.szAuthor), "Unknown Author");

				// Check if the keyvalues exist
				KeyValues* manifest = new KeyValues("AddonInfo");
				if (manifest->LoadFromFile(g_pFullFileSystem, strAddonInfo.c_str(), bWorkshopFolder ? "WORKSTOPDL" : "WORKSHOP"))
				{
					// Go trough all keyvalues, and grab the ones we need
					for (KeyValues* sub = manifest->GetFirstSubKey(); sub != NULL; sub = sub->GetNextKey())
					{
						int length = Q_strlen(sub->GetString()) + 1;
						char* strValue = (char*)malloc(length);

						Q_strcpy(strValue, sub->GetString());

						if (!Q_stricmp(sub->GetName(), "Author"))
							Q_snprintf(MountAddon.szAuthor, sizeof(MountAddon.szAuthor), "%s", strValue);
						else if (!Q_stricmp(sub->GetName(), "Description"))
							Q_snprintf(MountAddon.szDesc, sizeof(MountAddon.szDesc), "%s", strValue);
						else if (!Q_stricmp(sub->GetName(), "Title"))
							Q_snprintf(MountAddon.szName, sizeof(MountAddon.szName), "%s", strValue);
					}
					// Go trough our flags
					KeyValues* pAddonFilterFlags = manifest->FindKey("Tags");
					if (pAddonFilterFlags)
					{
						if (pAddonFilterFlags->GetBool("maps")) MountAddon.iFilterFlag |= vgui2::FILTER_MAP;
						if (pAddonFilterFlags->GetBool("viewmodels")) MountAddon.iFilterFlag |= vgui2::FILTER_WEAPONS;
						if (pAddonFilterFlags->GetBool("sounds")) MountAddon.iFilterFlag |= vgui2::FILTER_SOUNDS;
						if (pAddonFilterFlags->GetBool("playermodels")) MountAddon.iFilterFlag |= vgui2::FILTER_PLAYERMODEL;
						if (pAddonFilterFlags->GetBool("HUD")) MountAddon.iFilterFlag |= vgui2::FILTER_HUD;
						if (pAddonFilterFlags->GetBool("resources")) MountAddon.iFilterFlag |= vgui2::FILTER_RESOURCES;
						if (pAddonFilterFlags->GetBool("sprays")) MountAddon.iFilterFlag |= vgui2::FILTER_SPRAYS;
						if (pAddonFilterFlags->GetBool("mp3")) MountAddon.iFilterFlag |= vgui2::FILTER_MUSIC;
						if (pAddonFilterFlags->GetBool("mapcycles")) MountAddon.iFilterFlag |= vgui2::FILTER_MAPCYCLE;
						if (pAddonFilterFlags->GetBool("trackerscheme")) MountAddon.iFilterFlag |= vgui2::FILTER_TRACKERSCHEME;
						if (pAddonFilterFlags->GetBool("locs")) MountAddon.iFilterFlag |= vgui2::FILTER_LOC;
						if (pAddonFilterFlags->GetBool("ctf")) MountAddon.iFilterFlag |= vgui2::FILTER_CTF;
						if (pAddonFilterFlags->GetBool("dom")) MountAddon.iFilterFlag |= vgui2::FILTER_DOM;
					}
				}
				manifest->deleteThis();
#if defined( _DEBUG ) || defined(CLOSED_BETA)
				// Only show on Debug mode
				ConPrintf(
					Color(255, 255, 0, 255),
					"Workshop Addon [%llu] Added | Flags: %i\n",
					MountAddon.uWorkshopID,
					MountAddon.iFilterFlag
				);
#endif
				// Auto mount, if found.
				AutoMountWorkshopItem(MountAddon);
				m_Items.push_back(MountAddon);
			}
		}

		fn = g_pFullFileSystem->FindNext(fh);
	} while (fn);

	g_pFullFileSystem->FindClose(fh);
}

void CGameUIViewport::AutoMountWorkshopItem(vgui2::WorkshopItem &WorkshopFile)
{
	if (ShouldAutoMount(WorkshopFile.uWorkshopID))
	{
		CGameUIViewport::Get()->ShowWorkshopInfoBox(WorkshopFile.szName, WorkshopInfoBoxState::State_Mounting);
		CGameUIViewport::Get()->MountWorkshopItem(WorkshopFile, nullptr, nullptr);
		return;
	}
	if (!WorkshopIDIsMounted(WorkshopFile.uWorkshopID)) return;
	WorkshopFile.bMounted = true;
}

struct CopyPath
{
	std::string from;
	std::string to;
	uint64 item;
};

// From: https://stackoverflow.com/questions/6163611/compare-two-files
bool CompareFiles(const std::string& p1, const std::string& p2)
{
	std::ifstream f1(p1, std::ifstream::binary | std::ifstream::ate);
	std::ifstream f2(p2, std::ifstream::binary | std::ifstream::ate);

	if (f1.fail() || f2.fail())
	{
		return false; //file problem
	}

	if (f1.tellg() != f2.tellg())
	{
		return false; //size mismatch
	}

	//seek back to beginning and use std::equal to compare contents
	f1.seekg(0, std::ifstream::beg);
	f2.seekg(0, std::ifstream::beg);
	return std::equal(std::istreambuf_iterator<char>(f1.rdbuf()),
		std::istreambuf_iterator<char>(),
		std::istreambuf_iterator<char>(f2.rdbuf()));
}

unsigned CopyFilesToNewDestination(void* Data)
{
	bool bNoChange = CompareFiles(((CopyPath*)Data)->from, ((CopyPath*)Data)->to);
	if (std::filesystem::exists(((CopyPath*)Data)->to))
		CGameUIViewport::Get()->SetConflictingFiles(((CopyPath*)Data)->item, !bNoChange);
	std::ifstream src(((CopyPath*)Data)->from, std::ios::binary);
	std::ofstream dest(((CopyPath*)Data)->to, std::ios::binary);
	dest << src.rdbuf();
	CGameUIViewport::Get()->ShowWorkshopInfoBox(CGameUIViewport::Get()->GetWorkshopItem(((CopyPath*)Data)->item).szName, WorkshopInfoBoxState::State_Done);
	return 1;
}

struct DeleteFile
{
	std::string file;
};

unsigned RemoveFilesFromAddons(void* Data)
{
	g_pFullFileSystem->RemoveFile(((DeleteFile*)Data)->file.c_str(), "ADDON");
	CGameUIViewport::Get()->ShowWorkshopInfoBox(CGameUIViewport::Get()->GetWorkshopItem(((CopyPath*)Data)->item).szName, WorkshopInfoBoxState::State_Done);
	return 1;
}

static bool MountingCheck(const char* szStrFile)
{
	const char* ext = strrchr(szStrFile, '.');
	if (!ext || !ext[1])
	{
		// No extension => deny
		return false;
	}

	// Skip the dot
	ext = ext + 1;

	// Allowed extensions (lowercase). Add or remove as needed.
	const char* allowedExts[] = {
		"wad", "wav", "mp3", "ogg"
	};

	const size_t allowedCount = sizeof(allowedExts) / sizeof(allowedExts[0]);

	bool bAllowed = false;
	for (size_t i = 0; i < allowedCount; ++i)
	{
		if (Q_stricmp(ext, allowedExts[i]) == 0)
		{
			bAllowed = true;
			break;
		}
	}
	return bAllowed;
}

void CGameUIViewport::MountWorkshopItem(vgui2::WorkshopItem WorkshopFile, const char* szPath, const char* szRootPath)
{
	// Copies files to zp_addon,
	// or remove them from zp_addon.
	// Depends really, if we are mounting or not.

	bool bIsRoot = szRootPath ? false : true;

	char path[4028];
	char pathRoot[4028];
	if (!szPath)
		Q_snprintf(path, sizeof(path), "%llu/", WorkshopFile.uWorkshopID);
	else
		Q_snprintf(path, sizeof(path), "%s", szPath);

	if (!szRootPath)
		Q_snprintf(pathRoot, sizeof(pathRoot), "%s", path);
	else
		Q_snprintf(pathRoot, sizeof(pathRoot), "%s", szRootPath);

	// Load our data from zamnhlmp_workshop
	FileFindHandle_t fh;
	char const* fn = g_pFullFileSystem->FindFirst(vgui2::VarArgs("%s*.*", path), &fh, WorkshopFile.bIsWorkshopDownload ? "WORKSTOPDL" : "WORKSHOP");
	if (!fn) return;
	do
	{
		// Setup the path string, and lowercase it, so we don't need to search for both uppercase, and lowercase files.
		char strFile[4028];
		bool bIsValidFile = true;
		strFile[0] = 0;
		V_strcpy_safe(strFile, fn);
		Q_strlower(strFile);

		// Ignore the same folder
		// And ignore content folder, unless we are loading workshop content trough it.
		if (vgui2::FStrEq(strFile, ".") || vgui2::FStrEq(strFile, ".."))
			bIsValidFile = false;

		// If we are a root dir.
		if (bIsRoot && !g_pFullFileSystem->FindIsDirectory(fh))
		{
			bIsValidFile = MountingCheck(strFile);
		}
		// If root, and is a dir, make sure we only allow certain folders.
		else if (bIsRoot && g_pFullFileSystem->FindIsDirectory(fh))
		{
			// WORKSHOP ITEM DIRECTORY WHITELIST
			bIsValidFile = false;
			if (vgui2::FStrEq(strFile, "logos")
				|| vgui2::FStrEq(strFile, "gfx")
				|| vgui2::FStrEq(strFile, "gamemodes")
				|| vgui2::FStrEq(strFile, "ctf")
				|| vgui2::FStrEq(strFile, "dom")
				|| vgui2::FStrEq(strFile, "ff")
				|| vgui2::FStrEq(strFile, "mapcycles")
				|| vgui2::FStrEq(strFile, "locs")
				|| vgui2::FStrEq(strFile, "cfg")
				|| vgui2::FStrEq(strFile, "video")
				|| vgui2::FStrEq(strFile, "maps")
				|| vgui2::FStrEq(strFile, "media")
				|| vgui2::FStrEq(strFile, "models")
				|| vgui2::FStrEq(strFile, "resource")
				|| vgui2::FStrEq(strFile, "sound")
				|| vgui2::FStrEq(strFile, "ui")
				|| vgui2::FStrEq(strFile, "sprites"))
				bIsValidFile = true;
		}

		if (g_pFullFileSystem->FindIsDirectory(fh) && bIsValidFile)
		{
			char strNewPath[4028];
			Q_snprintf(strNewPath, sizeof(strNewPath), "%s%s/", path, strFile);

			// Create our folders, so we can copy the files over!
			std::string strNewPathDir(strNewPath);
			vgui2::STDReplaceString(strNewPathDir, pathRoot, "");
			g_pFullFileSystem->CreateDirHierarchy(strNewPathDir.c_str(), "ADDON");

			MountWorkshopItem(WorkshopFile, strNewPath, pathRoot);
		}
		else if (!g_pFullFileSystem->FindIsDirectory(fh) && bIsValidFile)
		{
			std::string strNewFilePath(std::string(path) + std::string(strFile));
			std::string strNewFilePathDest(strNewFilePath);
			vgui2::STDReplaceString(strNewFilePathDest, pathRoot, "");
			CopyPath* data = new CopyPath;

			if (WorkshopFile.bIsWorkshopDownload)
				data->from = "../../workshop/content/3416640/" + strNewFilePath; // AURA_APPID
			else
				data->from = "zamnhlmp_workshop/" + strNewFilePath;

			data->to = "zamnhlmp_addon/" + strNewFilePathDest;
			data->item = WorkshopFile.uWorkshopID;
			if (!WorkshopFile.bMounted)
				CreateSimpleThread(CopyFilesToNewDestination, data);
			else
			{
				DeleteFile* data = new DeleteFile;
				data->file = strNewFilePathDest;
				CreateSimpleThread(RemoveFilesFromAddons, data);
			}
		}

		fn = g_pFullFileSystem->FindNext(fh);
	} while (fn);

	SetMountedState(WorkshopFile.uWorkshopID, !WorkshopFile.bMounted);
	UpdateAddonList();

	g_pFullFileSystem->FindClose(fh);
}

bool CGameUIViewport::HasConflictingFiles(vgui2::WorkshopItem WorkshopFile)
{
	return WorkshopFile.bFoundConflictingFiles;
}

vgui2::WorkshopItem CGameUIViewport::GetWorkshopItem(PublishedFileId_t nWorkshopID)
{
	for (int iID = 0; iID < m_Items.size(); iID++)
	{
		vgui2::WorkshopItem& WorkshopAddon = m_Items[iID];
		if (WorkshopAddon.uWorkshopID == nWorkshopID)
			return WorkshopAddon;
	}
	return vgui2::WorkshopItem();
}

void CGameUIViewport::SetMountedState(PublishedFileId_t nWorkshopID, bool state)
{
	for (int iID = 0; iID < m_Items.size(); iID++)
	{
		vgui2::WorkshopItem& WorkshopAddon = m_Items[iID];
		if (WorkshopAddon.uWorkshopID == nWorkshopID)
			WorkshopAddon.bMounted = state;
	}
}

void CGameUIViewport::SetConflictingFiles(PublishedFileId_t nWorkshopID, bool state)
{
	for (int iID = 0; iID < m_Items.size(); iID++)
	{
		vgui2::WorkshopItem& WorkshopAddon = m_Items[iID];
		if (WorkshopAddon.uWorkshopID == nWorkshopID)
			WorkshopAddon.bFoundConflictingFiles = state;
	}
}

void CGameUIViewport::ShowWorkshopInfoBox(const char* szText, WorkshopInfoBoxState nState)
{
	if (!szText) return;

	if (!m_hWorkshopInfoBox)
	{
		m_hWorkshopInfoBox = new CCreateWorkshopInfoBox(this);
		m_hWorkshopInfoBox->Activate();
	}

	char buffer[32];
	Q_snprintf(buffer, sizeof(buffer), "%s", szText);
	if (szText[0] == '#')
	{
		wchar_t* pStr = g_pVGuiLocalize->Find(szText);
		if (pStr)
			g_pVGuiLocalize->ConvertUnicodeToANSI(pStr, buffer, sizeof(buffer));
	}

	m_hWorkshopInfoBox->SetData(buffer, nState);
	m_hWorkshopInfoBox->MoveToCenterOfScreen();
	m_hWorkshopInfoBox->MoveToFront();
}

void CGameUIViewport::SetWorkshopInfoBoxProgress(float flProgress)
{
	if (!m_hWorkshopInfoBox) return;
	m_hWorkshopInfoBox->SetProgressState(flProgress);
}

bool CGameUIViewport::WorkshopIDIsMounted(PublishedFileId_t nWorkshopID)
{
	KeyValues* pAddonList = new KeyValues("AddonList");
	KeyValuesAD autodel(pAddonList);
	if (pAddonList->LoadFromFile(g_pFullFileSystem, "addonlist.txt", "WORKSHOP"))
	{
		std::string strWorkshopID(std::to_string(nWorkshopID));
		return pAddonList->GetBool(strWorkshopID.c_str(), false);
	}
	return false;
}

bool CGameUIViewport::ShouldAutoMount(PublishedFileId_t nWorkshopID)
{
	KeyValues* pAddonList = new KeyValues("AddonList");
	KeyValuesAD autodel(pAddonList);
	if (pAddonList->LoadFromFile(g_pFullFileSystem, "addonlist.txt", "WORKSHOP"))
	{
		bool bShouldMount = false;
		std::string strWorkshopID(std::to_string(nWorkshopID));
		KeyValues* pKey = pAddonList->FindKey(strWorkshopID.c_str());
		if (!pKey)
			bShouldMount = true;

		if (!pAddonList->FindKey(strWorkshopID.c_str()))
			return true;
	}
	return false;
}

void CGameUIViewport::OpenFileExplorer(int eFilter, const char* szFolder, const char* szPathID, DialogSelected_t pFunction)
{
	g_pFileDialogManager->OpenFileBrowser(eFilter, szFolder, szPathID, pFunction);
}

void CGameUIViewport::OpenFileExplorer(const char* szFolder, const char* szPathID, DialogSelected_t pFunction)
{
	g_pFileDialogManager->OpenFolderBrowser(szFolder, szPathID, pFunction);
}

void CGameUIViewport::ShowMessageDialog(const char* szTitle, const char* szDescription)
{
	vgui2::MessageBox* pMessageBox = new vgui2::MessageBox(szTitle, szDescription, this);
	pMessageBox->SetOKButtonVisible(true);
	pMessageBox->SetCancelButtonVisible(false);
	pMessageBox->DoModal();
}

void CGameUIViewport::DownloadWorkshopAddon(PublishedFileId_t nWorkshopID)
{
	if (!SteamUGC()->DownloadItem(nWorkshopID, true)) return;
	PrepareForDownload data;
	data.IsDownloading = false;
	data.WorkshopID = nWorkshopID;
	Q_snprintf(data.Title, sizeof(data.Title), "%llu", nWorkshopID);
	m_QueryRequests.push_back(data);

	ShowWorkshopInfoBox(data.Title, WorkshopInfoBoxState::State_GatheringData);

	SetQueryWait(1.15);
	m_bDownloadedItemsReady = false;
	m_bPrepareForQueryDownload = true;
}

// ===================================
// Purpose: Mount said content.
// ===================================
void CGameUIViewport::OnSendQueryUGCRequest(SteamUGCQueryCompleted_t* pCallback, bool bIOFailure)
{
	int childrencount = 0;
	bool bFailed = (bIOFailure || (pCallback->m_eResult != k_EResultOK));
	if (bFailed)
	{
#if defined( SPDLOG )
		SpdLog(
			"workshop_client",
			UTIL_CurrentMapLog(),
			LOGTYPE_WARN,
			"Failed to send query. ErrorID: %i",
			pCallback->m_eResult
		);
#else
		ConPrintf(Color(255, 22, 22, 255), "[Workshop] Failed to send query. ErrorID: %i\n", pCallback->m_eResult);
#endif
		SteamUGC()->ReleaseQueryUGCRequest(handle);
		m_bDownloadedItemsReady = false;
		m_bPrepareForQueryDownload = true;
		return;
	}

	// Create it
	SteamUGCDetails_t* pDetails = new SteamUGCDetails_t;

	// Get our info
	if (SteamUGC()->GetQueryUGCResult(pCallback->m_handle, 0, pDetails))
	{
		PrepareForDownload data;
		data.IsDownloading = false;
		data.WorkshopID = pDetails->m_nPublishedFileId;
		Q_snprintf(data.Title, sizeof(data.Title), "%s", pDetails->m_rgchTitle);
		m_QueryRequests.push_back(data);

		// Show the addon we want to mount
		ShowWorkshopInfoBox(pDetails->m_rgchTitle, WorkshopInfoBoxState::State_GatheringData);
	}

	// Delete it
	if (pDetails)
		delete pDetails;

	SteamUGC()->ReleaseQueryUGCRequest(handle);

	m_flQueryWait = 1.15f;
	m_bPrepareForQueryDownload = true;
}

void CGameUIViewport::SetQueryWait(const float& flTime)
{
	m_flQueryWait = flTime * 60;
}

bool CGameUIViewport::PrepareForQueryDownload()
{
	for (size_t i = 0; i < m_QueryRequests.size(); i++)
	{
		PrepareForDownload& data = m_QueryRequests[i];
		if (data.IsDownloading) continue;
		data.IsDownloading = true;
		bool bCanDownload = SteamUGC()->DownloadItem(m_QueryRequests[i].WorkshopID, true);
		if (bCanDownload)
		{
			ConPrintf(Color(0, 255, 255, 255), "[Workshop] Downloading Workshop Item: %llu\n", data.WorkshopID);
			ShowWorkshopInfoBox(data.Title, WorkshopInfoBoxState::State_Downloading);
			m_CurrentQueryItem = data;
			m_steamcallback_OnDownloadItemResult.Register(this, &CGameUIViewport::OnDownloadItemResult);
			return true;
		}
	}
	return false;
}
