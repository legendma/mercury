#pragma once
#include "universe.hpp"

void Engine_ChangeResolutions( const uint16_t width, const uint16_t height );
bool Engine_Init();
void Engine_DoFrame( float frame_delta );
bool Engine_Destroy();



