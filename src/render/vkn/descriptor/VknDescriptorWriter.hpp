#pragma once

#include "VknCommon.hpp"
#include "VknDescriptorPoolTypes.hpp"
#include "VknDescriptorWriterTypes.hpp"
#include "VknShaderParamTypes.hpp"


void VKN_descriptor_writer_create
    (
    const VkDevice     logical,     /* logical device               */
    VKN_shader_param_type
                       *parameters, /* shader parameters            */
    VKN_descriptor_pool_type
                       *pool,       /* descriptor pool              */
    VKN_descriptor_writer_type
                       *writer      /* output new descriptor writer */
    );
