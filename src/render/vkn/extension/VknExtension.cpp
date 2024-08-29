#include "VknCommon.hpp"

static int              s_magic;    /* magic init sentinel          */
static PFN_vkCreateDebugUtilsMessengerEXT
                        vkCreateDebugUtilsMessenger = NULL;
static PFN_vkDestroyDebugUtilsMessengerEXT
                        vkDestroyDebugUtilsMessenger = NULL;
static PFN_vkSetDebugUtilsObjectNameEXT
                        vkSetDebugUtilsObjectName = NULL;


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_extensions_init
*
*   DESCRIPTION:
*       Initialize the extension wrangler.
*
*********************************************************************/

void VKN_extensions_init
    (
    const VkInstance    instance    /* vulkan instance              */
    )
{
if( s_magic == MAGIC_INIT )
    {
    return;
    }

s_magic = MAGIC_INIT;

vkCreateDebugUtilsMessenger  =  (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr( instance, "vkCreateDebugUtilsMessengerEXT" );
vkDestroyDebugUtilsMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr( instance, "vkDestroyDebugUtilsMessengerEXT" );
vkSetDebugUtilsObjectName    =    (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr( instance, "vkSetDebugUtilsObjectNameEXT" );

}   /* VKN_extensions_init() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       vkCreateDebugUtilsMessengerEXT
*
*********************************************************************/

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT
    (
    VkInstance          instance,
    const VkDebugUtilsMessengerCreateInfoEXT
                       *pCreateInfo,
    const VkAllocationCallbacks
                       *pAllocator,
    VkDebugUtilsMessengerEXT
                       *pMessenger
    )
{
if( !vkCreateDebugUtilsMessenger )
    {
    return( VK_ERROR_INCOMPATIBLE_DRIVER );
    }

return( vkCreateDebugUtilsMessenger( instance, pCreateInfo, pAllocator, pMessenger ) );

}   /* vkCreateDebugUtilsMessengerEXT() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       vkDestroyDebugUtilsMessengerEXT
*
*********************************************************************/

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT
    (
    VkInstance          instance,
    VkDebugUtilsMessengerEXT
                        messenger,
    const VkAllocationCallbacks
                       *pAllocator
    )
{
if( !vkDestroyDebugUtilsMessenger )
    {
    return;
    }

vkDestroyDebugUtilsMessenger( instance, messenger, pAllocator );

}   /* vkDestroyDebugUtilsMessengerEXT() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       vkSetDebugUtilsObjectNameEXT
*
*********************************************************************/

VKAPI_ATTR VkResult VKAPI_CALL vkSetDebugUtilsObjectNameEXT
    (
    VkDevice            device,
    const VkDebugUtilsObjectNameInfoEXT
                       *pNameInfo
    )
{
if( !vkSetDebugUtilsObjectName )
    {
    return( VK_ERROR_INCOMPATIBLE_DRIVER );
    }

return( vkSetDebugUtilsObjectName( device, pNameInfo ) );

}   /* vkDestroyDebugUtilsMessengerEXT() */
