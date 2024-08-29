#pragma once

#include <cstring>

#include "Global.hpp"
#include "Utilities.hpp"

#include "VknCommon.hpp"
#include "VknLogicalDevice.hpp"
#include "VknLogicalDeviceTypes.hpp"
#include "VknReleaser.hpp"


typedef enum
    {
    REQUIRE_QUEUE_COMPUTE,
    REQUIRE_QUEUE_GRAPHICS,
    REQUIRE_QUEUE_PRESENT,
    REQUIRE_QUEUE_TRANSFER,
    /* count */
    REQUIRE_QUEUE_CNT
    } require_queue_type;


typedef u32 find_queue_index_type
    (
    const VKN_logical_device_build_type
                       *builder     /* logical device builder       */
    );


/*********************************************************************
*
*   PROCEDURE NAME:
*       has_name
*
*********************************************************************/

static __inline bool has_name
    (
    const char         *check,      /* name to check                */
    const char        **names,      /* existing names               */
    const u32           name_cnt    /* number of existing names     */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     i;          /* loop counter                 */

for( i = 0; i < name_cnt; i++ )
    {
    if( !strcmp( check, names[ i ] ) )
        {
        return( TRUE );
        }
    }

return( FALSE );

}   /* has_name() */


static bool add_extension_safe
    (
    const char         *name,       /* extension name               */
    VKN_logical_device_build_extensions_type
                       *extensions  /* extension list               */
    );

static find_queue_index_type find_compute;
static find_queue_index_type find_dedicated_compute;
static find_queue_index_type find_dedicated_graphics;
static find_queue_index_type find_dedicated_transfer;
static find_queue_index_type find_graphics;
static find_queue_index_type find_present;
static find_queue_index_type find_transfer;

static void get_queue
    (
    const VkDevice     logical,     /* logical device               */
    const VKN_logical_device_build_require_queue_type
                       *requirement,/* queue requirement            */
    VKN_logical_device_queue_type
                       *queue       /* output queue                 */
    );

static VKN_logical_device_build_require_compute_queue_proc_type require_compute_queue;
static VKN_logical_device_build_require_graphics_queue_proc_type require_graphics_queue;

static void require_queue
    (
    const require_queue_type
                        name,       /* type of queue                */
    const bool          require_separate,
                                    /* need a separate queue?       */
    const bool          prefer_dedicated,
                                    /* would prefer dedicated?      */
    VKN_logical_device_build_type
                       *builder     /* logical device builder       */
    );

static VKN_logical_device_build_require_transfer_queue_proc_type require_transfer_queue;
static VKN_logical_device_build_set_allocation_callbacks_proc_type set_allocation_callbacks;


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_logical_device_create
*
*   DESCRIPTION:
*       Create a logical device via the given builder.
*
*********************************************************************/

bool VKN_logical_device_create
    (
    const VKN_logical_device_build_type
                       *builder,    /* logical device builder       */
    VKN_logical_device_type
                       *device      /* output new logical device    */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkDeviceCreateInfo      ci_device;  /* device create info           */

clr_struct( device );

/*----------------------------------------------------------
Create the device
----------------------------------------------------------*/
clr_struct( &ci_device );
ci_device.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
ci_device.pNext                   = builder->state.features.head;
ci_device.pQueueCreateInfos       = builder->state.ci_queues;
ci_device.queueCreateInfoCount    = builder->state.ci_queue_cnt;
ci_device.ppEnabledExtensionNames = builder->state.extensions.names;
ci_device.enabledExtensionCount   = builder->state.extensions.count;

if( VKN_failed( vkCreateDevice( builder->state.physical, &ci_device, builder->state.allocator, &device->logical ) ) )
    {
    return( FALSE );
    }

device->physical       = builder->state.physical;
device->surface        = builder->state.surface;
device->allocator      = builder->state.allocator;
device->queue_families = builder->state.queue_families;

/*----------------------------------------------------------
Retrieve the queues
----------------------------------------------------------*/
get_queue( device->logical, &builder->state.require_queues[ REQUIRE_QUEUE_COMPUTE  ], &device->compute  );
VKN_return_fail( VKN_name_object( device->logical, device->compute.queue, VK_OBJECT_TYPE_QUEUE, "compute_queue" ) );
get_queue( device->logical, &builder->state.require_queues[ REQUIRE_QUEUE_GRAPHICS ], &device->graphics );
VKN_return_fail( VKN_name_object( device->logical, device->compute.queue, VK_OBJECT_TYPE_QUEUE, "graphics_queue" ) );
get_queue( device->logical, &builder->state.require_queues[ REQUIRE_QUEUE_PRESENT  ], &device->present  );
VKN_return_fail( VKN_name_object( device->logical, device->compute.queue, VK_OBJECT_TYPE_QUEUE, "present_queue" ) );
get_queue( device->logical, &builder->state.require_queues[ REQUIRE_QUEUE_TRANSFER ], &device->transfer );
VKN_return_fail( VKN_name_object( device->logical, device->compute.queue, VK_OBJECT_TYPE_QUEUE, "transfer_queue" ) );

return( TRUE );

}   /* VKN_logical_device_create() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_logical_device_destroy
*
*   DESCRIPTION:
*       Destroy the given logical device.
*
*********************************************************************/

void VKN_logical_device_destroy
    (
    VKN_releaser_type  *releaser,   /* release buffer               */
    VKN_logical_device_type
                       *device      /* logical device to destroy    */
    )
{
VKN_releaser_auto_mini_begin( releaser, use );
use->i->release_device( device->allocator, device->logical, use );

VKN_releaser_auto_mini_end( use );
clr_struct( device );

}   /* VKN_logical_device_destroy() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_logical_device_init_builder
*
*   DESCRIPTION:
*       Initialize a logical device builder.
*
*********************************************************************/

VKN_LOGICAL_DEVICE_CONFIG_API VKN_logical_device_init_builder
    (
    const VkPhysicalDevice
                        physical,   /* associated physical device   */
    const VkSurfaceKHR  surface,    /* VK_NULL_HANDLE if headless   */
    const char * const *extensions, /* extension names              */
    const u32           extension_cnt,
                                    /* number of extension names    */
    const VKN_features_type
                       *features,   /* required features            */
    const VkQueueFamilyProperties   /* physical queue families      */
                       *queue_families,
    const u32           queue_family_cnt,
                                    /* number of queue families     */
    VKN_logical_device_build_type
                       *builder     /* logical device builder       */
    )
{
/*----------------------------------------------------------
Local constants
----------------------------------------------------------*/
static const VKN_logical_device_build_config_type CONFIG =
    {
    require_compute_queue,
    require_graphics_queue,
    require_transfer_queue,
    set_allocation_callbacks
    };

debug_assert( physical != VK_NULL_HANDLE
             && features );

/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     i;          /* loop counter                 */

clr_struct( builder );
builder->config = &CONFIG;

builder->state.physical = physical;
builder->state.surface  = surface;
builder->state.features = *features;

/*----------------------------------------------------------
Extensions
----------------------------------------------------------*/
for( i = 0; i < extension_cnt; i++ )
    {
    if( !add_extension_safe( extensions[ i ], &builder->state.extensions ) )
        {
        debug_assert_always();
        break;
        }
    }

if( surface != VK_NULL_HANDLE
 && !add_extension_safe( VK_KHR_SWAPCHAIN_EXTENSION_NAME, &builder->state.extensions ) )
    {
    debug_assert_always();
    }

/*----------------------------------------------------------
Queue families
----------------------------------------------------------*/
for( i = 0; i < queue_family_cnt; i++ )
    {
    if( i >= cnt_of_array( builder->state.queue_families.queue_families ) )
        {
        debug_assert_always();
        break;
        }

    builder->state.queue_families.queue_families[ builder->state.queue_families.count++ ] = queue_families[ i ];
    }

/*----------------------------------------------------------
If we have a surface, we need a present queue
----------------------------------------------------------*/
if( builder->state.surface != VK_NULL_HANDLE )
    {
    require_queue( REQUIRE_QUEUE_PRESENT, TRUE, FALSE, builder );
    }

return( builder->config );

}   /* VKN_logical_device_init_builder() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       add_extension_safe
*
*********************************************************************/

static bool add_extension_safe
    (
    const char         *name,       /* extension name               */
    VKN_logical_device_build_extensions_type
                       *extensions  /* extension list               */
    )
{
if( has_name( name, extensions->names, extensions->count ) )
    {
    return( TRUE );
    }
else if( extensions->count >= cnt_of_array( extensions->names ) )
    {
    debug_assert_always();
    return( FALSE );
    }

extensions->names[ extensions->count++ ] = name;
return( TRUE );

}   /* add_extension_safe() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       find_compute
*
*********************************************************************/

static u32 find_compute
    (
    const VKN_logical_device_build_type
                       *builder     /* logical device builder       */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
const VkQueueFamilyProperties
                       *family;     /* working family               */
u32                     i;          /* loop counter                 */
u32                     ret;        /* return family index          */

ret = VKN_INVALID_FAMILY_INDEX;
for( i = 0; ret == VKN_INVALID_FAMILY_INDEX && i < builder->state.queue_families.count; i++ )
    {
    family = &builder->state.queue_families.queue_families[ i ];
    if( test_bits( family->queueFlags, VK_QUEUE_COMPUTE_BIT ) )
        {
        ret = i;
        }
    }

return( ret );

}   /* find_compute() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       find_dedicated_compute
*
*********************************************************************/

static u32 find_dedicated_compute
    (
    const VKN_logical_device_build_type
                       *builder     /* logical device builder       */
    )
{
/*----------------------------------------------------------
Local literals
----------------------------------------------------------*/
#define DEDICATED_FAMILY_MASK       ( VK_QUEUE_COMPUTE_BIT  \
                                    | VK_QUEUE_GRAPHICS_BIT )

/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
const VkQueueFamilyProperties
                       *family;     /* working family               */
u32                     i;          /* loop counter                 */
u32                     ret;        /* return family index          */

ret = VKN_INVALID_FAMILY_INDEX;
for( i = 0; ret == VKN_INVALID_FAMILY_INDEX && i < builder->state.queue_families.count; i++ )
    {
    family = &builder->state.queue_families.queue_families[ i ];
    if( ( family->queueFlags & DEDICATED_FAMILY_MASK ) == VK_QUEUE_COMPUTE_BIT )
        {
        ret = i;
        }
    }

return( ret );

#undef DEDICATED_FAMILY_MASK
}   /* find_dedicated_compute() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       find_dedicated_graphics
*
*********************************************************************/

static u32 find_dedicated_graphics
    (
    const VKN_logical_device_build_type
                       *builder     /* logical device builder       */
    )
{
/*----------------------------------------------------------
Local literals
----------------------------------------------------------*/
#define DEDICATED_FAMILY_MASK       ( VK_QUEUE_COMPUTE_BIT  \
                                    | VK_QUEUE_GRAPHICS_BIT )

/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
const VkQueueFamilyProperties
                       *family;     /* working family               */
u32                     i;          /* loop counter                 */
u32                     ret;        /* return family index          */

ret = VKN_INVALID_FAMILY_INDEX;
for( i = 0; ret == VKN_INVALID_FAMILY_INDEX && i < builder->state.queue_families.count; i++ )
    {
    family = &builder->state.queue_families.queue_families[ i ];
    if( ( family->queueFlags & DEDICATED_FAMILY_MASK ) == VK_QUEUE_GRAPHICS_BIT )
        {
        ret = i;
        }
    }

return( ret );

#undef DEDICATED_FAMILY_MASK
}   /* find_dedicated_graphics() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       find_dedicated_transfer
*
*********************************************************************/

static u32 find_dedicated_transfer
    (
    const VKN_logical_device_build_type
                       *builder     /* logical device builder       */
    )
{
/*----------------------------------------------------------
Local literals
----------------------------------------------------------*/
#define DEDICATED_FAMILY_MASK       ( VK_QUEUE_COMPUTE_BIT  \
                                    | VK_QUEUE_GRAPHICS_BIT \
                                    | VK_QUEUE_TRANSFER_BIT )

/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
const VkQueueFamilyProperties
                       *family;     /* working family               */
u32                     i;          /* loop counter                 */
u32                     ret;        /* return family index          */

ret = VKN_INVALID_FAMILY_INDEX;
for( i = 0; ret == VKN_INVALID_FAMILY_INDEX && i < builder->state.queue_families.count; i++ )
    {
    family = &builder->state.queue_families.queue_families[ i ];
    if( ( family->queueFlags & DEDICATED_FAMILY_MASK ) == VK_QUEUE_TRANSFER_BIT )
        {
        ret = i;
        }
    }

return( ret );

#undef DEDICATED_FAMILY_MASK
}   /* find_dedicated_transfer() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       find_graphics
*
*********************************************************************/

static u32 find_graphics
    (
    const VKN_logical_device_build_type
                       *builder     /* logical device builder       */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
const VkQueueFamilyProperties
                       *family;     /* working family               */
u32                     i;          /* loop counter                 */
u32                     ret;        /* return family index          */

ret = VKN_INVALID_FAMILY_INDEX;
for( i = 0; ret == VKN_INVALID_FAMILY_INDEX && i < builder->state.queue_families.count; i++ )
    {
    family = &builder->state.queue_families.queue_families[ i ];
    if( test_bits( family->queueFlags, VK_QUEUE_GRAPHICS_BIT ) )
        {
        ret = i;
        }
    }

return( ret );

}   /* find_graphics() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       find_present
*
*********************************************************************/

static u32 find_present
    (
    const VKN_logical_device_build_type
                       *builder     /* logical device builder       */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkBool32                has_support;/* has support for queue?       */
u32                     i;          /* loop counter                 */
u32                     ret;        /* return family index          */

ret = VKN_INVALID_FAMILY_INDEX;
for( i = 0; ret == VKN_INVALID_FAMILY_INDEX && i < builder->state.queue_families.count; i++ )
    {
    if( VKN_failed( vkGetPhysicalDeviceSurfaceSupportKHR( builder->state.physical, i, builder->state.surface, &has_support ) )
     || has_support == VK_FALSE )
        {
        continue;
        }

    ret = i;
    }

return( ret );

}   /* find_present() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       find_transfer
*
*********************************************************************/

static u32 find_transfer
    (
    const VKN_logical_device_build_type
                       *builder     /* logical device builder       */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
const VkQueueFamilyProperties
                       *family;     /* working family               */
u32                     i;          /* loop counter                 */
u32                     ret;        /* return family index          */

ret = VKN_INVALID_FAMILY_INDEX;
for( i = 0; ret == VKN_INVALID_FAMILY_INDEX && i < builder->state.queue_families.count; i++ )
    {
    family = &builder->state.queue_families.queue_families[ i ];
    if( test_bits( family->queueFlags, VK_QUEUE_TRANSFER_BIT ) )
        {
        ret = i;
        }
    }

return( ret );

}   /* find_transfer() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       get_queue
*
*********************************************************************/

static void get_queue
    (
    const VkDevice     logical,     /* logical device               */
    const VKN_logical_device_build_require_queue_type
                       *requirement,/* queue requirement            */
    VKN_logical_device_queue_type
                       *queue       /* output queue                 */
    )
{
queue->family = VKN_INVALID_FAMILY_INDEX;
if( !requirement->create_info )
    {
    return;
    }

queue->family = requirement->create_info->queueFamilyIndex;
vkGetDeviceQueue( logical, queue->family, requirement->queue_index, &queue->queue );

}   /* get_queue() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       require_compute_queue
*
*********************************************************************/

static VKN_LOGICAL_DEVICE_CONFIG_API require_compute_queue
    (
    const bool          require_separate,
                                    /* need a separate queue?       */
    const bool          prefer_dedicated,
                                    /* would prefer dedicated?      */
    struct _VKN_logical_device_build_type
                       *builder     /* logical device builder       */
    )
{
require_queue( REQUIRE_QUEUE_COMPUTE, require_separate, prefer_dedicated, builder );

return( builder->config );

}   /* require_compute_queue() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       require_graphics_queue
*
*********************************************************************/

static VKN_LOGICAL_DEVICE_CONFIG_API require_graphics_queue
    (
    const bool          require_separate,
                                    /* need a separate queue?       */
    const bool          prefer_dedicated,
                                    /* would prefer dedicated?      */
    struct _VKN_logical_device_build_type
                       *builder     /* logical device builder       */
    )
{
require_queue( REQUIRE_QUEUE_GRAPHICS, require_separate, prefer_dedicated, builder );

return( builder->config );

}   /* require_graphics_queue() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       require_queue
*
*********************************************************************/

static void require_queue
    (
    const require_queue_type
                        name,       /* type of queue                */
    const bool          require_separate,
                                    /* need a separate queue?       */
    const bool          prefer_dedicated,
                                    /* would prefer dedicated?      */
    VKN_logical_device_build_type
                       *builder     /* logical device builder       */
    )
{
/*----------------------------------------------------------
Local constants
----------------------------------------------------------*/
static const float PRIORITY = 1.0f;

/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkDeviceQueueCreateInfo
                       *ci_queue;   /* working create info          */
find_queue_index_type  *find_dedicated;
                                    /* find dedicated               */
find_queue_index_type  *find;       /* find any                     */
u32                     family_index;
                                    /* queue family index           */
u32                     i;          /* loop counter                 */
VKN_logical_device_build_require_queue_type
                       *require;    /* requirement record           */

/*----------------------------------------------------------
Find the queue family index
----------------------------------------------------------*/
switch( name )
    {
    case REQUIRE_QUEUE_COMPUTE:
        find_dedicated = find_dedicated_compute;
        find           = find_compute;
        break;

    case REQUIRE_QUEUE_GRAPHICS:
        find_dedicated = find_dedicated_graphics;
        find           = find_graphics;
        break;

    case REQUIRE_QUEUE_PRESENT:
        find_dedicated = find_present;
        find           = find_present;
        break;

    default:
        /* REQUIRE_QUEUE_TRANSFER */
        find_dedicated = find_dedicated_transfer;
        find           = find_transfer;
        break;
    }

family_index = VKN_INVALID_FAMILY_INDEX;
if( prefer_dedicated )
    {
    family_index = find_dedicated( builder );
    }

if( family_index == VKN_INVALID_FAMILY_INDEX )
    {
    family_index = find( builder );
    }

/*----------------------------------------------------------
Fill out the create info
----------------------------------------------------------*/
require = &builder->state.require_queues[ name ];
if( require->create_info )
    {
    debug_assert( require->create_info->queueFamilyIndex == family_index );
    return;
    }
else
    {
    /*------------------------------------------------------
    Check if this family already exists
    ------------------------------------------------------*/
    for( i = 0; i < builder->state.ci_queue_cnt; i++ )
        {
        ci_queue = &builder->state.ci_queues[ i ];
        if( ci_queue->queueFamilyIndex != family_index )
            {
            continue;
            }

        /*--------------------------------------------------
        Merge with this one
        --------------------------------------------------*/
        require->create_info = ci_queue;
        require->queue_index = ci_queue->queueCount - 1;

        if( require_separate )
            {
            require->queue_index = ci_queue->queueCount;
            ci_queue->queueCount++;
            }

        return;
        }
    }

require->create_info = &builder->state.ci_queues[ builder->state.ci_queue_cnt++ ];

clr_struct( require->create_info );
require->create_info->sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
require->create_info->queueFamilyIndex = family_index;
require->create_info->queueCount       = 1;
require->create_info->pQueuePriorities = &PRIORITY;

}   /* require_queue() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       require_transfer_queue
*
*********************************************************************/

static VKN_LOGICAL_DEVICE_CONFIG_API require_transfer_queue
    (
    const bool          require_separate,
                                    /* need a separate queue?       */
    const bool          prefer_dedicated,
                                    /* would prefer dedicated?      */
    struct _VKN_logical_device_build_type
                       *builder     /* logical device builder       */
    )
{
require_queue( REQUIRE_QUEUE_TRANSFER, require_separate, prefer_dedicated, builder );

return( builder->config );

}   /* require_transfer_queue() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_allocation_callbacks
*
*********************************************************************/

static VKN_LOGICAL_DEVICE_CONFIG_API set_allocation_callbacks
    (
    VkAllocationCallbacks
                       *allocator,  /* custom allocator             */
    struct _VKN_logical_device_build_type
                       *builder     /* logical device builder       */
    )
{
builder->state.allocator = allocator;

return( builder->config );

}   /* set_allocation_callbacks() */
