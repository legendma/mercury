#pragma once
#include "universe.hpp"


bool Sound_Init( ECS::Universe *universe );
void Sound_Update( float frame_delta, ECS::Universe *universe );
void Sound_Destroy( ECS::Universe *universe );
bool Sound_StopSound( const char *sound_name, ECS::Universe *universe );
bool Sound_PlaySound( const ECS::EntityId entity, ECS::Universe *universe );
bool Sound_AssignSoundtToEntity( const char *sound_name, const bool overwrite_sound, const ECS::EntityId entity, ECS::Universe *universe );
