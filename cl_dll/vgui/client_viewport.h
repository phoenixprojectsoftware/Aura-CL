#ifndef VGUI_CLIENT_VIEWPORT_H
#define VGUI_CLIENT_VIEWPORT_H
#include <vgui_controls/Frame.h>

class CClientViewport : public vgui2::EditablePanel
{
	DECLARE_CLASS_SIMPLE(CClientViewport, vgui2::EditablePanel);

public:
	CClientViewport();
	void ReloadScheme(const char* fromFile);
	void ActivateClientUI();
	void HideClientUI();
};

extern CClientViewport* g_pViewport;

#endif // VGUI_CLIENT_VIEWPORT_H
