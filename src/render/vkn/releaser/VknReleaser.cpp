#include "Global.hpp"
#include "Utilities.hpp"

#include "VknCommon.hpp"
#include "VknReleaser.hpp"
#include "VknReleaserTypes.hpp"


static VKN_releaser_flush_proc_type flush;
static VKN_releaser_release_buffer_proc_type release_buffer;
static VKN_releaser_release_command_buffer_proc_type release_command_buffer;
static VKN_releaser_release_command_pool_proc_type release_command_pool;
static VKN_releaser_release_debug_messenger_proc_type release_debug_messenger;
static VKN_releaser_release_descriptor_pool_proc_type release_descriptor_pool;
static VKN_releaser_release_descriptor_set_layout_proc_type release_descriptor_set_layout;
static VKN_releaser_release_device_proc_type release_device;
static VKN_releaser_release_fence_proc_type release_fence;
static VKN_releaser_release_image_proc_type release_image;
static VKN_releaser_release_image_view_proc_type release_image_view;
static VKN_releaser_release_instance_proc_type release_instance;
static VKN_releaser_release_memory_proc_type release_memory;
static VKN_releaser_release_pipeline_proc_type release_pipeline;
static VKN_releaser_release_pipeline_layout_proc_type release_pipeline_layout;
static VKN_releaser_release_sampler_proc_type release_sampler;
static VKN_releaser_release_semaphore_proc_type release_semaphore;
static VKN_releaser_release_shader_module_proc_type release_shader_module;
static VKN_releaser_release_surface_proc_type release_surface;
static VKN_releaser_release_swap_chain_proc_type release_swap_chain;


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_releaser_create
*
*   DESCRIPTION:
*       Create a deferred releaser.
*
*********************************************************************/

void VKN_releaser_create
    (
    const u32           capacity,   /* number of variants in pool   */
    VKN_releaser_variant_type
                       *pool,       /* variant pool                 */
    VKN_releaser_type  *releaser    /* output new releaser          */
    )
{
/*----------------------------------------------------------
Local constants
----------------------------------------------------------*/
static const VKN_releaser_api_type API =
    {
    flush,
    release_buffer,
    release_command_buffer,
    release_command_pool,
    release_debug_messenger,
    release_descriptor_pool,
    release_descriptor_set_layout,
    release_device,
    release_fence,
    release_image,
    release_image_view,
    release_instance,
    release_memory,
    release_pipeline,
    release_pipeline_layout,
    release_sampler,
    release_semaphore,
    release_shader_module,
    release_surface,
    release_swap_chain
    };

/*----------------------------------------------------------
Create the releaser
----------------------------------------------------------*/
clr_struct( releaser );
releaser->i        = &API;
releaser->vars     = pool;
releaser->capacity = capacity;

}   /* VKN_releaser_create() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_releaser_destroy
*
*   DESCRIPTION:
*       Destroy the given releaser.
*
*********************************************************************/

void VKN_releaser_destroy
    (
    VKN_releaser_type  *releaser    /* releaser to destroy          */
    )
{
releaser->i->flush( releaser );
clr_struct( releaser );

}   /* VKN_releaser_destroy() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       flush
*
*********************************************************************/

static VKN_RELEASER_API flush
    (
    struct _VKN_releaser_type
                       *releaser
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_releaser_variant_type
                       *entry;      /* next entry                   */

for( ; releaser->count; releaser->count-- )
    {
    entry = &releaser->vars[ releaser->count - 1 ];
    switch( entry->tag )
        {
        case VK_OBJECT_TYPE_BUFFER:
            VKN_release_buffer( entry->args.buffer.logical, entry->args.buffer.allocator, &entry->args.buffer.buffer );
            break;

        case VK_OBJECT_TYPE_COMMAND_BUFFER:
            VKN_release_command_buffer( entry->args.command_buffer.logical, entry->args.command_buffer.pool, &entry->args.command_buffer.buffer );
            break;

        case VK_OBJECT_TYPE_COMMAND_POOL:
            VKN_release_command_pool( entry->args.command_pool.logical, entry->args.command_pool.allocator, &entry->args.command_pool.pool );
            break;

        case VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT:
            VKN_release_debug_messenger( entry->args.debug_messenger.instance, entry->args.debug_messenger.allocator, &entry->args.debug_messenger.messenger );
            break;

        case VK_OBJECT_TYPE_DESCRIPTOR_POOL:
            VKN_release_descriptor_pool( entry->args.descriptor_pool.logical, entry->args.descriptor_pool.allocator, &entry->args.descriptor_pool.pool );
            break;

        case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT:
            VKN_release_descriptor_set_layout( entry->args.descriptor_set_layout.logical, entry->args.descriptor_set_layout.allocator, &entry->args.descriptor_set_layout.layout );
            break;

        case VK_OBJECT_TYPE_DEVICE:
            VKN_release_device( entry->args.device.allocator, &entry->args.device.device );
            break;

        case VK_OBJECT_TYPE_DEVICE_MEMORY:
            VKN_release_memory( entry->args.memory.logical, entry->args.memory.allocator, &entry->args.memory.memory );
            break;

        case VK_OBJECT_TYPE_FENCE:
            VKN_release_fence( entry->args.fence.logical, entry->args.fence.allocator, &entry->args.fence.fence );
            break;

        case VK_OBJECT_TYPE_IMAGE:
            VKN_release_image( entry->args.image.logical, entry->args.image.allocator, &entry->args.image.image );
            break;

        case VK_OBJECT_TYPE_IMAGE_VIEW:
            VKN_release_image_view( entry->args.image_view.logical, entry->args.image_view.allocator, &entry->args.image_view.view );
            break;

        case VK_OBJECT_TYPE_INSTANCE:
            VKN_release_instance( entry->args.instance.allocator, &entry->args.instance.instance );
            break;

        case VK_OBJECT_TYPE_PIPELINE:
            VKN_release_pipeline( entry->args.pipeline.logical, entry->args.pipeline.allocator, &entry->args.pipeline.pipeline );
            break;

        case VK_OBJECT_TYPE_PIPELINE_LAYOUT:
            VKN_release_pipeline_layout( entry->args.pipeline_layout.logical, entry->args.pipeline_layout.allocator, &entry->args.pipeline_layout.layout );
            break;

        case VK_OBJECT_TYPE_SAMPLER:
            VKN_release_sampler( entry->args.sampler.logical, entry->args.sampler.allocator, &entry->args.sampler.sampler );
            break;

        case VK_OBJECT_TYPE_SEMAPHORE:
            VKN_release_semaphore( entry->args.semaphore.logical, entry->args.semaphore.allocator, &entry->args.semaphore.semaphore );
            break;

        case VK_OBJECT_TYPE_SHADER_MODULE:
            VKN_release_shader_module( entry->args.shader_module.logical, entry->args.shader_module.allocator, &entry->args.shader_module.shader );
            break;

        case VK_OBJECT_TYPE_SURFACE_KHR:
            VKN_release_surface( entry->args.surface.instance, entry->args.surface.allocator, &entry->args.surface.surface );
            break;

        case VK_OBJECT_TYPE_SWAPCHAIN_KHR:
            VKN_release_swap_chain( entry->args.swap_chain.logical, entry->args.swap_chain.allocator, &entry->args.swap_chain.swap_chain );
            break;

         default:
            debug_assert_always();
            break;
        }

    clr_struct( entry );
    }

return( releaser->i );

}   /* flush() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       release_buffer
*
*********************************************************************/

static VKN_RELEASER_API release_buffer
    (
    const VkDevice      logical,
    const VkAllocationCallbacks
                       *allocator,
    const VkBuffer      buffer,
    struct _VKN_releaser_type
                       *releaser
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_releaser_variant_type
                       *entry;      /* next entry                   */

if( releaser->count >= releaser->capacity )
    {
    debug_assert_always();
    return( releaser->i );
    }

entry = &releaser->vars[ releaser->count++ ];
entry->tag                   = VK_OBJECT_TYPE_BUFFER;
entry->args.buffer.logical   = logical;
entry->args.buffer.allocator = allocator;
entry->args.buffer.buffer    = buffer;

return( releaser->i );

}   /* release_buffer() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       release_command_buffer
*
*********************************************************************/

static VKN_RELEASER_API release_command_buffer
    (
    const VkDevice      logical,
    const VkCommandPool pool,
    const VkCommandBuffer
                        buffer,
    struct _VKN_releaser_type
                       *releaser
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_releaser_variant_type
                       *entry;      /* next entry                   */

if( releaser->count >= releaser->capacity )
    {
    debug_assert_always();
    return( releaser->i );
    }

entry = &releaser->vars[ releaser->count++ ];
entry->tag                         = VK_OBJECT_TYPE_COMMAND_BUFFER;
entry->args.command_buffer.logical = logical;
entry->args.command_buffer.pool    = pool;
entry->args.command_buffer.buffer  = buffer;

return( releaser->i );

}   /* release_command_buffer() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       release_command_pool
*
*********************************************************************/

static VKN_RELEASER_API release_command_pool
    (
    const VkDevice      logical,
    const VkAllocationCallbacks
                       *allocator,
    const VkCommandPool pool,
    struct _VKN_releaser_type
                       *releaser
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_releaser_variant_type
                       *entry;      /* next entry                   */

if( releaser->count >= releaser->capacity )
    {
    debug_assert_always();
    return( releaser->i );
    }

entry = &releaser->vars[ releaser->count++ ];
entry->tag                         = VK_OBJECT_TYPE_COMMAND_POOL;
entry->args.command_pool.logical   = logical;
entry->args.command_pool.allocator = allocator;
entry->args.command_pool.pool      = pool;

return( releaser->i );

}   /* release_command_pool() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       release_debug_messenger
*
*********************************************************************/

static VKN_RELEASER_API release_debug_messenger
    (
    const VkInstance    instance,
    const VkAllocationCallbacks
                       *allocator,
    const VkDebugUtilsMessengerEXT
                        messenger,
    struct _VKN_releaser_type
                       *releaser
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_releaser_variant_type
                       *entry;      /* next entry                   */

if( releaser->count >= releaser->capacity )
    {
    debug_assert_always();
    return( releaser->i );
    }

entry = &releaser->vars[ releaser->count++ ];
entry->tag                            = VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT;
entry->args.debug_messenger.instance  = instance;
entry->args.debug_messenger.allocator = allocator;
entry->args.debug_messenger.messenger = messenger;

return( releaser->i );

}   /* release_debug_messenger() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       release_descriptor_pool
*
*********************************************************************/

static VKN_RELEASER_API release_descriptor_pool
    (
    const VkDevice      logical,
    const VkAllocationCallbacks
                       *allocator,
    const VkDescriptorPool
                        pool,
    struct _VKN_releaser_type
                       *releaser
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_releaser_variant_type
                       *entry;      /* next entry                   */

if( releaser->count >= releaser->capacity )
    {
    debug_assert_always();
    return( releaser->i );
    }

entry = &releaser->vars[ releaser->count++ ];
entry->tag                            = VK_OBJECT_TYPE_DESCRIPTOR_POOL;
entry->args.descriptor_pool.logical   = logical;
entry->args.descriptor_pool.allocator = allocator;
entry->args.descriptor_pool.pool      = pool;

return( releaser->i );

}   /* release_descriptor_pool() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       release_descriptor_set_layout
*
*********************************************************************/

static VKN_RELEASER_API release_descriptor_set_layout
    (
    const VkDevice      logical,
    const VkAllocationCallbacks
                       *allocator,
    const VkDescriptorSetLayout
                        layout,
    struct _VKN_releaser_type
                       *releaser
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_releaser_variant_type
                       *entry;      /* next entry                   */

if( releaser->count >= releaser->capacity )
    {
    debug_assert_always();
    return( releaser->i );
    }

entry = &releaser->vars[ releaser->count++ ];
entry->tag                                  = VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT;
entry->args.descriptor_set_layout.logical   = logical;
entry->args.descriptor_set_layout.allocator = allocator;
entry->args.descriptor_set_layout.layout    = layout;

return( releaser->i );

}   /* release_descriptor_set_layout() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       release_device
*
*********************************************************************/

static VKN_RELEASER_API release_device
    (
    const VkAllocationCallbacks
                       *allocator,
    const VkDevice      device,
    struct _VKN_releaser_type
                       *releaser
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_releaser_variant_type
                       *entry;      /* next entry                   */

if( releaser->count >= releaser->capacity )
    {
    debug_assert_always();
    return( releaser->i );
    }

entry = &releaser->vars[ releaser->count++ ];
entry->tag                   = VK_OBJECT_TYPE_DEVICE;
entry->args.device.allocator = allocator;
entry->args.device.device    = device;

return( releaser->i );

}   /* release_device() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       release_fence
*
*********************************************************************/

static VKN_RELEASER_API release_fence
    (
    const VkDevice      logical,
    const VkAllocationCallbacks
                       *allocator,
    const VkFence       fence,
    struct _VKN_releaser_type
                       *releaser
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_releaser_variant_type
                       *entry;      /* next entry                   */

if( releaser->count >= releaser->capacity )
    {
    debug_assert_always();
    return( releaser->i );
    }

entry = &releaser->vars[ releaser->count++ ];
entry->tag                  = VK_OBJECT_TYPE_FENCE;
entry->args.fence.logical   = logical;
entry->args.fence.allocator = allocator;
entry->args.fence.fence     = fence;

return( releaser->i );

}   /* release_fence() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       release_image
*
*********************************************************************/

static VKN_RELEASER_API release_image
    (
    const VkDevice      logical,
    const VkAllocationCallbacks
                       *allocator,
    const VkImage       image,
    struct _VKN_releaser_type
                       *releaser
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_releaser_variant_type
                       *entry;      /* next entry                   */

if( releaser->count >= releaser->capacity )
    {
    debug_assert_always();
    return( releaser->i );
    }

entry = &releaser->vars[ releaser->count++ ];
entry->tag                  = VK_OBJECT_TYPE_IMAGE;
entry->args.image.logical   = logical;
entry->args.image.allocator = allocator;
entry->args.image.image     = image;

return( releaser->i );

}   /* release_image() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       release_image_view
*
*********************************************************************/

static VKN_RELEASER_API release_image_view
    (
    const VkDevice      logical,
    const VkAllocationCallbacks
                       *allocator,
    const VkImageView   view,
    struct _VKN_releaser_type
                       *releaser
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_releaser_variant_type
                       *entry;      /* next entry                   */

if( releaser->count >= releaser->capacity )
    {
    debug_assert_always();
    return( releaser->i );
    }

entry = &releaser->vars[ releaser->count++ ];
entry->tag                       = VK_OBJECT_TYPE_IMAGE_VIEW;
entry->args.image_view.logical   = logical;
entry->args.image_view.allocator = allocator;
entry->args.image_view.view      = view;

return( releaser->i );

}   /* release_image_view() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       release_instance
*
*********************************************************************/

static VKN_RELEASER_API release_instance
    (
    const VkAllocationCallbacks
                       *allocator,
    const VkInstance    instance,
    struct _VKN_releaser_type
                       *releaser
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_releaser_variant_type
                       *entry;      /* next entry                   */

if( releaser->count >= releaser->capacity )
    {
    debug_assert_always();
    return( releaser->i );
    }

entry = &releaser->vars[ releaser->count++ ];
entry->tag                     = VK_OBJECT_TYPE_INSTANCE;
entry->args.instance.allocator = allocator;
entry->args.instance.instance  = instance;

return( releaser->i );

}   /* release_instance() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       release_memory
*
*********************************************************************/

static VKN_RELEASER_API release_memory
    (
    const VkDevice      logical,
    const VkAllocationCallbacks
                       *allocator,
    const VkDeviceMemory
                        memory,
    struct _VKN_releaser_type
                       *releaser
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_releaser_variant_type
                       *entry;      /* next entry                   */

if( releaser->count >= releaser->capacity )
    {
    debug_assert_always();
    return( releaser->i );
    }

entry = &releaser->vars[ releaser->count++ ];
entry->tag                   = VK_OBJECT_TYPE_DEVICE_MEMORY;
entry->args.memory.logical   = logical;
entry->args.memory.allocator = allocator;
entry->args.memory.memory    = memory;

return( releaser->i );

}   /* release_pipeline() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       release_pipeline
*
*********************************************************************/

static VKN_RELEASER_API release_pipeline
    (
    const VkDevice      logical,
    const VkAllocationCallbacks
                       *allocator,
    const VkPipeline    pipeline,
    struct _VKN_releaser_type
                       *releaser
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_releaser_variant_type
                       *entry;      /* next entry                   */

if( releaser->count >= releaser->capacity )
    {
    debug_assert_always();
    return( releaser->i );
    }

entry = &releaser->vars[ releaser->count++ ];
entry->tag                     = VK_OBJECT_TYPE_PIPELINE;
entry->args.pipeline.logical   = logical;
entry->args.pipeline.allocator = allocator;
entry->args.pipeline.pipeline  = pipeline;

return( releaser->i );

}   /* release_pipeline() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       release_pipeline_layout
*
*********************************************************************/

static VKN_RELEASER_API release_pipeline_layout
    (
    const VkDevice      logical,
    const VkAllocationCallbacks
                       *allocator,
    const VkPipelineLayout
                        layout,
    struct _VKN_releaser_type
                       *releaser
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_releaser_variant_type
                       *entry;      /* next entry                   */

if( releaser->count >= releaser->capacity )
    {
    debug_assert_always();
    return( releaser->i );
    }

entry = &releaser->vars[ releaser->count++ ];
entry->tag                            = VK_OBJECT_TYPE_PIPELINE_LAYOUT;
entry->args.pipeline_layout.logical   = logical;
entry->args.pipeline_layout.allocator = allocator;
entry->args.pipeline_layout.layout    = layout;

return( releaser->i );

}   /* release_pipeline_layout() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       release_sampler
*
*********************************************************************/

static VKN_RELEASER_API release_sampler
    (
    const VkDevice      logical,
    const VkAllocationCallbacks
                       *allocator,
    const VkSampler     sampler,
    struct _VKN_releaser_type
                       *releaser
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_releaser_variant_type
                       *entry;      /* next entry                   */

if( releaser->count >= releaser->capacity )
    {
    debug_assert_always();
    return( releaser->i );
    }

entry = &releaser->vars[ releaser->count++ ];
entry->tag                    = VK_OBJECT_TYPE_SAMPLER;
entry->args.sampler.logical   = logical;
entry->args.sampler.allocator = allocator;
entry->args.sampler.sampler   = sampler;

return( releaser->i );

}   /* release_sampler() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       release_semaphore
*
*********************************************************************/

static VKN_RELEASER_API release_semaphore
    (
    const VkDevice      logical,
    const VkAllocationCallbacks
                       *allocator,
    const VkSemaphore   semaphore,
    struct _VKN_releaser_type
                       *releaser
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_releaser_variant_type
                       *entry;      /* next entry                   */

if( releaser->count >= releaser->capacity )
    {
    debug_assert_always();
    return( releaser->i );
    }

entry = &releaser->vars[ releaser->count++ ];
entry->tag                      = VK_OBJECT_TYPE_SEMAPHORE;
entry->args.semaphore.logical   = logical;
entry->args.semaphore.allocator = allocator;
entry->args.semaphore.semaphore = semaphore;

return( releaser->i );

}   /* release_semaphore() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       release_shader_module
*
*********************************************************************/

static VKN_RELEASER_API release_shader_module
    (
    const VkDevice      logical,
    const VkAllocationCallbacks
                       *allocator,
    const VkShaderModule
                        shader,
    struct _VKN_releaser_type
                       *releaser
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_releaser_variant_type
                       *entry;      /* next entry                   */

if( releaser->count >= releaser->capacity )
    {
    debug_assert_always();
    return( releaser->i );
    }

entry = &releaser->vars[ releaser->count++ ];
entry->tag                          = VK_OBJECT_TYPE_SHADER_MODULE;
entry->args.shader_module.logical   = logical;
entry->args.shader_module.allocator = allocator;
entry->args.shader_module.shader    = shader;

return( releaser->i );

}   /* release_shader_module() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       release_surface
*
*********************************************************************/

static VKN_RELEASER_API release_surface
    (
    const VkInstance    instance,
    const VkAllocationCallbacks
                       *allocator,
    const VkSurfaceKHR  surface,
    struct _VKN_releaser_type
                       *releaser
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_releaser_variant_type
                       *entry;      /* next entry                   */

if( releaser->count >= releaser->capacity )
    {
    debug_assert_always();
    return( releaser->i );
    }

entry = &releaser->vars[ releaser->count++ ];
entry->tag                    = VK_OBJECT_TYPE_SURFACE_KHR;
entry->args.surface.instance  = instance;
entry->args.surface.allocator = allocator;
entry->args.surface.surface   = surface;

return( releaser->i );

}   /* release_surface() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       release_swap_chain
*
*********************************************************************/

static VKN_RELEASER_API release_swap_chain
    (
    const VkDevice      logical,
    const VkAllocationCallbacks
                       *allocator,
    const VkSwapchainKHR
                        swap_chain,
    struct _VKN_releaser_type
                       *releaser
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_releaser_variant_type
                       *entry;      /* next entry                   */

if( releaser->count >= releaser->capacity )
    {
    debug_assert_always();
    return( releaser->i );
    }

entry = &releaser->vars[ releaser->count++ ];
entry->tag                        = VK_OBJECT_TYPE_SWAPCHAIN_KHR;
entry->args.swap_chain.logical    = logical;
entry->args.swap_chain.allocator  = allocator;
entry->args.swap_chain.swap_chain = swap_chain;

return( releaser->i );

}   /* release_swap_chain() */