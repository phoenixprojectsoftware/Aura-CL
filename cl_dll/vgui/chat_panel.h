#ifndef AURA_VGUI_CHAT_PANEL_H
#define AURA_VGUI_CHAT_PANEL_H

#include <vgui/KeyCode.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/RichText.h>
#include <vgui_controls/TextEntry.h>

#include "IViewportPanel.h"

namespace vgui2
{
	class IScheme;
	class Label;
}

enum AuraMessageMode
{
	MM_NONE = 0,
	MM_SAY,
	MM_SAY_TEAM
};

constexpr int MAX_CHAT_INPUT_STRING_LEN = 256;
constexpr int MAX_CHAT_COMMAND_LEN = 384;

class CAuraChatHistory final
	: public vgui2::RichText
{
	DECLARE_CLASS_SIMPLE(
		CAuraChatHistory,
		vgui2::RichText);

public:
	CAuraChatHistory(
		vgui2::Panel* parent,
		const char* panelName);

protected:
	void ApplySchemeSettings(
		vgui2::IScheme* scheme) override;
};

class CAuraChatLine final
	: public vgui2::RichText
{
	DECLARE_CLASS_SIMPLE(
		CAuraChatLine,
		vgui2::RichText);

public:
	CAuraChatLine(
		vgui2::Panel* parent,
		const char* panelName);

	void SetMessage(
		const char* text,
		int clientIndex);

	void SetExpireTime();
	void ClearMessage();

	bool IsReadyToExpire() const;
	bool IsActive() const;

	void PerformFadeout();

protected:
	void ApplySchemeSettings(
		vgui2::IScheme* scheme) override;

private:
	void ApplyFormattedText(
		int alpha);

	Color GetClientColor(
		int clientIndex) const;

	char m_szText[4096];

	int m_iClientIndex;

	double m_flStartTime;
	double m_flExpireTime;

	vgui2::HFont m_hChatFont;

	Color m_DefaultTextColor;

	bool m_bActive;
};

class CAuraChatEntry final
	: public vgui2::TextEntry
{
	DECLARE_CLASS_SIMPLE(
		CAuraChatEntry,
		vgui2::TextEntry);

public:
	CAuraChatEntry(
		vgui2::Panel* parent,
		const char* panelName,
		vgui2::Panel* chatPanel);

protected:
	void ApplySchemeSettings(
		vgui2::IScheme* scheme) override;

	void OnKeyCodeTyped(
		vgui2::KeyCode code) override;

private:
	vgui2::Panel* m_pChatPanel;
};


class CAuraChatInputLine final
	: public vgui2::Panel
{
	DECLARE_CLASS_SIMPLE(
		CAuraChatInputLine,
		vgui2::Panel);

public:
	CAuraChatInputLine(
		vgui2::Panel* parent,
		const char* panelName);

	void SetPrompt(
		const wchar_t* prompt);

	void ClearEntry();

	void SetEntry(
		const wchar_t* entry);

	void GetMessageText(
		wchar_t* buffer,
		int bufferSizeBytes);

	vgui2::Panel* GetInputPanel();

	vgui2::VPANEL GetCurrentKeyFocus() override;

protected:
	void ApplySchemeSettings(
		vgui2::IScheme* scheme) override;

	void PerformLayout() override;

private:
	vgui2::Label* m_pPrompt;
	CAuraChatEntry* m_pInput;
};

class CChatPanel final
	: public vgui2::EditablePanel,
	public IViewportPanel
{
	DECLARE_CLASS_SIMPLE(
		CChatPanel,
		vgui2::EditablePanel);

public:
	explicit CChatPanel(
		vgui2::Panel* parent);
	~CChatPanel() override;

	// IViewportPanel
	const char* GetName() override;
	void Reset() override;
	void ShowPanel(bool state) override;
	vgui2::VPANEL GetVPanel() override;
	bool IsVisible() override;
	void SetParent(vgui2::VPANEL parent) override;

	void Print(
		const char* text,
		int clientIndex);

	void StartMessageMode(int messageMode);

	void StopMessageMode();

	void Send();

	int GetMessageMode() const
	{
		return m_iMessageMode;
	}

	vgui2::Panel* GetInputPanel();

	MESSAGE_FUNC(OnChatEntrySend, "ChatEntrySend");
	MESSAGE_FUNC(OnChatEntryStopMessageMode, "ChatEntryStopMessageMode");

protected:
	void ApplySchemeSettings(
		vgui2::IScheme* scheme) override;

	void PerformLayout() override;
	void OnThink() override;

private:
	void AddToHistory(
		const char* text,
		int clientIndex);

	Color GetClientColor(
		int clientIndex) const;

	CAuraChatHistory* m_pChatHistory;
	CAuraChatLine* m_pChatLine;

	vgui2::HFont m_hChatFont;
	Color m_DefaultTextColor;

	CAuraChatInputLine* m_pChatInput;
	int m_iMessageMode;
};

#endif
