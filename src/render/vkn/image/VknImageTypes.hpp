#pragma once

#include "Global.hpp"

#include "VknCommon.hpp"
#include "VknMemoryTypes.hpp"
#include "VknStagingTypes.hpp"


#define VKN_IMAGE_MAX_SHARING_FAMILY_CNT \
                                    ( 3 )
#define VKN_IMAGE_MAX_TEXTURE_FORMAT_CNT \
                                    ( 10 )
#define VKN_IMAGE_ANISO_QUALITY_MAX ( 9999.0f )

#define VKN_IMAGE_CONFIG_API        const struct _VKN_image_build_config_type *

typedef enum
    {
    VKN_IMAGE_USAGE_SAMPLE_ONLY,
    VKN_IMAGE_USAGE_COLOR_ATTACH,
    VKN_IMAGE_USAGE_DEPTH_STENCIL_ATTACH
    } VKN_image_usage_type;

typedef enum
    {
    VKN_IMAGE_DATA_FORMAT_UNDEFINED,
    VKN_IMAGE_DATA_FORMAT_RGBA8,
    VKN_IMAGE_DATA_FORMAT_RGBX8,
    VKN_IMAGE_DATA_FORMAT_ALPHA,
    VKN_IMAGE_DATA_FORMAT_DEPTH,
    VKN_IMAGE_DATA_FORMAT_DEPTH_STENCIL
    } VKN_image_data_format_type;


typedef VKN_IMAGE_CONFIG_API VKN_image_build_add_sharing_family_proc_type
    (
    const u32           index,      /* family index to share with   */
    struct _VKN_image_build_type
                       *builder     /* image builder                */
    );

typedef VKN_IMAGE_CONFIG_API VKN_image_build_enable_anisotropy_proc_type
    (
    const bool          should,     /* use anisotropic filter?      */
    const float         quality,    /* max sampling                 */
    struct _VKN_image_build_type
                       *builder     /* image builder                */
    );

typedef VKN_IMAGE_CONFIG_API VKN_image_build_generate_mip_maps_proc_type
    (
    const bool          should,     /* should we generate mip-maps? */
    const bool          is_linear,  /* linear mip-mapping?          */
    struct _VKN_image_build_type
                       *builder     /* image builder                */
    );

typedef VKN_IMAGE_CONFIG_API VKN_image_build_reset_proc_type
    (
    struct _VKN_image_build_type
                       *builder     /* image builder                */
    );

typedef VKN_IMAGE_CONFIG_API VKN_image_build_set_addressing_proc_type
    (
    VkSamplerAddressMode
                        u,          /* horizontal addressing        */
    VkSamplerAddressMode
                        v,          /* vertical addressing          */
    struct _VKN_image_build_type
                       *builder     /* image builder                */
    );

typedef VKN_IMAGE_CONFIG_API VKN_image_build_set_allocation_callbacks_proc_type
    (
    VkAllocationCallbacks
                       *allocator,  /* custom allocator             */
    struct _VKN_image_build_type
                       *builder     /* image builder                */
    );

typedef VKN_IMAGE_CONFIG_API VKN_image_build_set_extent_proc_type
    (
    const u32          width,      /* image width                  */
    const u32          height,     /* image height                 */
    struct _VKN_image_build_type
                       *builder     /* image builder                */
    );

typedef VKN_IMAGE_CONFIG_API VKN_image_build_set_extent_proc_type
    (
    const u32           width,      /* image width                  */
    const u32           height,     /* image height                 */
    struct _VKN_image_build_type
                       *builder     /* image builder                */
    );

typedef VKN_IMAGE_CONFIG_API VKN_image_build_set_filter_proc_type
    (
    const VkFilter      minify,     /* minification filter          */
    const VkFilter      magnify,    /* magnification filter         */
    struct _VKN_image_build_type
                       *builder     /* image builder                */
    );

typedef VKN_IMAGE_CONFIG_API VKN_image_build_set_image_data_format_proc_type
    (
    const VKN_image_data_format_type
                        format,     /* image data format            */
    struct _VKN_image_build_type
                       *builder     /* image builder                */
    );

typedef VKN_IMAGE_CONFIG_API VKN_image_build_set_image_view_proc_type
    (
    const VkImageViewType
                        view,       /* type of image view           */
    struct _VKN_image_build_type
                       *builder     /* image builder                */
    );

typedef VKN_IMAGE_CONFIG_API VKN_image_build_set_msaa_proc_type
    (
    const VKN_msaa_type msaa,       /* multi-sampling level         */
    struct _VKN_image_build_type
                       *builder     /* image builder                */
    );

typedef VKN_IMAGE_CONFIG_API VKN_image_build_set_usage_proc_type
    (
    const VKN_image_usage_type
                        usage,      /* image usage                  */
    struct _VKN_image_build_type
                       *builder     /* image builder                */
    );

typedef struct _VKN_image_build_config_type
    {
    VKN_image_build_add_sharing_family_proc_type
                       *add_sharing_family;
                                    /* add family to share with     */
    VKN_image_build_enable_anisotropy_proc_type
                       *enable_anisotropy;
                                    /* enable anisotropic filtering?*/
    VKN_image_build_generate_mip_maps_proc_type
                       *generate_mip_maps;
                                    /* generate mip maps?           */
    VKN_image_build_reset_proc_type
                       *reset;      /* reset configuration defaults */
    VKN_image_build_set_addressing_proc_type
                       *set_addressing;
                                    /* set addressing mode          */
    VKN_image_build_set_allocation_callbacks_proc_type
                       *set_allocation_callbacks;
                                    /* set custom allocator         */
    VKN_image_build_set_extent_proc_type
                       *set_extent; /* set the image dimensions     */
    VKN_image_build_set_filter_proc_type
                       *set_filter; /* set the min/mag filter       */
    VKN_image_build_set_image_data_format_proc_type
                       *set_image_data_format;
                                    /* set image data format        */
    VKN_image_build_set_image_view_proc_type
                       *set_image_view;
                                    /* set type of image view       */
    VKN_image_build_set_msaa_proc_type
                       *set_msaa;   /* set anti-aliasing level      */
    VKN_image_build_set_usage_proc_type
                       *set_usage;  /* set intended usage           */
    } VKN_image_build_config_type;

typedef struct
    {
    u32                 indices[ VKN_IMAGE_MAX_SHARING_FAMILY_CNT ];
                                    /* family indices               */
    u32                 count;      /* number of indices            */
    } VKN_image_build_family_indices_type;

typedef struct
    {
    VkFormat            format;     /* format type                  */
    VkFormatProperties  props;      /* format support               */
    } VKN_image_texture_format_type;

typedef struct
    {
    u8                  min_memory_map_alignment;
    u8                  framebuffer_color_sample_counts;
    u8                  framebuffer_depth_sample_counts;
    u8                  sampled_image_color_sample_counts;
    float               max_sampler_anisotropy;
    } VKN_image_build_limits_type;

typedef struct
    {
    bool                generate_mip_maps : 1;
                                    /* generate image mip maps?     */
    bool                use_aniso : 1;
                                    /* use anisotropic filtering?   */
    float               aniso_max;  /* max anisotropic filter sample*/
    VKN_image_build_limits_type
                        limits;     /* physical device limits       */
    VKN_image_data_format_type
                        format;     /* external format              */
    VKN_msaa_type       msaa;       /* multi-sampling level         */
    VkImageViewType     view;       /* view type                    */
    const VkAllocationCallbacks
                       *allocator;  /* allocation callbacks         */
    VKN_memory_type    *memory;     /* device memory allocator      */
    VkDevice            logical;    /* associated logical device    */
    VkExtent2D          extent;     /* image extent                 */
    VKN_image_build_family_indices_type
                        families;   /* queue families               */
    VKN_image_texture_format_type   /* support for texture formats  */
                        format_support[ VKN_IMAGE_MAX_TEXTURE_FORMAT_CNT ];
    VKN_image_usage_type
                        usage;      /* how is image to be used?     */
    VkFilter            filter_mag; /* magnification filter         */
    VkFilter            filter_min; /* minification filter          */
    VkSamplerMipmapMode mipmode;    /* mip-mapping mode             */
    VkSamplerAddressMode
                        address_u;  /* horizontal addressing        */
    VkSamplerAddressMode
                        address_v;  /* vertical addressing          */
    } VKN_image_build_state_type;

typedef struct _VKN_image_build_type
    {
    const VKN_image_build_config_type
                       *config;     /* configuration interface      */
    VKN_image_build_state_type
                        state;      /* builder state                */
    } VKN_image_build_type;

typedef void VKN_image_load_proc_type
    (
    const void         *data,       /* raw linear image data        */
    const u32           offset_x,   /* x offset in data buffer      */
    const u32           offset_y,   /* y offset in data buffer      */
    const u32           width,      /* data buffer width            */
    const u32           height,     /* data buffer height           */
    VKN_staging_type   *staging,    /* staging buffer               */
    struct _VKN_image_type
                       *image       /* image to load                */
    );

typedef struct
    {
    VKN_image_load_proc_type
                       *load;       /* load data into the image     */
    } VKN_image_api_type;

typedef struct _VKN_image_type
    {
    u8                  upload_alignment;
                                    /* required alignment for upload*/
    u32                 mip_levels; /* number of mip levels         */
    VkFormat            format;     /* image format                 */
    VkImageUsageFlags   image_usage;/* how image will be used       */
    const VkAllocationCallbacks
                       *allocator;  /* allocation callbacks         */
    const VKN_image_api_type
                       *i;          /* image interface              */
    VkDevice            logical;    /* owning logical device        */
    VkImage             image;      /* image handle                 */
    VkImageView         view;       /* image view handle            */
    VkSampler           sampler;    /* sampler handle               */
    VKN_memory_type    *memory;     /* device memory allocator      */
    VkExtent3D          extent;     /* image size                   */
    VKN_memory_allocation_type
                        allocation; /* device memory                */
    } VKN_image_type;
