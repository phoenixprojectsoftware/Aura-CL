#include <KeyValues.h>
#include <vgui_controls/AnimationController.h>
#include <IEngineVGui.h>

#include "client_viewport.h"
#include "../client_vgui.h"
#include "../hud.h"

CClientViewport* g_pViewport = nullptr;

CClientViewport::CClientViewport() : BaseClass(nullptr, "CClientViewport")
{
	Assert(!g_pViewport);
	g_pViewport = this;

	SetParent(g_pEngineVGui->GetPanel(PANEL_CLIENTDLL));

	SetSize(10, 10);
	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);

	// Load scheme
	ReloadScheme("ui/resource/ClientScheme.res");

	HideClientUI();
}

void CClientViewport::ReloadScheme(const char* fromFile)
{
	if (fromFile != NULL)
	{
		vgui2::HScheme scheme = vgui2::scheme()->LoadSchemeFromFile(fromFile, "HudScheme");

		SetScheme(scheme);
		SetProportional(true);
	}
}

void CClientViewport::ActivateClientUI()
{
	SetVisible(true);
}

void CClientViewport::HideClientUI()
{
	SetVisible(false);
}