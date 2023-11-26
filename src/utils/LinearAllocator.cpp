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

static inline uint32_t get_free_bytes( const LinearAllocator *allocator )
{
return( allocator->capacity - allocator->head );

} /* get_free_bytes() */


static void *allocate( const uint32_t sz, LinearAllocator *allocator );


/*******************************************************************
*
*   LinearAllocator_Allocate()
*
*   DESCRIPTION:
*       Create an unaligned allocation in the allocator.
*
*******************************************************************/

void * LinearAllocator_Allocate( const uint32_t sz, LinearAllocator *allocator )
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

void * LinearAllocator_AllocateAligned( const uint32_t sz, const uint8_t alignment, LinearAllocator *allocator )
{
uint8_t adjust = align_adjust( &allocator->pool[ allocator->head ], alignment );
uint32_t allocation_sz = sz + (uint32_t)adjust;

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
if( allocator->pool )
    {
    free( allocator->pool );
    }

*allocator = {};

} /* LinearAllocator_Destroy() */


/*******************************************************************
*
*   LinearAllocator_Init()
*
*   DESCRIPTION:
*       Initialize a new linear allocator with the given capacity.
*
*******************************************************************/

bool LinearAllocator_Init( const uint32_t capacity, LinearAllocator *allocator )
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

return( true );

} /* LinearAllocator_Init() */


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
*   allocate()
*
*   DESCRIPTION:
*       Make a new allocation.
*
*******************************************************************/

static void * allocate( const uint32_t sz, LinearAllocator *allocator )
{
if( sz > get_free_bytes( allocator ) )
    {
    return( NULL );
    }

void *ret = &allocator->pool[ allocator->head ];
allocator->head += sz;

return( ret );

} /* allocate() */

