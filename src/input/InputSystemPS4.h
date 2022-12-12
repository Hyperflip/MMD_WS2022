#pragma once

#include <cstdint>
#include <iostream>

#include <pad.h>
#include <user_service.h>

#include "GamepadInputTypes.h"

/*
 * The implementation of the InputSystem is up to you. It has to work on both platforms, PS4 and PC,
 * without having to provide anything else to the provided API methods below. You can decide how
 * initialization and shutdown of the InputSystems works, but make sure to release all resources
 * properly at the end of it's lifetime.
 */


const int PS4_BUTTONS[10] = {
	SCE_PAD_BUTTON_TRIANGLE,
	SCE_PAD_BUTTON_CROSS,
	SCE_PAD_BUTTON_SQUARE,
	SCE_PAD_BUTTON_CIRCLE,

	SCE_PAD_BUTTON_UP,
	SCE_PAD_BUTTON_DOWN,
	SCE_PAD_BUTTON_LEFT,
	SCE_PAD_BUTTON_RIGHT,

	SCE_PAD_BUTTON_L1,
	SCE_PAD_BUTTON_R1
};

class InputSystem
{
public:
	InputSystem()
		: m_UserId(-1)
		, m_PortHandle(-1)
		, m_ButtonDowns(0)
		, m_ButtonUps(0)
		, m_States(0)
		, m_PreviousStates(0)
	{
		sceUserServiceInitialize(NULL);
		scePadInit();

		int ret = sceUserServiceGetInitialUser(&m_UserId);
		if (ret < 0) {
			std::cout << "MMD: Failed to obtain user id" << std::endl;
			return;
		}
		else {
			std::cout << "MMD: Successfully obtained user id" << std::endl;
		}

		m_PortHandle = scePadOpen(m_UserId, SCE_PAD_PORT_TYPE_STANDARD, 0, NULL);
		if (m_PortHandle < 0) {
			std::cout << "MMD: Setting failed" << std::endl;
		}
		else {
			std::cout << "MMD: Setting suceeded" << std::endl;
		}
	}
	/*
		* Queries the state of the provided button and checks if the provided action was done
		*
		* BUTTON_HOLD - returns true as long as the button is done, otherwise false
		* BUTTON_PRESSED - returns true when the button was pressed, otherwise false
		* BUTTON_RELEASED - returns true when the button was release, otherwise false
		*
		*/
	bool QueryGameButtonState(Input::GamepadButtons button, Input::InputAction action)
	{
		int queriedButton = PS4_BUTTONS[(int)button];
		bool consumed = false;

		switch (action) {
		case Input::InputAction::BUTTON_PRESSED:
			consumed = (queriedButton & m_ButtonDowns) != 0;
			if (consumed) {
				m_ButtonDowns = queriedButton ^ m_ButtonDowns;
			}
			return consumed;
		case Input::InputAction::BUTTON_HOLD:
			return (queriedButton & m_States & m_PreviousStates) != 0;
		case Input::InputAction::BUTTON_RELEASED:
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
		* Queries whether a gamepad is currently connected or not
		* Note: normally this would take a handle or ID for the gamepad, but in
		* this assignment, we assume there's only * one * gamepad at a time
		*/
	bool IsGamepadConnected() const
	{
		ScePadData data;
		int ret = scePadReadState(m_PortHandle, &data);
		return data.connected;
	}

	/*
		* Update the internals of the input system (poll gamepads, update states, ...)
		* For now, this will be called once per frame in the main loop
		*/
	void Update() {
		ScePadData data;
		int ret = scePadReadState(m_PortHandle, &data);

		// Data successfully returned
		if (ret >= 0) {

			m_PreviousStates = m_States;
			m_States = data.buttons;
			int changes = m_PreviousStates ^ m_States;

			if (m_States > 0) {
				m_ButtonDowns = m_ButtonDowns | changes;
			}
			else {
				m_ButtonUps = m_ButtonUps | changes;
			}
		}
	}

	/*
		* Applies a vibration effect to the gamepad by letting a user provide a value (could come from a curve asset for
		* gameplay effects for example)
		*/
	void ApplyVibrationEffect(uint32_t motorSpeed) {
		ScePadVibrationParam vibration;
		vibration.largeMotor = motorSpeed;
		vibration.smallMotor = motorSpeed;

		scePadSetVibration(m_PortHandle, &vibration);
	}

private:
	SceUserServiceUserId m_UserId;
	int32_t m_PortHandle;

	int m_ButtonDowns;
	int m_ButtonUps;
	int m_States;
	int m_PreviousStates;
};