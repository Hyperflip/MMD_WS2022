#pragma once

#include <cstdint>

#include "kernel.h"

class ClockPS4
{
public:
	
	typedef uint64_t Cycles;

	static void Init()
	{
		const uint64_t Frequency = sceKernelGetProcessTimeCounterFrequency();
		OneOverFrequency = 1.0f / static_cast< float >( Frequency );
	}

	ClockPS4()
		: StartCycles( 0u )
	{
		Start();
	}
	
	void Start()
	{
		StartCycles = sceKernelGetProcessTimeCounter();
		IsRunning = true;
	}

	void Stop()
	{
		StartCycles = 0u;
		IsRunning = false;
	}

	Cycles QueryPassedCycles() const
	{
		if (!IsRunning)
		{
			return 0;
		}
		
		Cycles PassedCycles = 0u;
		PassedCycles = sceKernelGetProcessTimeCounter();
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

float ClockPS4::OneOverFrequency = 1.0f;

typedef ClockPS4 Clock;