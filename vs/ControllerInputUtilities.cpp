#include "ComponentClass.hpp"
#include "Math.hpp"


using namespace ECS;

bool IsDpadCenterPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_DPAD_CENTER ) );

}


bool IsDpadUpPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_DPAD_UP ) );

}


bool IsDpadUpRightPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_DPAD_UPRIGHT ) );

}


bool IsDpadRightPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_DPAD_RIGHT ) );

}


bool IsDpadDownRightPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_DPAD_DOWNRIGHT ) );

}


bool IsDpadDownPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_DPAD_DOWN ) );

}


bool IsDpadDownLeftPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_DPAD_DOWNLEFT ) );

}


bool IsDpadLeftPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_DPAD_LEFT ) );

}


bool IsDpadUpLeftPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_DPAD_UPLEFT ) );

}


bool IsLeftStickPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_LEFT_STICK_CLICK ) );

}


bool IsRightStickPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_RIGHT_STICK_CLICK ) );

}


bool IsL1Pressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_L1 ) );

}


bool IsR1Pressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_R1 ) );

}


bool IsL2Pressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_L2 ) );

}


bool IsR2Pressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_R2 ) );

}


bool IsMenuPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_MENU ) );

}


bool IsOptionsPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_OPTIONS ) );

}


bool IsCenterPadPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_CENTER_PAD ) );

}


bool IsPowerPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_POWER ) );

}


bool IsMutePressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_MUTE ) );

}


bool IsBotButtonPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_BOT_BUTTON ) );

}


bool IsTopButtonPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_TOP_BUTTON ) );

}


bool IsLeftButtonPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_LEFT_BUTTON ) );

}


bool IsRightButtonPressed( const SingletonControllerInputComponent *component )
{
return( Math_BitArrayIsSet( component->button_state.ba, CONTROLLER_BUTTON_RIGHT_BUTTON ) );

}




