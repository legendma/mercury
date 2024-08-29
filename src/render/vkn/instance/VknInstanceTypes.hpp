#pragma once

#include "Global.hpp"

#include "VknCommon.hpp"


#define VKN_INSTANCE_MAX_LAYERS_CNT \
                                    ( 10 )
#define VKN_INSTANCE_MAX_EXTENSIONS_CNT \
                                    ( 10 )

#define VKN_INSTANCE_CONFIG_API     const struct _VKN_instance_build_config_type *


typedef VKN_INSTANCE_CONFIG_API VKN_instance_build_add_extension_proc_type
    (
    const char         *name,       /* extension name               */
    struct _VKN_instance_build_type
                       *builder     /* instance builder             */
    );

typedef VKN_INSTANCE_CONFIG_API VKN_instance_build_add_layer_proc_type
    (
    const char         *name,       /* layer name                   */
    struct _VKN_instance_build_type
                       *builder     /* instance builder             */
    );

typedef VKN_INSTANCE_CONFIG_API VKN_instance_build_enable_validation_layers_proc_type
    (
    const bool          is_enable,  /* enable validation layers?    */
    struct _VKN_instance_build_type
                       *builder     /* instance builder             */
    );

typedef VKN_INSTANCE_CONFIG_API VKN_instance_build_set_allocation_callbacks_proc_type
    (
    VkAllocationCallbacks
                       *allocator,  /* custom allocator             */
    struct _VKN_instance_build_type
                       *builder     /* instance builder             */
    );

typedef VKN_INSTANCE_CONFIG_API VKN_instance_build_set_app_name_proc_type
    (
    const char         *app_name,   /* application name             */
    struct _VKN_instance_build_type
                       *builder     /* instance builder             */
    );

typedef VKN_INSTANCE_CONFIG_API VKN_instance_build_set_debug_message_kinds_proc_type
    (
    const VkDebugUtilsMessageTypeFlagsEXT
                        kinds,      /* message kinds to print       */
    struct _VKN_instance_build_type
                       *builder     /* instance builder             */
    );

typedef VKN_INSTANCE_CONFIG_API VKN_instance_build_set_debug_message_severities_proc_type
    (
    const VkDebugUtilsMessageSeverityFlagBitsEXT
                        severities, /* message severities to print  */
    struct _VKN_instance_build_type
                       *builder     /* instance builder             */
    );

typedef VKN_INSTANCE_CONFIG_API VKN_instance_build_set_engine_name_proc_type
    (
    const char         *engine_name,/* engine name                  */
    struct _VKN_instance_build_type
                       *builder     /* instance builder             */
    );

typedef VKN_INSTANCE_CONFIG_API VKN_instance_build_set_minimum_version_proc_type
    (
    const u32           major,      /* major version number         */
    const u32           minor,      /* minor version number         */
    const u32           patch,      /* patch version number         */
    struct _VKN_instance_build_type
                       *builder     /* instance builder             */
    );

typedef VKN_INSTANCE_CONFIG_API VKN_instance_build_use_debug_messenger_proc_type
    (
    struct _VKN_instance_build_type
                       *builder     /* instance builder             */
    );

typedef struct _VKN_instance_build_config_type
    {
    VKN_instance_build_add_extension_proc_type
                       *add_extension;
                                    /* add requested extension      */
    VKN_instance_build_add_layer_proc_type
                       *add_layer;  /* add requested layer          */
    VKN_instance_build_enable_validation_layers_proc_type
                       *enable_validation_layers;
                                    /* enable validation layers?    */
    VKN_instance_build_set_allocation_callbacks_proc_type
                       *set_allocation_callbacks;
                                    /* set custom allocator         */
    VKN_instance_build_set_app_name_proc_type
                       *set_app_name;
                                    /* set the application name     */
    VKN_instance_build_set_debug_message_kinds_proc_type
                       *set_debug_message_kinds;
                                    /* set kinds to debug print     */
    VKN_instance_build_set_debug_message_severities_proc_type
                       *set_debug_message_severities;
                                    /* set severities to debug print*/
    VKN_instance_build_set_engine_name_proc_type
                       *set_engine_name;
                                    /* set the engine name          */
    VKN_instance_build_set_minimum_version_proc_type
                       *set_minimum_version;
                                    /* set the min vulkan version   */
    VKN_instance_build_use_debug_messenger_proc_type
                       *use_debug_messenger;
                                    /* enable debug messenger       */
    } VKN_instance_build_config_type;

typedef struct
    {
    const char         *names[ VKN_INSTANCE_MAX_EXTENSIONS_CNT ];
                                    /* extension names              */
    u32                 count;      /* number of extensions         */
    } VKN_instance_build_extensions_type;

typedef struct
    {
    const char         *names[ VKN_INSTANCE_MAX_LAYERS_CNT ];
                                    /* layer names                  */
    u32                 count;      /* number of layers             */
    } VKN_instance_build_layers_type;

typedef struct
    {
    bool                use_validation_layers : 1;
                                    /* should use validation layers?*/
    bool                use_debug_messenger : 1;
                                    /* should use debug messenger?  */
    u32                 min_version;/* minimum version required     */
    VkDebugUtilsMessageSeverityFlagBitsEXT
                        debug_message_severities;
                                    /* severity indication bits     */
    VkDebugUtilsMessageTypeFlagsEXT /* message type indication bits */
                        debug_message_kinds;
    const VkAllocationCallbacks
                       *allocator;  /* allocation callbacks         */
    const char         *app_name;   /* application name             */
    const char         *engine_name;/* engine name                  */
    VKN_instance_build_layers_type
                        layers;     /* requested layers             */
    VKN_instance_build_extensions_type
                        extensions; /* requested extensions         */
    } VKN_instance_build_state_type;

typedef struct _VKN_instance_build_type
    {
    VKN_instance_build_state_type
                        state;      /* builder state                */
    const VKN_instance_build_config_type
                       *config;     /* configuration interface      */
    } VKN_instance_build_type;

typedef struct
    {
    VkInstance          instance;   /* vulkan instance              */
    const VkAllocationCallbacks
                       *allocator;  /* allocation callbacks         */
    VkDebugUtilsMessengerEXT
                        messenger;  /* debug messenger              */
    } VKN_instance_type;
