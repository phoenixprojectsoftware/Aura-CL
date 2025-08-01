// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SERVER_BROWSER_DIALOG_SERVERPASSWORD_H
#define SERVER_BROWSER_DIALOG_SERVERPASSWORD_H

#include <vgui_controls/MessageBox.h>
#include <vgui_controls/TextEntry.h>

// I don't think we need to use this.
#if 0
class CDialogServerPassword : public vgui2::MessageBox
{
	DECLARE_CLASS_SIMPLE(CDialogServerPassword, vgui2::MessageBox);

public:
	CDialogServerPassword(vgui2::Panel* pParent);

protected:
	virtual void ApplySchemeSettings(vgui2::IScheme* pScheme);
	virtual void OnCommand(const char* szCommand);

private:
	vgui2::TextEntry* m_pEntry;
};
#endif

#endif