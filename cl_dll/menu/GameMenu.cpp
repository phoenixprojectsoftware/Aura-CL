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

#include "GameMenu.h"

#include "../hud.h"
#include "../cl_util.h"

#include <RmlUi/Core.h>

CGameMenu g_GameMenu;

std::string gamedir(gEngfuncs.pfnGetGameDirectory());

std::string MainMenuPath = gamedir + "ui/menu/main.rml";

CGameMenu::CGameMenu() : m_bInitialized(false), m_bVisible(false), m_pContext(nullptr), m_pDocument(nullptr)
{
}

CGameMenu::~CGameMenu()
{
	Shutdown();
}

bool CGameMenu::Init()
{
	if (m_bInitialized)
		return true;

	if (!Rml::Initialise())
	{
		gEngfuncs.Con_Printf("CGameMenu: Rml::Initialise failed\n");
		return false;
	}

	m_pContext = Rml::CreateContext("AuraGameMenu", Rml::Vector2i(ScreenWidth, ScreenHeight));

	if (!m_pContext)
	{
		gEngfuncs.Con_Printf("CGameMenu: failed to create RmlUi context\n");
		Rml::Shutdown();
		return false;
	}

	m_bInitialized = true;

	gEngfuncs.Con_Printf("CGameMenu initialized\n");
	return true;
}

void CGameMenu::Shutdown()
{
	if (!m_bInitialized)
		return;

	if (m_pDocument)
	{
		m_pDocument->Close();
		m_pDocument = nullptr;
	}

	m_pContext = nullptr;

	Rml::Shutdown();

	m_bInitialized = false;
	m_bVisible = false;

	gEngfuncs.Con_Printf("CGameMenu: shutdown\n");
}

void CGameMenu::VidInit()
{
	if (!m_bInitialized || !m_pContext)
		return;

	m_pContext->SetDimensions(Rml::Vector2i(ScreenWidth, ScreenHeight));
}

void CGameMenu::Update(float flTime)
{
	if (!m_bInitialized || !m_pContext || !m_bVisible)
		return;

	m_pContext->Update();
}

void CGameMenu::Render()
{
	if (!m_bInitialized || !m_pContext || !m_bVisible)
		return;

	m_pContext->Render();
}

bool CGameMenu::IsInitialized() const
{
	return m_bInitialized;
}

bool CGameMenu::IsVisible() const
{
	return m_bVisible;
}

void CGameMenu::Show()
{
	if (!m_bInitialized || !m_pContext)
		return;

	m_bVisible = true;

	if (!m_pDocument)
	{
		m_pDocument = m_pContext->LoadDocument(MainMenuPath);

		const char* path = MainMenuPath.c_str();

		if (!m_pDocument)
		{
			gEngfuncs.Con_Printf("CGameMenu: failed to load %s\n", path);
			return;
		}
	}

	m_pDocument->Show();
}

void CGameMenu::Hide()
{
	m_bVisible = false;

	if (m_pDocument)
		m_pDocument->Hide();
}

void CGameMenu::Toggle()
{
	if (m_bVisible)
		Hide();
	else
		Show();
}
