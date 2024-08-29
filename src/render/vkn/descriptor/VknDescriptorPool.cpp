#include "Global.hpp"
#include "Utilities.hpp"

#include "VknCommon.hpp"
#include "VknDescriptorPool.hpp"
#include "VknDescriptorPoolTypes.hpp"
#include "VknReleaser.hpp"


compiler_assert( VKN_DESCRIPTOR_POOL_MAX_RATIO_CNT == VKN_DESCRIPTOR_POOL_MAX_DESCRIPTOR_POOL_SIZE_CNT, VKN_DESCRIPTOR_POOL_C );
#define REQUIRED_SET_COUNT_PER_POOL ( 4096 )


static VKN_descriptor_pool_build_add_default_ratios_proc_type add_default_ratios;
static VKN_descriptor_pool_build_add_ratio_proc_type add_ratio;

static bool add_ratio_safe
    (
    const VkDescriptorType
                        kind,       /* type of descriptor           */
    const float         ratio,      /* ratio type count to set count*/
    VKN_descriptor_pool_build_ratios_type
                       *ratios      /* ratio list                   */
    );

static VKN_descriptor_pool_allocate_set_proc_type allocate_set;
static VKN_descriptor_pool_begin_frame_proc_type begin_frame;

static bool create_new_pool
    (
    VKN_descriptor_pool_type
                       *pool        /* descriptor pool to wipe      */
    );

static bool get_clean_pool
    (
    VKN_descriptor_pool_type
                       *pool        /* descriptor pool to wipe      */
    );

static VKN_descriptor_pool_build_set_allocation_callbacks_proc_type set_allocation_callbacks;
static VKN_descriptor_pool_build_set_max_pool_sets_proc_type set_max_pool_sets;
    
static bool try_allocate
    (
    const VkDescriptorSetLayout
                        layout,     /* desired layout of set        */
    VkDescriptorSet    *out_set,    /* output new set               */
    VKN_descriptor_pool_type
                       *pool        /* descriptor pool              */
    );


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_descriptor_pool_create
*
*   DESCRIPTION:
*       Create a descriptor pool via the given builder.
*
*********************************************************************/

bool VKN_descriptor_pool_create
    (
    const VKN_descriptor_pool_build_type
                       *builder,    /* shader builder               */
    VKN_descriptor_pool_type
                       *pool        /* output new descriptor pool   */
    )
{
/*----------------------------------------------------------
Local constants
----------------------------------------------------------*/
static const VKN_descriptor_pool_api_type API =
    {
    allocate_set,
    begin_frame
    };

/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     i;          /* loop counter                 */
VkDescriptorPoolSize   *pool_size;  /* working descriptor type count*/

/*----------------------------------------------------------
Create the descriptor pool
----------------------------------------------------------*/
clr_struct( pool );
pool->i = &API;

pool->state.logical       = builder->state.logical;
pool->state.allocator     = builder->state.allocator;
pool->state.max_set_count = builder->state.max_set_count;

if( pool->state.max_set_count > REQUIRED_SET_COUNT_PER_POOL )
    {
    debug_assert_always();
    pool->state.max_set_count = REQUIRED_SET_COUNT_PER_POOL;
    }

for( i = 0; i < builder->state.ratios.count; i++ )
    {
    pool_size = &pool->state.sizes.sizes[ pool->state.sizes.count++ ];
    pool_size->type            = builder->state.ratios.ratios[ i ].kind;
    pool_size->descriptorCount = (u32)( 0.5f + ( builder->state.ratios.ratios[ i ].ratio * (float)pool->state.max_set_count ) );
    }

if( !create_new_pool( pool ) )
    {
    debug_assert_always();
    VKN_descriptor_pool_destroy( NULL, pool );
    return( FALSE );
    }

return( TRUE );

}   /* VKN_descriptor_pool_create() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_descriptor_pool_destroy
*
*   DESCRIPTION:
*       Destroy the given descriptor pool.
*
*********************************************************************/

void VKN_descriptor_pool_destroy
    (
    VKN_releaser_type  *releaser,   /* release buffer               */
    VKN_descriptor_pool_type
                       *pool        /* descriptor pool to destroy */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     i;          /* loop counter               */

VKN_releaser_auto_mini_begin( releaser, use );
use->i->release_descriptor_pool( pool->state.logical, pool->state.allocator, pool->state.active, use );
for( i = 0; i < pool->state.dirty.count; i++ )
    {
    use->i->release_descriptor_pool( pool->state.logical, pool->state.allocator, pool->state.dirty.pools[ i ], use );
    }

for( i = 0; i < pool->state.clean.count; i++ )
    {
    use->i->release_descriptor_pool( pool->state.logical, pool->state.allocator, pool->state.clean.pools[ i ], use );
    }

VKN_releaser_auto_mini_end( use );
clr_struct( pool );

}   /* VKN_descriptor_pool_destroy() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_descriptor_pool_init_builder
*
*   DESCRIPTION:
*       Initialize a descriptor pool builder.
*
*********************************************************************/

VKN_DESCRIPTOR_POOL_CONFIG_API VKN_descriptor_pool_init_builder
    (
    const VkDevice      logical,    /* associated logical device    */
    VKN_descriptor_pool_build_type
                       *builder     /* descriptor pool builder      */
    )
{
/*----------------------------------------------------------
Local literals
----------------------------------------------------------*/
#define DEFAULT_MAX_SET_CNT         ( 200 )

/*----------------------------------------------------------
Local constants
----------------------------------------------------------*/
static const VKN_descriptor_pool_build_config_type CONFIG =
    {
    add_default_ratios,
    add_ratio,
    set_allocation_callbacks,
    set_max_pool_sets
    };

/*----------------------------------------------------------
Initialize
----------------------------------------------------------*/
clr_struct( builder );
builder->config = &CONFIG;

builder->state.logical       = logical;
builder->state.max_set_count = DEFAULT_MAX_SET_CNT;

return( builder->config );

#undef DEFAULT_MAX_SET_CNT
}   /* VKN_descriptor_pool_init_builder() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       add_default_ratios
*
*********************************************************************/

static VKN_DESCRIPTOR_POOL_CONFIG_API add_default_ratios
    (
    struct _VKN_descriptor_pool_build_type
                       *builder     /* descriptor pool builder      */
    )
{
/*----------------------------------------------------------
These are just rough guesses, don't rely on them
----------------------------------------------------------*/
add_ratio_safe( VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1.0f, &builder->state.ratios );
add_ratio_safe( VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         2.0f, &builder->state.ratios );

return( builder->config );

}   /* add_default_ratios() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       add_ratio
*
*********************************************************************/

static VKN_DESCRIPTOR_POOL_CONFIG_API add_ratio
    (
    const VkDescriptorType
                        kind,       /* type of descriptor           */
    const float         ratio,      /* ratio type count to set count*/
    struct _VKN_descriptor_pool_build_type
                       *builder     /* descriptor pool builder      */
    )
{
add_ratio_safe( kind, ratio, &builder->state.ratios );

return( builder->config );

}   /* add_ratio() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       add_ratio_safe
*
*********************************************************************/

static bool add_ratio_safe
    (
    const VkDescriptorType
                        kind,       /* type of descriptor           */
    const float         ratio,      /* ratio type count to set count*/
    VKN_descriptor_pool_build_ratios_type
                       *ratios      /* ratio list                   */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     i;          /* loop counter                 */
VKN_descriptor_pool_build_ratio_type
                       *value;      /* ratio value to write         */

value = NULL;
for( i = 0; i < ratios->count; i++ )
    {
    if( ratios->ratios[ i ].kind == kind )
        {
        value = &ratios->ratios[ i ];
        }
    }

if( !value
 && ratios->count < cnt_of_array( ratios->ratios ) )
    {
    value = &ratios->ratios[ ratios->count++ ];
    }

if( !value )
    {
    debug_assert_always();
    return( FALSE );
    }

value->kind  = kind;
value->ratio = ratio;

return( TRUE );

}   /* add_ratio_safe() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       allocate_set
*
*********************************************************************/
    
static VkDescriptorSet allocate_set
    (
    const VkDescriptorSetLayout
                        layout,     /* desired layout of set        */
    struct _VKN_descriptor_pool_type
                       *pool        /* descriptor pool              */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkDescriptorSet         ret;        /* return new set               */

ret = VK_NULL_HANDLE;
if( !try_allocate( layout, &ret, pool ) )
    {
    debug_assert_always();
    return( VK_NULL_HANDLE );
    }
else if( ret != VK_NULL_HANDLE )
    {
    return( ret );
    }

get_clean_pool( pool );
(void)try_allocate( layout, &ret, pool );

return( ret );

}   /* allocate_set() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       create_new_pool
*
*********************************************************************/

static bool create_new_pool
    (
    VKN_descriptor_pool_type
                       *pool        /* descriptor pool to wipe      */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkDescriptorPoolCreateInfo
                        ci_pool;    /* descriptor pool create info  */

if( pool->state.active != VK_NULL_HANDLE )
    {
    debug_assert_always();
    return( FALSE );
    }

clr_struct( &ci_pool );
ci_pool.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
ci_pool.maxSets       = pool->state.max_set_count;
ci_pool.pPoolSizes    = pool->state.sizes.sizes;
ci_pool.poolSizeCount = pool->state.sizes.count;

return( !VKN_failed( vkCreateDescriptorPool( pool->state.logical, &ci_pool, pool->state.allocator, &pool->state.active ) ) );

}   /* create_new_pool() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       get_clean_pool
*
*********************************************************************/

static bool get_clean_pool
    (
    VKN_descriptor_pool_type
                       *pool        /* descriptor pool to wipe      */
    )
{
if( pool->state.active != VK_NULL_HANDLE )
    {
    debug_assert( pool->state.dirty.count < cnt_of_array( pool->state.dirty.pools ) );
    pool->state.dirty.pools[ pool->state.dirty.count++ ] = pool->state.active;
    pool->state.active = VK_NULL_HANDLE;
    }

if( pool->state.dirty.count >= cnt_of_array( pool->state.dirty.pools ) )
    {
    debug_assert_always();
    return( FALSE );
    }

if( pool->state.clean.count )
    {
    pool->state.active = pool->state.clean.pools[ pool->state.clean.count - 1 ];
    pool->state.clean.pools[ pool->state.clean.count - 1 ] = VK_NULL_HANDLE;
    pool->state.clean.count--;
    return( TRUE );
    }

return( create_new_pool( pool ) );

}   /* get_clean_pool() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       begin_frame
*
*********************************************************************/

static void begin_frame
    (
    struct _VKN_descriptor_pool_type
                       *pool        /* descriptor pool to wipe      */
    )
{
if( pool->state.active != VK_NULL_HANDLE )
    {
    pool->state.dirty.pools[ pool->state.dirty.count++ ] = pool->state.active;
    pool->state.active = VK_NULL_HANDLE;
    }

while( pool->state.dirty.count )
    {
    if( VKN_failed( vkResetDescriptorPool( pool->state.logical, pool->state.dirty.pools[ pool->state.dirty.count - 1 ], 0 ) ) )
        {
        debug_assert_always();
        return;
        }
    
    pool->state.clean.pools[ pool->state.clean.count++ ] = pool->state.dirty.pools[ pool->state.dirty.count - 1 ];
    pool->state.dirty.pools[ --pool->state.dirty.count ] = VK_NULL_HANDLE;
    }

pool->state.dirty.count = 0;
get_clean_pool( pool );

}   /* begin_frame() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_allocation_callbacks
*
*********************************************************************/

static VKN_DESCRIPTOR_POOL_CONFIG_API set_allocation_callbacks
    (
    VkAllocationCallbacks
                       *allocator,  /* custom allocator             */
    struct _VKN_descriptor_pool_build_type
                       *builder     /* descriptor pool builder      */
    )
{
builder->state.allocator = allocator;

return( builder->config );

}   /* set_allocation_callbacks() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_max_pool_sets
*
*********************************************************************/

static VKN_DESCRIPTOR_POOL_CONFIG_API set_max_pool_sets
    (
    const u32           max_sets_per_pool,
                                    /* maximum number sets per pool */
    struct _VKN_descriptor_pool_build_type
                       *builder     /* descriptor pool builder      */
    )
{
builder->state.max_set_count = max_sets_per_pool;

return( builder->config );

}   /* set_max_pool_sets() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       try_allocate
*
*********************************************************************/
    
static bool try_allocate
    (
    const VkDescriptorSetLayout
                        layout,     /* desired layout of set        */
    VkDescriptorSet    *out_set,    /* output new set               */
    VKN_descriptor_pool_type
                       *pool        /* descriptor pool              */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkDescriptorSetAllocateInfo
                        info;       /* allocation details           */
VkResult                result;     /* request result               */

*out_set = VK_NULL_HANDLE;
clr_struct( &info );
info.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
info.descriptorPool     = pool->state.active;
info.descriptorSetCount = 1;
info.pSetLayouts        = &layout;

result = vkAllocateDescriptorSets( pool->state.logical, &info, out_set );

return( result == VK_SUCCESS
     || result == VK_ERROR_OUT_OF_POOL_MEMORY );

}   /* try_allocate() */
