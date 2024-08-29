#include "Global.hpp"
#include "Utilities.hpp"

#include "VknCommon.hpp"
#include "VknTransitioner.hpp"
#include "VknTransitionerTypes.hpp"


/*********************************************************************
*
*   PROCEDURE NAME:
*       alloc_barrier
*
*********************************************************************/

static __inline VKN_transitioner_barrier_type * alloc_barrier
    (
    VKN_transitioner_type
                       *transitioner/* resource transitioner        */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_transitioner_barrier_type
                       *ret;        /* return barrier record        */

ret = transitioner->state.barrier_free;
if( ret )
    {
    transitioner->state.barrier_free = ret->next;
    ret->next = NULL;
    }

return( ret );

}   /* alloc_barrier() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       alloc_resource
*
*********************************************************************/

static __inline VKN_transitioner_resource_type * alloc_resource
    (
    VKN_transitioner_type
                       *transitioner/* resource transitioner        */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_transitioner_resource_type
                       *ret;        /* return resource record       */

ret = transitioner->state.resource_free;
if( ret )
    {
    transitioner->state.resource_free = ret->next;
    ret->next = NULL;
    ret->all_subresources = VK_IMAGE_LAYOUT_MAX_ENUM;
    }

return( ret );

}   /* alloc_resource() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       alloc_subresource
*
*********************************************************************/

static __inline VKN_transitioner_subresource_type * alloc_subresource
    (
    VKN_transitioner_type
                       *transitioner/* resource transitioner        */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_transitioner_subresource_type
                       *ret;        /* return subresource record    */

ret = transitioner->state.subresource_free;
if( ret )
    {
    transitioner->state.subresource_free = ret->next;
    ret->next = NULL;
    ret->state = VK_IMAGE_LAYOUT_MAX_ENUM;
    }

return( ret );

}   /* alloc_subresource() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       do_image_barrier
*
*********************************************************************/

static __inline void do_image_barrier
    (
    const VkPipelineStageFlags
                        src_stages, /* source pipeline stages       */
    const VkPipelineStageFlags
                        dst_stages, /* destination pipeline stages  */
    const VkImageMemoryBarrier
                       *barrier,    /* resource barrier             */
    VkCommandBuffer     commands    /* command buffer for barrier   */
    )
{
if( !commands )
    {
    return;
    }

vkCmdPipelineBarrier( commands, src_stages, dst_stages, 0,/*flags*/ 0, NULL, 0, NULL, 1, barrier );

}   /* do_image_barrier() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       free_barrier
*
*********************************************************************/

static __inline void free_barrier
    (
    VKN_transitioner_barrier_type
                       *to_free,    /* barrier to free              */
    VKN_transitioner_barrier_type
                      **head,       /* head of barriers             */
    VKN_transitioner_type
                       *transitioner/* resource transitioner        */
    )
{
*head = to_free->next;

clr_struct( to_free );
to_free->next = transitioner->state.barrier_free;
transitioner->state.barrier_free = to_free;

}   /* free_barrier() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       push_back_barrier
*
*********************************************************************/

static __inline void push_back_barrier
    (
    VKN_transitioner_barrier_type
                       *to_push,    /* barrier to push              */
    VKN_transitioner_barrier_type
                      **head,       /* head of barriers             */
    VKN_transitioner_barrier_type
                      **tail        /* tail of barriers             */
    )
{
if( !*head )
    {
    *head = to_push;
    }

if( *tail )
    {
    (*tail)->next = to_push;
    }

*tail = to_push;

}   /* push_back_barrier() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       push_resource
*
*********************************************************************/

static __inline void push_resource
    (
    VKN_transitioner_resource_type
                       *to_push,    /* resource to push             */
    VKN_transitioner_resource_type
                      **head        /* head of resources            */
    )
{
if( *head )
    {
    to_push->next = *head;
    }

*head = to_push;

}   /* push_resource() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       push_subresource
*
*********************************************************************/

static __inline void push_subresource
    (
    VKN_transitioner_subresource_type
                       *to_push,    /* subresource to push          */
    VKN_transitioner_subresource_type
                      **head        /* head of resources            */
    )
{
if( *head )
    {
    to_push->next = *head;
    }

*head = to_push;

}   /* push_subresource() */


static VKN_transitioner_barrier_proc_type barrier;
static VKN_transitioner_begin_frame_proc_type begin_frame;
static VKN_transitioner_commit_proc_type commit;

static void create_resource_from_barrier
    (
    const u32           uid,        /* image unique id              */
    const VkImage       image,      /* image resource               */
    const u32           mip_levels, /* image number of mip levels   */
    const VkImageMemoryBarrier
                       *barrier,    /* resource barrier             */
    VKN_transitioner_type
                       *transitioner/* resource transitioner        */
    );

static void do_unregister
    (
    const u32           uid,        /* resource uid                 */
    struct _VKN_transitioner_type
                       *transitioner/* resource transitioner        */
    );

static VKN_transitioner_resource_type * find_resource_by_uid
    (
    const u32           uid,        /* resource uid                 */
    VKN_transitioner_type
                       *transitioner/* resource transitioner        */
    );

static void free_resource
    (
    VKN_transitioner_resource_type
                       *to_free,    /* resource to free             */
    VKN_transitioner_resource_type
                      **head,       /* head of resources            */
    VKN_transitioner_type
                       *transitioner/* resource transitioner        */
    );

static void free_subresource
    (
    VKN_transitioner_subresource_type
                       *to_free,    /* subresource to free          */
    VKN_transitioner_subresource_type
                      **head,       /* head of subresources         */
    VKN_transitioner_type
                       *transitioner/* resource transitioner        */
    );

static VKN_transitioner_register_new_proc_type register_new;

static void reset
    (
    struct _VKN_transitioner_type
                       *transitioner/* transitioner to reset        */
    );

static void transition_resource
    (
    const VkPipelineStageFlags
                        src_stages, /* source pipeline stages       */
    const VkPipelineStageFlags
                        dst_stages, /* destination pipeline stages  */
    const VkImageMemoryBarrier
                       *barrier,    /* resource barrier             */
    VkCommandBuffer     commands,   /* command buffer for barrier   */
    VKN_transitioner_resource_type
                       *resource,   /* resource to transition       */
    VKN_transitioner_type
                       *transitioner/* resource transitioner        */
    );

static void transition_subresources
    (
    const VkPipelineStageFlags
                        src_stages, /* source pipeline stages       */
    const VkPipelineStageFlags
                        dst_stages, /* destination pipeline stages  */
    const VkImageMemoryBarrier
                       *barrier,    /* resource barrier             */
    VkCommandBuffer     commands,   /* command buffer for barrier   */
    VKN_transitioner_resource_type
                       *resource,   /* resource to transition       */
    VKN_transitioner_type
                       *transitioner/* resource transitioner        */
    );

static VKN_transitioner_unregister_proc_type unregister;


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_transitioner_create
*
*   DESCRIPTION:
*       Create a resource transitioner.
*
*********************************************************************/

void VKN_transitioner_create
    (
    const bool          is_master,  /* is this the master ledger?   */
    const u32           barrier_capacity,
                                    /* max barriers                 */
    const u32           resource_capacity,
                                    /* max resources                */
    const u32           subresource_capacity,
                                    /* max subresources             */
    const u32           unregister_capacity,
                                    /* max deferred unregisters     */
    VKN_transitioner_barrier_type
                       *barriers,   /* barrier records              */
    VKN_transitioner_resource_type
                       *resources,  /* resource records             */
    VKN_transitioner_subresource_type
                       *subresources,
                                    /* subresource records          */
    VKN_transitioner_unregister_type
                       *unregisters,/* unregister records           */
    VKN_transitioner_type
                       *transitioner/* new resource transitioner    */
    )
{
/*----------------------------------------------------------
Local constants
----------------------------------------------------------*/
static const VKN_transitioner_api_type API =
    {
    barrier,
    begin_frame,
    commit,
    register_new,
    unregister
    };

/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_transitioner_unregister_frame_type
                       *frame;      /* working frame                */
u32                     i;          /* loop counter                 */

/*----------------------------------------------------------
Create the transitioner
----------------------------------------------------------*/
clr_struct( transitioner );
transitioner->i = &API;

transitioner->state.is_master = is_master;

for( i = 0; i < barrier_capacity; i++ )
    {
    clr_struct( &barriers[ i ] );
    barriers[ i ].next = transitioner->state.barrier_free;
    transitioner->state.barrier_free = &barriers[ i ];
    }
    
for( i = 0; i < resource_capacity; i++ )
    {
    clr_struct( &resources[ i ] );
    resources[ i ].next = transitioner->state.resource_free;
    transitioner->state.resource_free = &resources[ i ];
    }
    
for( i = 0; i < subresource_capacity; i++ )
    {
    clr_struct( &subresources[ i ] );
    subresources[ i ].next = transitioner->state.subresource_free;
    transitioner->state.subresource_free = &subresources[ i ];
    }

/*----------------------------------------------------------
Master-specific
----------------------------------------------------------*/
if( transitioner->state.is_master )
    {
    /* TODO <MPA> - Should probably do this with a ring-buffer */
    for( i = 0; i < cnt_of_array( transitioner->state.unregister ); i++ )
        {
        frame = &transitioner->state.unregister[ i ];
        frame->capacity = unregister_capacity / cnt_of_array( transitioner->state.unregister );
        frame->arr = &unregisters[ i * frame->capacity ];

        if( i == cnt_of_array( transitioner->state.unregister ) - 1 )
            {
            /*----------------------------------------------
            Grab any remainder for last frame
            ----------------------------------------------*/
            frame->capacity += unregister_capacity % cnt_of_array( transitioner->state.unregister );
            }
        }
    }

}   /* VKN_transitioner_create() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       barrier
*
*********************************************************************/

static void barrier
    (
    const u32           uid,        /* image unique id              */
    const u32           mip_levels, /* image number of mip levels   */
    const VkPipelineStageFlags
                        src_stages, /* source pipeline stages       */
    const VkPipelineStageFlags
                        dst_stages, /* destination pipeline stages  */
    const VkImageMemoryBarrier
                       *barrier,    /* resource barrier             */
    VkCommandBuffer     commands,   /* command buffer for barrier   */
    struct _VKN_transitioner_type
                       *transitioner/* resource transitioner        */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_transitioner_barrier_type
                       *new_barrier;/* working barrier             */
VKN_transitioner_resource_type
                       *resource;   /* working resource            */

/*----------------------------------------------------------
Validate
----------------------------------------------------------*/
if( !barrier
 || !barrier->subresourceRange.levelCount
 || barrier->image == VK_NULL_HANDLE )
    {
    debug_assert( !barrier || barrier->image != VK_NULL_HANDLE );
    return;
    }

/*----------------------------------------------------------
Process the barrier
----------------------------------------------------------*/
resource = find_resource_by_uid( uid, transitioner );
if( !resource )
    {
    if( transitioner->state.is_master )
        {
        /*--------------------------------------------------
        Unable to find the resource on master, did you
        forget to register it?
        --------------------------------------------------*/
        debug_assert_always();
        return;
        }

    create_resource_from_barrier( uid, barrier->image, mip_levels, barrier, transitioner );
    new_barrier = alloc_barrier( transitioner );
    if( !new_barrier )
        {
        debug_assert_always();
        return;
        }

    new_barrier->uid        = uid;
    new_barrier->src_stages = src_stages;
    new_barrier->dst_stages = dst_stages;
    new_barrier->barrier    = *barrier;
    push_back_barrier( new_barrier, &transitioner->state.prepends, &transitioner->state.prepends_tail );
    return;
    }

debug_assert( resource->mip_count == mip_levels );
transition_resource( src_stages, dst_stages, barrier, commands, resource, transitioner );

}   /* barrier() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       begin_frame
*
*********************************************************************/

static void begin_frame
    (
    struct _VKN_transitioner_type
                       *transitioner/* resource transitioner        */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_transitioner_unregister_frame_type
                       *frame;      /* current frame                */
u32                     i;          /* loop counter                 */

/*----------------------------------------------------------
Just reset non-masters
----------------------------------------------------------*/
if( !transitioner->state.is_master )
    {
    reset( transitioner );
    return;
    }

/*----------------------------------------------------------
Unregister deferred items to clean this frame
----------------------------------------------------------*/
transitioner->state.frame_num++;
transitioner->state.frame_num %= cnt_of_array( transitioner->state.unregister );

frame = &transitioner->state.unregister[ transitioner->state.frame_num ];
for( i = 0; i < frame->count; i++ )
    {
    do_unregister( frame->arr[ i ].uid, transitioner );
    clr_struct( &frame->arr[ i ] );
    }

frame->count = 0;

}   /* begin_frame() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       commit
*
*********************************************************************/

static void commit
    (
    const VkCommandBuffer
                        commands,   /* on which to commit           */
    struct _VKN_transitioner_type
                       *master,     /* master resource transitioner */
    struct _VKN_transitioner_type
                       *transitioner/* resource transitioner        */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_transitioner_barrier_type
                       *barrier;    /* working barrier             */
VKN_transitioner_resource_type
                       *resource;   /* working resource            */
VKN_transitioner_subresource_type
                       *subresource;/* working subresource         */
VKN_transitioner_subresource_type   /* working subresource head    */
                      **subresource_head;
VKN_transitioner_resource_type
                       *to_save;    /* resource to save            */
VKN_transitioner_subresource_type   /* subresource to save         */
                       *to_save_subresource;

debug_assert( master->state.is_master
             && !transitioner->state.is_master );

/*----------------------------------------------------------
Run the instance's barrier transitions over master state
----------------------------------------------------------*/
while( transitioner->state.prepends )
    {
    barrier = transitioner->state.prepends;
    resource = find_resource_by_uid( barrier->uid, master );
    debug_assert( resource );
    if( resource )
        {
        master->i->barrier( resource->uid, resource->mip_count, barrier->src_stages, barrier->dst_stages, &barrier->barrier, commands, master );
        }

    free_barrier( barrier, &transitioner->state.prepends, transitioner );
    }

/*----------------------------------------------------------
Save the final state of all resources
----------------------------------------------------------*/
for( to_save = transitioner->state.resources; to_save; to_save = to_save->next )
    {
    resource = find_resource_by_uid( to_save->uid, master );
    if( !resource )
        {
        /*--------------------------------------------------
        Unable to find the resource on master, did you
        forget to register it?
        --------------------------------------------------*/
        debug_assert_always();
        return;
        }
    
    if( resource->all_subresources != VK_IMAGE_LAYOUT_MAX_ENUM
     && to_save->all_subresources != VK_IMAGE_LAYOUT_MAX_ENUM )
        {
        /*--------------------------------------------------
        Both are tracked at the 'all resources' level
        --------------------------------------------------*/
        resource->all_subresources = to_save->all_subresources;
        }
    else if( resource->all_subresources == VK_IMAGE_LAYOUT_MAX_ENUM )
        {
        /*--------------------------------------------------
        Master moving from 'subresources' to 'all resources'
        --------------------------------------------------*/
        while( resource->subresources )
            {
            free_subresource( resource->subresources, &resource->subresources, transitioner );
            }

        resource->all_subresources = to_save->all_subresources;
        }
    else
        {
        /*--------------------------------------------------
        Master moving from 'all resources' to 'subresources'
        --------------------------------------------------*/
        subresource_head = &resource->subresources;
        for( to_save_subresource = to_save->subresources; to_save_subresource; to_save_subresource = to_save_subresource->next )
            {
            subresource = alloc_subresource( master );
            if( !subresource )
                {
                debug_assert_always();
                return;
                }

            *subresource = *to_save_subresource;
            subresource->next = NULL;
            push_subresource( subresource, subresource_head );
            subresource_head = &subresource->next;
            }
        }
    }

transitioner->state.prepends_tail = NULL;
   
}   /* commit() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       create_resource_from_barrier
*
*********************************************************************/

static void create_resource_from_barrier
    (
    const u32           uid,        /* image unique id              */
    const VkImage       image,      /* image resource               */
    const u32           mip_levels, /* image number of mip levels   */
    const VkImageMemoryBarrier
                       *barrier,    /* resource barrier             */
    VKN_transitioner_type
                       *transitioner/* resource transitioner        */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_transitioner_resource_type
                       *resource;   /* working resource             */
VKN_transitioner_subresource_type
                       *subresource;/* working subresource          */

debug_assert( barrier->subresourceRange.baseMipLevel + barrier->subresourceRange.levelCount <= mip_levels );

resource = alloc_resource( transitioner );
if( !resource )
    {
    debug_assert_always();
    return;
    }

push_resource( resource, &transitioner->state.resources );
resource->uid       = uid;
resource->mip_count = mip_levels;

/*----------------------------------------------------------
Simple create with all subresources
----------------------------------------------------------*/
if( barrier->subresourceRange.baseMipLevel == 0
 && barrier->subresourceRange.levelCount == mip_levels )
    {
    resource->all_subresources = barrier->newLayout;
    return;
    }

/*------------------------------------------------------
Disjoint subresources
------------------------------------------------------*/
/* succeeding range */
if( barrier->subresourceRange.baseMipLevel + barrier->subresourceRange.levelCount < mip_levels )
    {
    subresource = alloc_subresource( transitioner );
    if( !subresource )
        {
        debug_assert_always();
        free_resource( resource, &transitioner->state.resources, transitioner );
        return;
        }

    push_subresource( subresource, &resource->subresources );

    subresource->start_mip = barrier->subresourceRange.baseMipLevel + barrier->subresourceRange.levelCount;
    subresource->mip_count = mip_levels - subresource->start_mip;
    subresource = NULL;
    }

/* barrier range */
subresource = alloc_subresource( transitioner );
if( !subresource )
    {
    free_resource( resource, &transitioner->state.resources, transitioner );
    debug_assert_always();
    return;
    }

push_subresource( subresource, &resource->subresources );

subresource->start_mip = barrier->subresourceRange.baseMipLevel;
subresource->mip_count = barrier->subresourceRange.levelCount;
subresource->state     = barrier->newLayout;
subresource = NULL;

/* preceding range */
if( barrier->subresourceRange.baseMipLevel > 0 )
    {
    subresource = alloc_subresource( transitioner );
    if( !subresource )
        {
        debug_assert_always();
        free_resource( resource, &transitioner->state.resources, transitioner );
        return;
        }

    push_subresource( subresource, &resource->subresources );

    subresource->start_mip = 0;
    subresource->mip_count = barrier->subresourceRange.baseMipLevel;
    subresource = NULL;
    }

return;

}   /* create_resource_from_barrier() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       do_unregister
*
*********************************************************************/

static void do_unregister
    (
    const u32           uid,        /* resource uid                 */
    struct _VKN_transitioner_type
                       *transitioner/* resource transitioner        */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_transitioner_resource_type
                      **head;       /* working resource head        */
VKN_transitioner_resource_type
                       *precede;    /* preceding resource           */
VKN_transitioner_resource_type
                       *resource;   /* working resource             */

for( precede = NULL, resource = transitioner->state.resources; resource; precede = resource, resource = resource->next )
    {
    if( resource->uid != uid )
        {
        continue;
        }

    head = &transitioner->state.resources;
    if( precede )
        {
        head = &precede->next;
        }
    
    free_resource( resource, head, transitioner );
    return;
    }

}   /* do_unregister() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       find_resource_by_uid
*
*********************************************************************/

static VKN_transitioner_resource_type * find_resource_by_uid
    (
    const u32           uid,        /* resource uid                 */
    VKN_transitioner_type
                       *transitioner/* resource transitioner        */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_transitioner_resource_type
                       *resource;   /* working resource            */

for( resource = transitioner->state.resources; resource; resource = resource->next )
    {
    if( resource->uid == uid )
        {
        /*--------------------------------------------------
        Found it
        --------------------------------------------------*/
        break;
        }
    }

return( resource );

}   /* find_resource_by_uid() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       free_resource
*
*********************************************************************/

static void free_resource
    (
    VKN_transitioner_resource_type
                       *to_free,    /* resource to free             */
    VKN_transitioner_resource_type
                      **head,       /* head of resources            */
    VKN_transitioner_type
                       *transitioner/* resource transitioner        */
    )
{
if( head )
    {
    *head = to_free->next;
    }

while( to_free->subresources )
    {
    free_subresource( to_free->subresources, &to_free->subresources, transitioner );
    }

clr_struct( to_free );
push_resource( to_free, &transitioner->state.resource_free );

}   /* free_resource() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       free_subresource
*
*********************************************************************/

static void free_subresource
    (
    VKN_transitioner_subresource_type
                       *to_free,    /* subresource to free          */
    VKN_transitioner_subresource_type
                      **head,       /* head of subresources         */
    VKN_transitioner_type
                       *transitioner/* resource transitioner        */
    )
{
*head = to_free->next;
clr_struct( to_free );
push_subresource( to_free, &transitioner->state.subresource_free );

}   /* free_subresource() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       register_new
*
*********************************************************************/

static void register_new
    (
    const u32           uid,        /* image uid                    */
    const VkImage       image,      /* image resource               */
    const VkImageLayout initial,    /* initial layout               */
    const u32           mip_levels, /* image number of mip levels   */
    struct _VKN_transitioner_type
                       *transitioner/* resource transitioner        */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_transitioner_resource_type
                       *resource;   /* working resource            */

if( !transitioner->state.is_master )
    {
    /*------------------------------------------------------
    Registration is master only operation
    ------------------------------------------------------*/
    return;
    }

resource = find_resource_by_uid( uid, transitioner );
if( resource )
    {
    /*------------------------------------------------------
    Unexpected double registration of resource
    ------------------------------------------------------*/
    debug_assert( resource->all_subresources == initial );
    debug_assert( resource->mip_count == mip_levels );
    return;
    }

resource = alloc_resource( transitioner );
if( !resource )
    {
    debug_assert_always();
    return;
    }

push_resource( resource, &transitioner->state.resources );

resource->uid              = uid;
resource->all_subresources = initial;
resource->mip_count        = mip_levels;

}   /* register_new() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       reset
*
*********************************************************************/

static void reset
    (
    struct _VKN_transitioner_type
                       *transitioner/* transitioner to reset        */
    )
{
while( transitioner->state.resources )
    {
    free_resource( transitioner->state.resources, &transitioner->state.resources, transitioner );
    }
    
debug_assert( !transitioner->state.prepends
             && !transitioner->state.prepends_tail );

transitioner->state.prepends_tail = NULL;
while( transitioner->state.prepends )
    {
    free_barrier( transitioner->state.prepends, &transitioner->state.prepends, transitioner );
    }
    
}   /* reset() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       transition_resource
*
*********************************************************************/

static void transition_resource
    (
    const VkPipelineStageFlags
                        src_stages, /* source pipeline stages       */
    const VkPipelineStageFlags
                        dst_stages, /* destination pipeline stages  */
    const VkImageMemoryBarrier
                       *barrier,    /* resource barrier             */
    VkCommandBuffer     commands,   /* command buffer for barrier   */
    VKN_transitioner_resource_type
                       *resource,   /* resource to transition       */
    VKN_transitioner_type
                       *transitioner/* resource transitioner        */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkImageMemoryBarrier    new_barrier;/* working barrier             */
VKN_transitioner_subresource_type   
                       *subresource;/* working subresource         */

/*----------------------------------------------------------
Validate the request
----------------------------------------------------------*/
if( resource->mip_count < barrier->subresourceRange.baseMipLevel + barrier->subresourceRange.levelCount )
    {
    debug_assert_always();
    return;
    }

/*----------------------------------------------------------
All subresources to all subresources
----------------------------------------------------------*/
if( !resource->subresources
 && barrier->subresourceRange.baseMipLevel == 0
 && barrier->subresourceRange.levelCount == resource->mip_count )
    {
    debug_assert( resource->all_subresources != VK_IMAGE_LAYOUT_MAX_ENUM );
    if( resource->all_subresources == barrier->newLayout )
        {
        /*--------------------------------------------------
        Already in this state
        --------------------------------------------------*/
        return;
        }

    new_barrier           = *barrier;
    new_barrier.oldLayout = resource->all_subresources;
    do_image_barrier( src_stages, dst_stages, &new_barrier, commands );

    resource->all_subresources = barrier->newLayout;
    return;
    }

/*----------------------------------------------------------
Disjoint subresources to all subresources
----------------------------------------------------------*/
if( resource->subresources
 && barrier->subresourceRange.baseMipLevel == 0
 && barrier->subresourceRange.levelCount == resource->mip_count )
    {
    debug_assert( resource->all_subresources == VK_IMAGE_LAYOUT_MAX_ENUM );

    /*------------------------------------------------------
    Transition each subresource to the new layout
    ------------------------------------------------------*/
    for( subresource = resource->subresources; subresource; subresource = subresource->next )
        {
        if( subresource->state == barrier->newLayout )
            {
            continue;
            }

        new_barrier = *barrier;
        new_barrier.oldLayout                     = subresource->state;
        new_barrier.subresourceRange.baseMipLevel = subresource->start_mip;
        new_barrier.subresourceRange.levelCount   = subresource->mip_count;
        do_image_barrier( src_stages, dst_stages, &new_barrier, commands );
        }

    /*------------------------------------------------------
    Now convert it to track all subresources
    ------------------------------------------------------*/
    resource->all_subresources = barrier->newLayout;
    while( resource->subresources )
        {
        free_subresource( resource->subresources, &resource->subresources, transitioner );
        }

    return;
    }

/*----------------------------------------------------------
All subresources to disjoint subresources
----------------------------------------------------------*/
if( !resource->subresources )
    {
    /*------------------------------------------------------
    Convert it to subresource form so we can do the work
    generically
    ------------------------------------------------------*/
    resource->subresources = alloc_subresource( transitioner );
    if( !resource->subresources )
        {
        debug_assert_always();
        return;
        }

    resource->subresources->state     = resource->all_subresources;
    resource->subresources->start_mip = 0;
    resource->subresources->mip_count = resource->mip_count;

    resource->all_subresources = VK_IMAGE_LAYOUT_MAX_ENUM;
    }

transition_subresources( src_stages, dst_stages, barrier, commands, resource, transitioner );

}   /* transition_resource() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       transition_subresources
*
*********************************************************************/

static void transition_subresources
    (
    const VkPipelineStageFlags
                        src_stages, /* source pipeline stages       */
    const VkPipelineStageFlags
                        dst_stages, /* destination pipeline stages  */
    const VkImageMemoryBarrier
                       *barrier,    /* resource barrier             */
    VkCommandBuffer     commands,   /* command buffer for barrier   */
    VKN_transitioner_resource_type
                       *resource,   /* resource to transition       */
    VKN_transitioner_type
                       *transitioner/* resource transitioner        */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     affected_count;
                                    /* working mip count affected  */
u32                     end_mip;    /* subresource range end mip   */
u32                     end_mip_barrier;
                                    /* subresource range end mip   */
VkImageMemoryBarrier    new_barrier;/* working barrier             */
VKN_transitioner_subresource_type
                       *split;      /* split subresource           */
VKN_transitioner_subresource_type   
                       *subresource;/* working subresource         */

debug_assert( resource->all_subresources == VK_IMAGE_LAYOUT_MAX_ENUM
           && resource->subresources );
             
/*----------------------------------------------------------
First generate the barriers for affected intervals
----------------------------------------------------------*/
end_mip_barrier = ( barrier->subresourceRange.baseMipLevel + barrier->subresourceRange.levelCount ) - 1;
for( subresource = resource->subresources; subresource; subresource = subresource->next )
    {
    end_mip = ( subresource->start_mip + subresource->mip_count ) - 1;
    if( end_mip < barrier->subresourceRange.baseMipLevel
     || subresource->start_mip > end_mip_barrier
     || subresource->state == barrier->newLayout )
        {
        /*--------------------------------------------------
        Subresource unaffected
        --------------------------------------------------*/
        continue;
        }
    
    new_barrier = *barrier;
    new_barrier.oldLayout                     = subresource->state;
    new_barrier.subresourceRange.baseMipLevel = (u32)VKN_size_max( subresource->start_mip, barrier->subresourceRange.baseMipLevel );
    new_barrier.subresourceRange.levelCount   = 1 + (u32)VKN_size_min( end_mip, end_mip_barrier ) - new_barrier.subresourceRange.baseMipLevel;
    do_image_barrier( src_stages, dst_stages, &new_barrier, commands );
    }

/*----------------------------------------------------------
Next split and update intervals to reflect the new state
----------------------------------------------------------*/
for( subresource = resource->subresources; subresource; subresource = subresource->next )
    {
    end_mip = ( subresource->start_mip + subresource->mip_count ) - 1;
    if( end_mip < barrier->subresourceRange.baseMipLevel
     || subresource->start_mip > end_mip_barrier
     || subresource->state == barrier->newLayout )
        {
        /*--------------------------------------------------
        Subresource unaffected
        --------------------------------------------------*/
        continue;
        }
        
    affected_count = 1 + (u32)VKN_size_min( end_mip, end_mip_barrier )
                       - (u32)VKN_size_max( subresource->start_mip, barrier->subresourceRange.baseMipLevel );

    if( affected_count == subresource->mip_count )
        {
        /*--------------------------------------------------
        Whole range is affected, so just update the state
        --------------------------------------------------*/
        subresource->state = barrier->newLayout;
        continue;
        }

    /*------------------------------------------------------
    Split the subresource
    ------------------------------------------------------*/
    split = alloc_subresource( transitioner );
    if( !split )
        {
        debug_assert_always();
        return;
        }

    split->next       = subresource->next;
    subresource->next = split;

    if( subresource->start_mip < barrier->subresourceRange.baseMipLevel )
        {
        /* transitioned state is at end */
        split->state     = barrier->newLayout;
        split->start_mip = barrier->subresourceRange.baseMipLevel;
        split->mip_count = affected_count;

        subresource->mip_count -= split->mip_count;
        }
    else
        {
        /* transitioned state is at start */
        split->state     = subresource->state;
        split->start_mip = subresource->start_mip + affected_count;
        split->mip_count = subresource->mip_count - affected_count;

        subresource->state     = barrier->newLayout;
        subresource->mip_count = affected_count;
        }
    }

/*----------------------------------------------------------
Finally merge any contiguous regions that now have the same
state
----------------------------------------------------------*/
for( subresource = resource->subresources; subresource; )
    {
    if( !subresource->next
     || subresource->next->state != subresource->state )
        {
        subresource = subresource->next;
        continue;
        }

    /*------------------------------------------------------
    Merge the subresources
    ------------------------------------------------------*/
    subresource->mip_count += subresource->next->mip_count;
    free_subresource( subresource->next, &subresource->next, transitioner );
    }

}   /* transition_subresources() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       unregister
*
*********************************************************************/

static void unregister
    (
    const u32           uid,        /* resource uid                 */
    struct _VKN_transitioner_type
                       *transitioner/* resource transitioner        */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_transitioner_unregister_frame_type
                       *frame;      /* current frame                */

/*----------------------------------------------------------
Registration only takes place on master
----------------------------------------------------------*/
if( !transitioner->state.is_master )
    {
    return;
    }

/*----------------------------------------------------------
We want to defer release until begin_frame()
----------------------------------------------------------*/
frame = &transitioner->state.unregister[ transitioner->state.frame_num ];
if( frame->count >= frame->capacity )
    {
    debug_assert_always();
    return;
    }

frame->arr[ frame->count++ ].uid = uid;

}   /* unregister() */
