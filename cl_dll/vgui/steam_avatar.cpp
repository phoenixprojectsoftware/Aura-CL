/****
 *
 * Copyright (c) 2026 The Phoenix Project Software SVG. All Rights Reserved.
 *
 * AURA-CL STEAM AVATARS
 *
 * This code contains segments licensed to The Phoenix Project Software SVG from Valve. Copyright (c) 2022 Valve.
 *
 *
 ****/

#include <vgui/ISurface.h>

#include "steam_avatar.h"
#include <vgui_controls/Controls.h>
#include "../engineclientcmd.h"

CSteamAvatarImage::CSteamAvatarImage()
	: m_ulSteamID(0),
	m_iTextureID(-1),
	m_iX(0),
	m_iY(0),
	m_iWide(32),
	m_iTall(32),
	m_Color(255, 255, 255, 255),
	m_bValid(false),
	m_bLoadPending(false),
	m_flNextLoadAttempt(0.0f)
{}

CSteamAvatarImage::~CSteamAvatarImage()
{
	m_iTextureID = -1;
}

void CSteamAvatarImage::SetSteamID(uint64 steamID)
{
	if (steamID == m_ulSteamID)
		return;

	Clear();

	if (steamID == 0)
		return;

	m_ulSteamID = steamID;
	m_bLoadPending = true;
	m_flNextLoadAttempt = 0.0f;
}

void CSteamAvatarImage::Clear()
{
	m_ulSteamID = 0;
	m_bValid = false;
	m_bLoadPending = false;
	m_flNextLoadAttempt = 0.0f;

	// Keep the allocated VGUI texture ID. If this player slot later belongs
	// to another Steam user, DrawSetTextureRGBA can safely replace its data.
}

void CSteamAvatarImage::TryLoadAvatar()
{
	if (!m_bLoadPending || m_ulSteamID == 0)
		return;

	ISteamFriends* steamFriends = SteamFriends();
	ISteamUtils* steamUtils = SteamUtils();

	if (!steamFriends || !steamUtils)
		return;

	const CSteamID steamID(m_ulSteamID);

	if (!steamID.IsValid() ||
		!steamID.BIndividualAccount())
	{
		m_bLoadPending = false;
		return;
	}

	const float currentTime =
		gEngfuncs.GetClientTime();

	if (currentTime < m_flNextLoadAttempt)
		return;

	// true means Steam has started fetching this user's information.
	// Wait before asking for their avatar.
	if (steamFriends->RequestUserInformation(
		steamID,
		false))
	{
		m_flNextLoadAttempt =
			currentTime + 1.0f;

		return;
	}

	const int avatarHandle =
		steamFriends->GetSmallFriendAvatar(
			steamID);

	if (avatarHandle <= 0)
	{
		// -1 means it is still being fetched.
		// Treat 0 as retryable too, rather than permanently giving up.
		m_flNextLoadAttempt =
			currentTime + 1.0f;

		return;
	}

	uint32 avatarWide = 0;
	uint32 avatarTall = 0;

	if (!steamUtils->GetImageSize(
		avatarHandle,
		&avatarWide,
		&avatarTall) ||
		avatarWide == 0 ||
		avatarTall == 0)
	{
		m_flNextLoadAttempt =
			currentTime + 1.0f;

		return;
	}

	const uint32 bufferSize =
		avatarWide *
		avatarTall *
		4;

	unsigned char* rgba =
		new unsigned char[bufferSize];

	if (!steamUtils->GetImageRGBA(
		avatarHandle,
		rgba,
		bufferSize))
	{
		delete[] rgba;

		m_flNextLoadAttempt =
			currentTime + 1.0f;

		return;
	}

	if (m_iTextureID < 0)
	{
		m_iTextureID =
			vgui2::surface()->CreateNewTextureID(
				true);
	}

	if (m_iTextureID < 0)
	{
		delete[] rgba;
		m_bLoadPending = false;
		return;
	}

	vgui2::surface()->DrawSetTextureRGBA(
		m_iTextureID,
		rgba,
		static_cast<int>(avatarWide),
		static_cast<int>(avatarTall),
		true,
		false);

	delete[] rgba;

	m_bValid = true;
	m_bLoadPending = false;
}

void CSteamAvatarImage::Paint()
{
	if (m_bLoadPending)
	{
		TryLoadAvatar();
	}

	if (!m_bValid ||
		m_iTextureID < 0)
	{
		return;
	}

	vgui2::surface()->DrawSetColor(
		m_Color);

	vgui2::surface()->DrawSetTexture(
		m_iTextureID);

	vgui2::surface()->DrawTexturedRect(
		m_iX,
		m_iY,
		m_iX + m_iWide,
		m_iY + m_iTall);
}

void CSteamAvatarImage::SetPos(
	int x,
	int y)
{
	m_iX = x;
	m_iY = y;
}

void CSteamAvatarImage::GetContentSize(
	int& wide,
	int& tall)
{
	wide = m_iWide;
	tall = m_iTall;
}

void CSteamAvatarImage::GetSize(
	int& wide,
	int& tall)
{
	GetContentSize(
		wide,
		tall);
}

void CSteamAvatarImage::SetSize(
	int wide,
	int tall)
{
	m_iWide = wide;
	m_iTall = tall;
}

void CSteamAvatarImage::SetColor(
	Color color)
{
	m_Color = color;
}
