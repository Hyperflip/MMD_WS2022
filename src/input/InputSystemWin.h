#pragma once

#include <windows.h>
#include <cstdint>
#include <xinput.h>

#include "GamepadInputTypes.h"

/*
 * The implementation of the InputSystem is up to you. It has to work on both platforms, PS4 and PC,
 * without having to provide anything else to the provided API methods below. You can decide how
 * initialization and shutdown of the InputSystems works, but make sure to release all resources
 * properly at the end of it's lifetime.
 */

const int WIN_BUTTONS[10] = {
		XINPUT_GAMEPAD_Y,
		XINPUT_GAMEPAD_A,
		XINPUT_GAMEPAD_X,
		XINPUT_GAMEPAD_B,

		XINPUT_GAMEPAD_DPAD_UP,
		XINPUT_GAMEPAD_DPAD_DOWN,
		XINPUT_GAMEPAD_DPAD_LEFT,
		XINPUT_GAMEPAD_DPAD_RIGHT,

		XINPUT_GAMEPAD_LEFT_SHOULDER,
		XINPUT_GAMEPAD_RIGHT_SHOULDER
};

class InputSystem {
private:
	DWORD m_GamePadIndex = 0;

	int m_ButtonDowns;
	int m_ButtonUps;
	int m_States;
	int m_PreviousStates;

public:
	// member initialization list constructor
	InputSystem()
		: m_GamePadIndex(0)
		, m_ButtonDowns(0)
		, m_ButtonUps(0)
		, m_States(0)
		, m_PreviousStates(0) {}

	bool IsGamepadConnected() const {
		DWORD dwResult;
		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));

		dwResult = XInputGetState(m_GamePadIndex, &state);

		return dwResult == ERROR_SUCCESS;
	}

	/*
	* Queries the state of the provided button and checks if the provided action was done
	*
	* BUTTON_HOLD - returns true as long as the button is done, otherwise false
	* BUTTON_PRESSED - returns true when the button was pressed, otherwise false
	* BUTTON_RELEASED - returns true when the button was release, otherwise false
	*
	*/
	bool QueryGameButtonState(Input::GamepadButtons button, Input::InputAction action) {
		int queriedButton = WIN_BUTTONS[(int)button];
		bool consumed = false;

		switch (action) {
		case Input::InputAction::BUTTON_PRESSED:
			// check if queried button is part of buttons currently down
			consumed = (queriedButton & m_ButtonDowns) != 0;
			if (consumed) {
				// remove queried button from buttons down (in order to prevent positive return in next query for pressed)
				m_ButtonDowns = queriedButton ^ m_ButtonDowns;
			}
			return consumed;
		case Input::InputAction::BUTTON_HOLD:
			// return true if queried button in both previous and current state
			return (queriedButton & m_States & m_PreviousStates) != 0;
		case Input::InputAction::BUTTON_RELEASED:
			// same as pressed
			consumed = (queriedButton & m_ButtonUps) != 0;
			if (consumed) {
				m_ButtonUps = queriedButton ^ m_ButtonUps;
			}
			return consumed;
		default:
			return false;
		}
	}

	/*
	* Update the internals of the input system (poll gamepads, update states, ...)
	* For now, this will be called once per frame in the main loop
	*/
	void Update() {
		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));
		XInputGetState(m_GamePadIndex, &state);

		m_PreviousStates = m_States;
		m_States = state.Gamepad.wButtons;
		int changes = m_PreviousStates ^ m_States;

		// apply changes to button downs/ups
		if (m_States > 0) {
			m_ButtonDowns |= changes;
		}
		else {
			m_ButtonUps |= changes;
		}
	}

	void ApplyVibrationEffect(uint32_t motorSpeed) {
		XINPUT_VIBRATION vibration;
		ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
		vibration.wLeftMotorSpeed = motorSpeed;
		vibration.wRightMotorSpeed = motorSpeed;
		XInputSetState(m_GamePadIndex, &vibration);
	}
};