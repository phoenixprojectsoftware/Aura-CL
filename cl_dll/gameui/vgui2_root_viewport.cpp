#include "vgui2_root_viewport.h"
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/Label.h>
#include <IEngineVGui.h>

#include "../hud.h"
#include "../cl_util.h"


CRootViewport::CRootViewport() : BaseClass(nullptr, "RootViewport")
{
	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);
	SetProportional(false);

	SetScheme(vgui2::scheme()->LoadSchemeFromFile("ui/resource/ClientScheme.res", "ClientScheme"));

	SetSize(10, 10);

	SetParent(g_pEngineVGui->GetPanel(PANEL_CLIENTDLL));
}