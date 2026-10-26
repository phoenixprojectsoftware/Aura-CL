/****
*
* Copyright (c) 2021-2026 The Phoenix Project Software. Some Rights Reserved.
*
* AURA
*
* Phoenix emblem image
*
*
****/

#include <vgui/ISurface.h>
#include "phoenix_emblem.h"
#include <vgui_controls/Controls.h>

CPhoenixEmblemImage::CPhoenixEmblemImage()
	: m_iX(0),
	m_iY(0),
	m_iWide(16),
	m_iTall(16),
	m_iTextureID(-1),
	m_Color(255,255,255,255)
{}

void CPhoenixEmblemImage::EnsureTextureLoaded()
{
	if (m_iTextureID != -1)
		return;

	m_iTextureID =
		vgui2::surface()->CreateNewTextureID();

	vgui2::surface()->DrawSetTextureFile(m_iTextureID, "ui/gfx/vgui/PhoenixEmblem_TGA", true, false);
}

void CPhoenixEmblemImage::Paint()
{
	EnsureTextureLoaded();

	if (m_iTextureID == -1)
		return;

	vgui2::surface()->DrawSetColor(m_Color);
	vgui2::surface()->DrawSetTexture(m_iTextureID);

	vgui2::surface()->DrawTexturedRect(m_iX, m_iY, m_iX + m_iWide, m_iY + m_iTall);
}

void CPhoenixEmblemImage::SetPos(int x, int y)
{
	m_iX = x;
	m_iY = y;
}

void CPhoenixEmblemImage::GetContentSize(
	int& wide,
	int& tall)
{
	wide = m_iWide;
	tall = m_iTall;
}

void CPhoenixEmblemImage::GetSize(
	int& wide,
	int& tall)
{
	wide = m_iWide;
	tall = m_iTall;
}

void CPhoenixEmblemImage::SetSize(
	int wide,
	int tall)
{
	m_iWide = wide;
	m_iTall = tall;
}

void CPhoenixEmblemImage::SetColor(
	Color color)
{
	m_Color = color;
}
