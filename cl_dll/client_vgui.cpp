/****
*
* Copyright (c) 2021-2025 The Phoenix Project Software. Some Rights Reserved.
*
* AURA
*
* vgui2_client_impl - Implementation of the VGUI2 client interface.
*
*
****/

#include <tier0/dbg.h>
#include <tier1/interface.h>
#include <tier1/tier1.h>
#include <tier2/tier2.h>
#include <IEngineVGui.h>
#include <FileSystem.h>
#include <KeyValues.h>
#include <vgui/IPanel.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/Controls.h>
#include <convar.h>
#include "client_vgui.h"
#include "vgui/client_viewport.h"
#include "gameui/gameui_viewport.h"

namespace vgui2
{
	HScheme VGui_GetDefaultScheme()
	{
		return 0;
	}
}

// static CGameUIViewport* g_pGameUIViewport = nullptr;

EXPOSE_SINGLE_INTERFACE(CClientVGUI, IClientVGUI, ICLIENTVGUI_NAME);

void CClientVGUI::Initialize(CreateInterfaceFn* pFactories, int iNumFactories)
{
	Msg("CClientVGUI::Initialize called with %d factories\n", iNumFactories);
	ConnectTier1Libraries(pFactories, iNumFactories);
	ConnectTier2Libraries(pFactories, iNumFactories);

	if (!vgui2::VGui_InitInterfacesList("CLIENT", pFactories, iNumFactories))
	{
		Error("Failed to init VGUI2!!\n");
		Assert(false);
	}
	else
	{
		Msg("VGUI2 interfaces initialized successfully.\n");
	}

	static CClientViewport* s_pClientViewport = nullptr;
	static CGameUIViewport* s_pGameUIViewport = nullptr;

	if (!s_pClientViewport)
	{
		s_pClientViewport = new CClientViewport();
		Msg("CClientVGUI::Initialize: Created CClientViewport instance.\n");
	}
	if (!s_pGameUIViewport)
	{
		s_pGameUIViewport = new CGameUIViewport();
		Msg("CClientVGUI::Initialize: Created CGameUIViewport instance.\n");
	}
}

void CClientVGUI::Start()
{

}

void CClientVGUI::SetParent(vgui2::VPANEL parent)
{
	if (!vgui2::ivgui())
	{
		Error("CClientVGUI::SetParent called before vgui2::ivgui() was initialized!\n");
		Assert(false);
		return;
	}

	static bool initialized = false;
	if (!initialized)
	{
		initialized = true;

		new CGameUIViewport();
	}
}

int CClientVGUI::UseVGUI1()
{
	return true;
}

void CClientVGUI::HideScoreBoard()
{
	// g_pViewport->HideScoreBoard();
}

void CClientVGUI::HideAllVGUIMenu()
{
	// g_pViewport->HideAllVGUIMenu();
}

void CClientVGUI::ActivateClientUI()
{
	g_pViewport->ActivateClientUI();
}

void CClientVGUI::HideClientUI()
{
	g_pViewport->HideClientUI();
}

void CClientVGUI::Shutdown()
{
	// Warning! Only called for CS & CZ
	// Do not use!
}