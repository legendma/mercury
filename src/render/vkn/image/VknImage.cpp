#include "Global.hpp"
#include "Utilities.hpp"

#include "VknCommon.hpp"
#include "VknImage.hpp"
#include "VknImageTypes.hpp"
#include "VknReleaser.hpp"


static const VkFormat SUPPORTED_FORMATS[] =
    {
    VK_FORMAT_R8G8B8A8_UNORM,
    VK_FORMAT_R8G8B8A8_SRGB,
    VK_FORMAT_R8G8B8_UNORM,
    VK_FORMAT_R8G8B8_SRGB,
    VK_FORMAT_R8_UNORM,
    VK_FORMAT_R8_SRGB,
    VK_FORMAT_X8_D24_UNORM_PACK32,
    VK_FORMAT_D32_SFLOAT,
    VK_FORMAT_D24_UNORM_S8_UINT,
    VK_FORMAT_D32_SFLOAT_S8_UINT
    };
compiler_assert( cnt_of_array( SUPPORTED_FORMATS ) == VKN_IMAGE_MAX_TEXTURE_FORMAT_CNT, VKN_IMAGE_C );


/*********************************************************************
*
*   PROCEDURE NAME:
*       get_aspect_mask
*
*********************************************************************/
    
static __inline VkImageAspectFlags get_aspect_mask
    (
    const VKN_image_data_format_type
                        data_format /* external image format        */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkImageAspectFlags      ret;        /* return image aspect          */

ret = 0;
switch( data_format )
    {
    case VKN_IMAGE_DATA_FORMAT_DEPTH:
        set_bits( ret, VK_IMAGE_ASPECT_DEPTH_BIT );
        break;
    case VKN_IMAGE_DATA_FORMAT_DEPTH_STENCIL:
        set_bits( ret, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT );
        break;
    default:
        set_bits( ret, VK_IMAGE_ASPECT_COLOR_BIT );
        break;
    }

return( ret );

}   /* get_aspect_mask() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       get_format_bit_count
*
*********************************************************************/
    
static __inline u32 get_format_bit_count
    (
    const VkFormat      format      /* image data format            */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkImageAspectFlags      ret;        /* return image aspect          */

ret = 0;
switch( format )
    {
    case VK_FORMAT_R8G8B8A8_UNORM:
    case VK_FORMAT_R8G8B8A8_SRGB:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
    case VK_FORMAT_X8_D24_UNORM_PACK32:
    case VK_FORMAT_D32_SFLOAT:
        ret = 32;
        break;

    case VK_FORMAT_R8G8B8_UNORM:
    case VK_FORMAT_R8G8B8_SRGB:
        ret = 24;
        break;

    case VK_FORMAT_R8_UNORM:
    case VK_FORMAT_R8_SRGB:
        ret = 8;
        break;

    default:
        debug_assert_always();
        break;
    }    

return( ret );

}   /* get_format_bit_count() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       get_format_support
*
*********************************************************************/

static __inline const VKN_image_texture_format_type * get_format_support
    (
    const VkFormat      format,     /* for which to check support   */
    const VKN_image_build_type
                       *builder     /* image builder                */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     i;          /* loop counter                 */
const VKN_image_texture_format_type
                       *ret;        /* return format support        */

ret = NULL;
for( i = 0; i < cnt_of_array( builder->state.format_support ); i++ )
    {
    if( builder->state.format_support[ i ].format == format )
        {
        ret = &builder->state.format_support[ i ];
        break;
        }
    }

return( ret );

}   /* get_format_support() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       get_mip_levels
*
*********************************************************************/

static __inline u32 get_mip_levels
    (
    const bool          should,     /* should mip map?              */
    const VkExtent2D    base        /* base image extent            */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkExtent2D              extent;     /* working image extent         */
u32                     ret;        /* return mip level count       */

if( !should )
    {
    return( 1 );
    }

for( ret = 0, extent = base;
     extent.width > 0 || extent.height > 0;
     extent.width >>= 1, extent.height >>= 1, ret++ );

return( ret );

}   /* get_mip_levels() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       get_sample_count
*
*********************************************************************/

static __inline VkSampleCountFlags get_sample_count
    (
    const VkImageUsageFlags
                        usage,      /* how image is used            */
    const VKN_msaa_type msaa,       /* multi-sampling level         */
    const VKN_image_build_limits_type
                       *limits      /* physical device capabilities */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkSampleCountFlags      ret;        /* return sample count          */
VkSampleCountFlags      supported;  /* supported sample count mask  */

/*----------------------------------------------------------
Start with the requested amount
----------------------------------------------------------*/
switch( msaa )
    {
    case VKN_MSAA_4:
        ret = VK_SAMPLE_COUNT_4_BIT;
        break;

    case VKN_MSAA_8:
        ret = VK_SAMPLE_COUNT_8_BIT;
        break;

    case VKN_MSAA_16:
        ret = VK_SAMPLE_COUNT_16_BIT;
        break;

    default:
        ret = VK_SAMPLE_COUNT_1_BIT;
        break;
    }

/*----------------------------------------------------------
Now find the subset sample count supported, based on the
usage
----------------------------------------------------------*/
supported = VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
if( usage & VK_IMAGE_USAGE_SAMPLED_BIT )
    {
    supported &= limits->sampled_image_color_sample_counts;
    }

if( ( usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT )
 || ( usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT ) )
    {
    supported &= limits->framebuffer_color_sample_counts;
    supported &= limits->framebuffer_depth_sample_counts;
    }

/*----------------------------------------------------------
Finally find the closest sample count to the requested
sample count 
----------------------------------------------------------*/
do
    {
    if( ret & supported )
        {
        /*--------------------------------------------------
        GPU supports this count, we're done
        --------------------------------------------------*/
        break;
        }

    } while( ret >>= 1 );

if( !ret )
    {
    debug_assert_always();
    ret = VK_SAMPLE_COUNT_1_BIT;
    }

return( ret );

}   /* get_sample_count() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       get_type_of_image
*
*********************************************************************/

static __inline VkImageType get_type_of_image
    (
    const VkExtent3D    base        /* base image extent            */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkImageType             ret;        /* return image type            */

ret = VK_IMAGE_TYPE_MAX_ENUM;
if( base.width
 && base.height <= 1
 && base.depth  <= 1 )
    {
    ret = VK_IMAGE_TYPE_1D;
    }
else if( base.width
      && base.height
      && base.depth <= 1 )
    {
    ret = VK_IMAGE_TYPE_2D;
    }
else if( base.width  <= 1
      && base.height <= 1
      && base.depth  <= 1)
    {
    VK_IMAGE_TYPE_3D;
    }

return( ret );

}   /* get_type_of_image() */


/*------------------------------------------------------------------------------------------
                                         PROCEDURES
------------------------------------------------------------------------------------------*/

static VKN_image_build_add_sharing_family_proc_type add_sharing_family;

static void add_sharing_family_safe
    (
    const u32           index,      /* family index to share with   */
    VKN_image_build_type
                       *builder     /* image builder                */
    );

static VKN_image_build_enable_anisotropy_proc_type enable_anisotropy;
static VKN_image_build_generate_mip_maps_proc_type generate_mip_maps;

static VkComponentMapping get_component_mapping
    (
    const VKN_image_data_format_type
                        data_format /* for which to get mapping     */
    );

static VkFormat get_image_format
    (
    const VKN_image_data_format_type
                        data_format,/* external data format         */
    const VkImageUsageFlags
                        usage,      /* how image is to be used      */
    const VKN_image_build_type
                       *builder     /* image builder                */
    );

static VkImageViewType get_type_of_image_view
    (
    const VkImageViewType
                        requested,  /* user requested view          */
    const VkImageType   image,      /* image type                   */
    const VkImageCreateFlags
                        flags,      /* creation flags               */
    const VkExtent3D    extent,     /* extent of each layer         */
    const u32           layer_cnt   /* number of image layers       */
    );

static VKN_image_load_proc_type load;
static VKN_image_build_reset_proc_type reset;
static VKN_image_build_set_addressing_proc_type set_addressing;
static VKN_image_build_set_allocation_callbacks_proc_type set_allocation_callbacks;
static VKN_image_build_set_extent_proc_type set_extent;
static VKN_image_build_set_filter_proc_type set_filter;
static VKN_image_build_set_image_data_format_proc_type set_image_data_format;
static VKN_image_build_set_image_view_proc_type set_image_view;
static VKN_image_build_set_msaa_proc_type set_msaa;
static VKN_image_build_set_usage_proc_type set_usage;


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_image_create
*
*   DESCRIPTION:
*       Create an image via the given builder.
*
*********************************************************************/

bool VKN_image_create
    (
    const VKN_image_build_type
                       *builder,    /* image builder                */
    VKN_image_type     *image       /* output new image             */
    )
{
/*----------------------------------------------------------
Local constants
----------------------------------------------------------*/
static const VKN_image_api_type API =
    {
    load
    };

/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkImageCreateInfo       ci_image;   /* image create info            */
VkSamplerCreateInfo     ci_sampler; /* sampler create info          */
VkImageViewCreateInfo   ci_view;    /* image view create info       */
VkExtent3D              extent;     /* dimensions single layer      */
VkFormat                format;     /* image format                 */
VkImageType             image_kind; /* image type                   */
VkImageCreateFlags      image_flags;/* image creation flags         */
VkImageUsageFlags       image_usage;/* how image is to be used      */
u32                     layer_cnt;  /* number of image layers       */
u32                     mip_levels; /* number of mip levels         */
VkImageViewType         view;       /* type of image view           */

clr_struct( image );

/*----------------------------------------------------------
Validate
----------------------------------------------------------*/
if( builder->state.extent.width == 0 )
    {
    return( FALSE );
    }

/*----------------------------------------------------------
Determine the usage
----------------------------------------------------------*/
image_usage = VK_IMAGE_USAGE_SAMPLED_BIT;
switch( builder->state.usage )
    {
    case VKN_IMAGE_USAGE_COLOR_ATTACH:
        image_usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        break;

    case VKN_IMAGE_USAGE_DEPTH_STENCIL_ATTACH:
        image_usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        break;

    default:
        image_usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        break;
    }

/*----------------------------------------------------------
Determine the properties
----------------------------------------------------------*/
format = get_image_format( builder->state.format, image_usage, builder );
if( format == VK_FORMAT_UNDEFINED )
    {
    return( FALSE );
    }

image_flags   = 0;
extent.width  = builder->state.extent.width;
extent.height = builder->state.extent.height;
extent.depth  = 1;
mip_levels    = get_mip_levels( builder->state.generate_mip_maps, builder->state.extent );
layer_cnt     = 1;

image_kind = get_type_of_image( extent );
if( image_kind == VK_IMAGE_TYPE_MAX_ENUM )
    {
    return( FALSE );
    }

view = get_type_of_image_view( builder->state.view, image_kind, image_flags, extent, layer_cnt );
if( view == VK_IMAGE_VIEW_TYPE_MAX_ENUM )
    {
    return( FALSE );
    }

/*----------------------------------------------------------
Create the image
----------------------------------------------------------*/
image->i = &API;

image->logical          = builder->state.logical;
image->allocator        = builder->state.allocator;
image->memory           = builder->state.memory;
image->format           = format;
image->image_usage      = image_usage;
image->mip_levels       = mip_levels;
image->extent.width     = builder->state.extent.width;
image->extent.height    = builder->state.extent.height;
image->extent.depth     = 1;
image->upload_alignment = builder->state.limits.min_memory_map_alignment;

clr_struct( &ci_image );
ci_image.sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
ci_image.arrayLayers           = layer_cnt;
ci_image.extent                = image->extent;
ci_image.format                = format;
ci_image.imageType             = image_kind;
ci_image.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;
ci_image.mipLevels             = mip_levels;
ci_image.pQueueFamilyIndices   = builder->state.families.indices;
ci_image.queueFamilyIndexCount = builder->state.families.count;
ci_image.samples               = (VkSampleCountFlagBits)get_sample_count( image_usage, builder->state.msaa, &builder->state.limits );
ci_image.sharingMode           = ( builder->state.families.count > 1 ) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
ci_image.tiling                = VK_IMAGE_TILING_OPTIMAL;
ci_image.usage                 = image_usage;

if( VKN_failed( vkCreateImage( image->logical, &ci_image, image->allocator, &image->image ) ) )
    {
    VKN_image_destroy( NULL, image );
    return( FALSE );
    }

/*----------------------------------------------------------
Back it with device memory
----------------------------------------------------------*/
if( !image->memory->i->create_image_memory( image->image, VKN_MEMORY_HEAP_USAGE_DEFAULT, image->memory, &image->allocation ) )
    {
    VKN_image_destroy( NULL, image );
    return( FALSE );
    }

/*----------------------------------------------------------
Create the image view
----------------------------------------------------------*/
clr_struct( &ci_view );
ci_view.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
ci_view.components                      = get_component_mapping( builder->state.format );
ci_view.format                          = format;
ci_view.image                           = image->image;
ci_view.subresourceRange.aspectMask     = get_aspect_mask( builder->state.format );
ci_view.subresourceRange.baseMipLevel   = 0;
ci_view.subresourceRange.levelCount     = mip_levels;
ci_view.subresourceRange.baseArrayLayer = 0;
ci_view.subresourceRange.layerCount     = layer_cnt;
ci_view.viewType                        = get_type_of_image_view( builder->state.view, ci_image.imageType, ci_image.flags, image->extent, ci_image.arrayLayers );

if( VKN_failed( vkCreateImageView( image->logical, &ci_view, image->allocator, &image->view ) ) )
    {
    VKN_image_destroy( NULL, image );
    return( FALSE );
    }

/*----------------------------------------------------------
Create the sampler
----------------------------------------------------------*/
clr_struct( &ci_sampler );
ci_sampler.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
ci_sampler.flags                   = 0;
ci_sampler.magFilter               = builder->state.filter_mag;
ci_sampler.minFilter               = builder->state.filter_min;
ci_sampler.mipmapMode              = builder->state.mipmode;
ci_sampler.addressModeU            = builder->state.address_u;
ci_sampler.addressModeV            = builder->state.address_v;
ci_sampler.anisotropyEnable        = builder->state.use_aniso;
ci_sampler.maxAnisotropy           = builder->state.aniso_max;
ci_sampler.minLod                  = 0.0f;
ci_sampler.maxLod                  = (float)mip_levels;
ci_sampler.borderColor             = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
ci_sampler.unnormalizedCoordinates = VK_FALSE;

if( VKN_failed( vkCreateSampler( image->logical, &ci_sampler, image->allocator, &image->sampler ) ) )
    {
    VKN_image_destroy( NULL, image );
    return( FALSE );
    }

return( TRUE );

}   /* VKN_image_create() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_image_create_from_swap_chain
*
*   DESCRIPTION:
*       Create an image via a swap chain image.
*
*********************************************************************/

void VKN_image_create_from_swap_chain
    (
    const VkImage       handle,     /* image handle                 */
    const VkImageView   view,       /* image view handle            */
    const VkExtent2D    extent,     /* image extent                 */
    const VkFormat      format,     /* image format                 */
    VKN_image_type     *image       /* output new image             */
    )
{
/*----------------------------------------------------------
Local constants
----------------------------------------------------------*/
static const VKN_image_api_type API =
    {
    NULL
    };

clr_struct( image );

/*----------------------------------------------------------
Create the image
----------------------------------------------------------*/
image->i = &API;

image->format           = format;
image->image_usage      = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
image->mip_levels       = 1;
image->extent.width     = extent.width;
image->extent.height    = extent.height;
image->extent.depth     = 1;
image->image            = handle;
image->view             = view;
image->format           = format;

}   /* VKN_image_create_from_swap_chain() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_image_destroy
*
*   DESCRIPTION:
*       Destroy the given image.
*
*********************************************************************/

void VKN_image_destroy
    (
    VKN_releaser_type  *releaser,   /* release buffer               */
    VKN_image_type     *image       /* image to destroy             */
    )
{
VKN_releaser_auto_mini_begin( releaser, use );
use->i->release_image( image->logical, image->allocator, image->image, use );
if( image->memory )
    {
    image->memory->i->deallocate( image->memory, &image->allocation );
    }

use->i->release_image_view( image->logical, image->allocator, image->view, use );
use->i->release_sampler( image->logical, image->allocator, image->sampler, use );

VKN_releaser_auto_mini_end( use );
clr_struct( &image );

}   /* VKN_image_destroy() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_image_init_builder
*
*   DESCRIPTION:
*       Initialize an image builder.
*
*********************************************************************/

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
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     i;          /* loop counter                 */
VKN_image_texture_format_type       /* working image format         */
                       *working_format;                             

/*----------------------------------------------------------
Local constants
----------------------------------------------------------*/
static const VKN_image_build_config_type CONFIG =
    {
    add_sharing_family,
    enable_anisotropy,
    generate_mip_maps,
    reset,
    set_addressing,
    set_allocation_callbacks,
    set_extent,
    set_filter,
    set_image_data_format,
    set_image_view,
    set_msaa,
    set_usage
    };

/*----------------------------------------------------------
Initialize
----------------------------------------------------------*/
clr_struct( builder );
builder->config = &CONFIG;

builder->state.logical                                  = logical;
builder->state.memory                                   = memory;
builder->state.limits.framebuffer_color_sample_counts   = (u8)props->limits.framebufferColorSampleCounts;
builder->state.limits.framebuffer_depth_sample_counts   = (u8)props->limits.framebufferDepthSampleCounts;
builder->state.limits.sampled_image_color_sample_counts = (u8)props->limits.sampledImageColorSampleCounts;
builder->state.limits.min_memory_map_alignment          = (u8)props->limits.minMemoryMapAlignment;
builder->state.limits.max_sampler_anisotropy            = props->limits.maxSamplerAnisotropy;

reset( builder );

/*----------------------------------------------------------
Get device support for image formats we are interesting in
----------------------------------------------------------*/
for( i = 0; i < cnt_of_array( SUPPORTED_FORMATS ); i++ )
    {
    working_format = &builder->state.format_support[ i ];
    working_format->format = SUPPORTED_FORMATS[ i ];
    vkGetPhysicalDeviceFormatProperties( physical, working_format->format, &working_format->props );
    }

}   /* VKN_image_init_builder() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       add_sharing_family
*
*********************************************************************/

static VKN_IMAGE_CONFIG_API add_sharing_family
    (
    const u32           index,      /* family index to share with   */
    struct _VKN_image_build_type
                       *builder     /* image builder                */
    )
{
add_sharing_family_safe( index, builder );

return( builder->config );

}   /* add_sharing_family() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       add_sharing_family_safe
*
*********************************************************************/

static void add_sharing_family_safe
    (
    const u32           index,      /* family index to share with   */
    VKN_image_build_type
                       *builder     /* image builder                */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     i;          /* loop counter                 */

/*----------------------------------------------------------
Ignore invalid indices
----------------------------------------------------------*/
if( index == VKN_INVALID_FAMILY_INDEX )
    {
    return;
    }

/*----------------------------------------------------------
Verify that we don't already have this queue family
----------------------------------------------------------*/
for( i = 0; i < builder->state.families.count; i++ )
    {
    if( builder->state.families.indices[ i ] == index )
        {
        /*--------------------------------------------------
        Seen this already
        --------------------------------------------------*/
        return;
        }
    }

/*----------------------------------------------------------
Add it
----------------------------------------------------------*/
if( builder->state.families.count >= cnt_of_array( builder->state.families.indices ) )
    {
    debug_assert_always();
    return;
    }

builder->state.families.indices[ builder->state.families.count++ ] = index;

}   /* add_sharing_family_safe() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       enable_anisotropy
*
*********************************************************************/

static VKN_IMAGE_CONFIG_API enable_anisotropy
    (
    const bool          should,     /* use anisotropic filter?      */
    const float         quality,    /* max sampling                 */
    struct _VKN_image_build_type
                       *builder     /* image builder                */
    )
{
builder->state.use_aniso = should;
builder->state.aniso_max = 0.0f;
if( should
 && quality > 0.0f )
    {
    builder->state.aniso_max = quality;
    }

if( builder->state.aniso_max > builder->state.limits.max_sampler_anisotropy )
    {
    builder->state.aniso_max = builder->state.limits.max_sampler_anisotropy;
    }

return( builder->config );

}   /* enable_anisotropy() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       generate_mip_maps
*
*********************************************************************/

static VKN_IMAGE_CONFIG_API generate_mip_maps
    (
    const bool          should,     /* should we generate mip-maps? */
    const bool          is_linear,  /* linear mip-mapping?          */
    struct _VKN_image_build_type
                       *builder     /* image builder                */
    )
{
builder->state.generate_mip_maps = should;
builder->state.mipmode           = VK_SAMPLER_MIPMAP_MODE_NEAREST;
if( should
 && is_linear )
    {
    builder->state.mipmode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    }

return( builder->config );

}   /* generate_mip_maps() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       get_format_support
*
*********************************************************************/

static VkComponentMapping get_component_mapping
    (
    const VKN_image_data_format_type
                        data_format /* for which to get mapping     */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkComponentMapping      ret;        /* return component mapping     */

clr_struct( &ret );
ret.r = VK_COMPONENT_SWIZZLE_IDENTITY;
ret.g = VK_COMPONENT_SWIZZLE_IDENTITY;
ret.b = VK_COMPONENT_SWIZZLE_IDENTITY;
ret.a = VK_COMPONENT_SWIZZLE_IDENTITY;

switch( data_format )
    {
    case VKN_IMAGE_DATA_FORMAT_RGBX8:
        ret.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        ret.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        ret.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        ret.a = VK_COMPONENT_SWIZZLE_ONE;
        break;

    case VKN_IMAGE_DATA_FORMAT_ALPHA:
        ret.r = VK_COMPONENT_SWIZZLE_ONE;
        ret.g = VK_COMPONENT_SWIZZLE_ONE;
        ret.b = VK_COMPONENT_SWIZZLE_ONE;
        ret.a = VK_COMPONENT_SWIZZLE_R;
        break;

    default:
        debug_assert( data_format == VKN_IMAGE_DATA_FORMAT_UNDEFINED
                   || data_format == VKN_IMAGE_DATA_FORMAT_RGBA8
                   || data_format == VKN_IMAGE_DATA_FORMAT_DEPTH
                   || data_format == VKN_IMAGE_DATA_FORMAT_DEPTH_STENCIL );
        break;
    }

return( ret );

}   /* get_format_support() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       get_image_format
*
*********************************************************************/

static VkFormat get_image_format
    (
    const VKN_image_data_format_type
                        data_format,/* external data format         */
    const VkImageUsageFlags
                        usage,      /* how image is to be used      */
    const VKN_image_build_type
                       *builder     /* image builder                */
    )
{
/*----------------------------------------------------------
Local constants
----------------------------------------------------------*/
static const VkFormat RGBA8_FORMATS[] =
    {
    VK_FORMAT_R8G8B8A8_UNORM,
    VK_FORMAT_R8G8B8A8_SRGB
    };

static const VkFormat RGBX8_FORMATS[] =
    {
    VK_FORMAT_R8G8B8_UNORM,
    VK_FORMAT_R8G8B8_SRGB
    };

static const VkFormat ALPHA_FORMATS[] =
    {
    VK_FORMAT_R8_UNORM,
    VK_FORMAT_R8_SRGB
    };

static const VkFormat DEPTH_ONLY_FORMATS[] =
    {
    VK_FORMAT_X8_D24_UNORM_PACK32,
    VK_FORMAT_D32_SFLOAT
    };

static const VkFormat DEPTH_STENCIL_FORMATS[] =
    {
    VK_FORMAT_D24_UNORM_S8_UINT,
    VK_FORMAT_D32_SFLOAT_S8_UINT
    };

/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
bool                    has_support;/* is format supported?         */
u32                     i;          /* loop counter                 */
const VkFormat         *internals;  /* internal formats             */
u32                     internal_cnt;
                                    /* number of internal formats   */
VkFormat                ret;        /* return internal format       */
const VKN_image_texture_format_type
                       *support;    /* image format support         */

/*----------------------------------------------------------
Get the application internal formats
----------------------------------------------------------*/
internals = NULL;
internal_cnt = 0;
switch( data_format )
    {
    case VKN_IMAGE_DATA_FORMAT_RGBA8:
        internals = RGBA8_FORMATS;
        internal_cnt = cnt_of_array( RGBA8_FORMATS );
        break;

    case VKN_IMAGE_DATA_FORMAT_RGBX8:
        internals = RGBX8_FORMATS;
        internal_cnt = cnt_of_array( RGBX8_FORMATS );
        break;

    case VKN_IMAGE_DATA_FORMAT_ALPHA:
        internals = ALPHA_FORMATS;
        internal_cnt = cnt_of_array( ALPHA_FORMATS );
        break;

    case VKN_IMAGE_DATA_FORMAT_DEPTH:
        internals = DEPTH_ONLY_FORMATS;
        internal_cnt = cnt_of_array( DEPTH_ONLY_FORMATS );
        break;

    case VKN_IMAGE_DATA_FORMAT_DEPTH_STENCIL:
        internals = DEPTH_STENCIL_FORMATS;
        internal_cnt = cnt_of_array( DEPTH_STENCIL_FORMATS );
        break;

    default:
        break;
    }

/*----------------------------------------------------------
Find the first format supported by the physical device
----------------------------------------------------------*/
ret = VK_FORMAT_UNDEFINED;
for( i = 0; i < internal_cnt; i++ )
    {
    support = get_format_support( internals[ i ], builder );
    if( !support )
        {
        debug_assert_always();
        return( VK_FORMAT_UNDEFINED );
        }

    has_support = TRUE;

    /*------------------------------------------------------
    Sampling
    ------------------------------------------------------*/
    if( test_bits( usage, VK_IMAGE_USAGE_SAMPLED_BIT ) )
        {
        has_support &= test_bits( support->props.optimalTilingFeatures, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT );
        }

    /*------------------------------------------------------
    Color attach
    ------------------------------------------------------*/
    if( test_bits( usage, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT ) )
        {
        has_support &= test_bits( support->props.optimalTilingFeatures, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT );
        }

    /*------------------------------------------------------
    Depth attach
    ------------------------------------------------------*/
    if( test_bits( usage, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT ) )
        {
        has_support &= test_bits( support->props.optimalTilingFeatures, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT );
        }

    /*------------------------------------------------------
    Transfer destination
    ------------------------------------------------------*/
    if( test_bits( usage, VK_IMAGE_USAGE_TRANSFER_DST_BIT ) )
        {
        has_support &= test_bits( support->props.optimalTilingFeatures, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT );
        }

    if( has_support )
        {
        ret = support->format;
        break;
        }
    }

return( ret );

}   /* get_image_format() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       get_type_of_image_view
*
*********************************************************************/

static VkImageViewType get_type_of_image_view
    (
    const VkImageViewType
                        requested,  /* user requested view          */
    const VkImageType   image,      /* image type                   */
    const VkImageCreateFlags
                        flags,      /* creation flags               */
    const VkExtent3D    extent,     /* extent of each layer         */
    const u32           layer_cnt   /* number of image layers       */
    )
{
/*----------------------------------------------------------
Local types
----------------------------------------------------------*/
typedef struct
    {
    VkImageViewType     view;       /* type of image view           */
    VkImageType         image;      /* type of image                */
    } view_image_pair_type;

/*----------------------------------------------------------
Local constants
----------------------------------------------------------*/
static const view_image_pair_type COMPATIBLE[] =
    {
    { VK_IMAGE_VIEW_TYPE_1D,         VK_IMAGE_TYPE_1D },
    { VK_IMAGE_VIEW_TYPE_1D_ARRAY,   VK_IMAGE_TYPE_1D },
    { VK_IMAGE_VIEW_TYPE_2D,         VK_IMAGE_TYPE_2D },
    { VK_IMAGE_VIEW_TYPE_2D,         VK_IMAGE_TYPE_3D },
    { VK_IMAGE_VIEW_TYPE_2D_ARRAY,   VK_IMAGE_TYPE_2D },
    { VK_IMAGE_VIEW_TYPE_2D_ARRAY,   VK_IMAGE_TYPE_3D },
    { VK_IMAGE_VIEW_TYPE_CUBE,       VK_IMAGE_TYPE_2D },
    { VK_IMAGE_VIEW_TYPE_CUBE_ARRAY, VK_IMAGE_TYPE_2D },
    { VK_IMAGE_VIEW_TYPE_3D,         VK_IMAGE_TYPE_3D },
    };

/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkImageViewType         fallback;   /* if requested is invalid      */
u32                     i;          /* loop counter                 */
VkImageViewType         ret;        /* return image view type       */

/*----------------------------------------------------------
Determine a fallback if user requested incompatible view for
this image
----------------------------------------------------------*/
switch( image )
    {
    case VK_IMAGE_TYPE_1D:
        fallback = VK_IMAGE_VIEW_TYPE_1D;
        break;

    case VK_IMAGE_TYPE_2D:
        fallback = VK_IMAGE_VIEW_TYPE_2D;
        break;
    case VK_IMAGE_TYPE_3D:
        fallback = VK_IMAGE_VIEW_TYPE_3D;
        break;
    default:
        fallback = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
        break;
    }

if( fallback == VK_IMAGE_VIEW_TYPE_MAX_ENUM )
    {
    /*------------------------------------------------------
    Invalid extent or image type
    ------------------------------------------------------*/
    debug_assert_always();
    return( VK_IMAGE_VIEW_TYPE_MAX_ENUM );
    }

/*----------------------------------------------------------
If no user request, default to fallback
----------------------------------------------------------*/
if( requested == VK_IMAGE_VIEW_TYPE_MAX_ENUM )
    {
    return( fallback );
    }

/*----------------------------------------------------------
Validate the request
----------------------------------------------------------*/
ret = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
for( i = 0; i < cnt_of_array( COMPATIBLE ); i++ )
    {
    if( COMPATIBLE[ i ].view == requested
     && COMPATIBLE[ i ].image == image )
        {
        ret = requested;
        break;
        }
    }

if( ret == VK_IMAGE_VIEW_TYPE_MAX_ENUM )
    {
    debug_assert_always();
    ret = fallback;
    }

return( ret );

}   /* get_type_of_image_view() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       load
*
*********************************************************************/

static void load
    (
    const void         *data,       /* raw linear image data        */
    const u32           offset_x,   /* x offset in data buffer      */
    const u32           offset_y,   /* y offset in data buffer      */
    const u32           width,      /* data buffer width            */
    const u32           height,     /* data buffer height           */
    VKN_staging_type   *staging,    /* staging buffer               */
    struct _VKN_image_type
                       *image       /* image to load                */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkBufferImageCopy       copy;       /* copy definition              */
VkExtent2D              copy_extent;/* mip copy extent              */
u32                     i;          /* loop counter                 */
VKN_staging_upload_instruct_type
                        instruct;   /* upload instructions          */
u32                     size;       /* copy size                    */
VkImageMemoryBarrier    to_read;    /* transition to shader read    */
VkImageMemoryBarrier    to_transfer;/* transition to transfer target*/

debug_assert( offset_x + width  >= image->extent.width );
debug_assert( offset_y + height >= image->extent.height );

/*----------------------------------------------------------
Upload the image to staging
----------------------------------------------------------*/
size = width * height * get_format_bit_count( image->format ) / 8;
instruct = staging->i->upload( size, image->upload_alignment, staging );
memcpy( instruct.mapping, data, size );

/*----------------------------------------------------------
Transition image to transfer target
----------------------------------------------------------*/
clr_struct( &to_transfer );
to_transfer.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
to_transfer.srcAccessMask                   = VK_ACCESS_SHADER_READ_BIT;
to_transfer.dstAccessMask                   = VK_ACCESS_TRANSFER_WRITE_BIT;
to_transfer.oldLayout                       = VK_IMAGE_LAYOUT_UNDEFINED;
to_transfer.newLayout                       = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
to_transfer.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
to_transfer.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
to_transfer.image                           = image->image;
to_transfer.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
to_transfer.subresourceRange.baseMipLevel   = 0;
to_transfer.subresourceRange.levelCount     = image->mip_levels;
to_transfer.subresourceRange.baseArrayLayer = 0;
to_transfer.subresourceRange.layerCount     = 1;

vkCmdPipelineBarrier( instruct.commands,
                      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                      VK_PIPELINE_STAGE_TRANSFER_BIT,
                      0, /* flags */
                      0,
                      NULL,
                      0,
                      NULL,
                      1,
                      &to_transfer );

/*----------------------------------------------------------
Copy image from staging to image memory
----------------------------------------------------------*/
copy_extent.width  = image->extent.width;
copy_extent.height = image->extent.height;

for( i = 0; i < image->mip_levels; i++ )
    {
    clr_struct( &copy );
    copy.bufferOffset                    = instruct.offset;
    copy.bufferRowLength                 = width;
    copy.bufferImageHeight               = height;
    copy.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    copy.imageSubresource.mipLevel       = i;
    copy.imageSubresource.baseArrayLayer = 0;
    copy.imageSubresource.layerCount     = 1;
    copy.imageOffset.x                   = 0;
    copy.imageOffset.y                   = 0;
    copy.imageExtent.width               = copy_extent.width;
    copy.imageExtent.height              = copy_extent.height;
    copy.imageExtent.depth               = 1;

    vkCmdCopyBufferToImage( instruct.commands,
                            instruct.buffer,
                            image->image,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            1,
                            &copy );

    copy_extent.width  >>= 1;
    copy_extent.height >>= 1;
    }

/*----------------------------------------------------------
Transition image to shader read resource
----------------------------------------------------------*/
clr_struct( &to_read );
to_read.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
to_read.srcAccessMask                   = VK_ACCESS_TRANSFER_WRITE_BIT;
to_read.dstAccessMask                   = VK_ACCESS_SHADER_READ_BIT;
to_read.oldLayout                       = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
to_read.newLayout                       = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
to_read.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
to_read.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
to_read.image                           = image->image;
to_read.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
to_read.subresourceRange.baseMipLevel   = 0;
to_read.subresourceRange.levelCount     = image->mip_levels;
to_read.subresourceRange.baseArrayLayer = 0;
to_read.subresourceRange.layerCount     = 1;

vkCmdPipelineBarrier( instruct.commands,
                      VK_PIPELINE_STAGE_TRANSFER_BIT,
                      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                      0, /* flags */
                      0,
                      NULL,
                      0,
                      NULL,
                      1,
                      &to_read );

}   /* load() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       reset
*
*********************************************************************/

static VKN_IMAGE_CONFIG_API reset
    (
    struct _VKN_image_build_type
                       *builder     /* image builder                */
    )
{
builder->state.extent.width      = 0;
builder->state.extent.height     = 0;
builder->state.format            = VKN_IMAGE_DATA_FORMAT_UNDEFINED;
builder->state.generate_mip_maps = FALSE;
builder->state.msaa              = VKN_MSAA_OFF;
builder->state.usage             = VKN_IMAGE_USAGE_SAMPLE_ONLY;
builder->state.view              = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
builder->state.filter_min        = VK_FILTER_NEAREST;
builder->state.filter_mag        = VK_FILTER_NEAREST;
builder->state.mipmode           = VK_SAMPLER_MIPMAP_MODE_NEAREST;
builder->state.address_u         = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
builder->state.address_v         = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
builder->state.use_aniso         = VK_FALSE;
builder->state.aniso_max         = 0.0f;
clr_struct( &builder->state.families );

return( builder->config );

}   /* reset() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_addressing
*
*********************************************************************/

static VKN_IMAGE_CONFIG_API set_addressing
    (
    VkSamplerAddressMode
                        u,          /* horizontal addressing        */
    VkSamplerAddressMode
                        v,          /* vertical addressing          */
    struct _VKN_image_build_type
                       *builder     /* image builder                */
    )
{
builder->state.address_u = u;
builder->state.address_v = v;

return( builder->config );

}   /* set_addressing() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_allocation_callbacks
*
*********************************************************************/

static VKN_IMAGE_CONFIG_API set_allocation_callbacks
    (
    VkAllocationCallbacks
                       *allocator,  /* custom allocator             */
    struct _VKN_image_build_type
                       *builder     /* image builder                */
    )
{
builder->state.allocator = allocator;

return( builder->config );

}   /* set_allocation_callbacks() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_extent
*
*********************************************************************/

static VKN_IMAGE_CONFIG_API set_extent
    (
    const u32           width,      /* image width                  */
    const u32           height,     /* image height                 */
    struct _VKN_image_build_type
                       *builder     /* image builder                */
    )
{
builder->state.extent.width  = width;
builder->state.extent.height = height;

return( builder->config );

}   /* set_extent() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_filter
*
*********************************************************************/

static VKN_IMAGE_CONFIG_API set_filter
    (
    const VkFilter      minify,     /* minification filter          */
    const VkFilter      magnify,    /* magnification filter         */
    struct _VKN_image_build_type
                       *builder     /* image builder                */
    )
{
builder->state.filter_min = minify;
builder->state.filter_mag = magnify;

return( builder->config );

}   /* set_filter() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_image_data_format
*
*********************************************************************/

static VKN_IMAGE_CONFIG_API set_image_data_format
    (
    const VKN_image_data_format_type
                        format,     /* image data format            */
    struct _VKN_image_build_type
                       *builder     /* image builder                */
    )
{
builder->state.format = format;

return( builder->config );

}   /* set_image_data_format() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_image_view
*
*********************************************************************/

static VKN_IMAGE_CONFIG_API set_image_view
    (
    const VkImageViewType
                        view,       /* type of image view           */
    struct _VKN_image_build_type
                       *builder     /* image builder                */
    )
{
builder->state.view = view;

return( builder->config );

}   /* set_image_view() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_msaa
*
*********************************************************************/

static VKN_IMAGE_CONFIG_API set_msaa
    (
    const VKN_msaa_type msaa,       /* multi-sampling level         */
    struct _VKN_image_build_type
                       *builder     /* image builder                */
    )
{
builder->state.msaa = msaa;

return( builder->config );

}   /* set_msaa() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_usage
*
*********************************************************************/

static VKN_IMAGE_CONFIG_API set_usage
    (
    const VKN_image_usage_type
                        usage,      /* image usage                  */
    struct _VKN_image_build_type
                       *builder     /* image builder                */
    )
{
builder->state.usage = usage;

return( builder->config );

}   /* set_usage() */
