#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "LinearAllocator.hpp"
#include "Utilities.hpp"


/*******************************************************************
*
*   get_free_bytes()
*
*   DESCRIPTION:
*       Get the number of free bytes left in the allocator.
*
*******************************************************************/

static inline uint64_t get_free_bytes( const LinearAllocator *allocator )
{
return( allocator->capacity - allocator->head );

} /* get_free_bytes() */


static void *allocate( const uint64_t sz, LinearAllocator *allocator );


/*******************************************************************
*
*   LinearAllocator_Allocate()
*
*   DESCRIPTION:
*       Create an unaligned allocation in the allocator.
*
*******************************************************************/

void * LinearAllocator_Allocate( const uint64_t sz, LinearAllocator *allocator )
{
void *ret = allocate( sz, allocator );
debug_assert( ret != NULL );

allocator->allocations_cnt++;
return( ret );

} /* LinearAllocator_Allocate() */


/*******************************************************************
*
*   LinearAllocator_AllocateAligned()
*
*   DESCRIPTION:
*       Create an allocation in the allocator which satisfies the
*       given alignment.  This is the preferred method of making
*       allocations, unless you know what you're doing.
*
*******************************************************************/

void * LinearAllocator_AllocateAligned( const uint64_t sz, const uint64_t alignment, LinearAllocator *allocator )
{
uint64_t adjust = align_adjust( &allocator->pool[ allocator->head ], alignment );
uint64_t allocation_sz = sz + adjust;

void *ret = allocate( allocation_sz, allocator );
debug_assert( ret != NULL );

allocator->allocations_cnt++;
return( ret );

} /* LinearAllocator_AllocateAligned() */


/*******************************************************************
*
*   LinearAllocator_Destroy()
*
*   DESCRIPTION:
*       Free the resources of a linear allocator and reset it.
*
*******************************************************************/

void LinearAllocator_Destroy( LinearAllocator *allocator )
{
if( allocator->pool
 && allocator->is_own_pool )
    {
    free( allocator->pool );
    }

*allocator = {};

} /* LinearAllocator_Destroy() */


/*******************************************************************
*
*   LinearAllocator_GetResetToken()
*
*   DESCRIPTION:
*       Get a token that will allow the given allocator to be reset
*       to its current state.
*
*******************************************************************/

LinearAllocatorResetToken LinearAllocator_GetResetToken( LinearAllocator *allocator )
{
LinearAllocatorResetToken ret = {};
ret.allocations_cnt = allocator->allocations_cnt;
ret.head            = allocator->head;

#if defined( _DEBUG )
ret.pool = allocator->pool;
#endif

return( ret );

} /* LinearAllocator_GetResetToken() */


/*******************************************************************
*
*   LinearAllocator_Init()
*
*   DESCRIPTION:
*       Initialize a new linear allocator with the given capacity.
*
*******************************************************************/

bool LinearAllocator_Init( const uint64_t capacity, LinearAllocator *allocator )
{
*allocator = {};
uint8_t *pool = (uint8_t*)malloc( capacity );
if( !pool )
    {
    debug_assert_always();
    return( false );
    }

allocator->pool = pool;
allocator->capacity = capacity;
allocator->is_own_pool = true;

return( true );

} /* LinearAllocator_Init() */


/*******************************************************************
*
*   LinearAllocator_InitAttached()
*
*   DESCRIPTION:
*       Initialize a new linear allocator as bound to an existing
*       buffer with the given capacity.
*
*******************************************************************/

bool LinearAllocator_InitAttached( const uint64_t capacity, void *buffer, LinearAllocator *allocator )
{
*allocator = {};
uint8_t *pool = (uint8_t*)buffer;
if( !pool )
    {
    debug_assert_always();
    return( false );
    }

allocator->pool = pool;
allocator->capacity = capacity;

return( true );

} /* LinearAllocator_InitAttached() */


/*******************************************************************
*
*   LinearAllocator_InitDetached()
*
*   DESCRIPTION:
*       Initialize a new linear allocator not bound to a buffer, but
*       managing offsets and capacity.
*
*******************************************************************/

void LinearAllocator_InitDetached( const uint64_t capacity, LinearAllocator *allocator )
{
*allocator = {};
allocator->pool = (uint8_t*)LINEAR_ALLOCATOR_DETACHED_BASE_ADDRESS;
allocator->capacity = capacity;

} /* LinearAllocator_InitDetached() */


/*******************************************************************
*
*   LinearAllocator_Reset()
*
*   DESCRIPTION:
*       Reset the linear allocator, retaining the pool.
*
*******************************************************************/

void LinearAllocator_Reset( LinearAllocator *allocator )
{
debug_if( true,
    memset( allocator->pool, 0, allocator->capacity );
    );

allocator->head = 0;
allocator->allocations_cnt = 0;

} /* LinearAllocator_Reset() */


/*******************************************************************
*
*   LinearAllocator_ResetByToken()
*
*   DESCRIPTION:
*       Reset the allocator to the state it was when the given token
*       was generated.  
* 
*   NOTE:
*       This will *NOT* repair changes to any allocated memory
*       existing when the token was generated, but altered after the
*       token was generated.
*
*******************************************************************/

void LinearAllocator_ResetByToken( const LinearAllocatorResetToken token, LinearAllocator *allocator )
{
debug_assert( allocator->pool == token.pool );
debug_assert( allocator->head >= token.head );
debug_assert( allocator->allocations_cnt >= token.allocations_cnt );

allocator->head = token.head;
allocator->allocations_cnt = token.allocations_cnt;

} /* LinearAllocator_ResetByToken() */


/*******************************************************************
*
*   allocate()
*
*   DESCRIPTION:
*       Make a new allocation.
*
*******************************************************************/

static void * allocate( const uint64_t sz, LinearAllocator *allocator )
{
if( sz > get_free_bytes( allocator ) )
    {
    return( NULL );
    }

void *ret = &allocator->pool[ allocator->head ];
allocator->head += sz;

return( ret );

} /* allocate() */

