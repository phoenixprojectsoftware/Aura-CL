#pragma once

#include <RmlUi/Core/EventListener.h>
#include <RmlUi/Core/StringUtilities.h>

class CGameMenuEventListener final : public Rml::EventListener
{
public:
	CGameMenuEventListener();
	~CGameMenuEventListener() override;

	void ProcessEvent(Rml::Event& event) override;

private:
	void HandleClick(const Rml::String& elementId);
	void HandleMouseOver(const Rml::String& elementId);
};
