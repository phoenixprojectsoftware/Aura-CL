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
#include "console.h"
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

	// load workshop paths
	g_pFullFileSystem->AddSearchPath("../../workshop/content/3416640", "WORKSHOPDL");
	g_pFullFileSystem->AddSearchPath("zamnhlmp_workshop", "WORKSHOP");
	g_pFullFileSystem->AddSearchPath("zamnhlmp_addon", "ADDON");

	// HL25 == 640x480->1280x720
	vgui2::VGui_SetProportionalBaseCallback(&GetProportionalBase);

	// Add language files
	g_pVGuiLocalize->AddFile(g_pFullFileSystem, VGUI2_ROOT_DIR "resource/language/bugfixedhl_%language%.txt");

	new CGameUIViewport;
	new CClientViewport;
}

void CClientVGUI::Start()
{

}

void CClientVGUI::SetParent(vgui2::VPANEL parent)
{
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

void CClientVGUI::GetProportionalBase(int& wide, int& tall)
{
	// Pre-HL25
	wide = 640;
	tall = 480;
}

static void DumpPanel(vgui2::VPANEL panel, int offset, bool bParentVisible)
{
	char buf[256];
	memset(buf, ' ', sizeof(buf));
	if (offset * 10 >= sizeof(buf))
		offset = sizeof(buf) - 1;
	buf[offset] = '\0';

	int wide, tall;
	g_pVGuiPanel->GetSize(panel, wide, tall);
	bool bIsVisible = g_pVGuiPanel->IsVisible(panel) && bParentVisible;
	Color color = console::GetColor();
	if (bParentVisible)
	{
		if (bIsVisible)
			color = ConColor::Green;
		else
			color = ConColor::Red;
	}

	ConPrintf(color, "%s%s [%s %d x %d]\n", buf,
	    g_pVGuiPanel->GetName(panel),
	    g_pVGuiPanel->GetClassName(panel),
	    wide, tall);

	int count = g_pVGuiPanel->GetChildCount(panel);
	for (int i = 0; i < count; i++)
	{
		DumpPanel(g_pVGuiPanel->GetChild(panel, i), offset + 1, bIsVisible);
	}
}

CON_COMMAND(vgui_dumptree, "Dumps VGUI2 panel tree for debugging.")
{
	ConPrintf("Green - visible\nRed - hidden\n\n");
	DumpPanel(g_pEngineVGui->GetPanel(PANEL_ROOT), 0, true);
}