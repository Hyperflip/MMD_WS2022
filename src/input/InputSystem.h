#pragma once

#if PLATFORM_ORBIS
#include "InputSystemPS4.h"
#elif PLATFORM_WINDOWS
#include "InputSystemWin.h"
#else
#pragma error "Unsupported platform"
#endif
