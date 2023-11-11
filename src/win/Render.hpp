#pragma once
#include "Universe.hpp"

typedef void * RenderWindowInfo;

bool Render_Init( ECS::Universe* universe );
void Render_Destroy( ECS::Universe* universe );
void Render_DoFrame( float frame_delta, ECS::Universe* universe );


