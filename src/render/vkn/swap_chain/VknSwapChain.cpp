#include "Global.hpp"
#include "Utilities.hpp"

#include "VknCommon.hpp"
#include "VknReleaser.hpp"
#include "VknSwapChain.hpp"
#include "VknSwapChainTypes.hpp"


/*********************************************************************
*
*   PROCEDURE NAME:
*       find_alternate_format
*
*   DESCRIPTION:
*       Find a suitable replacement swap chain format, given an
*       unsupported format.
*
*********************************************************************/

static __inline VkFormat find_alternate_format
    (
    const VkFormat      format      /* format to replace            */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkFormat                ret;        /* return replacement format    */

ret = VK_FORMAT_UNDEFINED;
switch( format )
    {
    case VK_FORMAT_R8G8B8A8_UNORM:
        ret = VK_FORMAT_B8G8R8A8_UNORM;
        break;

    case VK_FORMAT_B8G8R8A8_UNORM:
        ret = VK_FORMAT_R8G8B8A8_UNORM;
        break;

    case VK_FORMAT_B8G8R8A8_SRGB:
        ret = VK_FORMAT_R8G8B8A8_SRGB;
        break;

    case VK_FORMAT_R8G8B8A8_SRGB:
        ret = VK_FORMAT_B8G8R8A8_SRGB;
        break;
    }

return( ret );

}   /* find_alternate_format */


/*********************************************************************
*
*   PROCEDURE NAME:
*       has_format
*
*********************************************************************/

static __inline bool has_format
    (
    const VkFormat      check_format,
                                    /* surface format to check      */
    const VkColorSpaceKHR           /* surface color space to check */
                        check_color_space,
    const VkSurfaceFormatKHR
                       *formats,    /* existing formats             */
    const u32           format_cnt  /* number of existing formats   */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     i;          /* loop counter                 */

for( i = 0; i < format_cnt; i++ )
    {
    if( formats[ i ].format == check_format
     && formats[ i ].colorSpace == check_color_space )
        {
        return( TRUE );
        }
    }

return( FALSE );

}   /* has_format() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       has_format
*
*********************************************************************/

static __inline bool has_present_mode
    (
    const VkPresentModeKHR
                        check_mode, /* presentation mode to check   */
    const VkPresentModeKHR
                       *modes,      /* existing modes               */
    const u32           mode_cnt  /* number of existing modes     */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     i;          /* loop counter                 */

for( i = 0; i < mode_cnt; i++ )
    {
    if( modes[ i ] == check_mode )
        {
        return( TRUE );
        }
    }

return( FALSE );

}   /* has_present_mode() */


static VKN_swap_chain_build_add_present_mode_proc_type add_present_mode;

static bool add_present_mode_safe
    (
    const VkPresentModeKHR
                        mode,       /* presentation mode            */
    VKN_swap_chain_build_present_modes_type
                       *modes       /* presentation mode list       */
    );

static VKN_swap_chain_build_add_surface_format_proc_type add_surface_format;

static bool add_surface_format_safe
    (
    const VkFormat      format,     /* surface format               */
    const VkColorSpaceKHR
                        color_space,/* surface color space          */
    VKN_swap_chain_build_surface_formats_type
                       *formats     /* format list                  */
    );

static VKN_swap_chain_build_commit_config_proc_type commit_config;

static bool create_image_views
    (
    VKN_swap_chain_type
                       *swap_chain  /* output new swap chain        */
    );

static bool init_builder
    (
    VKN_swap_chain_build_type
                       *builder     /* swap chain builder           */
    );

static VKN_swap_chain_build_reset_proc_type reset;

static VkExtent2D resolve_extent
    (
    const VkExtent2D    want,       /* desired extent               */
    const VkSurfaceCapabilitiesKHR
                       *have        /* supported extent             */
    );

static u32 resolve_image_count
    (
    const VkSurfaceCapabilitiesKHR
                       *capabilities/* supported extent             */
    );

static bool resolve_present_mode
    (
    const VKN_swap_chain_build_present_modes_type
                       *want,       /* desired modes                */
    const VKN_swap_chain_build_present_modes_type
                       *have,       /* supported modes              */
    VkPresentModeKHR   *out         /* output supported mode        */
    );

static bool resolve_surface_format
    (
    const VKN_swap_chain_build_surface_formats_type
                       *want,       /* desired formats              */
    const VKN_swap_chain_build_surface_formats_type
                       *have,       /* supported formats            */
    VkSurfaceFormatKHR *out         /* output supported format      */
    );

static bool retrieve_images
    (
    VKN_swap_chain_type
                       *swap_chain  /* output new swap chain        */
    );

static VKN_swap_chain_build_set_allocation_callbacks_proc_type set_allocation_callbacks;
static VKN_swap_chain_build_set_default_present_mode_proc_type set_default_present_mode;
static VKN_swap_chain_build_set_default_surface_format_proc_type set_default_surface_format;
static VKN_swap_chain_build_set_extent_proc_type set_extent;


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_swap_chain_create
*
*   DESCRIPTION:
*       Create a swap chain via the given builder.
*
*********************************************************************/

bool VKN_swap_chain_create
    (
    const u32           present_queue_index,
                                    /* presentation family index    */
    const u32           graphics_queue_index,
                                    /* graphics family index        */
    const VKN_swap_chain_build_type
                       *builder,    /* swap chain builder           */
    VKN_swap_chain_type
                       *swap_chain  /* output new swap chain        */
    )
{
/*----------------------------------------------------------
Local literals
----------------------------------------------------------*/
#define CONCURRENT_INDEX_CNT        ( 2 )

/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkSwapchainCreateInfoKHR            /* swap chain create info       */
                        ci_swap_chain;
VkExtent2D              extent;     /* surface extent               */
u32                     image_count;/* swap chain image count       */
VkPresentModeKHR        present_mode;
                                    /* presentation mode            */
u32                     queue_index_count;
                                    /* queue family index count     */
const u32              *queue_indices;
                                    /* queue family indices         */
u32                     queue_indices_arr[ CONCURRENT_INDEX_CNT ];
                                    /* queue family indices storage */
VkSurfaceFormatKHR      surface_format;
                                    /* swap chain image format      */
VkSharingMode           thread_model;
                                    /* queue threading model        */

clr_struct( swap_chain );

/*----------------------------------------------------------
Resolve desires with capabilities
----------------------------------------------------------*/
if( !resolve_present_mode( &builder->state.present_modes, &builder->state.capabilities.present_modes, &present_mode )
 || !resolve_surface_format( &builder->state.formats, &builder->state.capabilities.formats, &surface_format ) )
    {
    debug_assert_always();
    return( FALSE );
    }

extent      = resolve_extent( builder->state.extent, &builder->state.capabilities.capabilities );
image_count = resolve_image_count( &builder->state.capabilities.capabilities );

/*----------------------------------------------------------
Determine threading model
----------------------------------------------------------*/
thread_model      = VK_SHARING_MODE_EXCLUSIVE;
queue_indices     = NULL;
queue_index_count = 0;

if( present_queue_index != graphics_queue_index )
    {
    queue_indices_arr[ 0 ] = present_queue_index;
    queue_indices_arr[ 1 ] = graphics_queue_index;

    thread_model      = VK_SHARING_MODE_CONCURRENT;
    queue_indices     = queue_indices_arr;
    queue_index_count = cnt_of_array( queue_indices_arr );
    }

/*----------------------------------------------------------
Create the swap chain
----------------------------------------------------------*/
clr_struct( &ci_swap_chain );
ci_swap_chain.sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
ci_swap_chain.surface               = builder->state.surface;
ci_swap_chain.minImageCount         = image_count;
ci_swap_chain.imageFormat           = surface_format.format;
ci_swap_chain.imageColorSpace       = surface_format.colorSpace;
ci_swap_chain.imageExtent           = extent;
ci_swap_chain.imageArrayLayers      = 1;
ci_swap_chain.imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
ci_swap_chain.imageSharingMode      = thread_model;
ci_swap_chain.queueFamilyIndexCount = queue_index_count;
ci_swap_chain.pQueueFamilyIndices   = queue_indices;
ci_swap_chain.preTransform          = builder->state.capabilities.capabilities.currentTransform;
ci_swap_chain.compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
ci_swap_chain.presentMode           = present_mode;
ci_swap_chain.clipped               = VK_TRUE;

if( VKN_failed( vkCreateSwapchainKHR( builder->state.logical, &ci_swap_chain, builder->state.allocator, &swap_chain->swap_chain ) ) )
    {
    debug_assert_always();
    return( FALSE );
    }

swap_chain->logical   = builder->state.logical;
swap_chain->allocator = builder->state.allocator;
swap_chain->extent    = extent;
swap_chain->format    = surface_format.format;

/*----------------------------------------------------------
Retrieve the images and create their views
----------------------------------------------------------*/
if( !retrieve_images( swap_chain )
 || !create_image_views( swap_chain ) )
    {
    debug_assert_always();
    VKN_swap_chain_destroy( NULL, swap_chain );
    return( FALSE );
    }

return( TRUE );

#undef CONCURRENT_INDEX_CNT
}   /* VKN_swap_chain_create() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_swap_chain_destroy
*
*   DESCRIPTION:
*       Destroy the given swap chain.
*
*********************************************************************/

void VKN_swap_chain_destroy
    (
    VKN_releaser_type  *releaser,   /* release buffer               */
    VKN_swap_chain_type
                       *swap_chain  /* swap chain to destroy        */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     i;          /* loop counter                 */

VKN_releaser_auto_mini_begin( releaser, use );
use->i->release_swap_chain( swap_chain->logical, swap_chain->allocator, swap_chain->swap_chain, use );
for( i = 0; i < swap_chain->images.count; i++ )
    {
    use->i->release_image_view( swap_chain->logical, swap_chain->allocator, swap_chain->images.views[ i ], use );
    }

VKN_releaser_auto_mini_end( use );
clr_struct( swap_chain );

}   /* VKN_swap_chain_destroy() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_swap_chain_init_builder
*
*   DESCRIPTION:
*       Initialize a swap chain builder.
*
*********************************************************************/

VKN_SWAP_CHAIN_CONFIG_API VKN_swap_chain_init_builder
    (
    const VkPhysicalDevice
                        physical,   /* associated physical device   */
    const VkDevice      logical,    /* associated logical device    */
    VKN_swap_chain_build_type
                       *builder     /* swap chain builder           */
    )
{
/*----------------------------------------------------------
Local constants
----------------------------------------------------------*/
static const VKN_swap_chain_build_config_type CONFIG =
    {
    add_present_mode,
    add_surface_format,
    commit_config,
    reset,
    set_allocation_callbacks,
    set_default_present_mode,
    set_default_surface_format,
    set_extent
    };

/*----------------------------------------------------------
Initialize
----------------------------------------------------------*/
debug_assert( physical != VK_NULL_HANDLE
           && logical != VK_NULL_HANDLE );

clr_struct( builder );
builder->config = &CONFIG;

builder->state.physical = physical;
builder->state.logical  = logical;

return( builder->config );

}   /* VKN_swap_chain_init_builder() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       add_present_mode
*
*********************************************************************/

static VKN_SWAP_CHAIN_CONFIG_API add_present_mode
    (
    const VkPresentModeKHR
                        mode,       /* presentation mode            */
    struct _VKN_swap_chain_build_type
                       *builder     /* swap chain builder           */
    )
{
add_present_mode_safe( mode, &builder->state.present_modes );

return( builder->config );

}   /* add_present_mode() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       add_present_mode_safe
*
*********************************************************************/

static bool add_present_mode_safe
    (
    const VkPresentModeKHR
                        mode,       /* presentation mode            */
    VKN_swap_chain_build_present_modes_type
                       *modes       /* presentation mode list       */
    )
{
if( has_present_mode( mode, modes->modes, modes->count ) )
    {
    return( TRUE );
    }
else if( modes->count >= cnt_of_array( modes->modes ) )
    {
    debug_assert_always();
    return( FALSE );
    }

modes->modes[ modes->count++ ] = mode;
return( TRUE );

}   /* add_present_mode_safe() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       add_surface_format
*
*********************************************************************/

static VKN_SWAP_CHAIN_CONFIG_API add_surface_format
    (
    const VkFormat      format,     /* surface format               */
    const VkColorSpaceKHR
                        color_space,/* surface color space          */
    struct _VKN_swap_chain_build_type
                       *builder     /* swap chain builder           */
    )
{
add_surface_format_safe( format, color_space, &builder->state.formats );

return( builder->config );

}   /* add_surface_format() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       add_surface_format_safe
*
*********************************************************************/

static bool add_surface_format_safe
    (
    const VkFormat      format,     /* surface format               */
    const VkColorSpaceKHR
                        color_space,/* surface color space          */
    VKN_swap_chain_build_surface_formats_type
                       *formats     /* format list                  */
    )
{
if( has_format( format, color_space, formats->formats, formats->count ) )
    {
    return( TRUE );
    }
else if( formats->count >= cnt_of_array( formats->formats ) )
    {
    debug_assert_always();
    return( FALSE );
    }

formats->formats[ formats->count ].format     = format;
formats->formats[ formats->count ].colorSpace = color_space;
formats->count++;

return( TRUE );

}   /* add_surface_format_safe() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       create_image_views
*
*********************************************************************/

static bool create_image_views
    (
    VKN_swap_chain_type
                       *swap_chain  /* output new swap chain        */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkImageViewCreateInfo   ci_view;    /* image view create info       */
u32                     i;          /* loop counter                 */

clr_struct( &ci_view );
ci_view.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
ci_view.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
ci_view.format                          = swap_chain->format;
ci_view.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
ci_view.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
ci_view.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
ci_view.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
ci_view.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
ci_view.subresourceRange.baseMipLevel   = 0;
ci_view.subresourceRange.levelCount     = 1;
ci_view.subresourceRange.baseArrayLayer = 0;
ci_view.subresourceRange.layerCount     = 1;

for( i = 0; i < swap_chain->images.count; i++ )
    {
    ci_view.image = swap_chain->images.images[ i ];
    VKN_return_fail( vkCreateImageView( swap_chain->logical, &ci_view, swap_chain->allocator, &swap_chain->images.views[ i ] ) );
    }

return( TRUE );

}   /* create_image_views() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       commit_config
*
*********************************************************************/

static VKN_SWAP_CHAIN_CONFIG_API commit_config
    (
    const VkSurfaceKHR  surface,    /* associated display surface   */
    struct _VKN_swap_chain_build_type
                       *builder     /* swap chain builder           */
    )
{
builder->state.surface = surface;

if( !init_builder( builder ) )
    {
    debug_assert_always();
    }

return( builder->config );

}   /* commit_config() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       init_builder
*
*   DESCRIPTION:
*       Initialize the builder state.
*
*********************************************************************/

static bool init_builder
    (
    VKN_swap_chain_build_type
                       *builder     /* swap chain builder           */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_swap_chain_build_present_modes_type
                       *present_modes;
                                    /* handy present modes pointer  */
VKN_swap_chain_build_surface_formats_type
                       *surface_formats;
                                    /* handy surface formats pointer*/

/*----------------------------------------------------------
Device capabilities
----------------------------------------------------------*/
VKN_return_fail( vkGetPhysicalDeviceSurfaceCapabilitiesKHR( builder->state.physical, builder->state.surface, &builder->state.capabilities.capabilities ) );

/*----------------------------------------------------------
Device surface formats
----------------------------------------------------------*/
surface_formats = &builder->state.capabilities.formats;
VKN_return_fail( vkGetPhysicalDeviceSurfaceFormatsKHR( builder->state.physical, builder->state.surface, &surface_formats->count, NULL ) );
if( surface_formats->count > cnt_of_array( surface_formats->formats ) )
    {
    debug_assert_always();
    surface_formats->count = cnt_of_array( surface_formats->formats );
    }

VKN_return_fail( vkGetPhysicalDeviceSurfaceFormatsKHR( builder->state.physical, builder->state.surface, &surface_formats->count, surface_formats->formats ) );

/*----------------------------------------------------------
Device presentation modes
----------------------------------------------------------*/
present_modes = &builder->state.capabilities.present_modes;
VKN_return_fail( vkGetPhysicalDeviceSurfacePresentModesKHR( builder->state.physical, builder->state.surface, &present_modes->count, NULL ) );
if( present_modes->count > cnt_of_array( present_modes->modes ) )
    {
    debug_assert_always();
    present_modes->count = cnt_of_array( present_modes->modes );
    }

VKN_return_fail( vkGetPhysicalDeviceSurfacePresentModesKHR( builder->state.physical, builder->state.surface, &present_modes->count, present_modes->modes ) );

return( TRUE );

}   /* init_builder() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       reset
*
*********************************************************************/

static VKN_SWAP_CHAIN_CONFIG_API reset
    (
    struct _VKN_swap_chain_build_type
                       *builder     /* swap chain builder           */
    )
{
builder->state.surface = VK_NULL_HANDLE;
clr_struct( &builder->state.formats );
clr_struct( &builder->state.present_modes );
clr_struct( &builder->state.extent );
clr_struct( &builder->state.capabilities );

return( builder->config );

}   /* reset() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       resolve_extent
*
*********************************************************************/

static VkExtent2D resolve_extent
    (
    const VkExtent2D    want,       /* desired extent               */
    const VkSurfaceCapabilitiesKHR
                       *have        /* supported extent             */
    )
{
/*----------------------------------------------------------
Local literals
----------------------------------------------------------*/
#define INVALID_EXTENT  max_uint_value( u32 )

/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkExtent2D              ret;        /* return surface extent        */

if( have->currentExtent.width != INVALID_EXTENT )
    {
    /*------------------------------------------------------
    Surface has a defined size
    ------------------------------------------------------*/
    debug_assert( have->currentExtent.height != INVALID_EXTENT );
    return( have->currentExtent );
    }

/*----------------------------------------------------------
Ensure the dimensions are clamped within capabilities
----------------------------------------------------------*/
ret = want;
if( ret.width > have->maxImageExtent.width )
    {
    ret.width = have->maxImageExtent.width;
    }

if( ret.width < have->minImageExtent.width )
    {
    ret.width = have->minImageExtent.width;
    }

if( ret.height > have->maxImageExtent.height )
    {
    ret.height = have->maxImageExtent.height;
    }

if( ret.height < have->minImageExtent.height )
    {
    ret.height = have->minImageExtent.height;
    }

return( ret );

#undef INVALID_EXTENT
}   /* resolve_extent() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       resolve_image_count
*
*********************************************************************/

static u32 resolve_image_count
    (
    const VkSurfaceCapabilitiesKHR
                       *capabilities/* supported extent             */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     ret;        /* return image count           */

ret = capabilities->minImageCount + 1;
if( capabilities->maxImageCount > 0
 && ret > capabilities->maxImageCount )
    {
    ret = capabilities->maxImageCount;
    }

debug_assert( ret <= cnt_of_array( ( (VKN_swap_chain_type*)NULL )->images.images ) );

return( ret );

}   /* resolve_image_count() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       resolve_present_mode
*
*********************************************************************/

static bool resolve_present_mode
    (
    const VKN_swap_chain_build_present_modes_type
                       *want,       /* desired modes                */
    const VKN_swap_chain_build_present_modes_type
                       *have,       /* supported modes              */
    VkPresentModeKHR   *out         /* output supported mode        */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
bool                    found;      /* did we find a match?         */
u32                     i;          /* loop counter                 */

found = FALSE;
for( i = 0; !found && i < want->count; i++ )
    {
    *out = want->modes[ i ];
    found = has_present_mode( *out, have->modes, have->count );
    }

return( found );

}   /* resolve_present_mode() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       resolve_surface_format
*
*********************************************************************/

static bool resolve_surface_format
    (
    const VKN_swap_chain_build_surface_formats_type
                       *want,       /* desired formats              */
    const VKN_swap_chain_build_surface_formats_type
                       *have,       /* supported formats            */
    VkSurfaceFormatKHR *out         /* output supported format      */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkFormat                alternate;  /* alternate format             */
u32                     i;          /* loop counter                 */
bool                    ret;        /* did we find a match?         */

ret = FALSE;
for( i = 0; !ret && i < want->count; i++ )
    {
    *out  = want->formats[ i ];
    ret = has_format( out->format, out->colorSpace, have->formats, have->count );

    alternate = find_alternate_format( out->format );
    if( !ret
     && alternate != VK_FORMAT_UNDEFINED )
        {
        out->format = alternate;
        ret = has_format( out->format, out->colorSpace, have->formats, have->count );
        }
    }

return( ret );

}   /* resolve_surface_format() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       retrieve_images
*
*********************************************************************/

static bool retrieve_images
    (
    VKN_swap_chain_type
                       *swap_chain  /* output new swap chain        */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     image_count;/* number of images             */

if( VKN_failed( vkGetSwapchainImagesKHR( swap_chain->logical, swap_chain->swap_chain, &image_count, NULL ) )
 || image_count > cnt_of_array( swap_chain->images.images ) )
    {
    return( FALSE );
    }

swap_chain->images.count = image_count;
VKN_return_fail( vkGetSwapchainImagesKHR( swap_chain->logical, swap_chain->swap_chain, &swap_chain->images.count, swap_chain->images.images ) );

return( TRUE );

}   /* retrieve_images() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_allocation_callbacks
*
*********************************************************************/

static VKN_SWAP_CHAIN_CONFIG_API set_allocation_callbacks
    (
    VkAllocationCallbacks
                       *allocator,  /* custom allocator             */
    struct _VKN_swap_chain_build_type
                       *builder     /* swap chain builder           */
    )
{
builder->state.allocator = allocator;

return( builder->config );

}   /* set_allocation_callbacks() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_default_present_mode
*
*********************************************************************/

static VKN_SWAP_CHAIN_CONFIG_API set_default_present_mode
    (
    struct _VKN_swap_chain_build_type
                       *builder     /* swap chain builder           */
    )
{
add_present_mode_safe( VK_PRESENT_MODE_MAILBOX_KHR, &builder->state.present_modes );
add_present_mode_safe( VK_PRESENT_MODE_FIFO_KHR,    &builder->state.present_modes );

return( builder->config );

}   /* set_default_present_mode() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_default_surface_format
*
*********************************************************************/

static VKN_SWAP_CHAIN_CONFIG_API set_default_surface_format
    (
    struct _VKN_swap_chain_build_type
                       *builder     /* swap chain builder           */
    )
{
add_surface_format_safe( VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, &builder->state.formats );

return( builder->config );

}   /* set_default_surface_format() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_extent
*
*********************************************************************/

static VKN_SWAP_CHAIN_CONFIG_API set_extent
    (
    const u32           width,      /* surface width                */
    const u32           height,     /* surface height               */
    struct _VKN_swap_chain_build_type
                       *builder     /* swap chain builder           */
    )
{
builder->state.extent.width  = width;
builder->state.extent.height = height;

return( builder->config );

}   /* set_extent() */
