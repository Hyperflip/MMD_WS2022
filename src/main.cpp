#include <cstdio>

#include "input/InputSystem.h"
#include "threading/Sys_Threading.h"
#include "core/Clock.h"
#include "save/SaveSystemAPI.h"

namespace GameConstants
{
	constexpr float CONTROLLER_TICK_RATE = 250.0f;
	constexpr float CONTROLLER_TARGET_DELTA = 1000.0f / CONTROLLER_TICK_RATE;
	constexpr float GAME_TICK_RATE = 60.0f;
	constexpr float GAME_TARGET_DELTA = 1000.0f / GAME_TICK_RATE;
}

bool shouldExitGame = false;

void UpdateInput(InputSystem& input) {
	Clock::Init();
	Clock clock;
	clock.Start();

	float passedGameTime = 0.0f;

	while (!shouldExitGame) {
		input.Update();

		while (clock.ToMilliseconds() < passedGameTime + GameConstants::CONTROLLER_TARGET_DELTA);
		passedGameTime = clock.ToMilliseconds();
	}
}

/*

 Y - LOAD GAME
 A - SAVE GAME
 B - QUIT GAME
 X - APPLY VIBRATION
RB - INCREASE SCORE
LB - DECREASE SCORE

*/

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

	threadCreateParam_t params;
	params.function = reinterpret_cast<thread_t>(UpdateInput);
	params.params = &input;
	params.name = "UpdateInput";

	threadHandle_t handle = Sys_CreateThread(params);

	SaveData::SaveGame saveGame;
	SaveData::SaveSystem saveSystem;

	saveSystem.Initialize();

	// @note - lukas.vogl - Pseudo "game-loop" to simulate we are doing something (will ne necessary for further milestones)
	float passedGameTime = 0.0f;
	while ( !shouldExitGame )
	{
		// @note - lukas.vogl - We want to exit the game when the right button on the gamepad face is pressed ( B on XBox controllers, Circle on Dualshocks )
		if ( input.QueryGameButtonState( Input::GamepadButtons::FACE_BUTTON_RIGHT, Input::InputAction::BUTTON_PRESSED ) )
		{
			shouldExitGame = true;
		}

		if (!input.IsGamepadConnected()) {
			std::cout << "MMD: No controller found." << std::endl;
			shouldExitGame = true;
		}

		if (input.QueryGameButtonState(Input::GamepadButtons::FACE_BUTTON_DOWN, Input::InputAction::BUTTON_PRESSED))
		{
			SaveData::SaveFile saveFile;
			saveFile.data = reinterpret_cast<byte*>(&saveGame);
			saveFile.length = sizeof(saveGame);

			if (saveSystem.Save(saveFile, "save.dat")) {
				SaveData::SaveGame save = *reinterpret_cast<SaveData::SaveGame*>(saveFile.data);
				std::cout << "MMD: Saving current score: " << save.score << std::endl;
			}
		}

		if (input.QueryGameButtonState(Input::GamepadButtons::FACE_BUTTON_TOP, Input::InputAction::BUTTON_PRESSED))
		{
			SaveData::SaveFile* saveFile = saveSystem.Load("save.dat");

			if (saveFile->IsValid()) {
				saveGame = *reinterpret_cast<SaveData::SaveGame*>(saveFile->data);
				std::cout << "MMD: Loading last saved score: " << saveGame.score << std::endl;
			}
			else {
				std::cout << "MMD: Could not load save file. Make sure a save file exists. " << std::endl;
			}

			delete saveFile;
		}

		// // @note - lukas.vogl - We want to test the vibration feature with the down button (as long as it's hold, we vibrate)
		if ( input.QueryGameButtonState( Input::GamepadButtons::FACE_BUTTON_LEFT, Input::InputAction::BUTTON_HOLD ) )
		{
			input.ApplyVibrationEffect( 20000 );
		}
		else
		{
			input.ApplyVibrationEffect( 0 );
		}

		if (input.QueryGameButtonState(Input::GamepadButtons::SHOULDER_RIGHT, Input::InputAction::BUTTON_PRESSED)) {
			saveGame.score++;
			std::cout << "MMD: Increasing score: " << saveGame.score << std::endl;
		}

		if (input.QueryGameButtonState(Input::GamepadButtons::SHOULDER_LEFT, Input::InputAction::BUTTON_PRESSED)) {
			if (saveGame.score > 0) {
				saveGame.score--;
			}
			std::cout << "MMD: Decreasing score: " << saveGame.score << std::endl;
		}

		// @task - lukas.vogl - Add another function here that let's you (and later me) test that your input system reacts to presses, releases and hold actions for the supported buttons

		// @note - lukas.vogl - This is here to simulate a 60HZ game-loop and will later be used to show further optimizations we can do by using threads
		while ( clock.ToMilliseconds() < passedGameTime + GameConstants::GAME_TARGET_DELTA ) { /* busy wait here - not optimal, and not production ready code, but enough for the assignment */ }
		passedGameTime = clock.ToMilliseconds();
	}

	Sys_WaitForThread(handle);
	Sys_DestroyThread(handle);

	printf("Shutting dow ...\n");

	return 0;
}