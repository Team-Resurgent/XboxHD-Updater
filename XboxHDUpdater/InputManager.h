#pragma once

#include <xtl.h>

typedef enum ButtonState {
	ButtonStateNone,
	ButtonStatePressed
} ButtonState;

typedef enum JoystickButton {
	JoystickButtonA,
	JoystickButtonB,
	JoystickButtonX,
	JoystickButtonY, 
	JoystickButtonWhite,
	JoystickButtonBlack,
	JoystickButtonBack,
	JoystickButtonStart,
	JoystickButtonLeftThumb,
	JoystickButtonRightThumb,
	JoystickButtonDpadUp,
	JoystickButtonDpadRight,
	JoystickButtonDpadDown,
	JoystickButtonDpadLeft,
	JoystickButtonLeftTrigger,
	JoystickButtonRightTrigger
} JoystickButton;

typedef struct JoystickButtonStates
{
	ButtonState buttonA;
	ButtonState buttonB;
	ButtonState buttonX;
	ButtonState buttonY; 
	ButtonState buttonWhite;
	ButtonState buttonBlack;
	ButtonState buttonBack;
	ButtonState buttonStart;
	ButtonState buttonLeftThumb;
	ButtonState buttonRightThumb;
	ButtonState buttonDpadUp;
	ButtonState buttonDpadRight;
	ButtonState buttonDpadDown;
	ButtonState buttonDpadLeft;
	ButtonState buttonLeftTrigger;
	ButtonState buttonRightTrigger;
} JoystickButtonStates;

class InputManager
{
public:
	static void ProcessController();
	static bool ButtonDown(JoystickButton button, int port = -1);
	static bool ButtonPressed(JoystickButton button, int port = -1);
	static BYTE GetLeftTriggerPressure(int port = -1);
	static BYTE GetRightTriggerPressure(int port = -1);
};
