#include "ComponentClass.hpp"
#include "Math.hpp"


using namespace ECS;



/*******************************************************************
*
*   IsDpadCenterPressed()
*
*   DESCRIPTION:
*	Boolian functions to get the button state of the controller. 
*	returns true if the button is pressed and false if released.
*
*******************************************************************/

bool IsDpadCenterPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_DPAD_CENTER ) );

} /* IsDpadCenterPressed() */


/*******************************************************************
*
*   IsDpadUpPressed()
*
*   DESCRIPTION:
*	Boolian functions to get the button state of the controller.
*	returns true if the button is pressed and false if released.
*
*******************************************************************/

bool IsDpadUpPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_DPAD_UP ) );

} /* IsDpadUpPressed() */


/*******************************************************************
*
*   IsDpadUpRightPressed()
*
*   DESCRIPTION:
*	Boolian functions to get the button state of the controller.
*	returns true if the button is pressed and false if released.
*
*******************************************************************/
bool IsDpadUpRightPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_DPAD_UPRIGHT ) );

} /* IsDpadUpRightPressed() */


/*******************************************************************
*
*   IsDpadRightPressed()
*
*   DESCRIPTION:
*	Boolian functions to get the button state of the controller.
*	returns true if the button is pressed and false if released.
*
*******************************************************************/

bool IsDpadRightPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_DPAD_RIGHT ) );

} /* IsDpadRightPressed() */

/*******************************************************************
*
*   IsDpadDownRightPressed()
*
*   DESCRIPTION:
*	Boolian functions to get the button state of the controller.
*	returns true if the button is pressed and false if released.
*
*******************************************************************/

bool IsDpadDownRightPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_DPAD_DOWNRIGHT ) );

} /* IsDpadDownRightPressed() */


/*******************************************************************
*
*   IsDpadDownPressed()
*
*   DESCRIPTION:
*	Boolian functions to get the button state of the controller.
*	returns true if the button is pressed and false if released.
*
*******************************************************************/

bool IsDpadDownPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_DPAD_DOWN ) );

} /* IsDpadDownPressed() */


/*******************************************************************
*
*   IsDpadDownLeftPressed()
*
*   DESCRIPTION:
*	Boolian functions to get the button state of the controller.
*	returns true if the button is pressed and false if released.
*
*******************************************************************/

bool IsDpadDownLeftPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_DPAD_DOWNLEFT ) );

} /* IsDpadDownLeftPressed() */


/*******************************************************************
*
*   IsDpadLeftPressed()
*
*   DESCRIPTION:
*	Boolian functions to get the button state of the controller.
*	returns true if the button is pressed and false if released.
*
*******************************************************************/


bool IsDpadLeftPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_DPAD_LEFT ) );

} /* IsDpadLeftPressed() */


/*******************************************************************
*
*   IsDpadUpLeftPressed()
*
*   DESCRIPTION:
*	Boolian functions to get the button state of the controller.
*	returns true if the button is pressed and false if released.
*
*******************************************************************/

bool IsDpadUpLeftPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_DPAD_UPLEFT ) );

} /* IsDpadUpLeftPressed() */


/*******************************************************************
*
*   IsLeftStickPressed()
*
*   DESCRIPTION:
*	Boolian functions to get the button state of the controller.
*	returns true if the button is pressed and false if released.
*
*******************************************************************/

bool IsLeftStickPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_LEFT_STICK_CLICK ) );

} /* IsLeftStickPressed() */


/*******************************************************************
*
*   IsRightStickPressed()
*
*   DESCRIPTION:
*	Boolian functions to get the button state of the controller.
*	returns true if the button is pressed and false if released.
*
*******************************************************************/

bool IsRightStickPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_RIGHT_STICK_CLICK ) );

} /* IsRightStickPressed() */


/*******************************************************************
*
*   IsL1Pressed()
*
*   DESCRIPTION:
*	Boolian functions to get the button state of the controller.
*	returns true if the button is pressed and false if released.
*
*******************************************************************/

bool IsL1Pressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_L1 ) );

} /* IsL1Pressed() */


/*******************************************************************
*
*   IsR1Pressed()
*
*   DESCRIPTION:
*	Boolian functions to get the button state of the controller.
*	returns true if the button is pressed and false if released.
*
*******************************************************************/

bool IsR1Pressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_R1 ) );

} /* IsR1Pressed() */


/*******************************************************************
*
*   IsL2Pressed()
*
*   DESCRIPTION:
*	Boolian functions to get the button state of the controller.
*	returns true if the button is pressed and false if released.
*
*******************************************************************/

bool IsL2Pressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_L2 ) );

} /* IsL2Pressed() */


/*******************************************************************
*
*   IsR2Pressed()
*
*   DESCRIPTION:
*	Boolian functions to get the button state of the controller.
*	returns true if the button is pressed and false if released.
*
*******************************************************************/

bool IsR2Pressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_R2 ) );

} /* IsR2Pressed() */


/*******************************************************************
*
*   IsMenuPressed()
*
*   DESCRIPTION:
*	Boolian functions to get the button state of the controller.
*	returns true if the button is pressed and false if released.
*
*******************************************************************/

bool IsMenuPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_MENU ) );

} /* IsMenuPressed() */


/*******************************************************************
*
*   IsOptionsPressed()
*
*   DESCRIPTION:
*	Boolian functions to get the button state of the controller.
*	returns true if the button is pressed and false if released.
*
*******************************************************************/

bool IsOptionsPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_OPTIONS ) );

} /* IsOptionsPressed() */


/*******************************************************************
*
*   IsCenterPadPressed()
*
*   DESCRIPTION:
*	Boolian functions to get the button state of the controller.
*	returns true if the button is pressed and false if released.
*
*******************************************************************/

bool IsCenterPadPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_CENTER_PAD ) );

} /* IsCenterPadPressed() */


/*******************************************************************
*
*   IsPowerPressed()
*
*   DESCRIPTION:
*	Boolian functions to get the button state of the controller.
*	returns true if the button is pressed and false if released.
*
*******************************************************************/

bool IsPowerPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_POWER ) );

} /* IsPowerPressed() */


/*******************************************************************
*
*   IsMutePressed()
*
*   DESCRIPTION:
*	Boolian functions to get the button state of the controller.
*	returns true if the button is pressed and false if released.
*
*******************************************************************/

bool IsMutePressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_MUTE ) );

} /* IsMutePressed() */


/*******************************************************************
*
*   IsBotButtonPressed()
*
*   DESCRIPTION:
*	Boolian functions to get the button state of the controller.
*	returns true if the button is pressed and false if released.
*
*******************************************************************/

bool IsBotButtonPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_BOT_BUTTON ) );

} /* IsBotButtonPressed() */


/*******************************************************************
*
*   IsTopButtonPressed()
*
*   DESCRIPTION:
*	Boolian functions to get the button state of the controller.
*	returns true if the button is pressed and false if released.
*
*******************************************************************/

bool IsTopButtonPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_TOP_BUTTON ) );

} /* IsTopButtonPressed() */


/*******************************************************************
*
*   IsLeftButtonPressed()
*
*   DESCRIPTION:
*	Boolian functions to get the button state of the controller.
*	returns true if the button is pressed and false if released.
*
*******************************************************************/

bool IsLeftButtonPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_LEFT_BUTTON ) );

} /* IsLeftButtonPressed() */


/*******************************************************************
*
*   IsRightButtonPressed()
*
*   DESCRIPTION:
*	Boolian functions to get the button state of the controller.
*	returns true if the button is pressed and false if released.
*
*******************************************************************/

bool IsRightButtonPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_RIGHT_BUTTON ) );

} /* IsRightButtonPressed() */


/*******************************************************************
*
*   LeftStickQuery()
*
*   DESCRIPTION:
*	Returns the vector (x,y) state of the specified controller stick
*
*******************************************************************/

Vec2 LeftStickQuery( const SingletonControllerInputComponent *component )
{
return(component->axis_state[ CONTROLLER_AXIS_LEFT_STICK ]);

} /* LeftStickQuery() */


/*******************************************************************
*
*   RightStickQuery()
*
*   DESCRIPTION:
*	Returns the vector (x,y) state of the specified controller stick
*
*******************************************************************/

Vec2 RightStickQuery( const SingletonControllerInputComponent *component )
{
return(component->axis_state[ CONTROLLER_AXIS_RIGHT_STICK ]);

} /* RightStickQuery() */


/*******************************************************************
*
*   LeftStickXAxisQuery()
*
*   DESCRIPTION:
*	Returns the X Axis float data for the specified controller stick
*
*******************************************************************/

float LeftStickXAxisQuery( const SingletonControllerInputComponent *component )
{
return(component->axis_state[CONTROLLER_AXIS_LEFT_STICK].v.x );

} /* LeftStickXAxisQuery() */


/*******************************************************************
*
*   LeftStickYAxisQuery()
*
*   DESCRIPTION:
*	Returns the Y Axis float data for the specified controller stick
*
*******************************************************************/

float LeftStickYAxisQuery( const SingletonControllerInputComponent *component )
{
return(component->axis_state[CONTROLLER_AXIS_LEFT_STICK].v.y);

} /* LeftStickYAxisQuery() */


/*******************************************************************
*
*   RightStickXAxisQuery()
*
*   DESCRIPTION:
*	Returns the X Axis float data for the specified controller stick
*
*******************************************************************/

float RightStickXAxisQuery( const SingletonControllerInputComponent *component )
{
return(component->axis_state[CONTROLLER_AXIS_RIGHT_STICK].v.x);

} /* RightStickXAxisQuery() */


/*******************************************************************
*
*   RightStickYAxisQuery()
*
*   DESCRIPTION:
*	Returns the Y Axis float data for the specified controller stick
*
*******************************************************************/

float RightStickYAxisQuery( const SingletonControllerInputComponent *component )
{
return(component->axis_state[CONTROLLER_AXIS_RIGHT_STICK].v.y);

} /* RightStickYAxisQuery() */


/*******************************************************************
*
*   LeftTriggerQuery()
*
*   DESCRIPTION:
*	Returns the controller L2 Trigger float state. 
*	0.0 = fully unpressed, 1.0 = fully pressed
*
*******************************************************************/
float LeftTriggerQuery( const SingletonControllerInputComponent *component )
{
return( component->trigger_state[CONTROLLER_TRIGGERS_LEFT ]);

} /*  LeftTriggerQuery() */


/*******************************************************************
*
*   RightTriggerQuery()
*
*   DESCRIPTION:
*	Returns the controller R2 Trigger float state.
*	0.0 = fully unpressed, 1.0 = fully pressed
*
*******************************************************************/

float RightTriggerQuery( const SingletonControllerInputComponent *component )
{
return( component->trigger_state[CONTROLLER_TRIGGERS_RIGHT] );

} /* RightTriggerQuery() */


/*******************************************************************
*
*   InitializeControllerButtonUtilities()
*
*   DESCRIPTION:
*	Assigns the controller button functions into the component.
*	
*
*******************************************************************/

void InitializeControllerButtonUtilities( SingletonControllerInputComponent *component )
{
	component->is_pressed[ CONTROLLER_BUTTON_DPAD_CENTER ]		= IsDpadCenterPressed;
	component->is_pressed[ CONTROLLER_BUTTON_DPAD_UP]			= IsDpadUpPressed;
	component->is_pressed[ CONTROLLER_BUTTON_DPAD_UPRIGHT]		= IsDpadUpRightPressed;
	component->is_pressed[ CONTROLLER_BUTTON_DPAD_RIGHT]		= IsDpadRightPressed;
	component->is_pressed[ CONTROLLER_BUTTON_DPAD_DOWNRIGHT]	= IsDpadDownRightPressed;
	component->is_pressed[ CONTROLLER_BUTTON_DPAD_DOWN]			= IsDpadDownPressed;
	component->is_pressed[ CONTROLLER_BUTTON_DPAD_DOWNLEFT]		= IsDpadDownLeftPressed;
	component->is_pressed[ CONTROLLER_BUTTON_DPAD_LEFT]			= IsDpadLeftPressed;
	component->is_pressed[ CONTROLLER_BUTTON_DPAD_UPLEFT]		= IsDpadUpLeftPressed;
	component->is_pressed[ CONTROLLER_BUTTON_LEFT_STICK_CLICK]	= IsLeftStickPressed;
	component->is_pressed[ CONTROLLER_BUTTON_RIGHT_STICK_CLICK] = IsRightStickPressed;
	component->is_pressed[ CONTROLLER_BUTTON_L1]				= IsL1Pressed;
	component->is_pressed[ CONTROLLER_BUTTON_R1]				= IsR1Pressed;
	component->is_pressed[ CONTROLLER_BUTTON_L2]				= IsL2Pressed;
	component->is_pressed[ CONTROLLER_BUTTON_R2]				= IsR2Pressed;
	component->is_pressed[ CONTROLLER_BUTTON_MENU]				= IsMenuPressed;
	component->is_pressed[ CONTROLLER_BUTTON_OPTIONS]			= IsOptionsPressed;
	component->is_pressed[ CONTROLLER_BUTTON_CENTER_PAD]		= IsCenterPadPressed;
	component->is_pressed[ CONTROLLER_BUTTON_POWER]				= IsPowerPressed;
	component->is_pressed[ CONTROLLER_BUTTON_MUTE]				= IsMutePressed;
	component->is_pressed[ CONTROLLER_BUTTON_BOT_BUTTON]		= IsBotButtonPressed;
	component->is_pressed[ CONTROLLER_BUTTON_TOP_BUTTON]		= IsTopButtonPressed;
	component->is_pressed[ CONTROLLER_BUTTON_LEFT_BUTTON]		= IsLeftButtonPressed;
	component->is_pressed[ CONTROLLER_BUTTON_RIGHT_BUTTON]		= IsRightButtonPressed;

} /* InitializeControllerButtonUtilities */

