#pragma once
#include "Universe.hpp"

namespace Game
{

bool GameMode_Init( ECS::Universe *universe );
void GameMode_Destroy( ECS::Universe *universe );
void GameMode_DoFrame( float frame_delta, ECS::Universe *universe );

} /* namespace Game */