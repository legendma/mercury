#include "Global.hpp"
#include "Utilities.hpp"

#include "VknCommon.hpp"
#include "VknReleaser.hpp"
#include "VknStaging.hpp"


static VKN_staging_build_add_sharing_family_proc_type add_sharing_family;

static void add_sharing_family_safe
    (
    const u32           index,      /* family index to share with   */
    VKN_staging_build_type
                       *builder     /* staging builder              */
    );

static void block_for_frame
    (
    VKN_staging_frame_type
                       *frame,      /* frame to block for           */
    VKN_staging_type   *staging     /* resource staging             */
    );

static VKN_staging_flush_proc_type flush;
static VKN_staging_build_set_frame_size_proc_type set_frame_size;
static VKN_staging_upload_proc_type upload;


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_staging_create
*
*   DESCRIPTION:
*       Create a resource staging via the given builder.
*
*********************************************************************/

bool VKN_staging_create
    (
    const VKN_staging_build_type
                       *builder,    /* resource staging builder     */
    VKN_staging_type   *staging     /* output new resource staging  */
    )
{
/*----------------------------------------------------------
Local constants
----------------------------------------------------------*/
static const VKN_staging_api_type API =
    {
    flush,
    upload
    };

/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkCommandBufferBeginInfo
                        begin_info; /* begin command buffer         */
VkBufferCreateInfo      ci_buffer;  /* buffer create info           */
VkCommandPoolCreateInfo ci_pool;    /* command pool create info     */
VkCommandBufferAllocateInfo
                        ci_command; /* command buffer create info   */
VkFenceCreateInfo       ci_fence;   /* fence create info            */
VKN_staging_frame_type *frame;      /* working frame                */
u32                     i;          /* loop counter                 */

/*----------------------------------------------------------
Create the resource staging object
----------------------------------------------------------*/
clr_struct( staging );
staging->i = &API;

staging->state.logical         = builder->state.logical;
staging->state.allocator       = builder->state.allocator;
staging->state.queue           = builder->state.queue;
staging->state.queue_index     = builder->state.queue_index;
staging->state.memory          = builder->state.memory;
staging->state.buffer_frame_sz = builder->state.buffer_frame_sz;

/*----------------------------------------------------------
Create command pool
----------------------------------------------------------*/
clr_struct( &ci_pool );
ci_pool.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
ci_pool.queueFamilyIndex = builder->state.queue_index;
ci_pool.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

if( VKN_failed( vkCreateCommandPool( builder->state.logical, &ci_pool, builder->state.allocator, &staging->state.command_pool ) ) )
    {
    VKN_staging_destroy( NULL, staging );
    return( FALSE );
    }

/*----------------------------------------------------------
Create frames
----------------------------------------------------------*/
clr_struct( &ci_buffer );
ci_buffer.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
ci_buffer.usage                 = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
ci_buffer.size                  = builder->state.buffer_frame_sz;
ci_buffer.pQueueFamilyIndices   = builder->state.families.indices;
ci_buffer.queueFamilyIndexCount = builder->state.families.count;
ci_buffer.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
if( builder->state.families.count > 1 )
    {
    ci_buffer.sharingMode = VK_SHARING_MODE_CONCURRENT;
    }

clr_struct( &ci_command );
ci_command.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
ci_command.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
ci_command.commandBufferCount = 1;
ci_command.commandPool        = staging->state.command_pool;

clr_struct( &ci_fence );
ci_fence.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

clr_struct( &begin_info );
begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

for( i = 0; i < cnt_of_array( staging->state.frames ); i++ )
    {
    frame = &staging->state.frames[ i ];

    /*----------------------------------------------------------
    Command buffer, fence, and upload buffer
    ----------------------------------------------------------*/
    if( VKN_failed( vkAllocateCommandBuffers( staging->state.logical, &ci_command, &frame->commands ) )
     || VKN_failed( vkCreateFence( staging->state.logical, &ci_fence, staging->state.allocator, &frame->fence ) )
     || VKN_failed( vkCreateBuffer( staging->state.logical, &ci_buffer, staging->state.allocator, &frame->buffer ) )
     || !staging->state.memory->i->create_buffer_memory( frame->buffer, VKN_MEMORY_HEAP_USAGE_UPLOAD, staging->state.memory, &frame->allocation ) )
        {
        VKN_staging_destroy( NULL, staging );
        return( FALSE );
        }

    vkBeginCommandBuffer( staging->state.frames[ i ].commands, &begin_info );
    VKN_name_object( staging->state.logical, frame->commands, VK_OBJECT_TYPE_COMMAND_BUFFER, "Staging_CmdBuff[%d]", i );
    VKN_name_object( staging->state.logical, frame->fence,    VK_OBJECT_TYPE_FENCE,          "Staging_Fence[%d]", i );
    VKN_name_object( staging->state.logical, frame->buffer,   VK_OBJECT_TYPE_BUFFER,         "Staging_Buffer[%d]", i );
    }

return( TRUE );

}   /* VKN_staging_create() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_staging_destroy
*
*   DESCRIPTION:
*       Destroy the given resource staging.
*
*********************************************************************/

void VKN_staging_destroy
    (
    VKN_releaser_type  *releaser,   /* release buffer               */
    VKN_staging_type   *staging     /* resource staging to destroy  */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_staging_frame_type *frame;      /* working frame                */
u32                     i;          /* loop counter                 */

VKN_releaser_auto_mini_begin( releaser, use );
use->i->release_command_pool( staging->state.logical, staging->state.allocator, staging->state.command_pool, use );
for( i = 0; i < cnt_of_array( staging->state.frames ); i++ )
    {
    frame = &staging->state.frames[ i ];
    use->i->release_command_buffer( staging->state.logical, staging->state.command_pool, frame->commands, use )->
            release_fence( staging->state.logical, staging->state.allocator, frame->fence, use )->
            release_buffer( staging->state.logical, staging->state.allocator, frame->buffer, use );

    block_for_frame( frame, staging );
    if( staging->state.memory
     && staging->state.memory->i )
        {
        staging->state.memory->i->deallocate( staging->state.memory, &frame->allocation );
        }
    }

VKN_releaser_auto_mini_end( use );
clr_struct( staging );

}   /* VKN_staging_destroy() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_staging_init_builder
*
*   DESCRIPTION:
*       Initialize a resource staging builder.
*
*********************************************************************/

VKN_STAGING_CONFIG_API VKN_staging_init_builder
    (
    const VkDevice      logical,    /* associated logical device    */
    const VkQueue       queue,      /* queue on which to submit     */
    const u32           queue_index,/* family index of submit queue */
    VKN_memory_type    *memory,     /* memory allocator             */
    VKN_staging_build_type
                       *builder     /* resource staging builder     */
    )
{
/*----------------------------------------------------------
Local literals
----------------------------------------------------------*/
#define DEFAULT_FRAME_SZ           ( 2 * 1024 * 1024 )

/*----------------------------------------------------------
Local constants
----------------------------------------------------------*/
static const VKN_staging_build_config_type CONFIG =
    {
    add_sharing_family,
    set_frame_size
    };

clr_struct( builder );
builder->config = &CONFIG;

builder->state.logical         = logical;
builder->state.memory          = memory;
builder->state.buffer_frame_sz = DEFAULT_FRAME_SZ;
builder->state.queue           = queue;
builder->state.queue_index     = queue_index;

add_sharing_family_safe( builder->state.queue_index, builder );

return( builder->config );

#undef DEFAULT_FRAME_SZ
}   /* VKN_staging_init_builder() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       add_sharing_family
*
*********************************************************************/

static VKN_STAGING_CONFIG_API add_sharing_family
    (
    const u32           index,      /* family index to share with   */
    struct _VKN_staging_build_type
                       *builder     /* staging builder              */
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
    VKN_staging_build_type
                       *builder     /* staging builder              */
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
*       block_for_frame
*
*********************************************************************/

static void block_for_frame
    (
    VKN_staging_frame_type
                       *frame,      /* frame to block for           */
    VKN_staging_type   *staging     /* resource staging             */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkCommandBufferBeginInfo
                        begin;      /* begin command writing        */
VkMemoryBarrier         mem_barrier;/* global memory barrier        */

if( !frame->in_flight )
    {
    return;
    }

/*----------------------------------------------------------
Wait for frame to finish
----------------------------------------------------------*/
if( VKN_failed( vkWaitForFences( staging->state.logical, 1, &frame->fence, VK_FALSE, VKN_WAIT_INFINITE ) ) )
    {
    debug_assert_always();
    }

if( VKN_failed( vkResetFences( staging->state.logical, 1, &frame->fence ) ) )
    {
    debug_assert_always();
    }

frame->in_flight = FALSE;
frame->caret     = 0;

/*----------------------------------------------------------
Begin commands for this frame
----------------------------------------------------------*/
clr_struct( &begin );
begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

if( VKN_failed( vkBeginCommandBuffer( frame->commands, &begin ) ) )
    {
    debug_assert_always();
    }

/*----------------------------------------------------------
Insert a barrier at the beginning of the commands to block
vertex/index reads until the transfer is finished
----------------------------------------------------------*/
clr_struct( &mem_barrier );
mem_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
mem_barrier.srcAccessMask = VK_ACCESS_INDEX_READ_BIT
                          | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

vkCmdPipelineBarrier( frame->commands,
                      VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                      VK_PIPELINE_STAGE_TRANSFER_BIT,
                      0,/* flags */
                      1,
                      &mem_barrier,
                      0,
                      NULL,/* buffer barriers */
                      0,
                      NULL );/* image barriers */

}   /* block_for_frame() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       flush
*
*********************************************************************/

static void flush
    (
    struct _VKN_staging_type
                       *staging     /* resource staging to flush   */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_staging_frame_type *frame;      /* working frame                */
VkMemoryBarrier         mem_barrier;/* global memory barrier        */
VkMappedMemoryRange     mem_range;  /* mapped memory to flush       */
VkSubmitInfo            submit;     /* queue submission info        */

/*----------------------------------------------------------
Check if the current frame is empty or has already been
submitted
----------------------------------------------------------*/
frame = &staging->state.frames[ staging->state.frame_num ];
if( !frame->caret
 || frame->in_flight )
    {
    /*------------------------------------------------------
    Either we can't, or don't want to submit this frame, so
    ignore
    ------------------------------------------------------*/
    return;
    }

/*----------------------------------------------------------
Common wisdom in the graphics community is that no drivers 
support fine-grained barriers at the per-buffer level 
(VkBufferMemoryBarrier), and thus internally issue a global 
memory barrier.

To avoid thrashing the pipelines we always submit a global 
memory barrier here for buffer reads, to allow forgoing 
VkBufferMemoryBarrier in callers (which would just issue 
repeat global memory barriers).

Even though not every transfer will contain buffer uploads,
let's do the work once here regardless.
----------------------------------------------------------*/
clr_struct( &mem_barrier );
mem_barrier.sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
mem_barrier.dstAccessMask = VK_ACCESS_INDEX_READ_BIT
                          | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;

vkCmdPipelineBarrier( frame->commands,
                      VK_PIPELINE_STAGE_TRANSFER_BIT,
                      VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                      0,/* flags */
                      1,
                      &mem_barrier,
                      0,
                      NULL,/* buffer barriers */
                      0,
                      NULL );/* image barriers */

if( VKN_failed( vkEndCommandBuffer( frame->commands ) ) )
    {
    debug_assert_always();
    }
                     
/*----------------------------------------------------------
Make the writes available.  We use flush memory here to
guarantee cache is written, but we could optimize perf in
the future by exposing host coherency via the allocation
record, and take advantage of write-combines.
----------------------------------------------------------*/
clr_struct( &mem_range );
mem_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
mem_range.memory = frame->allocation.memory;
mem_range.offset = frame->allocation.offset;
mem_range.size   = frame->allocation.size;

if( VKN_failed( vkFlushMappedMemoryRanges( staging->state.logical, 1, &mem_range ) ) )
    {
    debug_assert_always();
    }

/*----------------------------------------------------------
Submit
----------------------------------------------------------*/
clr_struct( &submit );
submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
submit.commandBufferCount = 1;
submit.pCommandBuffers    = &frame->commands; 

if( VKN_failed( vkQueueSubmit( staging->state.queue, 1, &submit, frame->fence ) ) )
    {
    debug_assert_always();
    }

frame->in_flight = TRUE;

/*----------------------------------------------------------
Advance to the next frame
----------------------------------------------------------*/
staging->state.frame_num++;
staging->state.frame_num %= cnt_of_array( staging->state.frames );

}   /* flush() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_frame_size
*
*********************************************************************/

static VKN_STAGING_CONFIG_API set_frame_size
    (
    const u32           per_frame_sz,
                                    /* buffer size of each frame    */
    struct _VKN_staging_build_type
                       *builder     /* staging builder              */
    )
{
builder->state.buffer_frame_sz = per_frame_sz;

return( builder->config );

}   /* set_frame_size() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       upload
*
*********************************************************************/

static VKN_staging_upload_instruct_type upload
    (
    const u32           size,       /* size of the upload           */
    const u32           alignment,  /* alignment of the upload      */
    struct _VKN_staging_type
                       *staging     /* resource staging             */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_staging_frame_type *frame;      /* working frame                */
VkDeviceSize            padding;    /* wasted alignment space       */
VkDeviceSize            required_size;
                                    /* used sized after alignment   */
VKN_staging_upload_instruct_type
                        ret;        /* return instructions          */

/*----------------------------------------------------------
Validate the request
----------------------------------------------------------*/
clr_struct( &ret );
if( staging->state.buffer_frame_sz < size )
    {
    /*------------------------------------------------------
    Would never be able to satisfy this request, even with
    an empty buffer
    ------------------------------------------------------*/
    debug_assert_always();
    return( ret );
    }

/*----------------------------------------------------------
Determine if the allocation will fit in the current frame
----------------------------------------------------------*/
frame         = &staging->state.frames[ staging->state.frame_num ];
padding       = VKN_size_round_up_mult( frame->caret, alignment ) - frame->caret;
required_size = padding + size;

if( !frame->in_flight
 && staging->state.buffer_frame_sz < frame->caret + (u32)required_size )
    {
    /*------------------------------------------------------
    Submit the current frame and advance to the next
    ------------------------------------------------------*/
    flush( staging );
    padding = 0;
    required_size = size;
    }

frame = &staging->state.frames[ staging->state.frame_num ];
if( frame->in_flight )
    {
    /*------------------------------------------------------
    Current frame still in-flight.  Wait for it to finish.
    ------------------------------------------------------*/
    block_for_frame( frame, staging );
    }

frame->caret += (u32)padding;

/*----------------------------------------------------------
Fill out the instructions
----------------------------------------------------------*/
ret.commands = frame->commands;
ret.buffer   = frame->buffer;
ret.offset   = frame->caret;
ret.mapping  = &frame->allocation.mapping[ frame->caret ];

/*----------------------------------------------------------
Advance
----------------------------------------------------------*/
frame->caret += size;

return( ret );

}   /* upload() */
