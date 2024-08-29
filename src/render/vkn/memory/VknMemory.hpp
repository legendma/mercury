#pragma once

#include "Global.hpp"

#include "VknMemoryTypes.hpp"
#include "VknReleaserTypes.hpp"


bool VKN_memory_create
    (
    const VKN_memory_build_type
                       *builder,    /* memory allocator builder     */
    VKN_memory_type    *allocator   /* output new memory allocator  */
    );

void VKN_memory_destroy
    (
    VKN_releaser_type  *releaser,   /* release buffer               */
    VKN_memory_type    *allocator   /* output new memory allocator  */
    );

VKN_MEMORY_CONFIG_API VKN_memory_init_builder
    (
    const VkInstance    instance,   /* associated Vulkan instance   */
    const VkPhysicalDevice
                        physical,   /* associated physical device   */
    const VkDevice      logical,    /* logical device               */
    VKN_memory_build_type
                       *builder     /* memory allocator builder     */
    );
