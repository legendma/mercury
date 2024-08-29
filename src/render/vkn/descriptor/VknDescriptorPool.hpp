#pragma once

#include "Global.hpp"

#include "VknCommon.hpp"
#include "VknDescriptorPoolTypes.hpp"
#include "VknReleaserTypes.hpp"


bool VKN_descriptor_pool_create
    (
    const VKN_descriptor_pool_build_type
                       *builder,    /* shader builder               */
    VKN_descriptor_pool_type
                       *pool        /* output new descriptor pool   */
    );

void VKN_descriptor_pool_destroy
    (
    VKN_releaser_type  *releaser,   /* release buffer               */
    VKN_descriptor_pool_type
                       *pool        /* descriptor pool to destroy */
    );

VKN_DESCRIPTOR_POOL_CONFIG_API VKN_descriptor_pool_init_builder
    (
    const VkDevice      logical,    /* associated logical device    */
    VKN_descriptor_pool_build_type
                       *builder     /* descriptor pool builder      */
    );
