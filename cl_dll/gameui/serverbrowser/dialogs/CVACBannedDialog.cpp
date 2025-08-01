// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "CVACBannedDialog.h"

CVACBannedDialog::CVACBannedDialog(vgui2::Panel* pParent) :
	BaseClass("#VAC_ConnectionRefusedTitle", "#VAC_ConnectionRefusedDetail", pParent)
{
	m_pImage = new vgui2::ImagePanel(this, "Image");
	m_pImage->SetShouldScaleImage(true);
	m_pImage->SetImage("resource/VAC_shield");
}

void CVACBannedDialog::ApplySchemeSettings(vgui2::IScheme* pScheme)
{
	// Apply the image pos and size
	m_pImage->SetPos(5, 5);
	m_pImage->SetSize(80, 80);

	BaseClass::ApplySchemeSettings(pScheme);
}