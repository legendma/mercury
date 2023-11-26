#pragma once

#include "Universe.hpp"

bool Render_Init( ECS::Universe* universe );
void Render_Destroy( ECS::Universe* universe );
void Render_DoFrame( float frame_delta, ECS::Universe* universe );
bool Render_LoadModel( const char *asset_name, const ECS::EntityId entity, ECS::Universe *universe );
