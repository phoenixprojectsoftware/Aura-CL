// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SERVER_BROWSER_DIALOG_VACBANNED_H
#define SERVER_BROWSER_DIALOG_VACBANNED_H

#include <vgui_controls/MessageBox.h>
#include <vgui_controls/ImagePanel.h>

namespace vgui2
{
	class KeyValues;
}

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

int __cdecl PasswordCompare(vgui2::ListPanel* pPanel, const vgui2::ListPanelItem& p1, const vgui2::ListPanelItem& p2);
int __cdecl BotsCompare(vgui2::ListPanel* pPanel, const vgui2::ListPanelItem& p1, const vgui2::ListPanelItem& p2);
int __cdecl SecureCompare(vgui2::ListPanel* pPanel, const vgui2::ListPanelItem& p1, const vgui2::ListPanelItem& p2);
int __cdecl IPAddressCompare(vgui2::ListPanel* pPanel, const vgui2::ListPanelItem& p1, const vgui2::ListPanelItem& p2);
int __cdecl PingCompare(vgui2::ListPanel* pPanel, const vgui2::ListPanelItem& p1, const vgui2::ListPanelItem& p2);
int __cdecl MapCompare(vgui2::ListPanel* pPanel, const vgui2::ListPanelItem& p1, const vgui2::ListPanelItem& p2);
int __cdecl GameCompare(vgui2::ListPanel* pPanel, const vgui2::ListPanelItem& p1, const vgui2::ListPanelItem& p2);
int __cdecl ServerNameCompare(vgui2::ListPanel* pPanel, const vgui2::ListPanelItem& p1, const vgui2::ListPanelItem& p2);
int __cdecl PlayersCompare(vgui2::ListPanel* pPanel, const vgui2::ListPanelItem& p1, const vgui2::ListPanelItem& p2);
int __cdecl LastPlayedCompare(vgui2::ListPanel* pPanel, const vgui2::ListPanelItem& p1, const vgui2::ListPanelItem& p2);
int __cdecl TagsCompare(vgui2::ListPanel* pPanel, const vgui2::ListPanelItem& p1, const vgui2::ListPanelItem& p2);

#endif