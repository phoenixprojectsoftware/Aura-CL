#ifndef GAMEUI_VIEWPORT_H
#define GAMEUI_VIEWPORT_H
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/MessageBox.h>

#ifdef _STEAMWORKS
#include <steamworks/steam_api.h>
#endif

int iSB_AppID = 3416640;
#ifdef _HALO
#define AURA_APPID 70
#else
#define AURA_APPID 3416640
#endif

class CGameUITestPanel;
class CServerBrowser;

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

	bool IsVACBanned() const;

private:
	bool m_bPreventEscape = false;
	int m_iDelayedPreventEscapeFrame = 0;
	vgui2::DHANDLE<CGameUITestPanel> m_hTestPanel;
	vgui2::DHANDLE<CServerBrowser> m_hServerBrowser;

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
