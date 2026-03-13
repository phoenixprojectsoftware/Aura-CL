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

#include "push2_system.h"
#include <libusb.h>

bool CPush2System::Init()
{
	return m_Display.Init();
}

void CPush2System::Shutdown()
{
	m_Display.Shutdown();
}

void CPush2System::Update()
{
	m_Display.Present();
}
