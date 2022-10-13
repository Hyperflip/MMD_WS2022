#pragma once

#include "GamepadInputTypes.h"
#include <cstdint>
#include <pad.h>
#include <iostream>

/*
 * The implementation of the InputSystem is up to you. It has to work on both platforms, PS4 and PC,
 * without having to provide anything else to the provided API methods below. You can decide how
 * initialization and shutdown of the InputSystems works, but make sure to release all resources
 * properly at the end of it's lifetime.
 */

class InputSystem
{
public:
	
	/*
	 * Queries the state of the provided button and checks if the provided action was done
	 *
	 * BUTTON_HOLD - returns true as long as the button is done, otherwise false
	 * BUTTON_PRESSED - returns true when the button was pressed, otherwise false
	 * BUTTON_RELEASED - returns true when the button was release, otherwise false
	 *
	 */
	bool QueryGamepadButtonState( Input::GamepadButtons button, Input::InputAction action )
	{
		return false; // you can remove this and implement the body in the implementation file (doesn't need to be inline)
	}

	/*
	 * Queries whether a gamepad is currently connected or not
	 * Note: normally this would take a handle or ID for the gamepad, but in
	 * this assignment, we assume there's only * one * gamepad at a time
	 */
	bool IsGamepadConnected() const
	{
		return handle != -1;
	}

	/*
	 * Update the internals of the input system (poll gamepads, update states, ...)
	 * For now, this will be called once per frame in the main loop
	 */
	void Update() {
		
		if (!IsGamepadConnected()) {
			scePadInit();
			int ret;
			// Get user ID value
			ret = sceUserServiceGetInitialUser(&userId);
			if (ret < 0) {
				/* Failed to obtain user ID value */
				std::cout << "failed to obtain user ID\n";
				return;
			}

			// Example that specifies SCE_PAD_PORT_TYPE_STANDARD to type
			// Specify 0 for index
			// pParam is a reserved area (specify NULL)
			handle = scePadOpen(userId, SCE_PAD_PORT_TYPE_STANDARD, 0, NULL);
			if (handle < 0) {
				/* Setting failed */
				std::cout << "failed to connect controller\n";
			}
			
			std::cout << "controller connected\n";
		}
	}

	/*
	 * Applies a vibration effect to the gamepad by letting a user provide a value (could come from a curve asset for
	 * gameplay effects for example)
	 */
	void ApplyVibrationEffect( uint32_t motorSpeed ) {}

private:
	/*
	 * All that that you need to store for allowing the API to properly forward input status
	 */
	SceUserServiceUserId userId;
	int32_t handle = -1;
};