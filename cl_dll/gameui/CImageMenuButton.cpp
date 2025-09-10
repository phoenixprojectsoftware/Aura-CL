/****
 *
 * Copyright (c) 2025 Monochrome Games. Modified under license by The Phoenix Project Software SVG.
 *
 * Image Menu Button
 *
 * This code contains segments licensed to The Phoenix Project Software SVG from Valve. Copyright (c) 2022 Valve.
 *
 *
 ****/

#include <vgui/IVGui.h>
#include "CImageMenuButton.h"
#include <steamworks/steam_api.h>

CImageMenuButton::CImageMenuButton(vgui2::Panel* pParent, const char* szImage, const char* szURL)
	: BaseClass(pParent, "CImageMenuButton") 
{
	SetSize(100, 80);
	SetPos(0, 0);

	m_szURL[0] = 0;
	if (szURL)
		Q_strcpy(m_szURL, szURL);

	m_pPanel = new vgui2::ImagePanel(this, "Image");
	m_pPanel->SetSize(GetWide(), GetTall());
	m_pPanel->SetPos(0, 0);
	m_pPanel->SetShouldScaleImage(true);
	m_pPanel->SetImage(szImage);
	m_pPanel->SetMouseInputEnabled(false);
	m_pPanel->SetKeyBoardInputEnabled(false);
}

void CImageMenuButton::SetContent(const int& x, const int& y, const int& w, const int& h)
{
	SetPos(x, y);
	SetSize(w, h);
	m_pPanel->SetSize(GetWide(), GetTall());
}

void CImageMenuButton::OnMousePressed(vgui2::MouseCode code)
{
	if (code == vgui2::MouseCode::MOUSE_LEFT)
	{
		if (m_szURL && m_szURL[0])
			SteamFriends()->ActivateGameOverlayToWebPage(m_szURL);
		return;
	}

	if (code == vgui2::MouseCode::MOUSE_MIDDLE)
	{
		SteamFriends()->ActivateGameOverlayToWebPage("https://phoenixprojectsoftware.github.io/halogs-website/img/donaldson.jpg");
		return;
	}
	BaseClass::OnMousePressed(code);
}
