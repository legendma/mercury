#include "Global.hpp"

#include "VknCommon.hpp"
#include "VknPhysicalDeviceTypes.hpp"


bool VKN_physical_device_create
    (
    const VKN_physical_device_build_type
                       *builder,    /* physical device builder      */
    VKN_physical_device_type
                       *device      /* output new physical device   */
    );

VKN_PHYSICAL_DEVICE_CONFIG_API VKN_physical_device_init_builder
    (
    const VkInstance    instance,   /* associated vulkan instance   */
    const VkSurfaceKHR  surface,    /* VK_NULL_HANDLE if headless   */
    const u32           min_version,/* minimum version allowed      */
    VKN_physical_device_build_type
                       *builder     /* physical device builder      */
    );
