#include "Utilities.hpp"

#include "VknCommon.hpp"


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_surface_create
*
*   DESCRIPTION:
*       Create a new canvas surface, backed by a MSW window.
*
*********************************************************************/

bool VKN_surface_create
    (
    const VkInstance    instance,   /* vulkan instance              */
    const VkAllocationCallbacks
                       *allocator,  /* allocation callbacks         */
    HINSTANCE           hinstance,  /* application instance         */
    HWND                hwnd,       /* underlying window            */
    VkSurfaceKHR       *surface     /* newly created surface        */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkWin32SurfaceCreateInfoKHR
                        ci_surface; /* surface create info          */

/*----------------------------------------------------------
Create the surface
----------------------------------------------------------*/
clr_struct( &ci_surface );
ci_surface.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
ci_surface.hinstance = hinstance;
ci_surface.hwnd      = hwnd;

if( VKN_failed( vkCreateWin32SurfaceKHR( instance, &ci_surface, allocator, surface ) ) )
    {
    return( FALSE );
    }

return( TRUE );

}   /* VKN_surface_create() */
