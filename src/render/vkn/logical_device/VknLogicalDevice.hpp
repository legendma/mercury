#pragma once

#include "Global.hpp"

#include "VknLogicalDeviceTypes.hpp"
#include "VknReleaserTypes.hpp"


bool VKN_logical_device_create
    (
    const VKN_logical_device_build_type
                       *builder,    /* logical device builder       */
    VKN_logical_device_type
                       *device      /* output new logical device    */
    );

void VKN_logical_device_destroy
    (
    VKN_releaser_type  *releaser,   /* release buffer               */
    VKN_logical_device_type
                       *device      /* logical device to destroy    */
    );

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
    );
