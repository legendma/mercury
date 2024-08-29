#pragma once

#include "Global.hpp"

#include "VknBufferIndexTypes.hpp"
#include "VknCommon.hpp"
#include "VknMemoryTypes.hpp"
#include "VknReleaserTypes.hpp"


bool VKN_buffer_index_create
    (
    const u32           size,       /* size of buffer               */
    const VKN_buffer_index_build_type
                       *builder,    /* index buffer builder         */
    VKN_buffer_index_type
                       *buffer      /* output new index buffer      */
    );

void VKN_buffer_index_destroy
    (
    VKN_releaser_type  *releaser,   /* release buffer               */
    VKN_buffer_index_type
                       *buffer      /* buffer to destroy            */
    );

VKN_BUFFER_INDEX_CONFIG_API VKN_buffer_index_init_builder
    (
    const VkDevice      logical,    /* associated logical device    */
    const VkPhysicalDeviceProperties
                       *props,      /* device memory properties     */
    VKN_memory_type    *memory,     /* memory allocator             */
    VKN_buffer_index_build_type
                       *builder     /* index buffer builder         */
    );

