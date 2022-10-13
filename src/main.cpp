#include <cstdio>

#include "input/InputSystemAPI.h"
#include "core/Clock.h"

namespace GameConstants
{
	constexpr float GAME_TICK_RATE = 60.0f;
	constexpr float GAME_TARGET_DELTA = 1000.0f / GAME_TICK_RATE;
}


int main() 
{
	// @note - lukas.vogl - Setting up a basic clock that works on PC and PS4
	Clock::Init();
	Clock clock;
	clock.Start();

	printf("Hello MMP course development project\n");

	/*
	 * The setup of the input system is up to you. It can be initialized after the CTOR
	 * but you can also use `Initialize` methods when it makes sense for your implementation.
	 */
	InputSystem input;

	// @note - lukas.vogl - Pseudo "game-loop" to simulate we are doing something (will ne necessary for further milestones)
	float passedGameTime = 0.0f;
	bool shouldExitGame = false;
	while ( !shouldExitGame )
	{
		input.Update();

		// @note - lukas.vogl - We want to exit the game when the right button on the gamepad face is pressed ( B on XBox controllers, Circle on Dualshocks )
		if ( input.QueryGamepadButtonState( Input::GamepadButtons::FACE_BUTTON_RIGHT, Input::InputAction::BUTTON_PRESSED ) )
		{
			shouldExitGame = true;
		}

		// // @note - lukas.vogl - We want to test the vibration feature with the down button (as long as it's hold, we vibrate)
		if ( input.QueryGamepadButtonState( Input::GamepadButtons::FACE_BUTTON_RIGHT, Input::InputAction::BUTTON_HOLD ) )
		{
			input.ApplyVibrationEffect( 20000 );
		}
		else
		{
			input.ApplyVibrationEffect( 0 );
		}

		// @task - lukas.vogl - Add another function here that let's you (and later me) test that your input system reacts to presses, releases and hold actions for the supported buttons

		// @note - lukas.vogl - This is here to simulate a 60HZ game-loop and will later be used to show further optimizations we can do by using threads
		while ( clock.ToMilliseconds() < passedGameTime + GameConstants::GAME_TARGET_DELTA ) { /* busy wait here - not optimal, and not production ready code, but enough for the assignment */ }
		passedGameTime = clock.ToMilliseconds();
	}

	printf("Shutting dow ...\n");
}