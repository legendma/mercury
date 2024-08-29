#include <cstring>

#include "Global.hpp"
#include "Utilities.hpp"

#include "VknArena.hpp"
#include "VknCommon.hpp"
#include "VknBufferUniform.hpp"
#include "VknBufferUniformTypes.hpp"
#include "VknReleaser.hpp"


#define DEFAULT_PAGE_SIZE           (u32)VKN_size_round_up_mult( 64 * 1024, builder->state.uniform_alignment )


static VKN_buffer_uniform_build_add_sharing_family_proc_type add_sharing_family;

static void add_sharing_family_safe
    (
    const u32           index,      /* family index to share with   */
    VKN_buffer_uniform_build_type
                       *builder     /* uniform buffer builder       */
    );
    
static VKN_buffer_uniform_clear_proc_type clear;
static VKN_buffer_uniform_copy_proc_type copy;

static VKN_buffer_uniform_page_type * create_new_page
    (
    VKN_buffer_uniform_type
                       *buffer      /* uniform buffer               */
    );

static VKN_buffer_uniform_flush_proc_type flush;
static VKN_buffer_uniform_map_proc_type map;
static VKN_buffer_uniform_build_reset_proc_type reset;
static VKN_buffer_uniform_build_set_allocation_callbacks_proc_type set_allocation_callbacks;
static VKN_buffer_uniform_build_set_page_size_proc_type set_page_size;
static VKN_buffer_uniform_unmap_proc_type unmap;


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_buffer_uniform_create
*
*   DESCRIPTION:
*       Create a uniform buffer.
*
*********************************************************************/

void VKN_buffer_uniform_create
    (
    const VKN_buffer_uniform_build_type
                       *builder,    /* uniform buffer builder       */
    VKN_arena_type     *arena,      /* permanent arena              */
    VKN_buffer_uniform_type
                       *buffer      /* output new uniform buffer    */
    )
{
/*----------------------------------------------------------
Local constants
----------------------------------------------------------*/
static const VKN_buffer_uniform_api_type API =
    {
    clear,
    copy,
    flush,
    map,
    unmap
    };

clr_struct( buffer );
buffer->i = &API;

buffer->state.logical           = builder->state.logical;
buffer->state.allocator         = builder->state.allocator;
buffer->state.arena             = arena;
buffer->state.memory            = builder->state.memory;
buffer->state.uniform_alignment = builder->state.uniform_alignment;
buffer->state.page_size         = builder->state.page_size;
buffer->state.families          = builder->state.families;

}   /* VKN_buffer_uniform_create() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_buffer_uniform_destroy
*
*   DESCRIPTION:
*       Destroy a uniform buffer.
*
*********************************************************************/

void VKN_buffer_uniform_destroy
    (
    VKN_releaser_type  *releaser,   /* release buffer               */
    VKN_buffer_uniform_type
                       *buffer      /* buffer to destroy            */
    )
{
buffer->i->clear( buffer );

VKN_releaser_auto_mini_begin( releaser, use );
while( buffer->state.frees )
    {
    buffer->state.memory->i->deallocate( buffer->state.memory, &buffer->state.frees->allocation );
    use->i->release_buffer( buffer->state.logical, buffer->state.allocator, buffer->state.frees->buffer, use );

    buffer->state.frees = buffer->state.frees->next;
    }

clr_struct( buffer );
VKN_releaser_auto_mini_end( use );

}   /* VKN_buffer_uniform_destroy() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_buffer_uniform_init_builder
*
*   DESCRIPTION:
*       Initialize a uniform buffer builder.
*
*********************************************************************/

VKN_BUFFER_UNIFORM_CONFIG_API VKN_buffer_uniform_init_builder
    (
    const VkDevice      logical,    /* associated logical device    */
    const VkPhysicalDeviceProperties
                       *props,      /* device memory properties     */
    VKN_memory_type    *memory,     /* memory allocator             */
    VKN_buffer_uniform_build_type
                       *builder     /* uniform buffer builder       */
    )
{
/*----------------------------------------------------------
Local constants
----------------------------------------------------------*/
static const VKN_buffer_uniform_build_config_type CONFIG =
    {
    add_sharing_family,
    reset,
    set_allocation_callbacks,
    set_page_size
    };

/*----------------------------------------------------------
Initialize
----------------------------------------------------------*/
clr_struct( builder );
builder->config = &CONFIG;

builder->state.logical           = logical;
builder->state.memory            = memory;
builder->state.uniform_alignment = (u16)props->limits.nonCoherentAtomSize;
builder->state.max_page_size     = props->limits.maxUniformBufferRange;
builder->state.page_size         = DEFAULT_PAGE_SIZE;

return( builder->config );

}   /* VKN_buffer_uniform_init_builder() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       add_sharing_family
*
*********************************************************************/

static VKN_BUFFER_UNIFORM_CONFIG_API add_sharing_family
    (
    const u32           index,      /* family index to share with   */
    struct _VKN_buffer_uniform_build_type
                       *builder     /* uniform buffer builder       */
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
    VKN_buffer_uniform_build_type
                       *builder     /* uniform buffer builder       */
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
*       clear
*
*********************************************************************/
    
static void clear
    (
    struct _VKN_buffer_uniform_type
                       *buffer      /* uniform buffer to clear      */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_buffer_uniform_page_type
                       *page;       /* working page                 */

debug_assert( !buffer->state.is_dirty );

/*----------------------------------------------------------
Current
----------------------------------------------------------*/
page = buffer->state.current;
if( page )
    {
    buffer->state.current = NULL;

    page->caret      = 0;
    page->dirty_size = 0;
    
    page->next = buffer->state.frees;
    buffer->state.frees = page;
    }

/*----------------------------------------------------------
Dirtys
----------------------------------------------------------*/
while( buffer->state.dirtys )
    {
    page = buffer->state.dirtys;
    buffer->state.dirtys = page->next;

    page->caret      = 0;
    page->dirty_size = 0;
    
    page->next = buffer->state.frees;
    buffer->state.frees = page;
    }

buffer->state.is_dirty = FALSE;

}   /* clear() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       copy
*
*********************************************************************/
    
static void copy
    (
    const f32          *data,       /* data to copy                 */
    const u8            num_floats, /* number of floats in data     */
    struct _VKN_buffer_uniform_type
                       *buffer      /* uniform buffer to copy in    */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     adjustment; /* required adjust to align     */
u32                     movement;   /* alignment adjusted size      */
u32                     size;       /* data size                    */

/*----------------------------------------------------------
Determine movement
----------------------------------------------------------*/
size       = num_floats * sizeof( f32 );
adjustment = VKN_uniform_get_align_adjust( num_floats, buffer->state.current->caret );
movement   = adjustment + size;

debug_assert( buffer->state.current->caret + movement <= buffer->state.page_size );

/*----------------------------------------------------------
Make the copy and advance
----------------------------------------------------------*/
buffer->state.current->caret      += adjustment;
memcpy( &buffer->state.current->allocation.mapping[ buffer->state.current->caret ], data, (u32)size );
buffer->state.current->caret      += size;
buffer->state.current->dirty_size += movement;
buffer->state.is_dirty             = TRUE;
buffer->state.uniform_size        += movement;

}   /* copy() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       create_new_page
*
*********************************************************************/

static VKN_buffer_uniform_page_type * create_new_page
    (
    VKN_buffer_uniform_type
                       *buffer      /* uniform buffer               */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkBufferCreateInfo      ci_buffer;  /* buffer create info           */
VKN_buffer_uniform_page_type
                       *ret;        /* return new page              */
VKN_arena_token_type    token;      /* rewind token                 */

/*----------------------------------------------------------
Allocate the page record
----------------------------------------------------------*/
token = VKN_arena_get_token( buffer->state.arena );
ret   = VKN_arena_allocate_struct( VKN_buffer_uniform_page_type, buffer->state.arena );
VKN_return_bnull( ret );

/*----------------------------------------------------------
Create the buffer
----------------------------------------------------------*/
clr_struct( &ci_buffer );
ci_buffer.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
ci_buffer.flags                 = 0;
ci_buffer.size                  = buffer->state.page_size;
ci_buffer.usage                 = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
ci_buffer.sharingMode           = ( buffer->state.families.count > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE );
ci_buffer.queueFamilyIndexCount = buffer->state.families.count;
ci_buffer.pQueueFamilyIndices   = buffer->state.families.indices;

if( VKN_failed( vkCreateBuffer( buffer->state.logical, &ci_buffer, buffer->state.allocator, &ret->buffer ) )
 || !buffer->state.memory->i->create_buffer_memory( ret->buffer, VKN_MEMORY_HEAP_USAGE_UPLOAD, buffer->state.memory, &ret->allocation ) )
    {
    debug_assert_always();
    VKN_release_buffer( buffer->state.logical, buffer->state.allocator, &ret->buffer );
    VKN_arena_rewind_by_token( token, buffer->state.arena );
    return( NULL );
    }

return( ret );

}   /* create_new_page() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       flush
*
*********************************************************************/

static void flush
    (
    struct _VKN_buffer_uniform_type
                       *buffer      /* uniform buffer to flush      */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_buffer_uniform_page_type
                       *page;       /* working page                 */
VkMappedMemoryRange     range;      /* memory range                 */

if( !buffer->state.is_dirty )
    {
    return;
    }

clr_struct( &range );
range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;

/*----------------------------------------------------------
Current
----------------------------------------------------------*/
page = buffer->state.current;
if( page
 && page->dirty_size )
    {
    range.size   = page->dirty_size;
    range.offset = page->caret - range.size;
    range.memory = page->allocation.memory;

    vkFlushMappedMemoryRanges( buffer->state.logical, 1, &range );
    page->dirty_size = 0;
    }

/*----------------------------------------------------------
Dirtys
----------------------------------------------------------*/
for( page = buffer->state.dirtys; page; page = page->next )
    {
    if( !page->dirty_size )
        {
        /*--------------------------------------------------
        Pages are pushed to the front
        --------------------------------------------------*/
        break;
        }

    range.size   = page->dirty_size;
    range.offset = 1 + ( page->caret - range.size );
    range.memory = page->allocation.memory;

    vkFlushMappedMemoryRanges( buffer->state.logical, 1, &range );
    page->dirty_size = 0;
    }

buffer->state.is_dirty = FALSE;

}   /* flush() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       map
* 
*   DESCRIPTION:
*       Make sure given size has been calculated via
*       VKN_uniform_get_size().
*
*********************************************************************/

static VKN_buffer_uniform_map_result_type map
    (
    const u32           size,       /* size of memory to map        */
    struct _VKN_buffer_uniform_type
                       *buffer      /* uniform buffer to map        */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_buffer_uniform_map_result_type
                        ret;        /* return mapping               */
u32                     uniform_size;
                                    /* size in uniform buffer       */

uniform_size = (u32)VKN_size_round_up_mult( size, buffer->state.uniform_alignment );
clr_struct( &ret );
debug_assert( uniform_size <= buffer->state.page_size );
debug_assert( !buffer->state.is_mapped );

/*----------------------------------------------------------
Obtain current page
----------------------------------------------------------*/
if( buffer->state.current
 && buffer->state.current->caret + uniform_size > buffer->state.page_size )
    {
    buffer->state.current->next = buffer->state.dirtys;
    buffer->state.dirtys        = buffer->state.current;
    buffer->state.current       = NULL;
    }

if( !buffer->state.current )
    {
    buffer->state.current = buffer->state.frees;
    if( buffer->state.current )
        {
        buffer->state.frees         = buffer->state.current->next;
        buffer->state.current->next = NULL;
        }
    }

if( !buffer->state.current )
    {
    buffer->state.current = create_new_page( buffer );
    }

if( !buffer->state.current )
    {
    debug_assert_always();
    return( ret );
    }

/*----------------------------------------------------------
Map it
----------------------------------------------------------*/
debug_assert( buffer->state.current->caret % buffer->state.uniform_alignment == 0 );
ret.offset = buffer->state.current->caret;
ret.buffer = buffer->state.current->buffer;

buffer->state.is_mapped = TRUE;
buffer->state.uniform_size = 0;

return( ret );

}   /* map() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       reset
*
*********************************************************************/

static VKN_BUFFER_UNIFORM_CONFIG_API reset
    (
    struct _VKN_buffer_uniform_build_type
                       *builder     /* uniform buffer builder       */
    )
{
builder->state.page_size = DEFAULT_PAGE_SIZE;
clr_struct( &builder->state.families );

return( builder->config );

}   /* reset() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_allocation_callbacks
*
*********************************************************************/

static VKN_BUFFER_UNIFORM_CONFIG_API set_allocation_callbacks
    (
    VkAllocationCallbacks
                       *allocator,  /* custom allocator             */
    struct _VKN_buffer_uniform_build_type
                       *builder     /* uniform buffer builder       */
    )
{
builder->state.allocator = allocator;

return( builder->config );

}   /* set_allocation_callbacks() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_page_size
*
*********************************************************************/

static VKN_BUFFER_UNIFORM_CONFIG_API set_page_size
    (
    const u32           page_size,  /* individual page size         */
    struct _VKN_buffer_uniform_build_type
                       *builder     /* uniform buffer builder       */
    )
{
/*----------------------------------------------------------
Local literals
----------------------------------------------------------*/
#define MIN_PAGE_SIZE               (u32)VKN_size_round_up_mult( 1024, builder->state.uniform_alignment )

/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     aligned_page_size;
                                    /* make page size a multiple    */

aligned_page_size = (u32)VKN_size_round_up_mult( page_size, builder->state.uniform_alignment );
if( aligned_page_size > builder->state.max_page_size )
    {
    builder->state.page_size = (u32)VKN_size_round_down_mult( builder->state.max_page_size, builder->state.uniform_alignment );
    }
else if( aligned_page_size < MIN_PAGE_SIZE )
    {
    builder->state.page_size = MIN_PAGE_SIZE;
    }
else
    {
    builder->state.page_size = aligned_page_size;
    }

return( builder->config );

#undef MIN_PAGE_SIZE
}   /* set_page_size() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       unmap
*
*********************************************************************/
    
static u32 unmap
    (
    struct _VKN_buffer_uniform_type
                       *buffer      /* uniform buffer to unmap      */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     adjustment; /* required adjust to align     */

debug_assert( buffer->state.is_mapped );

adjustment = (u32)VKN_size_round_up_mult( buffer->state.current->caret, buffer->state.uniform_alignment ) - buffer->state.current->caret;

buffer->state.current->caret      += adjustment;
buffer->state.current->dirty_size += adjustment;
buffer->state.is_mapped            = FALSE;
buffer->state.uniform_size        += adjustment;

debug_assert( buffer->state.current->caret <= buffer->state.page_size );

return( buffer->state.uniform_size );

}   /* unmap() */
