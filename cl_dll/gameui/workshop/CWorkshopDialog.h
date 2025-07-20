// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef CWORKSHOP_DIALOG_H
#define CWORKSHOP_DIALOG_H

#include <vgui/VGUI2.h>
#include <vgui_controls/PropertyDialog.h>

class CWorkshopDialog : public vgui2::PropertyDialog
{
public:
	DECLARE_CLASS_SIMPLE(CWorkshopDialog, vgui2::PropertyDialog);

	CWorkshopDialog(vgui2::Panel* pParent);

	virtual void OnCommand(const char* command);
};

#endif
