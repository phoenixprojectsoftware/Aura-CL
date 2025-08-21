#include <tier1/interface.h>
#include <IEngineVGui.h>
#include <vgui_controls/PropertySheet.h>
#include "CWorkshopDialog.h"
#include "../../client_vgui.h"
#include "../gameui_viewport.h"
#include "IBaseUI.h"
#include "../../hud.h"
#include "../../cl_util.h"

#include "CWorkshopSubList.h"
#include "CWorkshopSubUploaded.h"
#include "CWorkshopSubUpload.h"

CON_COMMAND(gameui_workshop, "Opens Workshop dialog")
{
	CGameUIViewport::Get()->GetWorkshopDialog()->Activate();
	g_pBaseUI->ActivateGameUI();
}

CWorkshopDialog::CWorkshopDialog(vgui2::Panel* pParent)
	: BaseClass(pParent, "WorkshopDialog")
{
	SetBounds(0, 0, 800, 600);
	SetSizeable(false);
	SetDeleteSelfOnClose(true);

	SetTitle("#Phoenix_Workshop", true);

	AddPage(new CWorkshopSubList(this), "#Phoenix_Workshop_Tab_Subscribed");

	SetOKButtonVisible(false);
	SetApplyButtonVisible(false);
	EnableApplyButton(false);
	SetCancelButtonText("#PropertyDialog_Close");
	GetPropertySheet()->SetTabWidth(84);
	MoveToCenterOfScreen();
}

void CWorkshopDialog::OnCommand(const char* command)
{
	BaseClass::OnCommand(command);
}