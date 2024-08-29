#pragma once

#include "VknArenaTypes.hpp"
#include "VknBufferUniformTypes.hpp"
#include "VknCommon.hpp"
#include "VknMemoryTypes.hpp"
#include "VknReleaserTypes.hpp"


void VKN_buffer_uniform_create
    (
    const VKN_buffer_uniform_build_type
                       *builder,    /* uniform buffer builder       */
    VKN_arena_type     *arena,      /* permanent arena              */
    VKN_buffer_uniform_type
                       *buffer      /* output new uniform buffer    */
    );

void VKN_buffer_uniform_destroy
    (
    VKN_releaser_type  *releaser,   /* release buffer               */
    VKN_buffer_uniform_type
                       *buffer      /* buffer to destroy            */
    );

VKN_BUFFER_UNIFORM_CONFIG_API VKN_buffer_uniform_init_builder
    (
    const VkDevice      logical,    /* associated logical device    */
    const VkPhysicalDeviceProperties
                       *props,      /* device memory properties     */
    VKN_memory_type    *memory,     /* memory allocator             */
    VKN_buffer_uniform_build_type
                       *builder     /* uniform buffer builder       */
    );
