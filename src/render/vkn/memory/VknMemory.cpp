#include <cstring>

#include "Global.hpp"
#include "Utilities.hpp"

#include "VknCommon.hpp"
#include "VknMemory.hpp"
#include "VknMemoryTypes.hpp"
#include "VknReleaser.hpp"


#define VULKAN_BLOCK_SZ             ( 64 )


/*********************************************************************
*
*   PROCEDURE NAME:
*       count_bits
*
*********************************************************************/

static __inline u32 count_bits
    (
    const u32           to_count    /* bits to count                */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     i;          /* loop counter                 */
u32                     ret;        /* return number of bits        */

for( i = to_count, ret = 0; i; i >>= 1 )
    {
    ret += ( i & 1 );
    }

return( ret );

}   /* count_bits() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       find_block_by_id
*
*********************************************************************/

static __inline VKN_memory_block_type * find_block_by_id
    (
    const u32           id,         /* block id                     */
    VKN_memory_pool_type
                       *pool        /* owning pool                  */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_memory_block_type  *ret;        /* return block                 */

for( ret = pool->head_blocks; ret && ret->block_id != id; ret = ret->next );

return( ret );

}   /* find_block_by_id() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       find_pool_by_id
*
*********************************************************************/

static __inline VKN_memory_pool_type * find_pool_by_id
    (
    const u32           id,         /* pool id                      */
    VKN_memory_type    *allocator   /* owning allocator             */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_memory_pool_type   *ret;        /* return pool                  */

for( ret = allocator->state.head_pools; ret && ret->pool_id != id; ret = ret->next );

return( ret );

}   /* find_pool_by_id() */


static VKN_memory_allocate_proc_type allocate;

static VKN_memory_block_type * allocate_block
    (
    VKN_memory_type    *allocator   /* from which to allocate       */
    );

static bool allocate_from_pool
    (
    const u32           size,       /* required allocation size     */
    const u32           alignment,  /* required allocation alignment*/
    VKN_memory_pool_type
                       *pool,       /* from which to allocate       */
    VKN_memory_type    *allocator,  /* owning allocator             */
    VKN_memory_allocation_type
                       *allocation  /* output new allocation        */
    );

static VKN_memory_pool_type * allocate_pool
    (
    VKN_memory_type    *allocator   /* from which to allocate       */
    );

static VKN_memory_begin_frame_proc_type begin_frame;

static bool bind_image_to_memory
    (
    const VkDevice      logical,    /* image/memory owning device   */
    const VkImage       image,      /* image to bind                */
    const VkDeviceMemory
                        memory,     /* memory to bind               */
    const VkDeviceSize  offset      /* memory offset                */
    );

static VkDeviceSize calculate_min_alignment
    (
    const u32           memory_index,
                                    /* memory type index            */
    const VKN_memory_type
                       *allocator   /* allocator                    */
    );

static VkDeviceSize calculate_pool_min_block_size
    (
    const VkDeviceSize  pool_size,  /* the size of the pool         */
    VkDeviceSize        min_alignment,
                                    /* minimum alignment for pool   */
    const u32           max_allocations
                                    /* maximum allocations allowed  */
    );

static VkDeviceSize calculate_pool_size_for_heap
    (
    const VkDeviceSize  heap_size   /* size of device heap          */
    );

static VKN_memory_create_buffer_memory_proc_type create_buffer_memory;
static VKN_memory_create_image_memory_proc_type create_image_memory;

static VKN_memory_pool_type * create_new_pool
    (
    const VKN_memory_heap_usage_type
                        usage,      /* how memory is to be used     */
    const u32           memory_bits,/* memory type indices as bits  */
    struct _VKN_memory_type
                       *allocator   /* memory allocator             */
    );

static VKN_memory_deallocate_proc_type deallocate;

static bool find_memory_type_for_usage
    (
    const VKN_memory_heap_usage_type
                        usage,      /* how the heap is to used      */
    const u32           memory_bits,/* memory type indices as bits  */
    const VKN_memory_type
                       *allocator,  /* memory allocator             */
    u32                *memory_index/* output memory type index     */
    );

static void free_block
    (
    VKN_memory_block_type
                       *to_free,    /* block to free                */
    VKN_memory_pool_type
                       *pool,       /* owning pool                  */
    VKN_memory_type    *allocator   /* from which to allocate       */
    );

static void free_pool
    (
    VKN_memory_pool_type
                       *to_free,    /* pool to free                 */
    VKN_memory_type    *allocator   /* from which to allocate       */
    );

static VKN_memory_build_set_allocation_callbacks_proc_type set_allocation_callbacks;


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_memory_create
*
*   DESCRIPTION:
*       Create a memory allocator via the given builder.
*
*********************************************************************/

bool VKN_memory_create
    (
    const VKN_memory_build_type
                       *builder,    /* memory allocator builder     */
    VKN_memory_type    *allocator   /* output new memory allocator  */
    )
{
/*----------------------------------------------------------
Local constants
----------------------------------------------------------*/
static const VKN_memory_api_type API =
    {
    allocate,
    begin_frame,
    create_buffer_memory,
    create_image_memory,
    deallocate
    };

/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     i;          /* loop counter                 */

/*----------------------------------------------------------
Create the allocator
----------------------------------------------------------*/
clr_struct( allocator );
allocator->i = &API;

allocator->state.logical      = builder->state.logical;
allocator->state.next_pool_id = 1;
allocator->state.is_discrete  = builder->state.props.deviceType != VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
allocator->state.noncoherent_atom_size = builder->state.props.limits.nonCoherentAtomSize;

vkGetPhysicalDeviceMemoryProperties( builder->state.physical, &allocator->state.memory_props );
compiler_assert( cnt_of_array( allocator->state.heap_remaining )
              == cnt_of_array( allocator->state.memory_props.memoryHeaps ), VKN_MEMORY_C );
for( i = 0; i < allocator->state.memory_props.memoryHeapCount; i++ )
    {
    allocator->state.heap_remaining[ i ] = allocator->state.memory_props.memoryHeaps[ i ].size;
    allocator->state.heap_pool_size[ i ] = calculate_pool_size_for_heap( allocator->state.memory_props.memoryHeaps[ i ].size );
    }

/*----------------------------------------------------------
Initialize pool list
----------------------------------------------------------*/
allocator->state.free_pools = allocator->state.pools;
for( i = 0; i < cnt_of_array( allocator->state.pools ) - 1; i++ )
    {
    allocator->state.pools[ i ].next = &allocator->state.pools[ i + 1 ];
    }

/*----------------------------------------------------------
Initialize blocks list
----------------------------------------------------------*/
allocator->state.free_blocks = allocator->state.blocks;
for( i = 0; i < cnt_of_array( allocator->state.blocks ) - 1; i++ )
    {
    allocator->state.blocks[ i ].next = &allocator->state.blocks[ i + 1 ];
    }

return( TRUE );

}   /* VKN_memory_create() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_memory_destroy
*
*   DESCRIPTION:
*       Destroy the given memory allocator.
*
*********************************************************************/

void VKN_memory_destroy
    (
    VKN_releaser_type  *releaser,   /* release buffer               */
    VKN_memory_type    *allocator   /* output new memory allocator  */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_memory_pool_type   *pool;       /* pool iterator                */

VKN_releaser_auto_mini_begin( releaser, use );

/*----------------------------------------------------------
Unmap and free all the memory
----------------------------------------------------------*/
for( pool = allocator->state.head_pools; pool; pool = pool->next )
    {
    if( pool->mapping )
        {
        vkUnmapMemory( allocator->state.logical, pool->memory );
        }

    use->i->release_memory( allocator->state.logical, allocator->state.allocator, pool->memory, use );
    }

VKN_releaser_auto_mini_end( use );
clr_struct( allocator );

}   /* VKN_memory_destroy() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_memory_init_builder
*
*   DESCRIPTION:
*       Initialize a memory allocator builder.
*
*********************************************************************/

VKN_MEMORY_CONFIG_API VKN_memory_init_builder
    (
    const VkInstance    instance,   /* associated Vulkan instance   */
    const VkPhysicalDevice
                        physical,   /* associated physical device   */
    const VkDevice      logical,    /* logical device               */
    VKN_memory_build_type
                       *builder     /* memory allocator builder     */
    )
{
/*----------------------------------------------------------
Local constants
----------------------------------------------------------*/
static const VKN_memory_build_config_type CONFIG =
    {
    set_allocation_callbacks
    };

clr_struct( builder );
builder->config = &CONFIG;

builder->state.instance = instance;
builder->state.physical = physical;
builder->state.logical  = logical;

vkGetPhysicalDeviceProperties( builder->state.physical, &builder->state.props );

return( builder->config );

}   /* VKN_logical_device_init_builder() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       allocate
*
*********************************************************************/

static bool allocate
    (
    const u32           size,       /* required allocation size     */
    const u32           alignment,  /* required allocation alignment*/
    const u32           memory_bits,/* memory type indices as bits  */
    const VKN_memory_heap_usage_type
                        usage,      /* how memory is to be used     */
    struct _VKN_memory_type
                       *allocator,  /* memory allocator             */
    VKN_memory_allocation_type
                       *allocation  /* output new allocation        */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_memory_pool_type   *pool;       /* allocate from this pool      */

clr_struct( allocation );

/*----------------------------------------------------------
Try to allocate from existing pools
----------------------------------------------------------*/
for( pool = allocator->state.head_pools; pool; pool = pool->next )
    {
    if( pool->usage != usage
     || !test_any_bits( memory_bits, shift_bits( 1, pool->memory_index ) ) )
        {
        continue;
        }

    /*------------------------------------------------------
    Try to allocate from this pool
    ------------------------------------------------------*/
    if( allocate_from_pool( size, alignment, pool, allocator, allocation ) )
        {
        /*--------------------------------------------------
        We're done
        --------------------------------------------------*/
        return( TRUE );
        }
    }

/*----------------------------------------------------------
No pools sufficed, so try creating a new one for this usage
----------------------------------------------------------*/
pool = create_new_pool( usage, memory_bits, allocator );
if( !pool )
    {
    return( FALSE );
    }

return( allocate_from_pool( size, alignment, pool, allocator, allocation ) );

}   /* allocate() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       allocate_block
*
*********************************************************************/

static VKN_memory_block_type * allocate_block
    (
    VKN_memory_type    *allocator   /* from which to allocate       */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_memory_block_type  *ret;        /* return allocated block       */

ret = allocator->state.free_blocks;
if( !ret )
    {
    return( NULL );
    }

allocator->state.free_blocks = ret->next;
ret->next        = NULL;
ret->block_id    = allocator->state.next_block_id++;

return( ret );

}   /* allocate_block() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       allocate_pool
*
*********************************************************************/

static VKN_memory_pool_type * allocate_pool
    (
    VKN_memory_type    *allocator   /* from which to allocate       */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_memory_pool_type
                       *ret;        /* return allocated pool        */

ret = allocator->state.free_pools;
if( !ret )
    {
    return( NULL );
    }

allocator->state.free_pools = ret->next;
ret->next = NULL;

return( ret );

}   /* allocate_pool() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       allocate_from_pool
*
*********************************************************************/

static bool allocate_from_pool
    (
    const u32           size,       /* required allocation size     */
    const u32           alignment,  /* required allocation alignment*/
    VKN_memory_pool_type
                       *pool,       /* from which to allocate       */
    VKN_memory_type    *allocator,  /* owning allocator             */
    VKN_memory_allocation_type
                       *allocation  /* output new allocation        */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_memory_block_type  *block;      /* allocate from this block     */
VkDeviceSize            block_size; /* block size to use            */
VkDeviceSize            padding;    /* wasted alignment space       */
VkDeviceSize            required_alignment;
                                    /* required in particular pool  */
VkDeviceSize            required_size;
                                    /* required in particular block */
VKN_memory_block_type  *split;      /* newly split unused block     */

/*----------------------------------------------------------
Find an free block that is sufficient size
----------------------------------------------------------*/
required_alignment = VKN_size_max( alignment, pool->min_alignment );
for( block = pool->head_blocks; block; block = block->next )
    {
    if( block->is_utilized )
        {
        continue;
        }

    padding = VKN_size_round_up_mult( block->offset, required_alignment ) - block->offset;
    required_size = size + padding;
    if( block->size < required_size )
        {
        continue;
        }

    /*------------------------------------------------------
    Allocate from this block
    ------------------------------------------------------*/
    break;
    }

if( !block )
    {
    /*------------------------------------------------------
    No room in this pool
    ------------------------------------------------------*/
    return( FALSE );
    }

/*----------------------------------------------------------
Make the allocation
----------------------------------------------------------*/
block->is_utilized = TRUE;

allocation->pool_id  = pool->pool_id;
allocation->block_id = block->block_id;
allocation->offset   = block->offset + padding;
allocation->size     = size;
allocation->memory   = pool->memory;
allocation->mapping  = pool->mapping + allocation->offset;

/*----------------------------------------------------------
Look to split off any unused space at the end of the block
----------------------------------------------------------*/
block_size = VKN_size_min_power_of_two( required_size );
if( block->size < block_size
 && block->size - block_size < pool->min_block_size )
    {
    /*------------------------------------------------------
    Too small to split, we're done
    ------------------------------------------------------*/
    return( TRUE );
    }

split = allocate_block( allocator );
if( !split )
    {
    /*------------------------------------------------------
    Out of allocations, that's fine
    ------------------------------------------------------*/
    return( TRUE );
    }

/*----------------------------------------------------------
Fill out the split and link it after the block
----------------------------------------------------------*/
split->pool_id = pool->pool_id;
split->size    = block->size - block_size;
split->offset  = block->offset + block_size;
split->next    = block->next;
split->prev    = block;

/*----------------------------------------------------------
Update the block now to account for the split
----------------------------------------------------------*/
block->size -= split->size;
if( block->next )
    {
    block->next->prev = split;
    }

block->next = split;

return( TRUE );

}   /* allocate_pool() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       begin_frame
*
*********************************************************************/

static void begin_frame
    (
    struct _VKN_memory_type
                       *allocator   /* memory allocator             */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_memory_block_type **frame;     /* frame destruction            */
VKN_memory_pool_type   *pool;      /* owning pool                  */
VKN_memory_block_type  *to_destroy;/* destruction iterator         */

/*----------------------------------------------------------
Increment the frame counter
----------------------------------------------------------*/
allocator->state.frame_index++;
allocator->state.frame_index %= cnt_of_array( allocator->state.to_destroy );

/*----------------------------------------------------------
Merge any unused blocks with their neighbor when possible
----------------------------------------------------------*/
pool = NULL;
frame = &allocator->state.to_destroy[ allocator->state.frame_index ];
for( to_destroy = *frame; to_destroy; to_destroy = *frame )
    {
    /*------------------------------------------------------
    Unlink from destruction list
    ------------------------------------------------------*/
    *frame = to_destroy->next_destroy;
    to_destroy->next_destroy = NULL;

    /*------------------------------------------------------
    Obtain the owning pool
    ------------------------------------------------------*/
    if( !pool
     || pool->pool_id != to_destroy->pool_id )
        {
        pool = find_pool_by_id( to_destroy->pool_id, allocator );
        }

    if( !pool )
        {
        /*--------------------------------------------------
        State is corrupted
        --------------------------------------------------*/
        debug_assert_always();
        continue;
        }

    /*------------------------------------------------------
    Merge with 'previous'.  Important that this block
    survives and 'previous' is freed, in order to ensure we
    can process our 'next' neighbor.
    ------------------------------------------------------*/
    if( to_destroy->prev
     && !to_destroy->prev->is_utilized )
        {
        debug_assert( !to_destroy->prev->is_being_freed );
        to_destroy->size += to_destroy->prev->size;
        to_destroy->offset = to_destroy->prev->offset;
        free_block( to_destroy->prev, pool, allocator );
        }

    /*------------------------------------------------------
    Merge with 'next'.  Important that this block is freed
    and 'next' survives, in order to ensure validity of the
    destroy list.
    ------------------------------------------------------*/
    if( to_destroy->next
     && !to_destroy->next->is_utilized )
        {
        to_destroy->next->size += to_destroy->size;
        to_destroy->next->offset = to_destroy->offset;
        to_destroy->is_utilized = FALSE;
        free_block( to_destroy, pool, allocator );
        to_destroy = NULL;
        }

    /*------------------------------------------------------
    If block still exists, mark it as empty and ready for
    new allocations.
    ------------------------------------------------------*/
    if( to_destroy )
        {
        to_destroy->is_utilized = FALSE;
        to_destroy->is_being_freed = FALSE;
        }
    }

}   /* begin_frame() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       bind_image_to_memory
*
*********************************************************************/

static bool bind_image_to_memory
    (
    const VkDevice      logical,    /* image/memory owning device   */
    const VkImage       image,      /* image to bind                */
    const VkDeviceMemory
                        memory,     /* memory to bind               */
    const VkDeviceSize  offset      /* memory offset                */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkBindImageMemoryInfo   request;    /* bind request                 */

clr_struct( &request );
request.sType        = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
request.image        = image;
request.memory       = memory;
request.memoryOffset = offset;

return( !VKN_failed( vkBindImageMemory2( logical, 1, &request ) ) );

}   /* bind_image_to_memory() /*


/*********************************************************************
*
*   PROCEDURE NAME:
*       calculate_min_alignment
*
*********************************************************************/

static VkDeviceSize calculate_min_alignment
    (
    const u32           memory_index,
                                    /* memory type index            */
    const VKN_memory_type
                       *allocator   /* allocator                    */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkMemoryPropertyFlags   flags;
VkDeviceSize            ret;        /* return alignment             */

flags = allocator->state.memory_props.memoryTypes[ memory_index ].propertyFlags;
flags &= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

ret = 1;
if( flags == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT )
    {
    /*------------------------------------------------------
    Host cached memory must use the device's atomic size
    ------------------------------------------------------*/
    ret = allocator->state.noncoherent_atom_size;
    }

return( ret );

}   /* calculate_min_alignment() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       calculate_pool_min_block_size
*
*********************************************************************/

static VkDeviceSize calculate_pool_min_block_size
    (
    const VkDeviceSize  pool_size,  /* the size of the pool         */
    VkDeviceSize        min_alignment,
                                    /* minimum alignment for pool   */
    const u32           max_allocations
                                    /* maximum allocations allowed  */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     max_splits; /* max number of splits allowed */
VkDeviceSize            ret;        /* return smallest block size   */

/*----------------------------------------------------------
Determine how many splits we can do before exceeding the max
number of allocations allowed
----------------------------------------------------------*/
max_splits = 1;
while( pool_size > ( 2 * max_splits ) )
    {
    max_splits *= 2;
    }

max_splits -= 1;

/*----------------------------------------------------------
Now split the pool that many times to obtain the minimum
block size
----------------------------------------------------------*/
ret = pool_size >> max_splits;
ret = VKN_size_max( min_alignment, ret );

return( ret );

}   /* calculate_pool_min_block_size() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       calculate_pool_size_for_heap
*
*********************************************************************/

static VkDeviceSize calculate_pool_size_for_heap
    (
    const VkDeviceSize  heap_size   /* size of device heap          */
    )
{
/*----------------------------------------------------------
Local literals
----------------------------------------------------------*/
#define SMALL_HEAP_CUTOFF_SZ        ( 1024ull * 1024 * 1024 )
#define LARGE_HEAP_POOL_SZ          (  256ull * 1024 * 1024 )

/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkDeviceSize            ret;        /* return pool size             */

ret = LARGE_HEAP_POOL_SZ;
if( heap_size < SMALL_HEAP_CUTOFF_SZ )
    {
    ret = heap_size / 8;
    }

ret = VKN_size_round_down_mult( ret, VULKAN_BLOCK_SZ );

return( ret );

#undef SMALL_HEAP_CUTOFF_SZ
#undef LARGE_HEAP_POOL_SZ
}   /* calculate_pool_size_for_heap() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       create_buffer_memory
*
*********************************************************************/

static bool create_buffer_memory
    (
    const VkBuffer      buffer,     /* for which to create memory   */
    const VKN_memory_heap_usage_type
                        usage,      /* how memory is to be used     */
    struct _VKN_memory_type
                       *allocator,  /* memory allocator             */
    VKN_memory_allocation_type
                       *allocation  /* output new memory allocation */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkMemoryRequirements    required;   /* memory requirements          */

if( buffer == VK_NULL_HANDLE )
    {
    return( FALSE );
    }

/*----------------------------------------------------------
Get the memory requirements and allocate
----------------------------------------------------------*/
vkGetBufferMemoryRequirements( allocator->state.logical, buffer, &required );
if( !allocator->i->allocate( (u32)required.size, (u32)required.alignment, required.memoryTypeBits, usage, allocator, allocation ) )
    {
    return( FALSE );
    }

/*----------------------------------------------------------
Now bind the device memory to the buffer
----------------------------------------------------------*/
if( VKN_failed( vkBindBufferMemory( allocator->state.logical, buffer, allocation->memory, allocation->offset ) ) )
    {
    allocator->i->deallocate( allocator, allocation );
    return( FALSE );
    }

return( TRUE );

}   /* create_buffer_memory() /*


/*********************************************************************
*
*   PROCEDURE NAME:
*       create_image_memory
*
*********************************************************************/

static bool create_image_memory
    (
    const VkImage       image,      /* for which to create memory   */
    const VKN_memory_heap_usage_type
                        usage,      /* how memory is to be used     */
    struct _VKN_memory_type
                       *allocator,  /* memory allocator             */
    VKN_memory_allocation_type
                       *allocation  /* output new memory allocation */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkMemoryRequirements    required;   /* memory requirements          */

if( image == VK_NULL_HANDLE )
    {
    return( FALSE );
    }

/*----------------------------------------------------------
Get the memory requirements and allocate
----------------------------------------------------------*/
vkGetImageMemoryRequirements( allocator->state.logical, image, &required );
if( !allocator->i->allocate( (u32)required.size, (u32)required.alignment, required.memoryTypeBits, usage, allocator, allocation ) )
    {
    return( FALSE );
    }

/*----------------------------------------------------------
Now bind the device memory to the image
----------------------------------------------------------*/
if( VKN_failed( vkBindImageMemory( allocator->state.logical, image, allocation->memory, allocation->offset ) ) )
    {
    allocator->i->deallocate( allocator, allocation );
    return( FALSE );
    }

return( TRUE );

}   /* create_image_memory() /*


/*********************************************************************
*
*   PROCEDURE NAME:
*       create_new_pool
*
*********************************************************************/

static VKN_memory_pool_type * create_new_pool
    (
    const VKN_memory_heap_usage_type
                        usage,      /* how memory is to be used     */
    const u32           memory_bits,/* memory type indices as bits  */
    struct _VKN_memory_type
                       *allocator   /* memory allocator             */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_memory_block_type  *block;      /* initial entire pool block    */
VkMemoryAllocateInfo    ci_memory;  /* memory create info           */
u32                     heap_index; /* heap supporting memory type  */
VkDeviceMemory          memory;     /* handle to device memory      */
u32                     memory_index;
                                    /* memory type index            */
VkDeviceSize            pool_size;  /* size of pool to create       */
VKN_memory_pool_type   *ret;        /* return new pool              */

/*----------------------------------------------------------
Determine if we can create a pool
----------------------------------------------------------*/
if( !find_memory_type_for_usage( usage, memory_bits, allocator, &memory_index ) )
    {
    return( NULL );
    }

heap_index = allocator->state.memory_props.memoryTypes[ memory_index ].heapIndex;
pool_size  = allocator->state.heap_pool_size[ heap_index ];

if( allocator->state.heap_remaining[ heap_index ] < pool_size )
    {
    return( NULL );
    }

/*----------------------------------------------------------
Allocate the new pool and block, and link them
----------------------------------------------------------*/
ret = allocate_pool( allocator );
if( !ret )
    {
    /*------------------------------------------------------
    Out of unused pools
    ------------------------------------------------------*/
    return( NULL );
    }

ret->pool_id = allocator->state.next_pool_id++;

block = allocate_block( allocator );
if( !block )
    {
    /*------------------------------------------------------
    Out of unused blocks
    ------------------------------------------------------*/
    free_pool( ret, allocator );
    return( NULL );
    }

ret->next = allocator->state.head_pools;
allocator->state.head_pools = ret;

ret->head_blocks = block;

/*----------------------------------------------------------
Determine the final pool size, and request the memory from
vulkan
----------------------------------------------------------*/
if( allocator->state.heap_remaining[ heap_index ] < 2 * pool_size )
    {
    /*------------------------------------------------------
    Grab any crumbs so we utilize it all
    ------------------------------------------------------*/
    pool_size = allocator->state.heap_remaining[ heap_index ];
    }

clr_struct( &ci_memory );
ci_memory.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
ci_memory.allocationSize  = pool_size;
ci_memory.memoryTypeIndex = memory_index;
if( VKN_failed( vkAllocateMemory( allocator->state.logical, &ci_memory, NULL, &memory ) ) )
    {
    debug_assert_always();
    free_pool( ret, allocator );
    return( NULL );
    }

/*----------------------------------------------------------
Map the memory if CPU needs to access it
----------------------------------------------------------*/
if( usage == VKN_MEMORY_HEAP_USAGE_UPLOAD
 || usage == VKN_MEMORY_HEAP_USAGE_READBACK )
    {
    if( VKN_failed( vkMapMemory( allocator->state.logical, memory, 0, pool_size, 0, (void**)&ret->mapping ) ) )
        {
        debug_assert_always();
        free_pool( ret, allocator );
        return( NULL );
        }
    }

/*----------------------------------------------------------
Fill out the initial block
----------------------------------------------------------*/
block->pool_id = ret->pool_id;
block->size    = pool_size;

/*----------------------------------------------------------
Fill out rest of the pool
----------------------------------------------------------*/
ret->memory_index   = memory_index;
ret->usage          = usage;
ret->memory         = memory;
ret->size           = pool_size;
ret->min_alignment  = calculate_min_alignment( memory_index, allocator );
ret->min_block_size = calculate_pool_min_block_size( pool_size, ret->min_alignment, cnt_of_array( allocator->state.blocks ) );

return( ret );

}   /* create_new_pool() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       deallocate
*
*********************************************************************/

static void deallocate
    (
    struct _VKN_memory_type
                       *allocator,  /* memory allocator             */
    VKN_memory_allocation_type
                       *allocation  /* allocation to deallocate     */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_memory_block_type **head;       /* working block head           */
VKN_memory_block_type  *block;      /* allocation owning block      */
VKN_memory_pool_type   *pool;       /* allocation owning pool       */

/*----------------------------------------------------------
Find the block, but don't remove it from the used list
----------------------------------------------------------*/
pool = find_pool_by_id( allocation->pool_id, allocator );
if( !pool )
    {
    debug_assert_always();
    return;
    }

block = find_block_by_id( allocation->block_id, pool );
if( !block )
    {
    debug_assert_always();
    return;
    }

if( !block->is_utilized
 || block->is_being_freed )
    {
    debug_assert_always();
    return;
    }

block->is_being_freed = TRUE;
debug_assert( allocation->offset >= block->offset );
debug_assert( allocation->size + ( allocation->offset - block->offset ) <= block->size );

/*----------------------------------------------------------
Keep destruction list in order by pool id (major) and memory
offset (minor) in order to facilitate merges during
begin_frame()
----------------------------------------------------------*/
for( head = &allocator->state.to_destroy[ allocator->state.frame_index ];
     (*head)
  && ( (*head)->pool_id < block->pool_id
    || ( (*head)->pool_id == block->pool_id
      && (*head)->offset < block->offset ) );
     head = &(*head)->next_destroy );

block->next_destroy = *head;
*head = block;

clr_struct( allocation );

}   /* deallocate() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       find_memory_type_for_usage
*
*********************************************************************/

static bool find_memory_type_for_usage
    (
    const VKN_memory_heap_usage_type
                        usage,      /* how the heap is to used      */
    const u32           memory_bits,/* memory type indices as bits  */
    const VKN_memory_type
                       *allocator,  /* memory allocator             */
    u32                *memory_index/* output memory type index     */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     best;       /* best matching memory type    */
u32                     best_score; /* high score                   */
u32                     current_score;
                                    /* current score                */
VkMemoryPropertyFlags   excluded;   /* excluded properties          */
u32                     i;          /* loop counter                 */
VkMemoryPropertyFlags   preferred;  /* preferred properties         */
VkMemoryPropertyFlags   required;   /* required properties          */

/*----------------------------------------------------------
Determine parameters
----------------------------------------------------------*/
excluded = VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD;
required = 0;
preferred = 0;
switch( usage )
    {
    case VKN_MEMORY_HEAP_USAGE_DEFAULT:
        set_clear_bits( preferred, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, allocator->state.is_discrete );
        break;

    case VKN_MEMORY_HEAP_USAGE_UPLOAD:
        set_bits( required, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT );
        set_clear_bits( preferred, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, allocator->state.is_discrete );
        break;

    case VKN_MEMORY_HEAP_USAGE_READBACK:
        set_bits( required, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT );
        set_bits( preferred, VK_MEMORY_PROPERTY_HOST_CACHED_BIT );
        break;

    default:
        debug_assert_always();
        return( FALSE );
    }

/*----------------------------------------------------------
Find the best memory type that matches our needs
----------------------------------------------------------*/
best = 0;
best_score = 0;
for( i = 0; i < allocator->state.memory_props.memoryTypeCount; i++ )
    {
    if( !test_any_bits( memory_bits, shift_bits( 1, i ) )
     || test_any_bits( allocator->state.memory_props.memoryTypes[ i ].propertyFlags, excluded )
     || !test_bits( allocator->state.memory_props.memoryTypes[ i ].propertyFlags, required ) )
        {
        continue;
        }

    current_score = 1 + count_bits( (u32)( allocator->state.memory_props.memoryTypes[ i ].propertyFlags & preferred ) );
    if( current_score > best_score )
        {
        best_score = current_score;
        *memory_index = i;
        }
    }

return( best_score > 0 );

}   /* find_memory_type_for_usage() /*


/*********************************************************************
*
*   PROCEDURE NAME:
*       free_block
*
*********************************************************************/

static void free_block
    (
    VKN_memory_block_type
                       *to_free,    /* block to free                */
    VKN_memory_pool_type
                       *pool,       /* owning pool                  */
    VKN_memory_type    *allocator   /* from which to allocate       */
    )
{
debug_assert( !to_free->is_utilized );

/*----------------------------------------------------------
Unlink the pool from the used pools
----------------------------------------------------------*/
if( to_free->prev )
    {
    to_free->prev->next = to_free->next;
    }
else
    {
    pool->head_blocks = to_free->next;
    }

if( to_free->next )
    {
    to_free->next->prev = to_free->prev;
    }

/*----------------------------------------------------------
Clear it and return to free blocks
----------------------------------------------------------*/
clr_struct( to_free );
to_free->next = allocator->state.free_blocks;
allocator->state.free_blocks = to_free;

}   /* free_block() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       free_pool
*
*********************************************************************/

static void free_pool
    (
    VKN_memory_pool_type
                       *to_free,    /* pool to free                 */
    VKN_memory_type    *allocator   /* from which to allocate       */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_memory_block_type  *block;      /* head block                   */
VKN_memory_pool_type  **head;       /* used head                    */

/*----------------------------------------------------------
Unlink the pool from the used pools
----------------------------------------------------------*/
for( head = &allocator->state.head_pools; *head != to_free; head = &(*head)->next );
*head = to_free->next;

/*----------------------------------------------------------
Free all the blocks
----------------------------------------------------------*/
for( block = to_free->head_blocks; block; block = to_free->head_blocks )
    {
    free_block( block, to_free, allocator );
    }

/*----------------------------------------------------------
Clear it and return to free pools
----------------------------------------------------------*/
clr_struct( to_free );
to_free->next = allocator->state.free_pools;
allocator->state.free_pools = to_free;

}   /* free_pool() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_allocation_callbacks
*
*********************************************************************/

static VKN_MEMORY_CONFIG_API set_allocation_callbacks
    (
    VkAllocationCallbacks
                       *allocator,  /* custom allocator             */
    struct _VKN_memory_build_type
                       *builder     /* memory allocator builder     */
    )
{
builder->state.allocator = allocator;

return( builder->config );

}   /* set_allocation_callbacks() */
