#pragma once

#include <cstdint>

#define WIN_LEAN_AND_MEAN
#include "Windows.h"
#undef WIN_LEAN_AND_MEAN

class ClockWin
{
public:
	
	typedef uint64_t Cycles;

	static void Init(void)
	{
		LARGE_INTEGER Frequency;
		QueryPerformanceFrequency( &Frequency );
		OneOverFrequency = 1.0f / static_cast< float >( Frequency.QuadPart );
	}

	ClockWin()
		: StartCycles( 0u )
	{
		Start();
	}
	
	void Start()
	{
		QueryPerformanceCounter( reinterpret_cast< LARGE_INTEGER* >( &StartCycles ) );
		IsRunning = true;
	}

	void Stop()
	{
		StartCycles = 0u;
		IsRunning = false;
	}

	Cycles QueryPassedCycles() const
	{
		if ( !IsRunning )
		{
			return 0;
		}
		
		Cycles PassedCycles = 0u;
		QueryPerformanceCounter( reinterpret_cast< LARGE_INTEGER* >( &PassedCycles ) );
		return ( PassedCycles - StartCycles );
	}

	float ToMilliseconds() const
	{
		return ( static_cast< float >( QueryPassedCycles() ) * OneOverFrequency ) * 1000.0f;
	}

	float ToSecond() const
	{
		return static_cast< float >( QueryPassedCycles() ) * OneOverFrequency;
	}

private:
	bool IsRunning { false };
	Cycles StartCycles { 0u };
	static float OneOverFrequency;
};

float ClockWin::OneOverFrequency = 1.0f;

typedef ClockWin Clock;