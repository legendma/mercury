#pragma once

#include "VknCommon.hpp"


void VKN_surface_destroy
    (
    const VkInstance    instance,   /* vulkan instance              */
    const VkAllocationCallbacks
                       *allocator,  /* allocation callbacks         */
    VkSurfaceKHR       *surface     /* surface to destroy           */
    );

#if defined( VK_USE_PLATFORM_WIN32_KHR )
bool VKN_surface_create
    (
    const VkInstance    instance,   /* vulkan instance              */
    const VkAllocationCallbacks
                       *allocator,  /* allocation callbacks         */
    HINSTANCE           hinstance,  /* application instance         */
    HWND                hwnd,       /* underlying window            */
    VkSurfaceKHR       *surface     /* newly created surface        */
    );
#endif /* VK_USE_PLATFORM_WIN32_KHR */
