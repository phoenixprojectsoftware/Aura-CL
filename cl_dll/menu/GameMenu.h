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
#include "GameMenuRenderInterface.h"

#include <vgui/KeyCode.h>
#include <vgui/MouseCode.h>

class CGameMenu
{
public:
	CGameMenu();
	~CGameMenu();

	bool Init();
	void Shutdown();

	bool HasContext() const;

	void SetViewportSize(int width, int height);

	void VidInit();
	void Update(float flTime);
	void Render();

	bool IsInitialized() const;
	bool IsVisible() const;

	void Show();
	void Hide();
	void Toggle();

	void OnMouseMove(int x, int y);
	void OnMousePressed(vgui2::MouseCode code);
	void OnMouseReleased(vgui2::MouseCode code);
	void OnMouseWheeled(int delta);

	void OnKeyCodePressed(vgui2::KeyCode code);
	void OnKeyCodeReleased(vgui2::KeyCode code);
	void OnTextInput(wchar_t unichar);

	int m_iWidth;
	int m_iHeight;

private:
	bool m_bInitialized;
	bool m_bVisible;

	CGameMenuRenderInterface m_RenderInterface;

	Rml::Context* m_pContext;
	Rml::ElementDocument* m_pDocument;
};

extern CGameMenu g_GameMenu;
