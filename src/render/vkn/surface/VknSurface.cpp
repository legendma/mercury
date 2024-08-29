#include "Utilities.hpp"

#include "VknCommon.hpp"


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_surface_destroy
*
*   DESCRIPTION:
*       Destroy the given surface.
*
*********************************************************************/

void VKN_surface_destroy
    (
    const VkInstance    instance,   /* vulkan instance              */
    const VkAllocationCallbacks
                       *allocator,  /* allocation callbacks         */
    VkSurfaceKHR       *surface     /* surface to destroy           */
    )
{
if( *surface == VK_NULL_HANDLE )
    {
    return;
    }

vkDestroySurfaceKHR( instance, *surface, allocator );
*surface = VK_NULL_HANDLE;

}   /* VKN_surface_destroy() */
