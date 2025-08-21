#pragma once

#include "cl_dll.h"

extern cldll_enginefunc_t gEngfuncs;

#define VGUI2_Cmd (*gEngfuncs.pfnClientCmd)