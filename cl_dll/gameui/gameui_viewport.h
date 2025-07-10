/****
*
* Copyright © 2021-2025 The Phoenix Project Software. Some Rights Reserved.
*
* AURA
*
* gameui_viewport - GameUI viewport for the VGUI2 client interface. Based upon
* BugfixedHL-Rebased. Used as per the license agreement.
*
*
****/

#define VGUI2_ROOT_DIR "ui/"
#ifndef GAMEUI_VIEWPORT_H
#define GAMEUI_VIEWPORT_H
#include <vgui_controls/EditablePanel.h>

class CGameUITestPanel;

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

	void OpenTestPanel();

private:
	vgui2::DHANDLE<CGameUITestPanel> m_hTestPanel;

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
