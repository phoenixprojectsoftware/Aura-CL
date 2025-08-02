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

	CWorkshopSubUpload* pUploadPage = new CWorkshopSubUpload(this);
	CWorkshopSubUploaded* pUploaded = new CWorkshopSubUploaded(this);
	pUploaded->SetPropertyDialog(this);
	pUploaded->SetUploadPage(pUploadPage);

	AddPage(new CWorkshopSubList(this), "#Phoenix_Workshop_Tab_Subscribed");
	AddPage(pUploaded, "#Phoenix_Workshop_Tab_Uploaded");
	/*
	* Sabian: I've commented out the "Your Uploaded Addons"
	* page, because the File Browser is not working for some
	* reason. No clue what's causing it, but it will be diagnosed
	* in the future. For now, Workshop Man will take its place
	* (for heck knows how long).
	*/
	// AddPage(pUploadPage, "#Phoenix_Workshop_Tab_Upload");

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