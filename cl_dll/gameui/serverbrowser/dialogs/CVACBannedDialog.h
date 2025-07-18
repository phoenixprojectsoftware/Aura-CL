// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SERVER_BROWSER_DIALOG_VACBANNED_H
#define SERVER_BROWSER_DIALOG_VACBANNED_H

#include <vgui_controls/MessageBox.h>
#include <vgui_controls/ImagePanel.h>

class CVACBannedDialog : public vgui2::MessageBox
{
	DECLARE_CLASS_SIMPLE(CVACBannedDialog, vgui2::MessageBox);

public:
	CVACBannedDialog(vgui2::Panel* pParent);

protected:
	virtual void ApplySchemeSettings(vgui2::IScheme* pScheme);

private:
	vgui2::ImagePanel* m_pImage;
};

#endif