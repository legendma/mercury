#include <climits>
#include <cstdio>
#include <cstring>

#include "Global.hpp"
#include "Utilities.hpp"

#include "VknCommon.hpp"
#include "VknExtension.hpp"
#include "VknInstance.hpp"
#include "VknInstanceTypes.hpp"
#include "VknReleaser.hpp"


#if defined( _WIN32 )
static const char *PLATFORM_SURFACE_EXTENSION_NAMES[] =
    {
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME
    };
#endif


/*********************************************************************
*
*   PROCEDURE NAME:
*       string_copy
*
*********************************************************************/

static __inline int string_copy
    (
    const char         *src,        /* source string                */
    const u32           sz,         /* sizeof( dst ) includes NULL  */
    char               *dst         /* destination string           */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     end_idx;    /* end index                    */
u32                     i;          /* loop counter                 */

if( sz <= 1 )
    {
    return( -1 );
    }

end_idx = ( sz - 1 );
for( i = 0; i < end_idx && src[ i ]; i++ )
    {
    dst[ i ] = src[ i ];
    }

dst[ i ]       = 0;
dst[ end_idx ] = 0;

if( src[ i ] )
    {
    return( -1 );
    }

return( i + 1 );

}   /* string_copy() */


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
*       get_api_supported_version
*
*********************************************************************/

static __inline u32 get_api_supported_version
    (
    void
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkResult                result;     /* request result               */
u32                     ret;        /* return version               */


result = vkEnumerateInstanceVersion( &ret );
if( result != VK_SUCCESS )
    {
    ret = VK_API_VERSION_1_0;
    }

return( ret );

}   /* get_api_supported_version() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       make_version
*
*********************************************************************/

static __inline u32 make_version
    (
    const u32           major,      /* major version number         */
    const u32           minor,      /* minor version number         */
    const u32           patch       /* patch version number         */
    )
{
/*----------------------------------------------------------
Local literals
----------------------------------------------------------*/
#define MAJOR_MASK      ( 0x007f )
#define MINOR_MASK      ( 0x03ff )
#define PATCH_MASK      ( 0x1fff )

return( VK_MAKE_API_VERSION( 0, major & MAJOR_MASK, minor & MINOR_MASK, patch & PATCH_MASK ) );

#undef MAJOR_MASK
#undef MINOR_MASK
#undef PATCH_MASK
}   /* make_version() */


static VKN_instance_build_add_extension_proc_type add_extension;

static bool add_extension_safe
    (
    const char         *name,       /* extension name               */
    VKN_instance_build_extensions_type
                       *extensions  /* extension list               */
    );

static VKN_instance_build_add_layer_proc_type add_layer;

static bool add_layer_safe
    (
    const char         *name,       /* layer name                   */
    VKN_instance_build_layers_type
                       *layers      /* layers list                  */
    );

static void append_string_w_delimiter
    (
    const char         *append,     /* name to check                */
    const char         *delimiter,  /* existing names               */
    const u32           out_len,    /* length of output array       */
    char               *out         /* output string                */
    );

static VkBool32 debug_message_callback
    (
    VkDebugUtilsMessageSeverityFlagBitsEXT
                        severities, /* severity indication bits     */
    VkDebugUtilsMessageTypeFlagsEXT
                        kinds,      /* message type indication bits */
    const VkDebugUtilsMessengerCallbackDataEXT
                       *message,    /* message data                 */
    void               *user_data   /* (optional) user data         */
    );

static VKN_instance_build_enable_validation_layers_proc_type enable_validation_layers;

static VkResult get_application_info
    (
    const VKN_instance_build_type
                       *builder,    /* instance builder             */
    VkApplicationInfo  *info        /* application info             */
    );

static void get_debug_messenger_create_info
    (
    const VKN_instance_build_type
                       *builder,    /* instance builder             */
    VkDebugUtilsMessengerCreateInfoEXT
                       *ci_message  /* debug messenger info         */
    );

static void get_instance_create_info
    (
    const VKN_instance_build_extensions_type
                       *extensions, /* extension list               */
    const VKN_instance_build_layers_type
                       *layers,     /* layer list                   */
    const VkApplicationInfo
                       *app_info,   /* application info             */
    VkInstanceCreateInfo
                       *info        /* instance creation info       */
    );

static bool is_extensions_supported
    (
    const VKN_instance_build_extensions_type
                       *check       /* extensions needed            */
    );

static bool is_layers_supported
    (
    const VKN_instance_build_layers_type
                       *check       /* layers needed                */
    );

static VKN_instance_build_set_allocation_callbacks_proc_type set_allocation_callbacks;
static VKN_instance_build_set_app_name_proc_type set_app_name;
static VKN_instance_build_set_debug_message_kinds_proc_type set_debug_message_kinds;
static VKN_instance_build_set_debug_message_severities_proc_type set_debug_message_severities;
static VKN_instance_build_set_engine_name_proc_type set_engine_name;
static VKN_instance_build_set_minimum_version_proc_type set_minimum_version;

static int string_cat
    (
    const char         *src,        /* source string                */
    u32                 sz,         /* sizeof( dst ) includes NULL  */
    char               *dst         /* destination string           */
    );

static VKN_instance_build_use_debug_messenger_proc_type use_debug_messenger;


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_instance_create
*
*   DESCRIPTION:
*       Create a Vulkan instance via the given builder.
*
*********************************************************************/

bool VKN_instance_create
    (
    const VKN_instance_build_type
                       *builder,    /* instance builder             */
    VKN_instance_type  *instance    /* output new instance          */
    )
{
/*----------------------------------------------------------
Local literals
----------------------------------------------------------*/
#define VALIDATION_LAYER_NAME       "VK_LAYER_KHRONOS_validation"

/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkApplicationInfo       app_info;   /* application info             */
VkInstanceCreateInfo    ci_instance;/* instance create info         */
VkDebugUtilsMessengerCreateInfoEXT
                        ci_message; /* debug messenger info         */
VKN_instance_build_extensions_type
                        extensions; /* extension list               */
VKN_instance_build_layers_type
                        layers;     /* layer list                   */
VkBaseOutStructure     *tail;       /* linked list tail             */

clr_struct( instance );
instance->allocator = builder->state.allocator;
clr_struct( &ci_instance );
tail = (VkBaseOutStructure*)&ci_instance;

/*----------------------------------------------------------
Application info
----------------------------------------------------------*/
if( VKN_failed( get_application_info( builder, &app_info ) ) )
    {
    VKN_instance_destroy( NULL, instance );
    return( FALSE );
    }

/*----------------------------------------------------------
Extensions
----------------------------------------------------------*/
extensions = builder->state.extensions;

if( builder->state.use_debug_messenger )
    {
    add_extension_safe( VK_EXT_DEBUG_UTILS_EXTENSION_NAME, &extensions );
    }

if( !is_extensions_supported( &extensions ) )
    {
    VKN_instance_destroy( NULL, instance );
    return( FALSE );
    }

/*----------------------------------------------------------
Layers
----------------------------------------------------------*/
layers = builder->state.layers;

if( builder->state.use_validation_layers )
    {
    add_layer_safe( VALIDATION_LAYER_NAME, &layers );
    }

if( !is_layers_supported( &layers ) )
    {
    VKN_instance_destroy( NULL, instance );
    return( FALSE );
    }

/*----------------------------------------------------------
Debug messenger for instance create/destroy
----------------------------------------------------------*/
if( builder->state.use_debug_messenger )
    {
    get_debug_messenger_create_info( builder, &ci_message );

    tail->pNext = (VkBaseOutStructure*)&ci_message;
    tail = (VkBaseOutStructure*)&ci_message;
    }

/*----------------------------------------------------------
Create instance
----------------------------------------------------------*/
get_instance_create_info( &extensions, &layers, &app_info, &ci_instance );
if( VKN_failed( vkCreateInstance( &ci_instance, builder->state.allocator, &instance->instance ) ) )
    {
    VKN_instance_destroy( NULL, instance );
    return( FALSE );
    }

/*----------------------------------------------------------
Bring up the extensions
----------------------------------------------------------*/
VKN_extensions_init( instance->instance );

/*----------------------------------------------------------
Attach to the debug messenger
----------------------------------------------------------*/
if( builder->state.use_debug_messenger )
    {
    if( VKN_failed( vkCreateDebugUtilsMessengerEXT( instance->instance, &ci_message, instance->allocator, &instance->messenger ) ) )
        {
        debug_assert_always();
        VKN_instance_destroy( NULL, instance );
        return(FALSE);
        }
    }

return( TRUE );

#undef VALIDATION_LAYER_NAME
}   /* VKN_instance_create() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_instance_destroy
*
*   DESCRIPTION:
*       Destroy the given Vulkan instance.
*
*********************************************************************/

void VKN_instance_destroy
    (
    VKN_releaser_type  *releaser,   /* release buffer               */
    VKN_instance_type  *instance    /* output new instance          */
    )
{
VKN_releaser_auto_mini_begin( releaser, use );
use->i->release_instance( instance->allocator, instance->instance, use )->
        release_debug_messenger( instance->instance, instance->allocator, instance->messenger, use );

VKN_releaser_auto_mini_end( use );
clr_struct( instance );

}   /* VKN_instance_destroy() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_instance_init_builder
*
*   DESCRIPTION:
*       Initialize a Vulkan instance builder.
*
*********************************************************************/

VKN_INSTANCE_CONFIG_API VKN_instance_init_builder
    (
    const bool          is_headless,/* headless display instance?   */
    VKN_instance_build_type
                       *builder     /* instance builder             */
    )
{
/*----------------------------------------------------------
Local literals
----------------------------------------------------------*/
#define DEFAULT_DEBUG_MESSAGE_SEVERTIES               \
    ( VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT \
    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT )

#define DEFAULT_DEBUG_MESSAGE_KINDS                  \
    ( VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT    \
    | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT \
    | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT )

/*----------------------------------------------------------
Local constants
----------------------------------------------------------*/
static const VKN_instance_build_config_type CONFIG =
    {
    add_extension,
    add_layer,
    enable_validation_layers,
    set_allocation_callbacks,
    set_app_name,
    set_debug_message_kinds,
    set_debug_message_severities,
    set_engine_name,
    set_minimum_version,
    use_debug_messenger
    };

/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     i;          /* loop counter                 */

clr_struct( builder );
builder->config = &CONFIG;

builder->state.min_version              = VKN_MIN_VERSION;
builder->state.app_name                 = "Vulkan Application";
builder->state.engine_name              = "Unknown Engine";
builder->state.debug_message_severities = (VkDebugUtilsMessageSeverityFlagBitsEXT)DEFAULT_DEBUG_MESSAGE_SEVERTIES;
builder->state.debug_message_kinds      = DEFAULT_DEBUG_MESSAGE_KINDS;

/* required extensions */
if( !is_headless )
    {
    /*------------------------------------------------------
    Surface extensions required for monitor display
    ------------------------------------------------------*/
    add_extension_safe( VK_KHR_SURFACE_EXTENSION_NAME,   &builder->state.extensions );
    for( i = 0; i < cnt_of_array( PLATFORM_SURFACE_EXTENSION_NAMES ); i++ )
        {
        add_extension_safe( PLATFORM_SURFACE_EXTENSION_NAMES[ i ], &builder->state.extensions );
        }
    }

return( builder->config );

#undef DEFAULT_DEBUG_MESSAGE_SEVERTIES
#undef DEFAULT_DEBUG_MESSAGE_KINDS
}   /* VKN_instance_init_builder() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       add_extension
*
*********************************************************************/

static VKN_INSTANCE_CONFIG_API add_extension
    (
    const char         *name,       /* extension name               */
    struct _VKN_instance_build_type
                       *builder     /* instance builder             */
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
    VKN_instance_build_extensions_type
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
*       add_layer
*
*********************************************************************/

static VKN_INSTANCE_CONFIG_API add_layer
    (
    const char         *name,       /* layer name                   */
    struct _VKN_instance_build_type
                       *builder     /* instance builder             */
    )
{
add_layer_safe( name, &builder->state.layers );

return( builder->config );

}   /* add_layer() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       add_layer_safe
*
*********************************************************************/

static bool add_layer_safe
    (
    const char         *name,       /* layer name                   */
    VKN_instance_build_layers_type
                       *layers      /* layers list                  */
    )
{
if( has_name( name, layers->names, layers->count ) )
    {
    return( TRUE );
    }
else if( layers->count >= cnt_of_array( layers->names ) )
    {
    debug_assert_always();
    return( FALSE );
    }

layers->names[ layers->count++ ] = name;

return( TRUE );

}   /* add_layer_safe() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       append_string_w_delimiter
*
*********************************************************************/

static void append_string_w_delimiter
    (
    const char         *append,     /* name to check                */
    const char         *delimiter,  /* existing names               */
    const u32           out_len,    /* length of output array       */
    char               *out         /* output string                */
    )
{
if( *out )
    {
    string_cat( delimiter, out_len, out );
    }

string_cat( append, out_len, out );

}   /* append_string_w_delimiter() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       debug_message_callback
*
*********************************************************************/

static VkBool32 debug_message_callback
    (
    VkDebugUtilsMessageSeverityFlagBitsEXT
                        severities, /* severity indication bits     */
    VkDebugUtilsMessageTypeFlagsEXT
                        kinds,      /* message type indication bits */
    const VkDebugUtilsMessengerCallbackDataEXT
                       *message,    /* message data                 */
    void               *user_data   /* (optional) user data         */
    )
{
/*----------------------------------------------------------
Local macros
----------------------------------------------------------*/
#define scan_begin( _var ) \
    shift_bits( 1, sizeof( _var ) * CHAR_BIT - 1 )

/*----------------------------------------------------------
Local literals
----------------------------------------------------------*/
#define MAX_MESSAGE_STR_LEN         ( 120 )
#define SEVERE_DELIMITER            "/"
#define KIND_DELIMITER              " | "

/*----------------------------------------------------------
Local variable
----------------------------------------------------------*/
char                    kind_str[ MAX_MESSAGE_STR_LEN ];
                                    /* type string                  */
u32                     scan;       /* scan bit                     */
char                    severe_str[ MAX_MESSAGE_STR_LEN ];
                                    /* severity string              */

/*----------------------------------------------------------
Message severity
----------------------------------------------------------*/
clr_array( severe_str );
for( scan = scan_begin( scan ); scan; scan >>= 1 )
    {
    switch( severities & scan )
        {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            append_string_w_delimiter( "VERBOSE", SEVERE_DELIMITER, cnt_of_array( severe_str ), severe_str );
            break;

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            append_string_w_delimiter( "INFO", SEVERE_DELIMITER, cnt_of_array( severe_str ), severe_str );
            break;

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            append_string_w_delimiter( "WARNING", SEVERE_DELIMITER, cnt_of_array( severe_str ), severe_str );
            break;

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            append_string_w_delimiter( "ERROR", SEVERE_DELIMITER, cnt_of_array( severe_str ), severe_str );
            break;

        default:
            continue;
        }
    }

/*----------------------------------------------------------
Message type
----------------------------------------------------------*/
clr_array( kind_str );
for( scan = scan_begin( scan ); scan; scan >>= 1 )
    {
    switch( kinds & scan )
        {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
            append_string_w_delimiter( "General", KIND_DELIMITER, cnt_of_array( kind_str ), kind_str );
            break;

        case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
            append_string_w_delimiter( "Validation", KIND_DELIMITER, cnt_of_array( kind_str ), kind_str );
            break;

        case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
            append_string_w_delimiter( "Performance", KIND_DELIMITER, cnt_of_array( kind_str ), kind_str );
            break;

        default:
            continue;
        }
    }

/*----------------------------------------------------------
Print to console
----------------------------------------------------------*/
printf( "[%s, %s]: %s\n\n", severe_str, kind_str, message->pMessage );

return( VK_FALSE );

#undef scan_begin
#undef MAX_MESSAGE_STR_LEN
#undef SEVERE_DELIMITER
#undef KIND_DELIMITER
}   /* debug_message_callback() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       enable_validation_layers
*
*********************************************************************/

static VKN_INSTANCE_CONFIG_API enable_validation_layers
    (
    const bool          is_enable,  /* enable validation layers?    */
    struct _VKN_instance_build_type
                       *builder     /* instance builder             */
    )
{
builder->state.use_validation_layers = is_enable;

return( builder->config );

}   /* enable_validation_layers() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       get_debug_messenger_create_info
*
*********************************************************************/

static void get_debug_messenger_create_info
    (
    const VKN_instance_build_type
                       *builder,    /* instance builder             */
    VkDebugUtilsMessengerCreateInfoEXT
                       *ci_message  /* debug messenger info         */
    )
{
clr_struct( ci_message );

ci_message->sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
ci_message->messageSeverity = builder->state.debug_message_severities;
ci_message->messageType     = builder->state.debug_message_kinds;
ci_message->pfnUserCallback = debug_message_callback;

}   /* get_debug_messenger_create_info() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       get_application_info
*
*********************************************************************/

static VkResult get_application_info
    (
    const VKN_instance_build_type
                       *builder,    /* instance builder             */
    VkApplicationInfo  *info        /* application info             */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     api_version;/* version of vulkan            */
u32                     supported_api_version;
                                    /* version supported by driver  */

/*----------------------------------------------------------
Determine the api version to use
----------------------------------------------------------*/
api_version = VKN_MIN_VERSION;
if( builder->state.min_version > api_version )
    {
    supported_api_version = get_api_supported_version();
    if( builder->state.min_version < supported_api_version )
        {
        return( VK_ERROR_INCOMPATIBLE_DRIVER );
        }

    api_version = builder->state.min_version;
    }

/*----------------------------------------------------------
Fill out the information
----------------------------------------------------------*/
clr_struct( info );
info->sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
info->pApplicationName   = builder->state.app_name;
info->applicationVersion = make_version( 1, 0, 0 );
info->pEngineName        = builder->state.engine_name;
info->engineVersion      = make_version( 1, 0, 0 );
info->apiVersion         = api_version;

return( VK_SUCCESS );

}   /* get_application_info() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       get_instance_create_info
*
*********************************************************************/

static void get_instance_create_info
    (
    const VKN_instance_build_extensions_type
                       *extensions, /* extension list               */
    const VKN_instance_build_layers_type
                       *layers,     /* layer list                   */
    const VkApplicationInfo
                       *app_info,   /* application info             */
    VkInstanceCreateInfo
                       *info        /* instance creation info       */
    )
{
/*----------------------------------------------------------
Don't clear the info structure, as it contains the pNext
links
----------------------------------------------------------*/
info->sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
info->flags                   = 0;
info->pApplicationInfo        = app_info;
info->enabledLayerCount       = layers->count;
info->ppEnabledLayerNames     = layers->names;
info->enabledExtensionCount   = extensions->count;
info->ppEnabledExtensionNames = extensions->names;

}   /* get_instance_create_info() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       is_extensions_supported
*
*********************************************************************/

static bool is_extensions_supported
    (
    const VKN_instance_build_extensions_type
                       *check       /* extensions needed            */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     count;      /* supported extensions count   */
bool                    found;      /* did we find the extension?   */
u32                     i;          /* loop counter                 */
u32                     j;          /* loop counter                 */
VkExtensionProperties   supported[ 2 * cnt_of_array( check->names ) ];
                                    /* supported extensions         */

if( VKN_failed( vkEnumerateInstanceExtensionProperties( NULL, &count, NULL ) ) )
    {
    return( FALSE );
    }

if( count > cnt_of_array( supported ) )
    {
    debug_assert_always();
    count = cnt_of_array( supported );
    }

if( VKN_failed( vkEnumerateInstanceExtensionProperties( NULL, &count, supported ) ) )
    {
    return( FALSE );
    }

for( i = 0; i < check->count; i++ )
    {
    found = FALSE;
    for( j = 0; !found && j < count; j++ )
        {
        found = !strcmp( check->names[ i ], supported[ j ].extensionName );
        }

    if( !found )
        {
        /*--------------------------------------------------
        Couldn't find a required extension
        --------------------------------------------------*/
        return( FALSE );
        }
    }

return( TRUE );

}   /* is_extensions_supported() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       is_layers_supported
*
*********************************************************************/

static bool is_layers_supported
    (
    const VKN_instance_build_layers_type
                       *check       /* layers needed                */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     count;      /* supported layers count       */
bool                    found;      /* did we find the layer?       */
u32                     i;          /* loop counter                 */
u32                     j;          /* loop counter                 */
VkLayerProperties       supported[ 2 * cnt_of_array( check->names ) ];
                                    /* supported layers             */

vkEnumerateInstanceLayerProperties( &count, NULL );
if( count > cnt_of_array( supported ) )
    {
    debug_assert_always();
    count = cnt_of_array( supported );
    }

vkEnumerateInstanceLayerProperties( &count, supported );
for( i = 0; i < check->count; i++ )
    {
    found = FALSE;
    for( j = 0; !found && j < count; j++ )
        {
        found = !strcmp( check->names[ i ], supported[ j ].layerName );
        }

    if( !found )
        {
        /*--------------------------------------------------
        Couldn't find a required layer
        --------------------------------------------------*/
        return( FALSE );
        }
    }

return( TRUE );

}   /* is_layers_supported() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_allocation_callbacks
*
*********************************************************************/

static VKN_INSTANCE_CONFIG_API set_allocation_callbacks
    (
    VkAllocationCallbacks
                       *allocator,  /* custom allocator             */
    struct _VKN_instance_build_type
                       *builder     /* instance builder             */
    )
{
builder->state.allocator = allocator;

return( builder->config );

}   /* set_allocation_callbacks() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_app_name
*
*********************************************************************/

static VKN_INSTANCE_CONFIG_API set_app_name
    (
    const char         *app_name,   /* application name             */
    struct _VKN_instance_build_type
                       *builder     /* instance builder             */
    )
{
builder->state.app_name = app_name;

return( builder->config );

}   /* set_app_name() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_debug_message_kinds
*
*********************************************************************/

static VKN_INSTANCE_CONFIG_API set_debug_message_kinds
    (
    const VkDebugUtilsMessageTypeFlagsEXT
                        kinds,      /* message kinds to print       */
    struct _VKN_instance_build_type
                       *builder     /* instance builder             */
    )
{
builder->state.debug_message_kinds = kinds;

return( builder->config );

}   /* set_debug_message_kinds() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_debug_message_severities
*
*********************************************************************/

static VKN_INSTANCE_CONFIG_API set_debug_message_severities
    (
    const VkDebugUtilsMessageSeverityFlagBitsEXT
                        severities, /* message severities to print  */
    struct _VKN_instance_build_type
                       *builder     /* instance builder             */
    )
{
builder->state.debug_message_severities = severities;

return( builder->config );

}   /* set_debug_message_severities() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_engine_name
*
*********************************************************************/

static VKN_INSTANCE_CONFIG_API set_engine_name
    (
    const char         *engine_name,/* engine name                  */
    struct _VKN_instance_build_type
                       *builder     /* instance builder             */
    )
{
builder->state.engine_name = engine_name;

return( builder->config );

}   /* set_engine_name() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_minimum_version
*
*********************************************************************/

static VKN_INSTANCE_CONFIG_API set_minimum_version
    (
    const u32           major,      /* major version number         */
    const u32           minor,      /* minor version number         */
    const u32           patch,      /* patch version number         */
    struct _VKN_instance_build_type
                       *builder     /* instance builder             */
    )
{
builder->state.min_version = make_version( major, minor, patch );

return( builder->config );

}   /* set_minimum_version() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       string_cat
*
*********************************************************************/

static int string_cat
    (
    const char         *src,        /* source string                */
    u32                 sz,         /* sizeof( dst ) includes NULL  */
    char               *dst         /* destination string           */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     len;        /* length of dst                */

/*----------------------------------------------------------
Initialize
----------------------------------------------------------*/
len  = (u32)strlen( dst );

if( len >= sz )
    {
    return( -1 );
    }

return( string_copy( src, sz - len, &dst[ len ] ) );

}   /* string_cat() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       use_debug_messenger
*
*********************************************************************/

static VKN_INSTANCE_CONFIG_API use_debug_messenger
    (
    struct _VKN_instance_build_type
                       *builder     /* instance builder             */
    )
{
builder->state.use_debug_messenger = TRUE;

return( builder->config );

}   /* use_debug_messenger() */
