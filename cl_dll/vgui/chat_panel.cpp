#include <cstdio>
#include <cstring>
#include <cstdint>

#include <KeyValues.h>

#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/IInput.h>
#include <vgui/IInputInternal.h>
#include <vgui/ILocalize.h>

#include <vgui_controls/Label.h>
#include <vgui_controls/TextEntry.h>

#include "chat_panel.h"
#include "viewport_panel_names.h"

#include "bridge.h"

#include "../hud.h"

extern "C"
{
	void IN_DeactivateMouse();
	void IN_ActivateMouse();
	void IN_ClearStates();
}

extern int iTeamColors[5][3];
extern int iNumberOfTeamColors;

static void MessageModeVGUI2();
static void MessageMode2VGUI2();

namespace
{
	using EngineCommandFunction = void (*)();

	struct AuraCmdFunction
	{
		AuraCmdFunction* next;
		const char* name;
		EngineCommandFunction function;
		int flags;
	};

	static_assert(sizeof(unsigned int) == sizeof(void*), "Aura command-handle conversion requires 32-bit.");

	EngineCommandFunction g_pOriginalMessageMode = nullptr;
	EngineCommandFunction g_pOriginalMessageMode2 = nullptr;

	CChatPanel* g_pAuraChatPanel = nullptr;
}

static AuraCmdFunction* CommandFromHandle(unsigned int handle)
{
	if (handle == 0)
		return nullptr;

	return reinterpret_cast<AuraCmdFunction*>(static_cast<std::uintptr_t>(handle));
}

static void MessageModeVGUI2()
{
	if (!g_pAuraChatPanel)
	{
		if (g_pOriginalMessageMode)
			g_pOriginalMessageMode();

		return;
	}

	if (gEngfuncs.GetMaxClients() <= 1)
		return;

	if (gEngfuncs.Cmd_Argc() != 1)
	{
		if (g_pOriginalMessageMode)
			g_pOriginalMessageMode();

		return;
	}

	StartVGUI2ChatMessageMode(
		MM_SAY);
}

static void MessageMode2VGUI2()
{
	if (!g_pAuraChatPanel)
	{
		if (g_pOriginalMessageMode2)
			g_pOriginalMessageMode2();

		return;
	}

	if (gEngfuncs.GetMaxClients() <= 1)
		return;

	if (gEngfuncs.Cmd_Argc() != 1)
	{
		if (g_pOriginalMessageMode2)
			g_pOriginalMessageMode2();

		return;
	}

	StartVGUI2ChatMessageMode(
		MM_SAY_TEAM);
}

static bool HookMessageModeCommands()
{
	if (!gEngfuncs.GetFirstCmdFunctionHandle ||
		!gEngfuncs.GetNextCmdFunctionHandle ||
		!gEngfuncs.GetCmdFunctionName)
	{
		gEngfuncs.Con_Printf(
			"Chat: engine command-list API is unavailable\n");

		return false;
	}

	AuraCmdFunction* messageModeCommand = nullptr;
	AuraCmdFunction* messageMode2Command = nullptr;

	for (unsigned int handle =
		gEngfuncs.GetFirstCmdFunctionHandle();
		handle != 0;
		handle =
		gEngfuncs.GetNextCmdFunctionHandle(handle))
	{
		const char* commandName =
			gEngfuncs.GetCmdFunctionName(handle);

		if (!commandName)
			continue;

		if (strcmp(commandName, "messagemode") == 0)
		{
			messageModeCommand =
				CommandFromHandle(handle);
		}
		else if (
			strcmp(commandName, "messagemode2") == 0)
		{
			messageMode2Command =
				CommandFromHandle(handle);
		}
	}

	if (!messageModeCommand ||
		!messageMode2Command)
	{
		gEngfuncs.Con_Printf(
			"Chat: failed to locate messagemode commands\n");

		return false;
	}

	if (!messageModeCommand->function ||
		!messageMode2Command->function)
	{
		gEngfuncs.Con_Printf(
			"Chat: messagemode commands have null callbacks\n");

		return false;
	}

	g_pOriginalMessageMode =
		messageModeCommand->function;

	g_pOriginalMessageMode2 =
		messageMode2Command->function;

	messageModeCommand->function =
		MessageModeVGUI2;

	messageMode2Command->function =
		MessageMode2VGUI2;

	gEngfuncs.Con_Printf(
		"Chat: hooked messagemode and messagemode2\n");

	return true;
}

namespace
{
	constexpr double CHAT_DISPLAY_TIME = 12.0;
	constexpr double CHAT_FADE_TIME = 1.0;

	constexpr double CHAT_HISTORY_FADE_TIME = 0.25;

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
		int alpha,
		bool clearExisting)
	{
		if (!target ||
			!text ||
			text[0] == '\0')
		{
			return;
		}

		if (clearExisting)
		{
			target->SetText("");
		}

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

CAuraChatEntry::CAuraChatEntry(
	vgui2::Panel* parent,
	const char* panelName,
	vgui2::Panel* chatPanel)
	: BaseClass(
		parent,
		panelName),
	m_pChatPanel(chatPanel)
{
	SetCatchEnterKey(true);
	SetAllowNonAsciiCharacters(true);
	SetDrawLanguageIDAtLeft(true);
}

void CAuraChatEntry::ApplySchemeSettings(
	vgui2::IScheme* scheme)
{
	BaseClass::ApplySchemeSettings(
		scheme);

	SetPaintBorderEnabled(false);

	if (scheme)
	{
		SetFgColor(
			scheme->GetColor(
				"Chat.TypingText",
				Color(
					255,
					255,
					255,
					255)));
	}

	SetBgColor(
		Color(
			0,
			0,
			0,
			0));
}

void CAuraChatEntry::OnKeyCodePressed(
	vgui2::KeyCode code)
{
	if (code == vgui2::KEY_ESCAPE)
	{
		if (m_pChatPanel)
		{
			PostMessage(m_pChatPanel, new KeyValues("ChatEntryStopMessageMode"));
		}

		return;
	}

	BaseClass::OnKeyCodePressed(code);
}

void CAuraChatEntry::OnKeyCodeTyped(
	vgui2::KeyCode code)
{
	if (code == vgui2::KEY_ENTER ||
		code == vgui2::KEY_PAD_ENTER ||
		code == vgui2::KEY_ESCAPE)
	{
		if (code != vgui2::KEY_ESCAPE &&
			m_pChatPanel)
		{
			PostMessage(
				m_pChatPanel,
				new KeyValues(
					"ChatEntrySend"));
		}

		if (m_pChatPanel)
		{
			PostMessage(
				m_pChatPanel,
				new KeyValues(
					"ChatEntryStopMessageMode"));
		}

		return;
	}

	if (code == vgui2::KEY_TAB)
	{
		// Prevent VGUI from moving keyboard focus away from the entry.
		return;
	}

	BaseClass::OnKeyCodeTyped(code);
}

CAuraChatInputLine::CAuraChatInputLine(
	vgui2::Panel* parent,
	const char* panelName)
	: BaseClass(
		parent,
		panelName),
	m_pPrompt(nullptr),
	m_pInput(nullptr)
{
	SetMouseInputEnabled(false);

	m_pPrompt =
		new vgui2::Label(
			this,
			"ChatInputPrompt",
			L"Say:");

	m_pInput =
		new CAuraChatEntry(
			this,
			"ChatInput",
			parent);

	m_pInput->SetMaximumCharCount(
		MAX_CHAT_INPUT_STRING_LEN);
}

void CAuraChatInputLine::ApplySchemeSettings(
	vgui2::IScheme* scheme)
{
	BaseClass::ApplySchemeSettings(
		scheme);

	if (!scheme)
		return;

	const vgui2::HFont font =
		scheme->GetFont(
			"ChatFont",
			false);

	m_pPrompt->SetFont(font);
	m_pInput->SetFont(font);

	m_pPrompt->SetContentAlignment(
		vgui2::Label::a_west);

	m_pPrompt->SetTextInset(
		2,
		0);

	m_pPrompt->SetPaintBackgroundEnabled(
		true);

	m_pInput->SetMouseInputEnabled(
		true);

	SetPaintBackgroundEnabled(true);

	SetBgColor(
		Color(
			0,
			0,
			0,
			0));

	m_pPrompt->SetBgColor(
		Color(
			0,
			0,
			0,
			0));
}

void CAuraChatInputLine::SetPrompt(
	const wchar_t* prompt)
{
	if (!m_pPrompt ||
		!prompt)
	{
		return;
	}

	m_pPrompt->SetText(prompt);
	InvalidateLayout();
}

void CAuraChatInputLine::ClearEntry()
{
	SetEntry(L"");
}

void CAuraChatInputLine::SetEntry(
	const wchar_t* entry)
{
	if (m_pInput &&
		entry)
	{
		m_pInput->SetText(entry);
	}
}

void CAuraChatInputLine::GetMessageText(
	wchar_t* buffer,
	int bufferSizeBytes)
{
	if (!buffer ||
		bufferSizeBytes <= 0)
	{
		return;
	}

	if (!m_pInput)
	{
		buffer[0] = L'\0';
		return;
	}

	m_pInput->GetText(
		buffer,
		bufferSizeBytes);
}

void CAuraChatInputLine::PerformLayout()
{
	BaseClass::PerformLayout();

	int wide = 0;
	int tall = 0;

	GetSize(
		wide,
		tall);

	int promptWide = 0;
	int promptTall = 0;

	m_pPrompt->GetContentSize(
		promptWide,
		promptTall);

	m_pPrompt->SetBounds(
		0,
		0,
		promptWide,
		tall);

	m_pInput->SetBounds(
		promptWide + 2,
		0,
		wide - promptWide - 2,
		tall);
}

vgui2::Panel* CAuraChatInputLine::GetInputPanel()
{
	return m_pInput;
}

vgui2::VPANEL CAuraChatInputLine::GetCurrentKeyFocus()
{
	if (m_pInput)
	{
		return m_pInput->GetVPanel();
	}

	return BaseClass::GetCurrentKeyFocus();
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

	SetBgColor(
		Color(
			0,
			0,
			0,
			0));

	SetBorder(nullptr);
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
		alpha,
		true);
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
	m_DefaultTextColor(255, 178, 0, 255),
	m_iMessageMode(MM_NONE),
	m_flHistoryFadeTime(0.0)
{
	SetProportional(true);

	const vgui2::HScheme scheme =
		vgui2::scheme()->LoadSchemeFromFile(
			"ui/resource/ChatScheme.res",
			"AuraChatScheme");

	SetScheme(scheme);

	SetPaintBackgroundEnabled(true);
	SetPaintBorderEnabled(true);

	MakePopup();
	SetZPos(-30);

	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);

	m_pChatHistory =
		new CAuraChatHistory(
			this,
			"HudChatHistory");

	m_pChatHistory->SetMaximumCharCount(
		127 * 100);

	m_pChatHistory->SetVisible(false);

	m_pChatLine =
		new CAuraChatLine(
			this,
			"ChatLine1");

	m_pChatInput =
		new CAuraChatInputLine(
			this,
			"ChatInputLine");
	m_pChatInput->SetVisible(false);
	m_pChatInput->SetKeyBoardInputEnabled(false);
	m_pChatInput->SetMouseInputEnabled(false);

	LoadControlSettings(
		"ui/resource/Chat.res");

	InvalidateLayout(true, true);

	SetVisible(true);

	g_pAuraChatPanel = this;

	static bool attempedCommandHook = false;

	if (!attempedCommandHook)
	{
		attempedCommandHook = true;
		HookMessageModeCommands();
	}
}

CChatPanel::~CChatPanel()
{
	if (g_pAuraChatPanel == this)
	{
		g_pAuraChatPanel = nullptr;
	}
}

void CChatPanel::StartMessageMode(
	int messageMode)
{
	if (!m_pChatInput)
		return;

	if (messageMode != MM_SAY &&
		messageMode != MM_SAY_TEAM)
	{
		return;
	}

	m_iMessageMode =
		messageMode;

	m_pChatInput->ClearEntry();

	if (messageMode == MM_SAY)
	{
		m_pChatInput->SetPrompt(
			L"Say:");
	}
	else
	{
		m_pChatInput->SetPrompt(
			L"Say (TEAM):");
	}

	// Reveal the full persistent history while message mode is open.
	if (m_pChatHistory)
	{
		m_pChatHistory->SetVisible(true);

		m_pChatHistory->SetMouseInputEnabled(true);
		m_pChatHistory->SetKeyBoardInputEnabled(false);

		m_pChatHistory->SetVerticalScrollbar(true);
		m_pChatHistory->SetPaintBorderEnabled(true);

		// Reveal messages whose normal gameplay fade has completed.
		m_pChatHistory->ResetAllFades(true);

		m_pChatHistory->GotoTextEnd();
	}

	vgui2::Panel* parentPanel =
		GetParent();

	if (parentPanel)
	{
		parentPanel->SetVisible(true);
		parentPanel->SetKeyBoardInputEnabled(true);
		parentPanel->SetMouseInputEnabled(true);
	}

	vgui2::SETUP_PANEL(this);

	SetVisible(true);
	SetEnabled(true);

	SetKeyBoardInputEnabled(true);
	SetMouseInputEnabled(true);

	SetPaintBackgroundEnabled(true);
	SetPaintBackgroundType(2);

	m_pChatInput->SetVisible(true);
	m_pChatInput->SetEnabled(true);

	m_pChatInput->SetKeyBoardInputEnabled(true);
	m_pChatInput->SetMouseInputEnabled(true);

	m_pChatInput->SetPaintBorderEnabled(true);

	vgui2::Panel* inputPanel =
		m_pChatInput->GetInputPanel();

	if (inputPanel)
	{
		inputPanel->SetVisible(true);
		inputPanel->SetEnabled(true);

		inputPanel->SetKeyBoardInputEnabled(true);
		inputPanel->SetMouseInputEnabled(true);

		inputPanel->SetAlpha(255);
	}

	// Make sure the popup and its children have completed layout before
	// requesting focus.
	InvalidateLayout(true);
	PerformLayout();

	MoveToFront();

	// Release normal game mouse-look while chat is open.
	IN_DeactivateMouse();

	vgui2::surface()->CalculateMouseVisible();

	// Focus the actual TextEntry only after the popup is visible,
	// laid out, and moved to the front.
	if (inputPanel)
	{
		inputPanel->RequestFocus();
	}
	else
	{
		m_pChatInput->RequestFocus();
	}

	// Begin the translucent chat-box fade-in.
	m_flHistoryFadeTime =
		gEngfuncs.GetAbsoluteTime() +
		CHAT_HISTORY_FADE_TIME;

	Repaint();
}

void CChatPanel::StopMessageMode()
{
	if (m_iMessageMode == MM_NONE)
		return;

	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);

	if (m_pChatHistory)
	{
		m_pChatHistory->SetPaintBorderEnabled(false);
		m_pChatHistory->GotoTextEnd();
		m_pChatHistory->SetMouseInputEnabled(false);
		m_pChatHistory->SetVerticalScrollbar(false);

		m_pChatHistory->ResetAllFades(
			false,
			true,
			2.5f);

		m_pChatHistory->SelectNoText();

		// Keep the control alive and visible. Only its old text ranges
		// become faded; newly received messages will still appear.
		m_pChatHistory->SetVisible(true);
	}

	if (m_pChatInput)
	{
		m_pChatInput->ClearEntry();
		m_pChatInput->SetVisible(false);
		m_pChatInput->SetKeyBoardInputEnabled(false);
		m_pChatInput->SetMouseInputEnabled(false);
	}

	if (GetParent())
	{
		GetParent()->SetKeyBoardInputEnabled(false);
	}

	m_iMessageMode =
		MM_NONE;

	IN_ClearStates();
	IN_ActivateMouse();
	IN_ClearStates();

	PreventGameUIEscape(false);

	vgui2::surface()->CalculateMouseVisible();

	Repaint();

	m_flHistoryFadeTime = gEngfuncs.GetAbsoluteTime() + CHAT_HISTORY_FADE_TIME;
}

void CChatPanel::Send()
{
	if (!m_pChatInput ||
		m_iMessageMode == MM_NONE)
	{
		return;
	}

	wchar_t wideText[
		MAX_CHAT_INPUT_STRING_LEN + 1];

	wideText[0] =
		L'\0';

	m_pChatInput->GetMessageText(
		wideText,
		sizeof(wideText));

	char message[
		MAX_CHAT_INPUT_STRING_LEN * 4 + 1];

	message[0] =
		'\0';

	g_pVGuiLocalize->ConvertUnicodeToANSI(
		wideText,
		message,
		sizeof(message));

	std::size_t length =
		strlen(message);

	while (length > 0 &&
		(message[length - 1] == '\n' ||
			message[length - 1] == '\r' ||
			message[length - 1] == ' '))
	{
		message[length - 1] =
			'\0';

		--length;
	}

	char* firstCharacter =
		message;

	while (*firstCharacter == ' ' ||
		*firstCharacter == '\t')
	{
		++firstCharacter;
	}

	if (*firstCharacter == '\0')
	{
		m_pChatInput->ClearEntry();
		return;
	}

	// Prevent the quoted engine command from being terminated early.
	for (char* character = firstCharacter;
		*character != '\0';
		++character)
	{
		if (*character == '"')
		{
			*character = '\'';
		}

		if (*character == '\n' ||
			*character == '\r')
		{
			*character = ' ';
		}
	}

	char command[
		MAX_CHAT_COMMAND_LEN];

	snprintf(
		command,
		sizeof(command),
		m_iMessageMode == MM_SAY
		? "say \"%s\"\n"
		: "say_team \"%s\"\n",
		firstCharacter);

	gEngfuncs.pfnClientCmd(
		command);

	m_pChatInput->ClearEntry();
}

void CChatPanel::OnChatEntrySend()
{
	Send();
}

void CChatPanel::OnChatEntryStopMessageMode()
{
	StopVGUI2ChatMessageMode();
}

vgui2::Panel* CChatPanel::GetInputPanel()
{
	return m_pChatInput
		? m_pChatInput->GetInputPanel()
		: nullptr;
}

const char* CChatPanel::GetName()
{
	return PANEL_CHAT;
}

void CChatPanel::Reset()
{
	if (m_iMessageMode != MM_NONE)
		StopMessageMode();

	if (m_pChatHistory)
	{
		m_pChatHistory->SetText("");
		m_pChatHistory->InsertFade(-1, -1);
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

	const bool messageModeActive = m_iMessageMode != MM_NONE;

	SetKeyBoardInputEnabled(messageModeActive);
	SetMouseInputEnabled(messageModeActive);

	if (m_pChatInput)
	{
		m_pChatInput->SetVisible(
			messageModeActive);

		m_pChatInput->SetKeyBoardInputEnabled(
			messageModeActive);

		m_pChatInput->SetMouseInputEnabled(
			messageModeActive);
	}

	if (m_pChatHistory)
	{
		m_pChatHistory->SetVerticalScrollbar(
			false);
	}

	const int backgroundAlpha =
		messageModeActive
		? CHAT_HISTORY_ALPHA
		: 0;

	SetBgColor(
		Color(
			backgroundColor.r(),
			backgroundColor.g(),
			backgroundColor.b(),
			backgroundAlpha));
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

	SetVisible(true);

	InvalidateLayout();
	Repaint();
}

void CChatPanel::AddToHistory(
	const char* text,
	int clientIndex)
{
	if (!m_pChatHistory ||
		!text ||
		text[0] == '\0')
	{
		return;
	}

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

		if (cleanText[0] == '\0')
			return;

		// Source chat starts each message on a fresh line.
		m_pChatHistory->InsertString("\n");

		InsertFormattedChatText(
			m_pChatHistory,
			cleanText,
			clientIndex,
			m_DefaultTextColor,
			255,
			false);

		// The message remains stored, but fades from the gameplay view.
		m_pChatHistory->InsertFade(
			CHAT_DISPLAY_TIME,
			2.5f);

		// End the fade range so the next message gets its own timing.
		m_pChatHistory->InsertFade(
			-1,
			-1);

		m_pChatHistory->GotoTextEnd();
}

void CChatPanel::FadeChatHistory()
{
	const double remaining =
		m_flHistoryFadeTime -
		gEngfuncs.GetAbsoluteTime();

	double fraction =
		remaining /
		CHAT_HISTORY_FADE_TIME;

	if (fraction < 0.0)
		fraction = 0.0;

	if (fraction > 1.0)
		fraction = 1.0;

	int alpha =
		static_cast<int>(
			fraction *
			CHAT_HISTORY_ALPHA);

	if (alpha < 0)
		alpha = 0;

	if (alpha > CHAT_HISTORY_ALPHA)
		alpha = CHAT_HISTORY_ALPHA;

	if (m_iMessageMode != MM_NONE)
	{
		// Fade the box in while entering message mode.
		const int visibleAlpha =
			CHAT_HISTORY_ALPHA -
			alpha;

		SetBgColor(
			Color(
				GetBgColor().r(),
				GetBgColor().g(),
				GetBgColor().b(),
				visibleAlpha));

		if (m_pChatHistory)
		{
			m_pChatHistory->SetBgColor(
				Color(
					0,
					0,
					0,
					visibleAlpha));
		}

		SetPaintBackgroundEnabled(true);
		SetPaintBackgroundType(2);
	}
	else
	{
		// Fade the box out after sending/cancelling.
		SetBgColor(
			Color(
				GetBgColor().r(),
				GetBgColor().g(),
				GetBgColor().b(),
				alpha));

		if (m_pChatHistory)
		{
			m_pChatHistory->SetBgColor(
				Color(
					0,
					0,
					0,
					alpha));
		}

		if (alpha == 0)
		{
			SetPaintBackgroundEnabled(false);
		}
	}
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

	FadeChatHistory();
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

	if (m_pChatInput)
	{
		const int inputX =
			vgui2::scheme()
			->GetProportionalScaledValue(4);

		const int inputY =
			vgui2::scheme()
			->GetProportionalScaledValue(112);

		const int inputWide =
			vgui2::scheme()
			->GetProportionalScaledValue(272);

		const int inputTall =
			vgui2::scheme()
			->GetProportionalScaledValue(18);

		m_pChatInput->SetBounds(
			inputX,
			inputY,
			inputWide,
			inputTall);
	}
}
