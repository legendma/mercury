#pragma once

#include "Global.hpp"

#include "VknCommon.hpp"


#define VKN_PHYSICAL_DEVICE_MAX_DEVICE_CNT \
                                    ( 10 )
#define VKN_PHYSICAL_DEVICE_MAX_QUEUE_FAMILY_CNT \
                                    ( 10 )
#define VKN_PHYSICAL_DEVICE_MAX_EXTENSIONS_CNT \
                                    ( 30 )
#define VKN_PHYSICAL_DEVICE_MAX_SUPPORTED_EXTENSIONS_CNT \
                                    ( 400 )
#define VKN_PHYSICAL_DEVICE_MAX_SURFACE_FORMATS_CNT \
                                    ( 100 )
#define VKN_PHYSICAL_DEVICE_MAX_PRESENT_MODES_CNT \
                                    ( 10 )

#define VKN_PHYSICAL_DEVICE_CONFIG_API \
                                    const struct _VKN_physical_device_build_config_type *

typedef enum
    {
    VKN_PHYSICAL_DEVICE_BUILD_DEVICE_CLASS_ANY,
                                    /* any device                   */
    VKN_PHYSICAL_DEVICE_BUILD_DEVICE_CLASS_HARDWARE,
                                    /* hardware render device       */
    VKN_PHYSICAL_DEVICE_BUILD_DEVICE_CLASS_SOFTWARE
                                    /* software render device       */
    } VKN_physical_device_build_device_class_type;


typedef VKN_PHYSICAL_DEVICE_CONFIG_API VKN_physical_device_build_add_extension_proc_type
    (
    const char         *name,       /* extension name               */
    struct _VKN_physical_device_build_type
                       *builder     /* physical device builder      */
    );

typedef VKN_PHYSICAL_DEVICE_CONFIG_API VKN_physical_device_build_set_min_memory_proc_type
    (
    const VkDeviceSize  size,       /* memory size                  */
    struct _VKN_physical_device_build_type
                       *builder     /* physical device builder      */
    );

typedef VKN_PHYSICAL_DEVICE_CONFIG_API VKN_physical_device_build_set_required_device_class_proc_type
    (
    const VKN_physical_device_build_device_class_type
                        cls,        /* device class                 */
    struct _VKN_physical_device_build_type
                       *builder     /* physical device builder      */
    );

typedef VKN_PHYSICAL_DEVICE_CONFIG_API VKN_physical_device_build_set_required_features_proc_type
    (
    const VKN_features_type
                       *needed,     /* required features            */
    struct _VKN_physical_device_build_type
                       *builder     /* physical device builder      */
    );

typedef struct _VKN_physical_device_build_config_type
    {
    VKN_physical_device_build_add_extension_proc_type
                       *add_extension;
                                    /* add requested extension      */
    VKN_physical_device_build_set_min_memory_proc_type
                       *set_min_memory;
                                    /* set required minimum memory  */
    VKN_physical_device_build_set_required_device_class_proc_type
                       *set_required_device_class;
                                    /* set required device class    */
    VKN_physical_device_build_set_required_features_proc_type
                       *set_required_features;
                                    /* set required features        */                       
    } VKN_physical_device_build_config_type;

typedef struct
    {
    VkExtensionProperties
                        extensions[ VKN_PHYSICAL_DEVICE_MAX_SUPPORTED_EXTENSIONS_CNT ];
                                    /* supported extensions         */
    u32                 count;      /* number of extensions         */
    } VKN_physical_device_supported_extensions_type;

typedef struct
    {
    const char         *names[ VKN_PHYSICAL_DEVICE_MAX_EXTENSIONS_CNT ];
                                    /* extension names              */
    u32                 count;      /* number of extensions         */
    } VKN_physical_device_extensions_type;

typedef struct
    {
    VkQueueFamilyProperties         /* device supported queue family*/
                        queue_families[ VKN_PHYSICAL_DEVICE_MAX_QUEUE_FAMILY_CNT ];
    u32                 count;      /* number of queue families     */
    } VKN_physical_device_queue_families_type;

typedef struct
    {
    VkSurfaceFormatKHR  formats[ VKN_PHYSICAL_DEVICE_MAX_SURFACE_FORMATS_CNT ];
                                    /* supported surface formats    */
    u32                 count;      /* number of surface formats    */
    } VKN_physical_device_surface_formats_type;

typedef struct
    {
    VkPresentModeKHR    modes[ VKN_PHYSICAL_DEVICE_MAX_PRESENT_MODES_CNT ];
                                    /* supported presentation modes */
    u32                 count;      /* number of presentation modes */
    } VKN_physical_device_present_modes_type;

typedef struct
    {
    VkPhysicalDevice    devices[ VKN_PHYSICAL_DEVICE_MAX_DEVICE_CNT ];
                                    /* device list                  */
    VKN_physical_device_queue_families_type
                        queue_families[ VKN_PHYSICAL_DEVICE_MAX_DEVICE_CNT ];
                                    /* device supported queue family*/
    VkPhysicalDeviceProperties      /* device properties            */
                        props[ VKN_PHYSICAL_DEVICE_MAX_DEVICE_CNT ];
    VkPhysicalDeviceMemoryProperties/* device memory properties     */
                        memory_props[ VKN_PHYSICAL_DEVICE_MAX_DEVICE_CNT ];
    VKN_physical_device_supported_extensions_type
                        extensions[ VKN_PHYSICAL_DEVICE_MAX_DEVICE_CNT ];
                                    /* device supported extensions  */
    VKN_features_type   features[ VKN_PHYSICAL_DEVICE_MAX_DEVICE_CNT ];
                                    /* device supported features    */
    VKN_physical_device_present_modes_type
                        present_modes[ VKN_PHYSICAL_DEVICE_MAX_DEVICE_CNT ];
                                    /* supported presentation modes */
    VKN_physical_device_surface_formats_type
                        surface_formats[ VKN_PHYSICAL_DEVICE_MAX_DEVICE_CNT ];
                                    /* supported surface formats    */
    u32                 count;      /* number of devices            */
    } VKN_physical_device_build_device_type;

typedef struct
    {
    VKN_physical_device_build_device_class_type
                        required_class;
                                    /* software/hardware/etc        */
    u32                 min_version;/* min Vulkan version allowed   */
    VkDeviceSize        min_memory; /* minimum graphics memory      */
    VkInstance          instance;   /* vulkan instance to use       */
    VkSurfaceKHR        surface;    /* surface requiring support    */
    VKN_physical_device_build_device_type
                        found_devices;
                                    /* physical devices found       */
                                    /* supported extensions         */
    VKN_physical_device_extensions_type
                        extensions; /* user required extensions     */
    VKN_features_type   features;   /* user required features       */
    } VKN_physical_device_build_state_type;

typedef struct _VKN_physical_device_build_type
    {
    VKN_physical_device_build_state_type
                        state;      /* builder state                */
    const VKN_physical_device_build_config_type
                       *config;     /* configuration interface      */
    } VKN_physical_device_build_type;

typedef struct
    {
    VkPhysicalDevice    physical_device;
                                    /* physical device handle       */
    VKN_physical_device_queue_families_type
                        queue_families;
                                    /* device supported queue family*/
    VkPhysicalDeviceProperties
                        props;      /* device properties            */
    VkPhysicalDeviceMemoryProperties/* device memory properties     */
                        memory_props;
    VKN_features_type   features;   /* device features to enable    */
    VKN_physical_device_extensions_type
                        extensions; /* required extensions          */
    } VKN_physical_device_type;
