#pragma once

#include "Global.hpp"

#include "VknCommon.hpp"
#include "VknReleaserTypes.hpp"
#include "VknShaderTypes.hpp"


bool VKN_shader_create
    (
    const VKN_shader_build_type
                       *builder,    /* shader builder               */
    VKN_shader_type    *shader      /* output new shader            */
    );

void VKN_shader_destroy
    (
    VKN_releaser_type  *releaser,   /* release buffer               */
    VKN_shader_type    *shader      /* shader to destroy            */
    );

VKN_SHADER_CONFIG_API VKN_shader_init_builder
    (
    const VkDevice      logical,    /* associated logical device    */
    VKN_shader_build_type
                       *builder     /* shader builder               */
    );
