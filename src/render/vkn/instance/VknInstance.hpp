#pragma once

#include "Global.hpp"

#include "VknInstanceTypes.hpp"
#include "VknReleaserTypes.hpp"


bool VKN_instance_create
    (
    const VKN_instance_build_type
                       *builder,    /* instance builder             */
    VKN_instance_type  *instance    /* output new instance          */
    );

void VKN_instance_destroy
    (
    VKN_releaser_type  *releaser,   /* release buffer               */
    VKN_instance_type  *instance    /* output new instance          */
    );

VKN_INSTANCE_CONFIG_API VKN_instance_init_builder
    (
    const bool          is_headless,/* headless display instance?   */
    VKN_instance_build_type
                       *builder     /* instance builder             */
    );
