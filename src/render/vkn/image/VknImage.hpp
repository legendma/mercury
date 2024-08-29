#pragma once

#include "Global.hpp"

#include "VknCommon.hpp"
#include "VknImageTypes.hpp"
#include "VknPhysicalDeviceTypes.hpp"
#include "VknReleaserTypes.hpp"


bool VKN_image_create
    (
    const VKN_image_build_type
                       *builder,    /* image builder                */
    VKN_image_type     *image       /* output new image             */
    );

void VKN_image_create_from_swap_chain
    (
    const VkImage       handle,     /* image handle                 */
    const VkImageView   view,       /* image view handle            */
    const VkExtent2D    extent,     /* image extent                 */
    const VkFormat      format,     /* image format                 */
    VKN_image_type     *image       /* output new image             */
    );

void VKN_image_destroy
    (
    VKN_releaser_type  *releaser,   /* release buffer               */
    VKN_image_type     *image       /* image to destroy             */
    );

void VKN_image_init_builder
    (
    const VkDevice      logical,    /* associated logical device    */
    const VkPhysicalDevice
                        physical,   /* associated physical device   */
    const VkPhysicalDeviceProperties
                       *props,      /* device memory properties     */
    VKN_memory_type    *memory,     /* device memory allocator      */
    VKN_image_build_type
                       *builder     /* image builder                */
    );
