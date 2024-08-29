#pragma once

#include "VknArenaTypes.hpp"
#include "VknBufferVertexDynamicTypes.hpp"
#include "VknCommon.hpp"
#include "VknMemoryTypes.hpp"
#include "VknReleaserTypes.hpp"


void VKN_buffer_vertex_dynamic_create
    (
    const VKN_buffer_vertex_dynamic_build_type
                       *builder,    /* dynamic vertex buffer builder*/
    VKN_arena_type     *arena,      /* permanent arena              */
    VKN_buffer_vertex_dynamic_type
                       *buffer      /* output new vertex buffer     */
    );

void VKN_buffer_vertex_dynamic_destroy
    (
    VKN_releaser_type  *releaser,   /* release buffer               */
    VKN_buffer_vertex_dynamic_type
                       *buffer      /* buffer to destroy            */
    );

VKN_BUFFER_VERTEX_DYNAMIC_CONFIG_API VKN_buffer_vertex_dynamic_init_builder
    (
    const VkDevice      logical,    /* associated logical device    */
    const VkPhysicalDeviceProperties
                       *props,      /* device memory properties     */
    VKN_memory_type    *memory,     /* memory allocator             */
    VKN_buffer_vertex_dynamic_build_type
                       *builder     /* dynamic vertex buffer builder*/
    );
