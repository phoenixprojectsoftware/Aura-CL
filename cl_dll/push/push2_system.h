/****
*
* Copyright (c) 2021-2026 The Phoenix Project Software. Some Rights Reserved.
*
* AURA
*
* Ableton Push2 system.
*
*
****/

#pragma once

#include "push2_display.h"

class CPush2System
{
public:
	bool Init();
	void Shutdown();

	void Update();

private:
	CPush2Display m_Display;
};
