#pragma once

#include "Global.hpp"

#include "VknCommon.hpp"


#define VKN_LOGICAL_DEVICE_MAX_QUEUE_CNT \
                                    ( 4 )
#define VKN_LOGICAL_DEVICE_MAX_QUEUE_FAMILY_CNT \
                                    ( 10 )
#define VKN_LOGICAL_DEVICE_MAX_EXTENSIONS_CNT \
                                    ( 30 )

#define VKN_LOGICAL_DEVICE_CONFIG_API \
                                    const struct _VKN_logical_device_build_config_type *


typedef VKN_LOGICAL_DEVICE_CONFIG_API VKN_logical_device_build_require_compute_queue_proc_type
    (
    const bool          require_separate,
                                    /* need a separate queue?       */
    const bool          prefer_dedicated,
                                    /* would prefer dedicated?      */
    struct _VKN_logical_device_build_type
                       *builder     /* logical device builder       */
    );

typedef VKN_LOGICAL_DEVICE_CONFIG_API VKN_logical_device_build_require_graphics_queue_proc_type
    (
    const bool          require_separate,
                                    /* need a separate queue?       */
    const bool          prefer_dedicated,
                                    /* would prefer dedicated?      */
    struct _VKN_logical_device_build_type
                       *builder     /* logical device builder       */
    );

typedef VKN_LOGICAL_DEVICE_CONFIG_API VKN_logical_device_build_require_transfer_queue_proc_type
    (
    const bool          require_separate,
                                    /* need a separate queue?       */
    const bool          prefer_dedicated,
                                    /* would prefer dedicated?      */
    struct _VKN_logical_device_build_type
                       *builder     /* logical device builder       */
    );

typedef VKN_LOGICAL_DEVICE_CONFIG_API VKN_logical_device_build_set_allocation_callbacks_proc_type
    (
    VkAllocationCallbacks
                       *allocator,  /* custom allocator             */
    struct _VKN_logical_device_build_type
                       *builder     /* logical device builder       */
    );

typedef struct _VKN_logical_device_build_config_type
    {
    VKN_logical_device_build_require_compute_queue_proc_type
                       *require_compute_queue;
                                    /* want a compute queue?        */
    VKN_logical_device_build_require_graphics_queue_proc_type
                       *require_graphics_queue;
                                    /* want a graphics queue?       */
    VKN_logical_device_build_require_transfer_queue_proc_type
                       *require_transfer_queue;
                                    /* want a transfer queue?       */
    VKN_logical_device_build_set_allocation_callbacks_proc_type
                       *set_allocation_callbacks;
                                    /* set custom allocator         */
    } VKN_logical_device_build_config_type;

typedef struct
    {
    const char         *names[ VKN_LOGICAL_DEVICE_MAX_EXTENSIONS_CNT ];
                                    /* extension names              */
    u32                 count;      /* number of extensions         */
    } VKN_logical_device_build_extensions_type;

typedef struct
    {
    VkQueueFamilyProperties         /* device supported queue family*/
                        queue_families[ VKN_LOGICAL_DEVICE_MAX_QUEUE_FAMILY_CNT ];
    u32                 count;      /* number of queue families     */
    } VKN_logical_device_build_queue_families_type;

typedef struct
    {
    bool                prefer_dedicated;
                                    /* prefer dedicated queue?      */
    u32                 queue_index;/* index in family              */
    VkDeviceQueueCreateInfo
                       *create_info;/* create info record           */
    } VKN_logical_device_build_require_queue_type;

typedef struct
    {
    VkPhysicalDevice    physical;   /* associated physical device   */
    VkSurfaceKHR        surface;    /* associated surface           */
    const VkAllocationCallbacks
                       *allocator;  /* allocation callbacks         */
    VKN_logical_device_build_queue_families_type
                        queue_families;
                                    /* device supported queue family*/
    VKN_logical_device_build_extensions_type
                        extensions; /* required extensions          */
    VKN_features_type   features;   /* required features            */
    VKN_logical_device_build_require_queue_type
                        require_queues[ VKN_LOGICAL_DEVICE_MAX_QUEUE_CNT ];
                                    /* required queues              */
    VkDeviceQueueCreateInfo
                        ci_queues[ VKN_LOGICAL_DEVICE_MAX_QUEUE_CNT ];
                                    /* required queues create info  */
    u32                 ci_queue_cnt;
                                    /* queues create info count     */
    } VKN_logical_device_build_state_type;

typedef struct _VKN_logical_device_build_type
    {
    VKN_logical_device_build_state_type
                        state;      /* builder state                */
    const VKN_logical_device_build_config_type
                       *config;     /* configuration interface      */
    } VKN_logical_device_build_type;

typedef struct
    {
    VkQueue             queue;      /* queue handle                 */
    u32                 family;     /* family index                 */
    } VKN_logical_device_queue_type;

typedef struct
    {
    VkPhysicalDevice    physical;   /* associated physical device   */
    VkDevice            logical;    /* logical device               */
    VkSurfaceKHR        surface;    /* associated surface           */
    VKN_logical_device_queue_type
                        graphics;   /* graphics command queue       */
    VKN_logical_device_queue_type
                        compute;    /* compute command queue        */
    VKN_logical_device_queue_type
                        present;    /* present command queue        */
    VKN_logical_device_queue_type
                        transfer;   /* transfer command queue       */
    const VkAllocationCallbacks
                       *allocator;  /* allocation callbacks         */
    VKN_logical_device_build_queue_families_type
                        queue_families;
                                    /* device supported queue family*/
    } VKN_logical_device_type;
