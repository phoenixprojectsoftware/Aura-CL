#include <KeyValues.h>
#include <vgui_controls/AnimationController.h>

#include "client_viewport.h"
#include "hud.h"

CClientViewport* g_pViewport = nullptr;

CClientViewport::CClientViewport() : BaseClass(nullptr, "CClientViewport")
{
	Assert(!g_pViewport);
	g_pViewport = this;

	SetSize(10, 10);
	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);

	vgui2::HScheme scheme = vgui2::scheme()->LoadSchemeFromFile("resource/ClientScheme.res", "ClientScheme");
	SetScheme(scheme);
	SetProportional(true);
}

// no need for hud anims or tfvp stubz.