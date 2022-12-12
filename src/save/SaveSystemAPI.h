#pragma once

#ifdef PLATFORM_ORBIS
	#include "SaveSystemAPIPS4.h"
#elif PLATFORM_WINDOWS
	#include "SaveSystemAPIWin.h"
#else
	#pragma error "Unsupported platform"
#endif