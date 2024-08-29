#pragma once

#include "Global.hpp"

#include "VknMemoryTypes.hpp"
#include "VknStagingTypes.hpp"


bool VKN_staging_create
    (
    const VKN_staging_build_type
                       *builder,    /* resource staging builder     */
    VKN_staging_type   *staging     /* output new resource staging  */
    );

void VKN_staging_destroy
    (
    VKN_releaser_type  *releaser,   /* release buffer               */
    VKN_staging_type   *staging     /* resource staging to destroy  */
    );

VKN_STAGING_CONFIG_API VKN_staging_init_builder
    (
    const VkDevice      logical,    /* associated logical device    */
    const VkQueue       queue,      /* queue on which to submit     */
    const u32           queue_index,/* family index of submit queue */
    VKN_memory_type    *memory,     /* memory allocator             */
    VKN_staging_build_type
                       *builder     /* resource staging builder     */
    );
