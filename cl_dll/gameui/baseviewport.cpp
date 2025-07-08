/****
*
* Copyright © 2021-2025 The Phoenix Project Software. Some Rights Reserved.
*
* AURA
*
* baseviewport - Base viewport for the VGUI2 client interface. Based upon
* Source SDK 2013.
*
****/

#include "baseviewport.h"
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui_controls/Label.h>

using namespace vgui2;

BaseViewport::BaseViewport() : BaseClass(nullptr, "BaseViewport")
{
	SetBounds(0, 0, 640, 480);
	SetPaintBackgroundEnabled(true);
}

BaseViewport::~BaseViewport()
{
}

void BaseViewport::Init()
{
	Label* label = new Label(this, "TestLabel", "Hello VGUI2! This is the base viewport.");
	label->SetPos(100, 100);
}

void BaseViewport::ShowPanel(bool show)
{
	SetVisible(show);
}

void BaseViewport::SetParent(VPANEL parent)
{
	BaseClass::SetParent(parent);
}

void BaseViewport::ActivateClientUI()
{
	SetVisible(true);
	RequestFocus();
}

void BaseViewport::HideClientUI() 
{
	SetVisible(false);
}