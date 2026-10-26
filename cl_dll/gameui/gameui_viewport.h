#ifndef GAMEUI_VIEWPORT_H
#define GAMEUI_VIEWPORT_H
#include <vector>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/MessageBox.h>

#include <steamworks/steam_api.h>

#include "workshop/WorkshopItemList.h"
#include "filedialog/IFileDialogManager.h"
#include "CCreateWorkshopInfoBox.h"

#ifdef _HALO
#define AURA_APPID 70
#else
#define AURA_APPID 3416640
#endif

class CGameUITestPanel;
class C_AchievementDialog;
class CServerBrowser;
// TODO: class CAdvOptionsDialog;
// TODO: class C_AchievementDialog;
class CWorkshopDialog;
class CImageMenuButton;

class CGameUIViewport : public vgui2::EditablePanel
{
	DECLARE_CLASS_SIMPLE(CGameUIViewport, vgui2::EditablePanel);

public:
	static inline CGameUIViewport* Get()
	{
		return m_sInstance;
	}

	CGameUIViewport();
	~CGameUIViewport();

	// prevents esc from showing gameui
	// in reality hides gameui whenever it is enabled
	void PreventEscapeToShow(bool state);

	void OpenTestPanel();
	void OpenComposer();
#ifndef _HALO
	void OpenLeaderboard();
#endif
	C_AchievementDialog* GetAchievementDialog();
	CServerBrowser* GetServerBrowser();
	CWorkshopDialog* GetWorkshopDialog();

	virtual void OnThink() override;

	bool IsVACBanned() const;

	void DownloadWorkshopAddon(PublishedFileId_t nWorkshopID);

	void GetCurrentItems(std::vector<vgui2::WorkshopItem>& items);
	void AutoMountWorkshopItem(vgui2::WorkshopItem &WorkshopFile);
	void MountWorkshopItem(vgui2::WorkshopItem WorkshopFile, const char* szPath, const char* szRootPath);
	bool HasConflictingFiles(vgui2::WorkshopItem WorkshopFile);
	vgui2::WorkshopItem GetWorkshopItem(PublishedFileId_t nWorkshopID);
	void SetConflictingFiles(PublishedFileId_t nWorkshopID, bool state);
	void SetMountedState(PublishedFileId_t nWorkshopID, bool state);

	void ShowWorkshopInfoBox(const char* szText, WorkshopInfoBoxState nState);
	void SetWorkshopInfoBoxProgress(float flProgress);

	bool WorkshopIDIsMounted(PublishedFileId_t nWorkshopID);
	bool ShouldAutoMount(PublishedFileId_t nWorkshopID);

	void OpenFileExplorer(int eFilter, const char* szFolder, const char* szPathID, DialogSelected_t pFunction);
	void OpenFileExplorer(const char* szFolder, const char* szPathID, DialogSelected_t pFunction);

	void ShowMessageDialog(const char* szTitle, const char* szDescription);

	static inline CGameUIViewport* m_sInstance = nullptr;

protected:
	void UpdateAddonList();
	void LoadWorkshop();
	void CheckWorkshopSubscriptions();
	bool HasSubscribedToItem(PublishedFileId_t nWorkshopID);
	bool HasLoadedItem(PublishedFileId_t nWorkshopID);
	void LoadWorkshopItems(bool bWorkshopFolder);

	// Our subscribed items. If we sub to a new one we should mount it immediately.
	std::vector<PublishedFileId_t> m_SubscribedItems;

	// list of our sources
	std::vector<vgui2::WorkshopItem> m_Items;

	void OnSendQueryUGCRequest(SteamUGCQueryCompleted_t* pCallback, bool bIOFailure);
	CCallResult<CGameUIViewport, SteamUGCQueryCompleted_t> m_SteamCallResultOnSendQueryUGCRequest;
	UGCQueryHandle_t	handle;

	struct PrepareForDownload
	{
		PublishedFileId_t WorkshopID = 0;
		char Title[k_cchPublishedDocumentTitleMax];
		bool IsDownloading;
	};
	std::vector<PrepareForDownload> m_QueryRequests;
	PrepareForDownload m_CurrentQueryItem;
	bool m_bDownloadedItemsReady;
	float m_flQueryWait;
	void SetQueryWait(const float& flTime);
	bool m_bPrepareForQueryDownload;
	bool PrepareForQueryDownload();

private:
	bool m_bPreventEscape = false;
	int m_bDelayedPreventEscape = 0;
	vgui2::DHANDLE<CGameUITestPanel> m_hTestPanel;
	// vgui2::DHANDLE<CCustomGameComposer> m_hCustomGameComposer;
	vgui2::DHANDLE<C_AchievementDialog> m_hAchDialog;
	vgui2::DHANDLE<CServerBrowser> m_hServerBrowser;
	vgui2::DHANDLE<CWorkshopDialog> m_hWorkshopDialog;
	vgui2::DHANDLE<CCreateWorkshopInfoBox> m_hWorkshopInfoBox;
	vgui2::DHANDLE<CImageMenuButton> m_hImageButton;

	template <typename T>
	inline T* GetDialog(vgui2::DHANDLE<T>& handle)
	{
		if (!handle.Get())
		{
			handle = new T(this);
		}

		return handle;
	}

	// Grab our stats on creation.
	STEAM_CALLBACK(CGameUIViewport, OnDownloadItemResult, DownloadItemResult_t, m_steamcallback_OnDownloadItemResult);
};

#endif
