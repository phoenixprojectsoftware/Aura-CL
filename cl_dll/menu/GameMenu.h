/****
*
* Copyright (c) 2021-2026 The Phoenix Project Software. Some Rights Reserved.
*
* AURA
*
* Game Menu
*
*
****/

#pragma once

#include <RmlUi/Core.h>

class CGameMenu
{
public:
	CGameMenu();
	~CGameMenu();

	bool Init();
	void Shutdown();

	void VidInit();
	void Update(float flTime);
	void Render();

	bool IsInitialized() const;
	bool IsVisible() const;

	void Show();
	void Hide();
	void Toggle();

private:
	bool m_bInitialized;
	bool m_bVisible;

	Rml::Context* m_pContext;
	Rml::ElementDocument* m_pDocument;
};

extern CGameMenu g_GameMenu;
