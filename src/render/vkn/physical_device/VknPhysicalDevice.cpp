#include <cstring>

#include "Global.hpp"
#include "Utilities.hpp"

#include "VknCommon.hpp"
#include "VknPhysicalDevice.hpp"
#include "VknPhysicalDeviceTypes.hpp"


#define FEATURES_1_0_BOOL_CNT       ( 55 )
#define FEATURES_1_1_BOOL_CNT       ( 12 )
#define FEATURES_1_2_BOOL_CNT       ( 47 )
#define FEATURES_1_3_BOOL_CNT       ( 15 )
#define FEATURES_EXTENDED_DYNAMIC_STATE_BOOL_CNT \
                                    ( 1 )

#define FEATURES_1_0_FIRST_BOOL     robustBufferAccess
#define FEATURES_1_1_FIRST_BOOL     storageBuffer16BitAccess
#define FEATURES_1_2_FIRST_BOOL     samplerMirrorClampToEdge
#define FEATURES_1_3_FIRST_BOOL     robustImageAccess
#define FEATURES_EXTENDED_DYNAMIC_STATE_FIRST_BOOL \
                                    extendedDynamicState

typedef struct
    {
    u32                 present_cnt;/* # presentation queues found  */
    u32                 graphics_cnt;
                                    /* # graphics queues found      */
    u32                 transfer_cnt;
                                    /* # transfer queues found      */
    u32                 transfer_no_graphics_cnt;
                                    /* # non-graphics transfer found*/
    u32                 transfer_dedicated_cnt;
                                    /* # dedicated transfer found   */
    u32                 compute_cnt;/* # compute queues found       */
    u32                 compute_no_graphics_cnt;
                                    /* # non-graphics computes found*/
    u32                 compute_dedicated_cnt;
                                    /* # dedicated computes found   */
    } queue_stats_type;


/*********************************************************************
*
*   PROCEDURE NAME:
*       bool32_array_has_support
*
*********************************************************************/

static __inline bool bool32_array_has_support
    (
    const VkBool32     *supported,  /* array of supported values    */
    const VkBool32     *required,   /* array of required values     */
    const u32           length      /* array length                 */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     i;          /* loop counter                 */

for( i = 0; i < length; i++ )
    {
    if( required[ i ] != VK_FALSE
     && supported[ i ] == VK_FALSE )
        {
        return( FALSE );
        }
    }

return( TRUE );

}   /* bool32_array_has_support() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       bool32_array_is_empty
*
*********************************************************************/

static __inline bool bool32_array_is_empty
    (
    const VkBool32     *arr,        /* array to check               */
    const u32           length      /* array length                 */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     i;          /* loop counter                 */

for( i = 0; i < length; i++ )
    {
    if( arr[ i ] != VK_FALSE )
        {
        return( FALSE );
        }
    }

return( TRUE );

}   /* bool32_array_is_empty() */


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


/*********************************************************************
*
*   PROCEDURE NAME:
*       link_features
*
*********************************************************************/

static __inline void link_features
    (
    VKN_features_type  *features    /* features to link             */
    )
{
/*----------------------------------------------------------
Local types
----------------------------------------------------------*/
typedef struct
    {
    VkBaseOutStructure *base;
    VkBool32           *start;
    u32                 count;
    } element_type;

/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
element_type            elements[ 5 ];
                                    /* element access               */
u32                     i;          /* loop counter                 */
VkBaseOutStructure     *tail;       /* to link to                   */

features->v1_0.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
elements[ 0 ].base  = (VkBaseOutStructure*)&features->v1_0;
elements[ 0 ].start = &features->v1_0.features.FEATURES_1_0_FIRST_BOOL;
elements[ 0 ].count = FEATURES_1_0_BOOL_CNT;

features->v1_1.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
elements[ 1 ].base  = (VkBaseOutStructure*)&features->v1_1;
elements[ 1 ].start = &features->v1_1.FEATURES_1_1_FIRST_BOOL;
elements[ 1 ].count = FEATURES_1_1_BOOL_CNT;

features->v1_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
elements[ 2 ].base  = (VkBaseOutStructure*)&features->v1_2;
elements[ 2 ].start = &features->v1_2.FEATURES_1_2_FIRST_BOOL;
elements[ 2 ].count = FEATURES_1_2_BOOL_CNT;

features->v1_3.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
elements[ 3 ].base  = (VkBaseOutStructure*)&features->v1_3;
elements[ 3 ].start = &features->v1_3.FEATURES_1_3_FIRST_BOOL;
elements[ 3 ].count = FEATURES_1_3_BOOL_CNT;

features->extended_dynamic_state.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
elements[ 4 ].base  = (VkBaseOutStructure*)&features->extended_dynamic_state;
elements[ 4 ].start = &features->extended_dynamic_state.FEATURES_EXTENDED_DYNAMIC_STATE_FIRST_BOOL;
elements[ 4 ].count = FEATURES_EXTENDED_DYNAMIC_STATE_BOOL_CNT;

tail = NULL;
features->head = NULL;
for( i = 0; i < cnt_of_array( elements ); i++ )
    {
    elements[ i ].base->pNext = NULL;
    if( bool32_array_is_empty( elements[ i ].start, elements[ i ].count ) )
        {
        continue;
        }

    if( !features->head )
        {
        features->head = elements[ i ].base;
        }

    if( tail )
        {
        tail->pNext = elements[ i ].base;
        }

    tail = elements[ i ].base;
    }

}   /* link_features() */
    

static VKN_physical_device_build_add_extension_proc_type add_extension;

static bool add_extension_safe
    (
    const char         *name,       /* extension name               */
    VKN_physical_device_extensions_type
                       *extensions  /* extension list               */
    );

static bool discover_present_devices
    (
    VKN_physical_device_build_type
                       *builder     /* physical device builder      */
    );

static u32 get_device_fitness
    (
    const u32           device_index,
                                    /* index of device in cache     */
    const VKN_physical_device_build_type
                       *builder     /* physical device builder      */
    );

static void inspect_queue_engines
    (
    const VkPhysicalDevice
                        device,     /* associated physical device   */
    const VkSurfaceKHR  surface,    /* surface to support           */
    const VKN_physical_device_queue_families_type
                       *families,   /* queue families to inspect    */
    queue_stats_type   *out         /* output report                */
    );

static bool is_extensions_supported
    (
    const VKN_physical_device_supported_extensions_type
                       *supported,  /* device supported extensions  */
    const VKN_physical_device_extensions_type
                       *check       /* required extensions          */
    );

static bool is_features_supported
    (
    const VKN_features_type
                       *supported,  /* device supported features    */
    const VKN_features_type
                       *required    /* required features            */
    );

static VKN_physical_device_build_set_min_memory_proc_type set_min_memory;
static VKN_physical_device_build_set_required_device_class_proc_type set_required_device_class;
static VKN_physical_device_build_set_required_features_proc_type set_required_features;


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_physical_device_create
*
*   DESCRIPTION:
*       Create a physical device via the given builder.
*
*********************************************************************/

bool VKN_physical_device_create
    (
    const VKN_physical_device_build_type
                       *builder,    /* physical device builder      */
    VKN_physical_device_type
                       *device      /* output new physical device   */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     best_device;/* index of best device         */
u32                     best_score; /* score of best device         */
u32                     i;          /* loop counter                 */
u32                     score;      /* score of working device      */

/*----------------------------------------------------------
Local literals
----------------------------------------------------------*/
#define INVALID_DEVICE              max_uint_value( best_device )

clr_struct( device );
if( !builder->state.found_devices.count )
    {
    return( FALSE );
    }

/*----------------------------------------------------------
Find the best device for our configuration
----------------------------------------------------------*/
best_device = INVALID_DEVICE;
best_score  = 0;
for( i = 0; i < builder->state.found_devices.count; i++ )
    {
    score = get_device_fitness( i, builder );
    if( score > best_score )
        {
        best_device = i;
        best_score  = score;
        }
    }

if( best_device == INVALID_DEVICE )
    {
    /*------------------------------------------------------
    Couldn't find a suitable device
    ------------------------------------------------------*/
    return( FALSE );
    }

/*----------------------------------------------------------
Fill out the physical device
----------------------------------------------------------*/
device->physical_device = builder->state.found_devices.devices[ best_device ];
device->queue_families  = builder->state.found_devices.queue_families[ best_device ];
device->props           = builder->state.found_devices.props[ best_device ];
device->memory_props    = builder->state.found_devices.memory_props[ best_device ];
device->features        = builder->state.found_devices.features[ best_device ];
device->extensions      = builder->state.extensions;

link_features( &device->features );

return( TRUE );

#undef INVALID_DEVICE
}   /* VKN_physical_device_create() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_physical_device_init_builder
*
*   DESCRIPTION:
*       Initialize a physical device builder.
*
*********************************************************************/

VKN_PHYSICAL_DEVICE_CONFIG_API VKN_physical_device_init_builder
    (
    const VkInstance    instance,   /* associated vulkan instance   */
    const VkSurfaceKHR  surface,    /* VK_NULL_HANDLE if headless   */
    const u32           min_version,/* minimum version allowed      */
    VKN_physical_device_build_type
                       *builder     /* physical device builder      */
    )
{
/*----------------------------------------------------------
Local constants
----------------------------------------------------------*/
static const VKN_physical_device_build_config_type CONFIG =
    {
    add_extension,
    set_min_memory,
    set_required_device_class,
    set_required_features
    };

debug_assert( instance != VK_NULL_HANDLE
             && min_version >= VKN_MIN_VERSION );

/*----------------------------------------------------------
Initialize
----------------------------------------------------------*/
clr_struct( builder );
builder->config = &CONFIG;

builder->state.instance       = instance;
builder->state.surface        = surface;
builder->state.min_version    = min_version;
builder->state.required_class = VKN_PHYSICAL_DEVICE_BUILD_DEVICE_CLASS_ANY;

if( !discover_present_devices( builder ) )
    {
    debug_assert_always();
    }

return( builder->config );

}   /* VKN_physical_device_init_builder() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       add_extension
*
*********************************************************************/

static VKN_PHYSICAL_DEVICE_CONFIG_API add_extension
    (
    const char         *name,       /* extension name               */
    struct _VKN_physical_device_build_type
                       *builder     /* physical device builder      */
    )
{
add_extension_safe( name, &builder->state.extensions );

return( builder->config );

}   /* add_extension() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       add_extension_safe
*
*********************************************************************/

static bool add_extension_safe
    (
    const char         *name,       /* extension name               */
    VKN_physical_device_extensions_type
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
*       discover_present_devices
*
*********************************************************************/

static bool discover_present_devices
    (
    VKN_physical_device_build_type
                       *builder     /* physical device builder      */
    )
{
/*----------------------------------------------------------
Local literals
----------------------------------------------------------*/
#define NO_LAYER_NAME   ( NULL )

/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkPhysicalDevice        device;     /* working physical device      */
VKN_physical_device_build_device_type
                       *devices;    /* handy device list pointer    */
u32                     i;          /* loop counter                 */
VKN_physical_device_present_modes_type
                       *present_modes;
                                    /* handy present modes pointer  */
VKN_physical_device_queue_families_type
                       *queue_families;
                                    /* handy queue families pointer */
VkSurfaceKHR            surface;    /* surface requiring support    */
VKN_physical_device_surface_formats_type
                       *surface_formats;
                                    /* handy surface formats pointer*/

devices = &builder->state.found_devices;
surface = builder->state.surface;

/*----------------------------------------------------------
Query for Vulkan devices on this system
----------------------------------------------------------*/
VKN_return_fail( vkEnumeratePhysicalDevices( builder->state.instance, &devices->count, NULL ) );
if( devices->count > cnt_of_array( devices->devices ) )
    {
    debug_assert_always();
    devices->count = cnt_of_array( devices->devices );
    }

VKN_return_fail( vkEnumeratePhysicalDevices( builder->state.instance, &devices->count, devices->devices ) );

/*----------------------------------------------------------
Query the device properties
----------------------------------------------------------*/
for( i = 0; i < devices->count; i++ )
    {
    device = devices->devices[ i ];

    /*------------------------------------------------------
    Queue families
    ------------------------------------------------------*/
    queue_families = &devices->queue_families[ i ];
    vkGetPhysicalDeviceQueueFamilyProperties( device, &queue_families->count, NULL );
    if( queue_families->count > cnt_of_array( queue_families->queue_families ) )
        {
        debug_assert_always();
        queue_families->count = cnt_of_array( queue_families->queue_families );
        }

    vkGetPhysicalDeviceQueueFamilyProperties( device, &queue_families->count, queue_families->queue_families );

    /*------------------------------------------------------
    Device properties
    ------------------------------------------------------*/
    vkGetPhysicalDeviceProperties( device, &devices->props[ i ] );

    /*------------------------------------------------------
    Memory properties
    ------------------------------------------------------*/
    vkGetPhysicalDeviceMemoryProperties( device, &devices->memory_props[ i ] );

    /*------------------------------------------------------
    Features
    ------------------------------------------------------*/
    devices->features[ i ].head = (VkBaseOutStructure*)&devices->features[ i ].v1_0;

    devices->features[ i ].v1_0.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    devices->features[ i ].v1_0.pNext = &devices->features[ i ].v1_1;

    devices->features[ i ].v1_1.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    devices->features[ i ].v1_1.pNext = &devices->features[ i ].v1_2;

    devices->features[ i ].v1_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    devices->features[ i ].v1_2.pNext = &devices->features[ i ].v1_3;

    devices->features[ i ].v1_3.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    devices->features[ i ].v1_3.pNext = &devices->features[ i ].extended_dynamic_state;

    devices->features[ i ].extended_dynamic_state.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;

    vkGetPhysicalDeviceFeatures2( device, &devices->features[ i ].v1_0 );

    /*------------------------------------------------------
    Extensions
    ------------------------------------------------------*/    
    VKN_return_fail( vkEnumerateDeviceExtensionProperties( device, NO_LAYER_NAME, &devices->extensions[ i ].count, NULL ) );
    if( devices->extensions[ i ].count > cnt_of_array( devices->extensions[ i ].extensions ) )
        {
        debug_assert_always();
        devices->extensions[ i ].count = cnt_of_array( devices->extensions[ i ].extensions );
        }

    VKN_return_fail( vkEnumerateDeviceExtensionProperties( device, NO_LAYER_NAME, &devices->extensions[ i ].count, devices->extensions[ i ].extensions ) );

    /*------------------------------------------------------
    Non-headless
    ------------------------------------------------------*/
    if( surface != VK_NULL_HANDLE )
        {
        /*--------------------------------------------------
        Surface formats
        --------------------------------------------------*/
        surface_formats = &devices->surface_formats[ i ];
        VKN_return_fail( vkGetPhysicalDeviceSurfaceFormatsKHR( device, surface, &surface_formats->count, NULL ) );
        if( surface_formats->count > cnt_of_array( surface_formats->formats ) )
            {
            debug_assert_always();
            surface_formats->count = cnt_of_array( surface_formats->formats );
            }

        VKN_return_fail( vkGetPhysicalDeviceSurfaceFormatsKHR( device, surface, &surface_formats->count, surface_formats->formats ) );

        /*--------------------------------------------------
        Presentation Modes
        --------------------------------------------------*/
        present_modes = &devices->present_modes[ i ];
        VKN_return_fail( vkGetPhysicalDeviceSurfacePresentModesKHR( device, surface, &present_modes->count, NULL ) );
        if( present_modes->count > cnt_of_array( present_modes->modes ) )
            {
            debug_assert_always();
            present_modes->count = cnt_of_array( present_modes->modes );
            }

        VKN_return_fail( vkGetPhysicalDeviceSurfacePresentModesKHR( device, surface, &present_modes->count, present_modes->modes ) );
        }
    }

return( devices->count > 0 );

#undef NO_LAYER_NAME
}   /* discover_present_devices() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       get_device_fitness
*
*********************************************************************/

static u32 get_device_fitness
    (
    const u32           device_index,
                                    /* index of device in cache     */
    const VKN_physical_device_build_type
                       *builder     /* physical device builder      */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkPhysicalDevice        device;     /* device being scored          */
const VKN_physical_device_supported_extensions_type
                       *extensions; /* device extensions            */
const VKN_features_type
                       *features;   /* device features              */
bool                    has_memory; /* device has enough memory?    */
const VkMemoryHeap     *heap;       /* handy memory heap pointer    */
u32                     i;          /* loop counter                 */
const VkPhysicalDeviceMemoryProperties
                       *memory_props;
                                    /* device memory properties     */
queue_stats_type        queue_stats;/* queue statistics             */
const VKN_physical_device_present_modes_type
                       *present_modes;
                                    /* supported presentation modes */
const VkPhysicalDeviceProperties
                       *props;      /* device properties            */
const VKN_physical_device_queue_families_type
                       *queue_families;
                                    /* handy queue families pointer */
u32                     ret;        /* return fitness score         */
const VKN_physical_device_surface_formats_type
                       *surface_formats;
                                    /* supported surface formats    */

ret = 1;

device          = builder->state.found_devices.devices[ device_index ];
present_modes   = &builder->state.found_devices.present_modes[ device_index ];
surface_formats = &builder->state.found_devices.surface_formats[ device_index ];
props           = &builder->state.found_devices.props[ device_index ];
extensions      = &builder->state.found_devices.extensions[ device_index ];
features        = &builder->state.found_devices.features[ device_index ];
memory_props    = &builder->state.found_devices.memory_props[ device_index ];
queue_families  = &builder->state.found_devices.queue_families[ device_index ];

/*----------------------------------------------------------
Inspect the queues
----------------------------------------------------------*/
inspect_queue_engines( device, builder->state.surface, queue_families, &queue_stats );

/*----------------------------------------------------------
Version
----------------------------------------------------------*/
if( props->apiVersion < builder->state.min_version )
    {
    return( 0 );
    }

/*----------------------------------------------------------
Device class
----------------------------------------------------------*/
switch( builder->state.required_class )
    {
    case VKN_PHYSICAL_DEVICE_BUILD_DEVICE_CLASS_HARDWARE:
        if( props->deviceType != VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU
         && props->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU )
            {
            return( 0 );
            }
        break;

    case VKN_PHYSICAL_DEVICE_BUILD_DEVICE_CLASS_SOFTWARE:
        if( props->deviceType != VK_PHYSICAL_DEVICE_TYPE_CPU )
            {
            return( 0 );
            }
        break;

    default:
        debug_assert( builder->state.required_class == VKN_PHYSICAL_DEVICE_BUILD_DEVICE_CLASS_ANY );
        break;
    }

/*----------------------------------------------------------
Memory
----------------------------------------------------------*/
has_memory = FALSE;
for( i = 0; !has_memory && i < memory_props->memoryHeapCount; i++ )
    {
    heap = &memory_props->memoryHeaps[ i ];
    if( !test_bits( heap->flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ) )
        {
        /*--------------------------------------------------
        Only care about dedicated memory
        --------------------------------------------------*/
        continue;
        }

    has_memory = heap->size >= builder->state.min_memory;
    }

if( !has_memory )
    {
    return( 0 );
    }

/*----------------------------------------------------------
Extensions
----------------------------------------------------------*/
if( !is_extensions_supported( extensions, &builder->state.extensions ) )
    {
    return( 0 );
    }

/*----------------------------------------------------------
Features
----------------------------------------------------------*/
if( !is_features_supported( features, &builder->state.features ) )
    {
    return( 0 );
    }

/*----------------------------------------------------------
Swapchain support
----------------------------------------------------------*/
if( builder->state.surface != VK_NULL_HANDLE
 && ( !present_modes->count
   || !surface_formats->count
   || !queue_stats.present_cnt ) )
    {
    return( 0 );
    }

/*----------------------------------------------------------
Queue engines
----------------------------------------------------------*/
ret += queue_stats.graphics_cnt;
ret += queue_stats.compute_no_graphics_cnt;
ret += queue_stats.transfer_dedicated_cnt;

return( ret );

}   /* get_device_fitness() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       inspect_queue_engines
*
*********************************************************************/

static void inspect_queue_engines
    (
    const VkPhysicalDevice
                        device,     /* associated physical device   */
    const VkSurfaceKHR  surface,    /* surface to support           */
    const VKN_physical_device_queue_families_type
                       *families,   /* queue families to inspect    */
    queue_stats_type   *out         /* output report                */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
const VkQueueFamilyProperties
                       *family;     /* working family               */
VkBool32                has_present;/* does engine have present?    */
u32                     i;          /* loop counter                 */

clr_struct( out );
for( i = 0; i < families->count; i++ )
    {
    family = &families->queue_families[ i ];

    /*------------------------------------------------------
    Graphics
    ------------------------------------------------------*/
    if( test_bits( family->queueFlags, VK_QUEUE_GRAPHICS_BIT ) )
        {
        out->graphics_cnt += family->queueCount;
        }

    /*------------------------------------------------------
    Transfer
    ------------------------------------------------------*/
    if( test_bits( family->queueFlags, VK_QUEUE_TRANSFER_BIT ) )
        {
        out->transfer_cnt += family->queueCount;
        if( !test_bits( family->queueFlags, VK_QUEUE_GRAPHICS_BIT ) )
            {
            out->transfer_no_graphics_cnt += family->queueCount;
            if( !test_bits( family->queueFlags, VK_QUEUE_COMPUTE_BIT ) )
                {
                out->transfer_dedicated_cnt += family->queueCount;
                }
            }
        }

    /*------------------------------------------------------
    Compute
    ------------------------------------------------------*/
    if( test_bits( family->queueFlags, VK_QUEUE_COMPUTE_BIT ) )
        {
        out->compute_cnt += family->queueCount;
        if( !test_bits( family->queueFlags, VK_QUEUE_GRAPHICS_BIT ) )
            {
            out->compute_no_graphics_cnt += family->queueCount;
            if( !test_bits( family->queueFlags, VK_QUEUE_TRANSFER_BIT ) )
                {
                out->compute_dedicated_cnt += family->queueCount;
                }
            }
        }

    /*------------------------------------------------------
    Presentation
    ------------------------------------------------------*/
    has_present = VK_FALSE;
    if( !VKN_failed( vkGetPhysicalDeviceSurfaceSupportKHR( device, i, surface, &has_present ) ) )
        {
        if( has_present == VK_TRUE )
            {
            out->present_cnt += family->queueCount;
            }
        }
    }

}   /* inspect_queue_engines() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       is_extensions_supported
*
*********************************************************************/

static bool is_extensions_supported
    (
    const VKN_physical_device_supported_extensions_type
                       *supported,  /* device supported extensions  */
    const VKN_physical_device_extensions_type
                       *check       /* required extensions          */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
bool                    found;      /* did we find the extension?   */
u32                     i;          /* loop counter                 */
u32                     j;          /* loop counter                 */

/*----------------------------------------------------------
Skip the query if nothing required
----------------------------------------------------------*/
if( !check->count )
    {
    return( TRUE );
    }

/*----------------------------------------------------------
Check each required extension against those supported
----------------------------------------------------------*/
for( i = 0; i < check->count; i++ )
    {
    found = FALSE;
    for( j = 0; !found && j < supported->count; j++ )
        {
        found = !strcmp( check->names[ i ], supported->extensions[ j ].extensionName );
        }

    if( !found )
        {
        /*--------------------------------------------------
        Couldn't find this one
        --------------------------------------------------*/
        return( FALSE );
        }
    }

return( TRUE );

}   /* is_extensions_supported() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       is_features_supported
*
*********************************************************************/

static bool is_features_supported
    (
    const VKN_features_type
                       *supported,  /* device supported features    */
    const VKN_features_type
                       *required    /* required features            */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkBool32               *arr_required;
                                    /* required compare start       */
VkBool32               *arr_supported;
                                    /* supported compare start      */
u32                     count;      /* compare count                */
VkBaseOutStructure     *r;          /* current required             */
VkBaseOutStructure     *s;          /* current supported            */

for( r = required->head; r; r = r->pNext )
    {
    for( s = supported->head; s; s = s->pNext )
        {
        if( s->sType != r->sType )
            {
            continue;
            }

        switch( s->sType )
            {
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2:
                arr_required  = &( (VkPhysicalDeviceFeatures2*)r )->features.FEATURES_1_0_FIRST_BOOL;
                arr_supported = &( (VkPhysicalDeviceFeatures2*)s )->features.FEATURES_1_0_FIRST_BOOL;
                count         = FEATURES_1_0_BOOL_CNT;
                break;

            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES:
                arr_required  = &( (VkPhysicalDeviceVulkan11Features*)r )->FEATURES_1_1_FIRST_BOOL;
                arr_supported = &( (VkPhysicalDeviceVulkan11Features*)s )->FEATURES_1_1_FIRST_BOOL;
                count         = FEATURES_1_1_BOOL_CNT;
                break;

            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES:
                arr_required  = &( (VkPhysicalDeviceVulkan12Features*)r )->FEATURES_1_2_FIRST_BOOL;
                arr_supported = &( (VkPhysicalDeviceVulkan12Features*)s )->FEATURES_1_2_FIRST_BOOL;
                count         = FEATURES_1_2_BOOL_CNT;
                break;

            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES:
                arr_required  = &( (VkPhysicalDeviceVulkan13Features*)r )->FEATURES_1_3_FIRST_BOOL;
                arr_supported = &( (VkPhysicalDeviceVulkan13Features*)s )->FEATURES_1_3_FIRST_BOOL;
                count         = FEATURES_1_3_BOOL_CNT;
                break;

            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT:
                arr_required  = &( (VkPhysicalDeviceExtendedDynamicStateFeaturesEXT*)r )->FEATURES_EXTENDED_DYNAMIC_STATE_FIRST_BOOL;
                arr_supported = &( (VkPhysicalDeviceExtendedDynamicStateFeaturesEXT*)s )->FEATURES_EXTENDED_DYNAMIC_STATE_FIRST_BOOL;
                count         = FEATURES_EXTENDED_DYNAMIC_STATE_BOOL_CNT;
                break;

            default:
                debug_assert_always();
                return( FALSE );
            }

        if( !bool32_array_has_support( arr_supported, arr_required, count ) )
            {
            return( FALSE );
            }
        }
    }
    
return( TRUE );

}   /* is_features_supported() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_min_memory
*
*********************************************************************/

static VKN_PHYSICAL_DEVICE_CONFIG_API set_min_memory
    (
    const VkDeviceSize  size,       /* memory size                  */
    struct _VKN_physical_device_build_type
                       *builder     /* physical device builder      */
    )
{
builder->state.min_memory = size;

return( builder->config );

}   /* set_min_memory() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_required_device_class
*
*********************************************************************/

static VKN_PHYSICAL_DEVICE_CONFIG_API set_required_device_class
    (
    const VKN_physical_device_build_device_class_type
                        cls,        /* device class                 */
    struct _VKN_physical_device_build_type
                       *builder     /* physical device builder      */
    )
{
builder->state.required_class = cls;

return( builder->config );

}   /* set_required_device_class() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_required_features
*
*********************************************************************/

static VKN_PHYSICAL_DEVICE_CONFIG_API set_required_features
    (
    const VKN_features_type
                       *needed,     /* required features            */
    struct _VKN_physical_device_build_type
                       *builder     /* physical device builder      */
    )
{
builder->state.features = *needed;
link_features( &builder->state.features );
    
return( builder->config );

}   /* set_required_features() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_surface
*
*********************************************************************/

static VKN_PHYSICAL_DEVICE_CONFIG_API set_surface
    (
    const VkSurfaceKHR  surface,    /* surface requiring support    */
    struct _VKN_physical_device_build_type
                       *builder     /* physical device builder      */
    )
{
builder->state.surface = surface;

return( builder->config );

}   /* set_surface() */
