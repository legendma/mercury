#pragma once

#include "Global.hpp"

#include "VknCommon.hpp"
#include "VknEffectTypes.hpp"
#include "VknReleaserTypes.hpp"


bool VKN_effect_create
    (
    const char         *str_name,   /* optional string for debug    */
    const VKN_effect_build_type
                       *builder,    /* shader effect builder        */
    VKN_effect_type    *effect      /* output new shader effect     */
    );

void VKN_effect_destroy
    (
    VKN_releaser_type  *releaser,   /* release buffer               */
    VKN_effect_type    *effect      /* shader effect to destroy     */
    );

VKN_EFFECT_CONFIG_API VKN_effect_init_builder
    (
    const VkDevice      logical,    /* associated logical device    */
    VKN_effect_build_type
                       *builder     /* shader effect builder        */
    );
