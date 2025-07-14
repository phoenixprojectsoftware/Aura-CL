/****
*
* Copyright � 2021-2025 The Phoenix Project Software. Some Rights Reserved.
*
* AURA
*
* vgui2_client_impl - Implementation of the VGUI2 client interface.
*
*
****/

#include <vgui/VGUI2.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui/IScheme.h>
#include <vgui/ISystem.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Label.h>
#include <vgui/ILocalize.h>

#include <tier0/dbg.h>
#include <tier1/interface.h>
#include <tier1/tier1.h>
#include <tier2/tier2.h>

#include "IClientVGUI.h"
#include "baseviewport.h"

#ifndef CLIENTVGUI_INTERFACE_VERSION
#define CLIENTVGUI_INTERFACE_VERSION "ClientVGUI001"
#endif

using namespace vgui2;

class CClientVGUI : public IClientVGUI
{
public:
	void Initialize(CreateInterfaceFn* pFactories, int iNumFactories) override;
	void Start() override {}
	void Shutdown() override {}
	void SetParent(VPANEL parent) override;
	int UseVGUI1() override
	{
		return false;
	}
	void HideScoreBoard() override {}
	void HideAllVGUIMenu() override {}
	void ActivateClientUI() override;
	void HideClientUI() override;

private:
	VPANEL m_hParent;
	CGameUIViewport* m_pViewport = nullptr;
};

static CClientVGUI g_ClientVGUI;
IClientVGUI* ClientVGUI = &g_ClientVGUI;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CClientVGUI, IClientVGUI, CLIENTVGUI_INTERFACE_VERSION, g_ClientVGUI);

namespace vgui2
{
	HScheme VGui_GetDefaultScheme() // hey silly bastad. dont forget the capital fukin letta.
	{
		return 0;
	}
}

void CClientVGUI::Initialize(CreateInterfaceFn* pFactories, int iNumFactories)
{
	ConnectTier1Libraries(pFactories, iNumFactories);
	ConnectTier2Libraries(pFactories, iNumFactories);

	if (!vgui2::VGui_InitInterfacesList("CLIENT", pFactories, iNumFactories)) {
		Warning("Failed to initialize VGUI2 interfaces.\n");
		Assert(false);
	}
}

void CClientVGUI::SetParent(VPANEL parent)
{
	m_hParent = parent;

	m_pViewport = new CGameUIViewport();
	m_pViewport->SetParent(m_hParent);
}
