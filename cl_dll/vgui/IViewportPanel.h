#pragma once

#include <vgui/VGUI2.h>

class IViewportPanel
{
public:
	virtual ~IViewportPanel() {}

	virtual const char* GetName() = 0;
	virtual void Reset() = 0;
	virtual void ShowPanel(bool state) = 0;
	virtual vgui2::VPANEL GetVPanel() = 0;
	virtual bool IsVisible() = 0;
	virtual void SetParent(vgui2::VPANEL parent) = 0;

	// later panels may reload their layout through this
	virtual void ReloadLayout() {}
};
