#include "Global.hpp"
#include "Utilities.hpp"

#include "VknCommon.hpp"
#include "VknBufferIndex.hpp"
#include "VknBufferIndexTypes.hpp"
#include "VknMemory.hpp"
#include "VknReleaser.hpp"


static VKN_buffer_index_build_add_sharing_family_proc_type add_sharing_family;

static void add_sharing_family_safe
    (
    const u32           index,      /* family index to share with   */
    VKN_buffer_index_build_type
                       *builder     /* index buffer builder         */
    );

static VKN_buffer_index_get_proc_type get;
static VKN_buffer_index_map_proc_type map;
static VKN_buffer_index_build_reset_proc_type reset;
static VKN_buffer_index_build_set_allocation_callbacks_proc_type set_allocation_callbacks;
static VKN_buffer_index_unmap_proc_type unmap;


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_buffer_index_create
*
*   DESCRIPTION:
*       Create an index buffer.
*
*********************************************************************/

bool VKN_buffer_index_create
    (
    const u32           size,       /* size of buffer               */
    const VKN_buffer_index_build_type
                       *builder,    /* index buffer builder         */
    VKN_buffer_index_type
                       *buffer      /* output new index buffer      */
    )
{
/*----------------------------------------------------------
Local constants
----------------------------------------------------------*/
static const VKN_buffer_index_api_type API =
    {
    get,
    map,
    unmap
    };

/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkBufferCreateInfo      ci_buffer;  /* buffer create info           */

clr_struct( buffer );
buffer->i = &API;

buffer->state.logical          = builder->state.logical;
buffer->state.allocator        = builder->state.allocator;;
buffer->state.upload_alignment = builder->state.upload_alignment;
buffer->state.memory           = builder->state.memory;
buffer->state.size             = size;

/*----------------------------------------------------------
Create the buffer
----------------------------------------------------------*/
clr_struct( &ci_buffer );
ci_buffer.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
ci_buffer.flags                 = 0;
ci_buffer.size                  = size;
ci_buffer.usage                 = VK_BUFFER_USAGE_INDEX_BUFFER_BIT
                                | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
ci_buffer.sharingMode           = ( builder->state.families.count > 1 ) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
ci_buffer.queueFamilyIndexCount = builder->state.families.count;
ci_buffer.pQueueFamilyIndices   = (uint32_t*)&builder->state.families.indices;

if( VKN_failed( vkCreateBuffer( buffer->state.logical, &ci_buffer, buffer->state.allocator, &buffer->state.object ) )
 || !buffer->state.memory->i->create_buffer_memory( buffer->state.object, VKN_MEMORY_HEAP_USAGE_DEFAULT, buffer->state.memory, &buffer->state.allocation ) )
    {
    debug_assert_always();
    VKN_buffer_index_destroy( NULL, buffer );
    return( FALSE );
    }

return( TRUE );

}   /* VKN_buffer_index_create() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_buffer_index_destroy
*
*   DESCRIPTION:
*       Destroy an index buffer.
*
*********************************************************************/

void VKN_buffer_index_destroy
    (
    VKN_releaser_type  *releaser,   /* release buffer               */
    VKN_buffer_index_type
                       *buffer      /* buffer to destroy            */
    )
{
VKN_releaser_auto_mini_begin( releaser, use );
use->i->release_buffer( buffer->state.logical, buffer->state.allocator, buffer->state.object, use );
if( buffer->state.memory )
    {
    buffer->state.memory->i->deallocate( buffer->state.memory, &buffer->state.allocation );
    }

clr_struct( buffer );

VKN_releaser_auto_mini_end( use );

}   /* VKN_buffer_index_destroy() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_buffer_index_init_builder
*
*   DESCRIPTION:
*       Initialize an index buffer builder.
*
*********************************************************************/

VKN_BUFFER_INDEX_CONFIG_API VKN_buffer_index_init_builder
    (
    const VkDevice      logical,    /* associated logical device    */
    const VkPhysicalDeviceProperties
                       *props,      /* device memory properties     */
    VKN_memory_type    *memory,     /* memory allocator             */
    VKN_buffer_index_build_type
                       *builder     /* index buffer builder         */
    )
{
/*----------------------------------------------------------
Local constants
----------------------------------------------------------*/
static const VKN_buffer_index_build_config_type CONFIG =
    {
    add_sharing_family,
    reset,
    set_allocation_callbacks
    };

/*----------------------------------------------------------
Initialize
----------------------------------------------------------*/
clr_struct( builder );
builder->config = &CONFIG;

builder->state.logical          = logical;
builder->state.memory           = memory;
builder->state.upload_alignment = (u8)props->limits.minMemoryMapAlignment;

return( builder->config );

}   /* VKN_buffer_index_init_builder() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       add_sharing_family
*
*********************************************************************/

static VKN_BUFFER_INDEX_CONFIG_API add_sharing_family
    (
    const u32           index,      /* family index to share with   */
    struct _VKN_buffer_index_build_type
                       *builder     /* index buffer builder         */
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
    VKN_buffer_index_build_type
                       *builder     /* index buffer builder         */
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
*       get
*
*********************************************************************/

static VkBuffer get
    (
    const struct _VKN_buffer_index_type
                       *buffer      /* index buffer to fetch        */
    )
{
return( buffer->state.object );

}   /* get() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       map
*
*********************************************************************/

static void * map
    (
    VKN_staging_type   *staging,    /* staging buffer               */
    struct _VKN_buffer_index_type
                       *buffer      /* index buffer to map          */
    )
{
if( buffer->state.is_mapped )
    {
    return( buffer->state.upload.mapping );
    }

buffer->state.upload    = staging->i->upload( buffer->state.size, buffer->state.upload_alignment, staging );
buffer->state.is_mapped = TRUE;
    
return( buffer->state.upload.mapping );

}   /* map() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       reset
*
*********************************************************************/

static VKN_BUFFER_INDEX_CONFIG_API reset
    (
    struct _VKN_buffer_index_build_type
                       *builder     /* index buffer builder         */
    )
{
clr_struct( &builder->state.families );

return( builder->config );

}   /* reset() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_allocation_callbacks
*
*********************************************************************/

static VKN_BUFFER_INDEX_CONFIG_API set_allocation_callbacks
    (
    VkAllocationCallbacks
                       *allocator,  /* custom allocator             */
    struct _VKN_buffer_index_build_type
                       *builder     /* index buffer builder         */
    )
{
builder->state.allocator = allocator;

return( builder->config );

}   /* set_allocation_callbacks() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       unmap
*
*********************************************************************/
    
static void unmap
    (
    struct _VKN_buffer_index_type
                       *buffer      /* index buffer to map          */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkBufferCopy            copy;       /* copy details                 */

if( !buffer->state.is_mapped )
    {
    debug_assert_always();
    return;
    }

clr_struct( &copy );
copy.srcOffset = buffer->state.upload.offset;
copy.dstOffset = 0;
copy.size      = buffer->state.size;

vkCmdCopyBuffer( buffer->state.upload.commands, buffer->state.upload.buffer, buffer->state.object, 1, &copy );
buffer->state.is_mapped = FALSE;

}   /* unmap() */
