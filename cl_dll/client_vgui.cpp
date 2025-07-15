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

	// HL25 == 640x480->1280x720
	vgui2::VGui_SetProportionalBaseCallback(&GetProportionalBase);

	// Add language files
	g_pVGuiLocalize->AddFile(g_pFullFileSystem, VGUI2_ROOT_DIR "resource/language/bugfixedhl_%language%.txt");

	new CClientViewport();
	new CGameUIViewport();
}

void CClientVGUI::Start()
{

}

void CClientVGUI::SetParent(vgui2::VPANEL parent)
{
}

int CClientVGUI::UseVGUI1()
{
	return false;
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

void CClientVGUI::GetProportionalBase(int& wide, int& tall)
{
	// Pre-HL25
	wide = 640;
	tall = 480;
}
