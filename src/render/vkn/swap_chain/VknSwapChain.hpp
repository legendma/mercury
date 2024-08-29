#pragma once

#include "Global.hpp"

#include "VknReleaserTypes.hpp"
#include "VknSwapChainTypes.hpp"


bool VKN_swap_chain_create
    (
    const u32           present_queue_index,
                                    /* presentation family index    */
    const u32           graphics_queue_index,
                                    /* graphics family index        */
    const VKN_swap_chain_build_type
                       *builder,    /* swap chain builder           */
    VKN_swap_chain_type
                       *swap_chain  /* output new swap chain        */
    );

void VKN_swap_chain_destroy
    (
    VKN_releaser_type  *releaser,   /* release buffer               */
    VKN_swap_chain_type
                       *swap_chain  /* output new swap chain        */
    );

VKN_SWAP_CHAIN_CONFIG_API VKN_swap_chain_init_builder
    (
    const VkPhysicalDevice
                        physical,   /* associated physical device   */
    const VkDevice      logical,    /* associated logical device    */
    VKN_swap_chain_build_type
                       *swap_chain  /* output new swap chain        */
    );
