#pragma once
#include "universe.hpp"

bool PlayerInput_Init( ECS::Universe* universe );
void PlayerInput_Destroy( ECS::Universe* universe );
void PlayerInput_DoFrame( float frame_delta, ECS::Universe* universe );

bool PlayerInput_IsKeyboardKeyGoingDown( ECS::KeyboardKeyScanCode key, ECS:: Universe *universe );
bool PlayerInput_IsKeyboardKeyGoingUp( ECS::KeyboardKeyScanCode key, ECS:: Universe *universe );
