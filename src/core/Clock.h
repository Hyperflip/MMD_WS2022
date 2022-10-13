#pragma once

#if PLATFORM_ORBIS
#include "ClockPS4.h"
#elif PLATFORM_WINDOWS
#include "ClockWin.h"
#else
#pragma error "Unsupported platform"
#endif
