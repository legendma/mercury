#pragma once
#include "universe.hpp"

bool PlayerInput_InitializeController(ECS::Universe* universe);
void PlayerInput_ShutdownController(ECS::Universe* universe);
void PlayerInput_GetCurrentInput(ECS::Universe* universe);


