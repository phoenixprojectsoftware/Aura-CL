// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include <vgui/IVGui.h>
#include "CCreateWorkshopInfoBox.h"
#include "gameui_viewport.h"
#include "../client_vgui.h"

CCreateWorkshopInfoBox::CCreateWorkshopInfoBox(vgui2::Panel* pParent)
	: BaseClass(pParent, "CreateWorkshopInfoBox")
{
	SetTitle("", false);
	SetSizeable(false);
	SetSize(100, 80);
	SetPos(0, 0);
	SetTitleBarVisible(false);
	SetDeleteSelfOnClose(true);

	m_pText = new vgui2::Label(this, "Text", "My Example Addon");
	m_pState = new vgui2::Label(this, "State", "#Phoenix_Workshop_InfoBox_GatheringData");
	m_pProgressBar = new vgui2::ProgressBar(this, "Progress");

	SetScheme(vgui2::scheme()->LoadSchemeFromFile(VGUI2_ROOT_DIR "resource/ClientSourceSchemeBase.res", "ClientSourceSchemeBase"));

	LoadControlSettings(VGUI2_ROOT_DIR "resource/workshop/infobox.res");

	SetScheme(CGameUIViewport::Get()->GetScheme());
	InvalidateLayout();

	vgui2::ivgui()->AddTickSignal(GetVPanel(), 1000);

	MoveToFront();

	m_RemoveTime = 1.0f;
}

void CCreateWorkshopInfoBox::SetData(const char* szString, WorkshopInfoBoxState nState)
{
	m_pText->SetColorCodedText(szString);
	switch (nState)
	{
	case State_GatheringData: m_pState->SetText("#Phoenix_Workshop_InfoBox_GatheringData"); break;
	case State_Downloading: m_pState->SetText("#Phoenix_Workshop_InfoBox_Downloading"); break;
	case State_Updating: m_pState->SetText("#Phoenix_Workshop_InfoBox_Updating"); break;
	case State_Dismounting: m_pState->SetText("#Phoenix_Workshop_InfoBox_Dismounting"); break;
	case State_Mounting: m_pState->SetText("#Phoenix_Workshop_InfoBox_Mounting"); break;
	case State_Done: m_pState->SetText(""); break;
	}
	if (nState == WorkshopInfoBoxState::State_Done)
		m_RemoveTime = 2.0f;
}

void CCreateWorkshopInfoBox::SetProgressState(float flProgress)
{
	m_pProgressBar->SetProgress(flProgress);
}

void CCreateWorkshopInfoBox::OnTick()
{
	BaseClass::OnTick();
	if (m_RemoveTime <= -1) return;
	if (m_RemoveTime <= 0)
		Close();
	m_RemoveTime--;
}
