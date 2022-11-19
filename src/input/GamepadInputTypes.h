#pragma once

namespace Input
{
	enum class GamepadButtons
	{
		FACE_BUTTON_TOP,
		FACE_BUTTON_DOWN,
		FACE_BUTTON_LEFT,
		FACE_BUTTON_RIGHT,

		DPAD_TOP,
		DPAD_DOWN,
		DPAD_LEFT,
		DPAD_RIGHT,

		SHOULDER_LEFT,
		SHOULDER_RIGHT,
	};

	enum class InputAction
	{
		BUTTON_HOLD,
		BUTTON_PRESSED,
		BUTTON_RELEASED,
	};

	enum class ButtonState {
		HOLD,
		PRESSED,
		RELEASED,
		IDLE,
	};
}
