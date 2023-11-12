#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "Component.hpp"
#include "Entity.hpp"
#include "Utilities.hpp"

#define REGISTRY_START_CNT          ( 4 )
#define REGISTRY_SWAP_STORAGE       ( 1 )

namespace ECS
{
static void EnsureStorageForEntity( const EntityId entity, ComponentRegistry *registry );
static void ExpandRegistry( const uint32_t new_size, ComponentRegistry *registry );
static void * GetStorageAtIndex( const uint32_t index, ComponentRegistry *registry );
static void SwapMemory( const size_t size, void *a, void *b, void *scratch );


/*******************************************************************
*
*   Component_AttachComponent()
*
*   DESCRIPTION:
*       Attach this registry's component to the given entity.
*
*******************************************************************/

void * Component_AttachComponent( const EntityId entity, ComponentRegistry *registry )
{
if( Component_EntityHasComponent( entity, registry ) )
    {
    assert( false );
    return( Component_GetComponent( entity, registry ) );
    }
else if( entity.id_and_version == INVALID_ENTITY_ID )
    {
    assert( false );
    return( NULL );
    }

EnsureStorageForEntity( entity, registry );

/* Add the new capacity and clear the storage */
uint32_t new_dense = registry->dense_count;
registry->sparse[ entity.u.id ] = new_dense;
registry->dense[ new_dense ] = entity;
registry->dense_count++;

void *ret = GetStorageAtIndex( new_dense, registry );
memset( ret, 0, registry->storage_stride );

return( ret );

}   /* Component_AttachComponent() */


/*******************************************************************
*
*   Component_DestroyRegistry()
*
*   DESCRIPTION:
*       Free the entire component registry, and return it to
*       uninitialized.
*
*******************************************************************/

void Component_DestroyRegistry( ComponentRegistry *registry )
{
free( registry->sparse );
free( registry->dense );
free( registry->storage );
*registry = {};

}   /* Component_DestroyRegistry() */


/*******************************************************************
*
*   Component_EntityHasComponent()
*
*   DESCRIPTION:
*       Does the given entity have a component of the type managed
*       by this registry?
*
*******************************************************************/

bool Component_EntityHasComponent( const EntityId entity, const ComponentRegistry *registry )
{
if( entity.u.id >= registry->capacity )
    {
    return( false );
    }

return( registry->dense[ registry->sparse[ entity.u.id ] ].id_and_version == entity.id_and_version );

}   /* Component_EntityHasComponent() */


/*******************************************************************
*
*   Component_GetComponent()
*
*   DESCRIPTION:
*       Allocate the registry's initial empty sparse array and
*       storage.
*
*******************************************************************/

void * Component_GetComponent( const EntityId entity, ComponentRegistry *registry )
{
if( !Component_EntityHasComponent( entity, registry ) )
    {
    return( NULL );
    }

return( GetStorageAtIndex( registry->sparse[ entity.u.id ], registry ) );

}   /* Component_GetComponent() */


/*******************************************************************
*
*   Component_GetComponentCount()
*
*   DESCRIPTION:
*       Get the current number of components in use.
*
*******************************************************************/

uint32_t Component_GetComponentCount( const ComponentRegistry *registry )
{
return( registry->dense_count );

}   /* Component_GetComponentCount() */


/*******************************************************************
*
*   Component_GetComponentAtDenseIndex()
*
*   DESCRIPTION:
*       Get the component by dense index.
*
*******************************************************************/

void * Component_GetComponentAtDenseIndex( const uint32_t dense_index, const ComponentRegistry *registry )
{
debug_assert( dense_index < registry->dense_count );
return( &registry->storage[ dense_index ] );

}   /* Component_GetComponentAtDenseIndex() */


/*******************************************************************
*
*   Component_GetEntityAtDenseIndex()
*
*   DESCRIPTION:
*       Get the entity that owns the component instance at the given
*       dense index.
*
*******************************************************************/

EntityId Component_GetEntityAtDenseIndex( const uint32_t dense_index, const ComponentRegistry *registry )
{
debug_assert( dense_index < registry->dense_count );
return( registry->dense[ dense_index ] );

}   /* Component_GetEntityAtDenseIndex() */


/*******************************************************************
*
*   Component_InitRegistry()
*
*   DESCRIPTION:
*       Allocate the registry's initial empty sparse array and
*       storage.
*
*******************************************************************/

void Component_InitRegistry( size_t storage_stride, ComponentRegistry *registry )
{
*registry = {};
registry->storage_stride = storage_stride;

ExpandRegistry( REGISTRY_START_CNT, registry );

}   /* Entity_InitRegistry() */


/*******************************************************************
*
*   Component_RemoveComponent()
*
*   DESCRIPTION:
*       Remove the component from the given entity.
*
*******************************************************************/

void Component_RemoveComponent( const EntityId entity, ComponentRegistry *registry )
{
if( !Component_EntityHasComponent( entity, registry ) )
    {
    assert( false );
    return;
    }

/* swap-pack the dense array and storage */
uint32_t sparse_remove = entity.u.id;
uint32_t dense_remove  = registry->sparse[ entity.u.id ];
uint32_t dense_swap    = registry->dense_count - 1;
uint32_t sparse_swap   = registry->dense[ dense_swap ].u.id;
assert( registry->sparse[ sparse_swap ] == dense_swap );

void *scratch = GetStorageAtIndex( registry->capacity, registry );
SwapMemory( registry->storage_stride,  GetStorageAtIndex( dense_remove, registry ), GetStorageAtIndex( dense_swap, registry ), scratch );
SwapMemory( sizeof(*registry->sparse), &registry->sparse[ sparse_remove ],          &registry->sparse[ sparse_swap ],          scratch );
SwapMemory( sizeof(*registry->dense),  &registry->dense[ dense_remove ],            &registry->dense[ dense_swap ],            scratch );

registry->dense[ dense_swap ].id_and_version = INVALID_ENTITY_ID;
registry->dense_count--;

}   /* Component_RemoveComponent() */


/*******************************************************************
*
*   EnsureStorageForEntity()
*
*   DESCRIPTION:
*       Enlarge the registry by reallocating the sparse, dense,
*       and storage arrays at a new size.
*
*******************************************************************/

static void EnsureStorageForEntity( const EntityId entity, ComponentRegistry *registry )
{
if( entity.u.id < registry->capacity )
    {
    /* We're already good */
    return;
    }

/* Expand */
uint32_t new_size = 2 * registry->capacity;
if( new_size <= entity.u.id )
    {
    new_size = entity.u.id + 1;
    }

ExpandRegistry( new_size, registry );

}   /* EnsureStorageForEntity() */


/*******************************************************************
*
*   ExpandRegistry()
*
*   DESCRIPTION:
*       Enlarge the registry by reallocating the sparse, dense,
*       and storage arrays at a new size.
*
*******************************************************************/

static void ExpandRegistry( const uint32_t new_size, ComponentRegistry *registry )
{
/* we are only ever allowed to grow, never shrink */
assert( new_size > registry->capacity );
uint32_t *new_sparse  = (uint32_t*)malloc( sizeof(*new_sparse) * new_size );
EntityId *new_dense   = (EntityId*)malloc( sizeof(*new_dense)  * new_size );
uint8_t  *new_storage = (uint8_t*)malloc( registry->storage_stride * ( new_size + REGISTRY_SWAP_STORAGE ) );

if( !new_sparse
 || !new_dense
 || !new_storage )
    {
    free( new_sparse );
    free( new_dense );
    free( new_storage );
    assert( false );
    return;
    }

if( registry->capacity > 0 )
    {
    /* Copy over the previous arrays to our larger new ones and then get rid of the old ones */
    memcpy( new_sparse,  registry->sparse,  registry->capacity * sizeof(*registry->sparse) );
    memcpy( new_dense,   registry->dense,   registry->capacity * sizeof(*registry->dense) );
    memcpy( new_storage, registry->storage, registry->capacity * sizeof(*registry->storage) * registry->storage_stride );

    free( registry->sparse );
    free( registry->dense );
    free( registry->storage );

    registry->sparse  = NULL;
    registry->dense   = NULL;
    registry->storage = NULL;
    }

registry->sparse   = new_sparse;
registry->dense    = new_dense;
registry->storage  = new_storage;

for( uint32_t i = registry->capacity; i < new_size; i++ )
    {
    /* mark the new unused dense entries as invalid */
    registry->sparse[ i ] = i;
    registry->dense[ i ].id_and_version = INVALID_ENTITY_ID;
    }

registry->capacity = new_size;

}   /* ExpandRegistry() */


/*******************************************************************
*
*   GetStorageAtIndex()
*
*   DESCRIPTION:
*       Address the component storage for the given dense index.
*
*******************************************************************/

static void * GetStorageAtIndex( const uint32_t index, ComponentRegistry *registry )
{
uint32_t index_of_storage = index * (uint32_t)registry->storage_stride;
return( (void*)&registry->storage[ index_of_storage ] );

}   /* GetStorageAtIndex() */


/*******************************************************************
*
*   SwapMemory()
*
*   DESCRIPTION:
*       Swap the memory and A and B, using the scratch buffer.
*
*******************************************************************/

static void SwapMemory( const size_t size, void *a, void *b, void *scratch )
{
memcpy( scratch, a, size );
memcpy( a, b, size );
memcpy( b, scratch, size );

}   /* SwapMemory() */


} /* namespace ECS */