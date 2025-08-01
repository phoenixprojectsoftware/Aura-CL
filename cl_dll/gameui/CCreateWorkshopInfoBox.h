// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef CREATE_WORKSHOP_INFO_BOX
#define CREATE_WORKSHOP_INFO_BOX
#include <vgui_controls/Frame.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ProgressBar.h>

enum WorkshopInfoBoxState
{
	State_GatheringData = 0,
	State_Downloading,
	State_Updating,
	State_Dismounting,
	State_Mounting,
	State_Done
};

class CCreateWorkshopInfoBox : public vgui2::Frame
{
	DECLARE_CLASS_SIMPLE(CCreateWorkshopInfoBox, vgui2::Frame);

public:
	CCreateWorkshopInfoBox(vgui2::Panel* pParent);
	void SetData(const char* szString, WorkshopInfoBoxState nState);

	void SetProgressState(float flProgress);

private:
	virtual void OnTick();
	float m_RemoveTime;
	WorkshopInfoBoxState m_state;
	vgui2::Label* m_pText;
	vgui2::Label* m_pState;
	vgui2::ProgressBar* m_pProgressBar;
};

#endif
