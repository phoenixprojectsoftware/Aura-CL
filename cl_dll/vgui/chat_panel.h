#ifndef AURA_VGUI_CHAT_PANEL_H
#define AURA_VGUI_CHAT_PANEL_H

#include <vgui/KeyCode.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/RichText.h>

#include "IViewportPanel.h"

namespace vgui2
{
	class IScheme;
}

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
};

#endif
