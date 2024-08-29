#pragma once

#include "VknPipelineGraphicsTypes.hpp"
#include "VknReleaserTypes.hpp"


bool VKN_pipeline_graphics_create
    (
    const VKN_render_flags_type
                        flags,      /* render state flags           */
    const VkPipelineLayout
                        layout,     /* pipeline layout              */
    const VKN_pipeline_graphics_build_type
                       *builder,    /* graphics pipeline builder    */
    VKN_pipeline_graphics_type
                       *pipeline    /* output new graphics pipeline */
    );

void VKN_pipeline_graphics_destroy
    (
    VKN_releaser_type  *releaser,   /* release buffer               */
    VKN_pipeline_graphics_type
                       *pipeline    /* graphics pipeline to destroy */
    );

VKN_PIPELINE_GRAPHICS_CONFIG_API VKN_pipeline_graphics_init_builder
    (
    const VkDevice      logical,    /* associated logical device    */
    VKN_pipeline_graphics_build_type
                       *builder     /* graphics pipeline builder    */
    );
