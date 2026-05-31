#include "../engineclientcmd.h"
#include "GameMenuEventListener.h"
#include "GameMenuSound.h"

#include "../gameui/gameui_viewport.h"

#include <RmlUi/Core/Event.h>
#include <RmlUi/Core/Element.h>

CGameMenuEventListener::CGameMenuEventListener() {}
CGameMenuEventListener::~CGameMenuEventListener() {}

void CGameMenuEventListener::ProcessEvent(Rml::Event& event)
{
	Rml::Element* element = event.GetCurrentElement();

	if (!element)
		return;

	const Rml::String elementId = element->GetId();
	const Rml::String eventType = event.GetType();

	if (elementId.empty())
		return;

	if (eventType == "click")
	{
		HandleClick(elementId);
	}
	else if (eventType == "mouseover")
	{
		HandleMouseOver(elementId);
	}
}

void CGameMenuEventListener::HandleClick(const Rml::String& elementId)
{
	Msg("CGameMenu: clicked '%s'\n", elementId.c_str());

	CGameMenuSound::PlayOpen();

	if (elementId == "serverbrowser")
	{
		VGUI2_Cmd("gameui_serverbrowser");
		return;
	}
	if (elementId == "composer")
	{
		VGUI2_Cmd("gameui_composer");
		return;
	}
	if (elementId == "workshop")
	{
		VGUI2_Cmd("gameui_workshop");
		return;
	}
	if (elementId == "quit")
	{
		VGUI2_Cmd("quit");
		return;
	}
}

void CGameMenuEventListener::HandleMouseOver(const Rml::String& elementId)
{
	if (elementId.empty())
		return;

	CGameMenuSound::PlayHover();
}
