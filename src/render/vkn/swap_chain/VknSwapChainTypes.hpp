#pragma once

#include "Global.hpp"

#include "VknCommon.hpp"


#define VKN_SWAP_CHAIN_MAX_FORMATS_CNT \
                                    ( 100 )
#define VKN_SWAP_CHAIN_MAX_PRESENT_MODES_CNT \
                                    ( 10 )
#define VKN_SWAP_CHAIN_MAX_IMAGE_CNT \
                                    ( 5 )

#define VKN_SWAP_CHAIN_CONFIG_API   const struct _VKN_swap_chain_build_config_type *


typedef VKN_SWAP_CHAIN_CONFIG_API VKN_swap_chain_build_add_present_mode_proc_type
    (
    const VkPresentModeKHR
                        mode,       /* presentation mode            */
    struct _VKN_swap_chain_build_type
                       *builder     /* swap chain builder           */
    );

typedef VKN_SWAP_CHAIN_CONFIG_API VKN_swap_chain_build_add_surface_format_proc_type
    (
    const VkFormat      format,     /* surface format               */
    const VkColorSpaceKHR
                        color_space,/* surface color space          */

    struct _VKN_swap_chain_build_type
                       *builder     /* swap chain builder           */
    );

typedef VKN_SWAP_CHAIN_CONFIG_API VKN_swap_chain_build_commit_config_proc_type
    (
    const VkSurfaceKHR  surface,    /* associated display surface   */
    struct _VKN_swap_chain_build_type
                       *builder     /* swap chain builder           */
    );

typedef VKN_SWAP_CHAIN_CONFIG_API VKN_swap_chain_build_reset_proc_type
    (
    struct _VKN_swap_chain_build_type
                       *builder     /* swap chain builder           */
    );

typedef VKN_SWAP_CHAIN_CONFIG_API VKN_swap_chain_build_set_allocation_callbacks_proc_type
    (
    VkAllocationCallbacks
                       *allocator,  /* custom allocator             */
    struct _VKN_swap_chain_build_type
                       *builder     /* swap chain builder           */
    );

typedef VKN_SWAP_CHAIN_CONFIG_API VKN_swap_chain_build_set_default_present_mode_proc_type
    (
    struct _VKN_swap_chain_build_type
                       *builder     /* swap chain builder           */
    );

typedef VKN_SWAP_CHAIN_CONFIG_API VKN_swap_chain_build_set_default_surface_format_proc_type
    (
    struct _VKN_swap_chain_build_type
                       *builder     /* swap chain builder           */
    );

typedef VKN_SWAP_CHAIN_CONFIG_API VKN_swap_chain_build_set_extent_proc_type
    (
    const u32           width,      /* surface width                */
    const u32           height,     /* surface height               */
    struct _VKN_swap_chain_build_type
                       *builder     /* swap chain builder           */
    );

typedef struct _VKN_swap_chain_build_config_type
    {
    VKN_swap_chain_build_add_present_mode_proc_type
                       *add_present_mode;
                                    /* add desired present mode     */
    VKN_swap_chain_build_add_surface_format_proc_type
                       *add_surface_format;
                                    /* add desired surface format   */
    VKN_swap_chain_build_commit_config_proc_type
                       *commit_config;
                                    /* commit by associated surface */
    VKN_swap_chain_build_reset_proc_type
                       *reset;      /* reset the configuration      */
    VKN_swap_chain_build_set_allocation_callbacks_proc_type
                       *set_allocation_callbacks;
                                    /* set custom allocator         */
    VKN_swap_chain_build_set_default_present_mode_proc_type
                       *set_default_present_mode;
                                    /* set default presentation mode*/
    VKN_swap_chain_build_set_default_surface_format_proc_type
                       *set_default_surface_format;
                                    /* set default surface format   */
    VKN_swap_chain_build_set_extent_proc_type
                       *set_extent; /* set surface dimensions       */
    } VKN_swap_chain_build_config_type;

typedef struct
    {
    VkPresentModeKHR    modes[ VKN_SWAP_CHAIN_MAX_PRESENT_MODES_CNT ];
                                    /* presentation flip modes      */
    u32                 count;      /* number of modes              */
    } VKN_swap_chain_build_present_modes_type;

typedef struct
    {
    VkSurfaceFormatKHR  formats[ VKN_SWAP_CHAIN_MAX_FORMATS_CNT ];
                                    /* surface formats              */
    u32                 count;      /* number of formats            */
    } VKN_swap_chain_build_surface_formats_type;

typedef struct
    {
    VkSurfaceCapabilitiesKHR        /* device surface capabilities  */
                        capabilities;
    VKN_swap_chain_build_surface_formats_type
                        formats;    /* surface formats              */
    VKN_swap_chain_build_present_modes_type
                        present_modes;
                                    /* presentation modes           */
    } VKN_swap_chain_build_capabilities_type;

typedef struct
    {
    VkPhysicalDevice    physical;   /* associated physical device   */
    VkDevice            logical;    /* associated logical device    */
    VkSurfaceKHR        surface;    /* associated surface           */
    VkAllocationCallbacks
                       *allocator;  /* custom allocator             */
    VKN_swap_chain_build_surface_formats_type
                        formats;    /* surface formats              */
    VKN_swap_chain_build_present_modes_type
                        present_modes;
                                    /* presentation modes           */
    VkExtent2D          extent;     /* surface dimensions           */
    VKN_swap_chain_build_capabilities_type
                        capabilities;
                                    /* device capabilities          */
    } VKN_swap_chain_build_state_type;

typedef struct _VKN_swap_chain_build_type
    {
    VKN_swap_chain_build_state_type
                        state;      /* builder state                */
    const VKN_swap_chain_build_config_type
                       *config;     /* configuration interface      */
    } VKN_swap_chain_build_type;

typedef struct
    {
    VkImage             images[ VKN_SWAP_CHAIN_MAX_IMAGE_CNT ];
                                    /* swap chain image handle      */
    VkImageView         views[ VKN_SWAP_CHAIN_MAX_IMAGE_CNT ];
                                    /* image views                  */
    u32                 count;      /* image count                  */
    } VKN_swap_chain_images_type;

typedef struct
    {
    VkDevice            logical;    /* associated logical device    */
    VkSwapchainKHR      swap_chain; /* swap chain handle            */
    VkAllocationCallbacks
                       *allocator;  /* custom allocator             */
    VkExtent2D          extent;     /* image dimensions             */
    VkFormat            format;     /* image format                 */
    VKN_swap_chain_images_type
                        images;     /* swap chain images            */
    } VKN_swap_chain_type;
