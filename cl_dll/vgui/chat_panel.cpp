#include <cstdio>
#include <cstring>

#include <vgui/IScheme.h>
#include <vgui/ISurface.h>

#include "chat_panel.h"
#include "viewport_panel_names.h"

#include "../hud.h"

extern int iTeamColors[5][3];
extern int iNumberOfTeamColors;

namespace
{
	constexpr double CHAT_DISPLAY_TIME = 12.0;
	constexpr double CHAT_FADE_TIME = 1.0;

	constexpr int CHAT_HISTORY_ALPHA = 64;

	Color GetAuraChatClientColor(
		int clientIndex)
	{
		if (clientIndex < 1 ||
			clientIndex > MAX_PLAYERS)
		{
			return Color(
				255,
				178,
				0,
				255);
		}

		if (g_IsSpectator[clientIndex])
		{
			return Color(
				180,
				180,
				180,
				255);
		}

		if (gHUD.m_Teamplay &&
			iNumberOfTeamColors > 0)
		{
			int colorIndex =
				g_PlayerExtraInfo[clientIndex].teamnumber %
				iNumberOfTeamColors;

			if (colorIndex < 0)
			{
				colorIndex +=
					iNumberOfTeamColors;
			}

			return Color(
				iTeamColors[colorIndex][0],
				iTeamColors[colorIndex][1],
				iTeamColors[colorIndex][2],
				255);
		}

		return Color(
			255,
			178,
			0,
			255);
	}

	void InsertFormattedChatText(
		vgui2::RichText* target,
		const char* text,
		int clientIndex,
		Color defaultColor,
		int alpha)
	{
		if (!target ||
			!text ||
			text[0] == '\0')
		{
			return;
		}

		target->SetText("");

		Color textColor(
			defaultColor.r(),
			defaultColor.g(),
			defaultColor.b(),
			alpha);

		if (clientIndex < 1 ||
			clientIndex > MAX_PLAYERS)
		{
			target->InsertColorChange(
				textColor);

			target->InsertString(
				text);

			return;
		}

		const char* separator =
			strchr(text, ':');

		if (!separator)
		{
			Color playerColor =
				GetAuraChatClientColor(
					clientIndex);

			playerColor[3] = alpha;

			target->InsertColorChange(
				playerColor);

			target->InsertString(
				text);

			return;
		}

		const std::size_t prefixLength =
			static_cast<std::size_t>(
				separator - text) + 1;

		char prefix[1024];

		std::size_t copyLength =
			prefixLength;

		if (copyLength >= sizeof(prefix))
		{
			copyLength =
				sizeof(prefix) - 1;
		}

		memcpy(
			prefix,
			text,
			copyLength);

		prefix[copyLength] =
			'\0';

		Color playerColor =
			GetAuraChatClientColor(
				clientIndex);

		playerColor[3] = alpha;

		target->InsertColorChange(
			playerColor);

		target->InsertString(
			prefix);

		target->InsertColorChange(
			textColor);

		target->InsertString(
			text + prefixLength);
	}
}


// -------------------------------------------------------------------------
// CAuraChatHistory
// -------------------------------------------------------------------------

CAuraChatHistory::CAuraChatHistory(
	vgui2::Panel* parent,
	const char* panelName)
	: BaseClass(
		parent,
		panelName)
{
	const vgui2::HScheme scheme =
		vgui2::scheme()->LoadSchemeFromFile(
			"ui/resource/ChatScheme.res",
			"AuraChatScheme");

	SetScheme(scheme);

	SetVerticalScrollbar(false);
	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);

	InsertFade(-1, -1);
}

void CAuraChatHistory::ApplySchemeSettings(
	vgui2::IScheme* scheme)
{
	BaseClass::ApplySchemeSettings(
		scheme);

	if (!scheme)
		return;

	SetFont(
		scheme->GetFont(
			"ChatFont",
			false));

	SetAlpha(255);
}


// -------------------------------------------------------------------------
// CAuraChatLine
// -------------------------------------------------------------------------

CAuraChatLine::CAuraChatLine(
	vgui2::Panel* parent,
	const char* panelName)
	: BaseClass(
		parent,
		panelName),
	m_iClientIndex(0),
	m_flStartTime(0.0),
	m_flExpireTime(0.0),
	m_hChatFont(vgui2::INVALID_FONT),
	m_DefaultTextColor(255, 178, 0, 255),
	m_bActive(false)
{
	m_szText[0] = '\0';

	const vgui2::HScheme scheme =
		vgui2::scheme()->LoadSchemeFromFile(
			"ui/resource/ChatScheme.res",
			"AuraChatScheme");

	SetScheme(scheme);

	SetVerticalScrollbar(false);
	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);

	SetPaintBackgroundEnabled(true);

	SetVisible(false);
}

void CAuraChatLine::ApplySchemeSettings(
	vgui2::IScheme* scheme)
{
	BaseClass::ApplySchemeSettings(
		scheme);

	if (!scheme)
		return;

	m_hChatFont =
		scheme->GetFont(
			"ChatFont",
			false);

	m_DefaultTextColor =
		scheme->GetColor(
			"ChatTextColor",
			Color(255, 178, 0, 255));

	SetFont(m_hChatFont);

	SetBgColor(
		Color(0, 0, 0, 0));

	SetBorder(nullptr);
}

void CAuraChatLine::SetMessage(
	const char* text,
	int clientIndex)
{
	if (!text ||
		text[0] == '\0')
	{
		ClearMessage();
		return;
	}

	strncpy(
		m_szText,
		text,
		sizeof(m_szText) - 1);

	m_szText[
		sizeof(m_szText) - 1] =
		'\0';

		std::size_t length =
			strlen(m_szText);

		while (length > 0 &&
			(m_szText[length - 1] == '\n' ||
				m_szText[length - 1] == '\r'))
		{
			m_szText[length - 1] =
				'\0';

			--length;
		}

		m_iClientIndex =
			clientIndex;

		SetExpireTime();

		m_bActive = true;

		ApplyFormattedText(255);

		SetAlpha(255);
		SetVisible(true);

		InvalidateLayout();
		Repaint();
}

void CAuraChatLine::SetExpireTime()
{
	m_flStartTime =
		gEngfuncs.GetAbsoluteTime();

	m_flExpireTime =
		m_flStartTime +
		CHAT_DISPLAY_TIME;
}

void CAuraChatLine::ClearMessage()
{
	SetText("");
	SetVisible(false);
	SetAlpha(0);

	m_szText[0] = '\0';

	m_iClientIndex = 0;

	m_flStartTime = 0.0;
	m_flExpireTime = 0.0;

	m_bActive = false;
}

bool CAuraChatLine::IsReadyToExpire() const
{
	return m_bActive &&
		gEngfuncs.GetAbsoluteTime() >=
		m_flExpireTime;
}

bool CAuraChatLine::IsActive() const
{
	return m_bActive;
}

void CAuraChatLine::ApplyFormattedText(
	int alpha)
{
	InsertFormattedChatText(
		this,
		m_szText,
		m_iClientIndex,
		m_DefaultTextColor,
		alpha);
}

void CAuraChatLine::PerformFadeout()
{
	if (!m_bActive)
		return;

	const double currentTime =
		gEngfuncs.GetAbsoluteTime();

	if (currentTime >= m_flExpireTime)
	{
		ClearMessage();
		return;
	}

	const double fadeStart =
		m_flExpireTime -
		CHAT_FADE_TIME;

	if (currentTime < fadeStart)
	{
		ApplyFormattedText(255);
		return;
	}

	const double fraction =
		(m_flExpireTime - currentTime) /
		CHAT_FADE_TIME;

	int alpha =
		static_cast<int>(
			fraction * 255.0);

	if (alpha < 0)
		alpha = 0;

	if (alpha > 255)
		alpha = 255;

	ApplyFormattedText(alpha);
}

Color CAuraChatLine::GetClientColor(
	int clientIndex) const
{
	return GetAuraChatClientColor(
		clientIndex);
}


// -------------------------------------------------------------------------
// CChatPanel
// -------------------------------------------------------------------------

CChatPanel::CChatPanel(
	vgui2::Panel* parent)
	: BaseClass(
		parent,
		PANEL_CHAT),
	m_pChatHistory(nullptr),
	m_pChatLine(nullptr),
	m_hChatFont(vgui2::INVALID_FONT),
	m_DefaultTextColor(255, 178, 0, 255)
{
	SetProportional(true);

	const vgui2::HScheme scheme =
		vgui2::scheme()->LoadSchemeFromFile(
			"ui/resource/ChatScheme.res",
			"AuraChatScheme");

	SetScheme(scheme);

	SetPaintBackgroundEnabled(true);
	SetPaintBorderEnabled(true);

	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);

	m_pChatHistory =
		new CAuraChatHistory(
			this,
			"HudChatHistory");

	m_pChatHistory->SetMaximumCharCount(
		127 * 100);

	m_pChatHistory->SetVisible(true);

	m_pChatLine =
		new CAuraChatLine(
			this,
			"ChatLine1");

	LoadControlSettings(
		"ui/resource/Chat.res");

	InvalidateLayout(true, true);

	SetVisible(true);
}

const char* CChatPanel::GetName()
{
	return PANEL_CHAT;
}

void CChatPanel::Reset()
{
	if (m_pChatHistory)
	{
		m_pChatHistory->SetText("");
		m_pChatHistory->InsertFade(-1, -1);
	}

	if (m_pChatLine)
	{
		m_pChatLine->ClearMessage();
	}

	SetVisible(true);
}

void CChatPanel::ShowPanel(
	bool state)
{
	SetVisible(state);
}

vgui2::VPANEL CChatPanel::GetVPanel()
{
	return BaseClass::GetVPanel();
}

bool CChatPanel::IsVisible()
{
	return BaseClass::IsVisible();
}

void CChatPanel::SetParent(
	vgui2::VPANEL parent)
{
	BaseClass::SetParent(parent);
}

void CChatPanel::ApplySchemeSettings(
	vgui2::IScheme* scheme)
{
	LoadControlSettings(
		"ui/resource/Chat.res");

	BaseClass::ApplySchemeSettings(
		scheme);

	if (!scheme)
		return;

	m_hChatFont =
		scheme->GetFont(
			"ChatFont",
			false);

	m_DefaultTextColor =
		scheme->GetColor(
			"ChatTextColor",
			Color(255, 178, 0, 255));

	const Color backgroundColor =
		scheme->GetColor(
			"ChatBgColor",
			Color(0, 0, 0, 64));

	SetBgColor(
		Color(
			backgroundColor.r(),
			backgroundColor.g(),
			backgroundColor.b(),
			CHAT_HISTORY_ALPHA));

	SetPaintBackgroundType(2);
	SetPaintBorderEnabled(true);

	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);

	if (m_pChatHistory)
	{
		m_pChatHistory->SetVerticalScrollbar(
			false);
	}
}

void CChatPanel::Print(
	const char* text,
	int clientIndex)
{
	if (!text ||
		text[0] == '\0')
	{
		return;
	}

	AddToHistory(
		text,
		clientIndex);

	if (m_pChatLine)
	{
		m_pChatLine->SetMessage(
			text,
			clientIndex);
	}

	SetVisible(true);

	InvalidateLayout();
	Repaint();
}

void CChatPanel::AddToHistory(
	const char* text,
	int clientIndex)
{
	if (!m_pChatHistory)
		return;

	char cleanText[4096];

	strncpy(
		cleanText,
		text,
		sizeof(cleanText) - 1);

	cleanText[
		sizeof(cleanText) - 1] =
		'\0';

		std::size_t length =
			strlen(cleanText);

		while (length > 0 &&
			(cleanText[length - 1] == '\n' ||
				cleanText[length - 1] == '\r'))
		{
			cleanText[length - 1] =
				'\0';

			--length;
		}

		InsertFormattedChatText(
			m_pChatHistory,
			cleanText,
			clientIndex,
			m_DefaultTextColor,
			255);

		m_pChatHistory->InsertString(
			"\n");

		m_pChatHistory->InsertFade(
			CHAT_DISPLAY_TIME,
			2.5f);

		m_pChatHistory->InsertFade(
			-1,
			-1);

		m_pChatHistory->GotoTextEnd();
}

Color CChatPanel::GetClientColor(
	int clientIndex) const
{
	return GetAuraChatClientColor(
		clientIndex);
}

void CChatPanel::OnThink()
{
	BaseClass::OnThink();

	if (!IsVisible())
		return;

	if (m_pChatLine &&
		m_pChatLine->IsActive())
	{
		m_pChatLine->PerformFadeout();
	}
}

void CChatPanel::PerformLayout()
{
	BaseClass::PerformLayout();

	int parentWide = 640;
	int parentTall = 480;

	if (GetParent())
	{
		GetParent()->GetSize(
			parentWide,
			parentTall);
	}

	const int x =
		vgui2::scheme()
		->GetProportionalScaledValue(10);

	const int bottomMargin =
		vgui2::scheme()
		->GetProportionalScaledValue(75);

	const int wide =
		vgui2::scheme()
		->GetProportionalScaledValue(280);

	const int tall =
		vgui2::scheme()
		->GetProportionalScaledValue(130);

	const int y =
		parentTall -
		bottomMargin -
		tall;

	SetBounds(
		x,
		y,
		wide,
		tall);

	const int historyX =
		vgui2::scheme()
		->GetProportionalScaledValue(4);

	const int historyY =
		vgui2::scheme()
		->GetProportionalScaledValue(18);

	const int historyWide =
		vgui2::scheme()
		->GetProportionalScaledValue(272);

	const int historyTall =
		vgui2::scheme()
		->GetProportionalScaledValue(94);

	if (m_pChatHistory)
	{
		m_pChatHistory->SetBounds(
			historyX,
			historyY,
			historyWide,
			historyTall);
	}

	if (m_pChatLine)
	{
		const int fontTall =
			m_hChatFont !=
			vgui2::INVALID_FONT
			? vgui2::surface()->GetFontTall(
				m_hChatFont) + 2
			: vgui2::scheme()
			->GetProportionalScaledValue(19);

		m_pChatLine->SetBounds(
			historyX,
			historyY +
			historyTall -
			fontTall,
			historyWide,
			fontTall);
	}
}
