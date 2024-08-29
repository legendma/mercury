#pragma once
#include "Universe.hpp"
#include "Vkn.hpp"

void Engine_ChangeResolutions( const uint16_t width, const uint16_t height );
bool Engine_Init( VkSurfaceKHR surface, VkInstance vulkan );
void Engine_DoFrame( float frame_delta );
bool Engine_Destroy();



