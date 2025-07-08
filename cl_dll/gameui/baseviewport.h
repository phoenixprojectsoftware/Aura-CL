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

#pragma once

#include <vgui_controls/EditablePanel.h>

class BaseViewport : public vgui2::EditablePanel
{
	DECLARE_CLASS_SIMPLE(BaseViewport, vgui2::EditablePanel);
public:
	BaseViewport();
	~BaseViewport();

	void Init();
	void ShowPanel(bool show);
	void SetParent(vgui2::VPANEL parent);
	void ActivateClientUI();
	void HideClientUI();

	//TODO: add private subpanels
};