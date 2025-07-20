#ifndef GAMEUI_VIEWPORT_H
#define GAMEUI_VIEWPORT_H
#include <vector>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/MessageBox.h>

#ifdef _STEAMWORKS
#include <steamworks/steam_api.h>
#endif

#include "workshop/WorkshopItemList.h"
#include "filedialog/IFileDialogManager.h"
#include "CCreateWorkshopInfoBox.h"

#ifdef _HALO
#define AURA_APPID 70
#else
#define AURA_APPID 3416640
#endif

class CGameUITestPanel;
class CServerBrowser;
// TODO: class CAdvOptionsDialog;
// TODO: class C_AchievementDialog;
class CWorkshopDialog;

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
	CServerBrowser* GetServerBrowser();
	CWorkshopDialog* GetWorkshopDialog();

	bool IsVACBanned() const;

	void GetCurrentItems(std::vector<vgui2::WorkshopItem>& items);
	void AutoMountWorkshopItem(vgui2::WorkshopItem WorkshopFile);
	void MountWorkshopItem(vgui2::WorkshopItem WorkshopFile, const char* szPath, const char* szRootPath);
	bool HasConflictingFiles(vgui2::WorkshopItem WorkshopFile);
	vgui2::WorkshopItem GetWorkshopItem(PublishedFileId_t nWorkshopID);
	void SetConflictingFiles(PublishedFileId_t nWorkshopID, bool state);
	void SetMountedState(PublishedFileId_t nWorkshopID, bool state);

	void ShowWorkshopInfoBox(const char* szText, WorkshopInfoBoxState nState);
	void SetWorkshopInfoBoxProgress(float flProgress);

	bool WorkshopIDIsMounted(PublishedFileId_t nWorkshopID);

	void OpenFileExplorer(int eFilter, const char* szFolder, const char* szPathID, DialogSelected_t pFunction);
	void OpenFileExplorer(const char* szFolder, const char* szPathID, DialogSelected_t pFunction);

	void ShowMessageDialog(const char* szTitle, const char* szDescription);

protected:
	void UpdateAddonList();
	void LoadWorkshop();
	bool HasLoadedItem(PublishedFileId_t nWorkshopID);
	void LoadWorkshopItems(bool bWorkshopFolder);

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
	float m_flQueryWait;
	bool m_bPrepareForQueryDownload;
	bool PrepareForQueryDownload();

private:
	bool m_bPreventEscape = false;
	int m_iDelayedPreventEscapeFrame = 0;
	vgui2::DHANDLE<CGameUITestPanel> m_hTestPanel;
	vgui2::DHANDLE<CServerBrowser> m_hServerBrowser;
	vgui2::DHANDLE<CWorkshopDialog> m_hWorkshopDialog;
	vgui2::DHANDLE<CCreateWorkshopInfoBox> m_hWorkshopInfoBox;

	template <typename T>
	inline T* GetDialog(vgui2::DHANDLE<T>& handle)
	{
		if (!handle.Get())
		{
			handle = new T(this);
		}

		return handle;
	}

	static inline CGameUIViewport* m_sInstance = nullptr;
};

#endif
