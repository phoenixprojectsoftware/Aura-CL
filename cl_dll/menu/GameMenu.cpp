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

//std::string gamedir(gEngfuncs.pfnGetGameDirectory());
std::string gamedir = "zamnhlmp_dev/";

std::string MainMenuPath = gamedir + "ui/menu/main.rml";

CGameMenu::CGameMenu()
	: m_iWidth(0)
	, m_iHeight(0)
	, m_bInitialized(false)
	, m_bVisible(false)
	, m_pContext(nullptr)
	, m_pDocument(nullptr)
{
}

CGameMenu::~CGameMenu()
{
	Shutdown();
}

static int VGuiMouseToRml(vgui2::MouseCode code)
{
	switch (code)
	{
	case vgui2::MOUSE_LEFT:
		return 0;

	case vgui2::MOUSE_RIGHT:
		return 1;

	case vgui2::MOUSE_MIDDLE:
		return 2;

	default:
		return -1;
	}
}

static Rml::Input::KeyIdentifier VGuiKeyToRml(vgui2::KeyCode code)
{
	using namespace vgui2;

	switch (code)
	{
	case KEY_ESCAPE: return Rml::Input::KI_ESCAPE;
	case KEY_ENTER: return Rml::Input::KI_RETURN;
	case KEY_SPACE: return Rml::Input::KI_SPACE;
	case KEY_BACKSPACE: return Rml::Input::KI_BACK;
	case KEY_TAB: return Rml::Input::KI_TAB;

	case KEY_LEFT: return Rml::Input::KI_LEFT;
	case KEY_RIGHT: return Rml::Input::KI_RIGHT;
	case KEY_UP: return Rml::Input::KI_UP;
	case KEY_DOWN: return Rml::Input::KI_DOWN;

	case KEY_HOME: return Rml::Input::KI_HOME;
	case KEY_END: return Rml::Input::KI_END;
	case KEY_DELETE: return Rml::Input::KI_DELETE;
	case KEY_INSERT: return Rml::Input::KI_INSERT;

	case KEY_LSHIFT:
	case KEY_RSHIFT:
		return Rml::Input::KI_LSHIFT;

	case KEY_LCONTROL:
	case KEY_RCONTROL:
		return Rml::Input::KI_LCONTROL;

	case KEY_LALT:
	case KEY_RALT:
		return Rml::Input::KI_LMENU;

	default:
		break;
	}

	if (code >= KEY_0 && code <= KEY_9)
		return static_cast<Rml::Input::KeyIdentifier>(Rml::Input::KI_0 + (code - KEY_0));

	if (code >= KEY_A && code <= KEY_Z)
		return static_cast<Rml::Input::KeyIdentifier>(Rml::Input::KI_A + (code - KEY_A));

	if (code >= KEY_F1 && code <= KEY_F12)
		return static_cast<Rml::Input::KeyIdentifier>(Rml::Input::KI_F1 + (code - KEY_F1));

	return Rml::Input::KI_UNKNOWN;
}

void CGameMenu::OnMouseMove(int x, int y)
{
	if (!m_bInitialized || !m_pContext || !m_bVisible)
		return;

	m_pContext->ProcessMouseMove(x, y, 0);
}

void CGameMenu::OnMousePressed(vgui2::MouseCode code)
{
	if (!m_bInitialized || !m_pContext || !m_bVisible)
		return;

	const int button = VGuiMouseToRml(code);

	if (button < 0)
		return;

	m_pContext->ProcessMouseButtonDown(button, 0);
}

void CGameMenu::OnMouseReleased(vgui2::MouseCode code)
{
	if (!m_bInitialized || !m_pContext || !m_bVisible)
		return;

	const int button = VGuiMouseToRml(code);

	if (button < 0)
		return;

	m_pContext->ProcessMouseButtonUp(button, 0);
}

void CGameMenu::OnMouseWheeled(int delta)
{
	if (!m_bInitialized || !m_pContext || !m_bVisible)
		return;

	m_pContext->ProcessMouseWheel(static_cast<float>(delta), 0);
}

void CGameMenu::OnKeyCodePressed(vgui2::KeyCode code)
{
	if (!m_bInitialized || !m_pContext || !m_bVisible)
		return;

	const Rml::Input::KeyIdentifier key = VGuiKeyToRml(code);

	if (key == Rml::Input::KI_UNKNOWN)
		return;

	m_pContext->ProcessKeyDown(key, 0);
}

void CGameMenu::OnKeyCodeReleased(vgui2::KeyCode code)
{
	if (!m_bInitialized || !m_pContext || !m_bVisible)
		return;

	const Rml::Input::KeyIdentifier key = VGuiKeyToRml(code);

	if (key == Rml::Input::KI_UNKNOWN)
		return;

	m_pContext->ProcessKeyUp(key, 0);
}

void CGameMenu::OnTextInput(wchar_t unichar)
{
	if (!m_bInitialized || !m_pContext || !m_bVisible)
		return;

	if (unichar == 0)
		return;

	m_pContext->ProcessTextInput(static_cast<Rml::Character>(unichar));
}

bool CGameMenu::Init()
{
	if (m_bInitialized)
		return true;

	Rml::SetRenderInterface(&m_RenderInterface);

	if (!Rml::Initialise())
	{
		gEngfuncs.Con_Printf("CGameMenu: Rml::Initialise failed\n");
		return false;
	}

	if (!Rml::LoadFontFace(gamedir + "resource/typeface/nokiafc22.ttf"))
	{
		gEngfuncs.Con_Printf(
			"CGameMenu: failed to load font\n"
		);
	}
	else
	{
		gEngfuncs.Con_Printf("CGameMenu: loaded menu font\n");
	}

	m_bInitialized = true;

	gEngfuncs.Con_Printf("CGameMenu: RmlUi initialized\n");
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

bool CGameMenu::HasContext() const
{
	return m_pContext != nullptr;
}

void CGameMenu::SetViewportSize(int width, int height)
{
	if (!m_bInitialized)
		return;

	if (width <= 0 || height <= 0)
	{
		gEngfuncs.Con_Printf("CGameMenu: invalid viewport %d x %d\n", width, height);
		return;
	}

	m_iWidth = width;
	m_iHeight = height;

	m_RenderInterface.SetViewportSize(width, height);

	if (!m_pContext)
	{
		m_pContext = Rml::CreateContext(
			"AuraGameMenu",
			Rml::Vector2i(width, height)
		);

		if (!m_pContext)
		{
			gEngfuncs.Con_Printf("CGameMenu: failed to create RmlUi context at %d x %d\n", width, height);
			return;
		}

		gEngfuncs.Con_Printf("CGameMenu: context created at %d x %d\n", width, height);
	}
	else
	{
		m_pContext->SetDimensions(Rml::Vector2i(width, height));
	}
}

void CGameMenu::VidInit()
{
	SetViewportSize(m_iWidth, m_iHeight);
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

void CGameMenu::RegisterDocumentEvents()
{
	if (!m_pDocument)
		return;

	const char* buttonIDs[] =
	{
		"serverbrowser",
		"play",
		"multiplayer",
		"composer",
		"workshop",
		"options",
		"extras",
		"quit"
	};

	for (size_t i = 0; i < ARRAYSIZE(buttonIDs); ++i)
	{
		Rml::Element* element = m_pDocument->GetElementById(buttonIDs[i]);

		if (!element)
			continue;

		element->AddEventListener("click", &m_EventListener);
		element->AddEventListener("mouseover", &m_EventListener);

		gEngfuncs.Con_Printf("CGameMenu: registered events for '%s'\n", buttonIDs[i]);
	}
}

void CGameMenu::Show()
{
	if (!m_bInitialized || !m_pContext)
	{
		gEngfuncs.Con_Printf("CGameMenu: Show failed, not initialized or no context\n");
		return;
	}

	if (!m_pDocument)
	{
		m_pDocument = m_pContext->LoadDocument(MainMenuPath);
		RegisterDocumentEvents();

		if (!m_pDocument)
		{
			gEngfuncs.Con_Printf("CGameMenu: failed to load ui/mainmenu/mainmenu.rml\n");
			return;
		}

		gEngfuncs.Con_Printf("CGameMenu: loaded %s\n", MainMenuPath.c_str());
	}

	m_bVisible = true;
	m_pDocument->Show();

	gEngfuncs.Con_Printf("CGameMenu: shown\n");
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
