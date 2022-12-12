#pragma once

#ifdef PLATFORM_ORBIS
	#include "Sys_ThreadingPS4.h"
#elif PLATFORM_WINDOWS
	#include "Sys_ThreadingWin.h"
#else
	#pragma error "Unsupported platform"
#endif