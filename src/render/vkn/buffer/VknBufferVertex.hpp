#pragma once

#include "Global.hpp"

#include "VknBufferVertexTypes.hpp"
#include "VknCommon.hpp"
#include "VknMemoryTypes.hpp"
#include "VknReleaserTypes.hpp"


bool VKN_buffer_vertex_create
    (
    const u32           size,       /* size of buffer               */
    const VKN_buffer_vertex_build_type
                       *builder,    /* vertex buffer builder        */
    VKN_buffer_vertex_type
                       *buffer      /* output new vertex buffer     */
    );

void VKN_buffer_vertex_destroy
    (
    VKN_releaser_type  *releaser,   /* release buffer               */
    VKN_buffer_vertex_type
                       *buffer      /* buffer to destroy            */
    );

VKN_BUFFER_VERTEX_CONFIG_API VKN_buffer_vertex_init_builder
    (
    const VkDevice      logical,    /* associated logical device    */
    const VkPhysicalDeviceProperties
                       *props,      /* device memory properties     */
    VKN_memory_type    *memory,     /* memory allocator             */
    VKN_buffer_vertex_build_type
                       *builder     /* vertex buffer builder        */
    );
