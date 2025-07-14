#ifndef VGUI2_CLIENT_VIEWPORT_H
#define VGUI2_CLIENT_VIEWPORT_H
#include <vgui_controls/Frame.h>

class CClientViewport : public vgui2::EditablePanel
{
	DECLARE_CLASS_SIMPLE(CClientViewport, vgui2::EditablePanel);

public:
	CClientViewport();
	// No msgfuncs here at all. VGUI1 Legacy is still in use.

private:
	vgui2::AnimationController* m_pAnimController = nullptr;
};

extern CClientViewport* g_pViewport;

#endif
