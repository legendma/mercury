#pragma once

#include "ComponentClass.hpp"
#include "Universe.hpp"
#include "Vkn.hpp"

void                  Render_ChangeResolutions( const uint16_t width, const uint16_t height, ECS::Universe *universe );
void                  Render_CreateVulkanInstance( VKN_instance_type *out );
void                  Render_Destroy( ECS::Universe* universe );
void                  Render_DoFrame( float frame_delta, ECS::Universe* universe );
bool                  Render_Init( VkSurfaceKHR surface, VkInstance instance, ECS::Universe* universe );
//ECS::ModelComponent * Render_LoadModel( const char *asset_name, const ECS::EntityId entity, ECS::Universe *universe );
