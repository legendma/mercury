#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "Component.hpp"
#include "Entity.hpp"
#include "Utilities.hpp"


#define REGISTRY_SWAP_STORAGE       ( 1 )
#define INVALID_DENSE_INDEX         max_uint_value( *( (ComponentRegistry*)NULL )->sparse )

namespace ECS
{
static void   EnsureStorageForEntity( const EntityId entity, ComponentRegistry *registry );
static void   ExpandDenseAndStorage( const uint32_t new_size, ComponentRegistry *registry );
static void   ExpandSparse( const uint32_t new_size, ComponentRegistry *registry );
static void * GetStorageAtIndex( const uint32_t index, ComponentRegistry *registry );
static void   SwapMemory( const size_t size, void *a, void *b, void *scratch );


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
if( entity.u.id >= registry->sparse_capacity )
    {
    return( false );
    }

uint32_t dense_index = registry->sparse[ entity.u.id ];
if( dense_index > registry->dense_count )
    {
    return( false );
    }

return( registry->dense[ dense_index ].id_and_version == entity.id_and_version );

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

void * Component_GetComponentAtDenseIndex( const uint32_t dense_index, ComponentRegistry *registry )
{
debug_assert( dense_index < registry->dense_count );
return( GetStorageAtIndex( dense_index, registry ) );

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

void Component_InitRegistry( const size_t storage_stride, const ComponentClass cls, ComponentRegistry *registry )
{
*registry = {};
registry->storage_stride = storage_stride;
registry->cls            = cls;

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

void *scratch = GetStorageAtIndex( registry->dense_storage_capacity, registry ); // TODO <MPA> - Rework this so we don't have to require scratch for singleton components which will never reach a size of more than 1
SwapMemory( registry->storage_stride,  GetStorageAtIndex( dense_remove, registry ), GetStorageAtIndex( dense_swap, registry ), scratch );
SwapMemory( sizeof(*registry->sparse), &registry->sparse[ sparse_remove ],          &registry->sparse[ sparse_swap ],          scratch );
SwapMemory( sizeof(*registry->dense),  &registry->dense[ dense_remove ],            &registry->dense[ dense_swap ],            scratch );

registry->dense[ dense_swap ].id_and_version = INVALID_ENTITY_ID;
registry->sparse[ sparse_remove ]            = INVALID_DENSE_INDEX;
registry->dense_count--;

}   /* Component_RemoveComponent() */


/*******************************************************************
*
*   Component_ReportMetrics()
*
*   DESCRIPTION:
*       Report the registry's technical metrics.
*
*******************************************************************/

void Component_ReportMetrics( const ComponentRegistry *registry, size_t *memory_usage )
{
/* memory usage */
if( memory_usage )
    {
    *memory_usage = registry->sparse_capacity                                    * sizeof(*registry->sparse)
                  + registry->dense_storage_capacity                             * sizeof(*registry->dense)
                  + ( registry->dense_storage_capacity + REGISTRY_SWAP_STORAGE ) * registry->storage_stride;
    }

}   /* Component_ReportMetrics() */


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
/* sparse */
if( entity.u.id >= registry->sparse_capacity )
    {
    uint32_t new_size = 2 * Utilities_ClampToMinU32( registry->sparse_capacity, entity.u.id + 1 );
    ExpandSparse( new_size, registry );
    }

/* dense and storage */
if( registry->dense_count >= registry->dense_storage_capacity )
    {
    uint32_t new_size = Utilities_ClampToMinU32( 2 * registry->dense_storage_capacity, 1 );
    ExpandDenseAndStorage( new_size, registry );
    }

}   /* EnsureStorageForEntity() */


/*******************************************************************
*
*   ExpandDenseAndStorage()
*
*   DESCRIPTION:
*       Enlarge the dense and storage by reallocating at a new size.
*
*******************************************************************/

static void ExpandDenseAndStorage( const uint32_t new_size, ComponentRegistry *registry )
{
/* we are only ever allowed to grow, never shrink */
assert( new_size > registry->dense_storage_capacity );
EntityId *new_dense   = (EntityId*)realloc( registry->dense, sizeof(*new_dense) * new_size );
uint8_t  *new_storage = (uint8_t*)realloc( registry->storage, registry->storage_stride * ( new_size + REGISTRY_SWAP_STORAGE ) );

if( !new_dense
 || !new_storage )
    {
    free( new_dense );
    free( new_storage );
    assert( false );
    return;
    }

registry->dense    = new_dense;
registry->storage  = new_storage;

for( uint32_t i = registry->dense_storage_capacity; i < new_size; i++ )
    {
    /* mark the new unused dense entries as invalid */
    registry->dense[ i ].id_and_version = INVALID_ENTITY_ID;
    }

registry->dense_storage_capacity = new_size;

}   /* ExpandDenseAndStorage() */


/*******************************************************************
*
*   ExpandSparse()
*
*   DESCRIPTION:
*       Enlarge the sparse by reallocating at a new size.
*
*******************************************************************/

static void ExpandSparse( const uint32_t new_size, ComponentRegistry *registry )
{
/* we are only ever allowed to grow, never shrink */
assert( new_size > registry->sparse_capacity );
uint32_t *new_sparse  = (uint32_t*)realloc( registry->sparse, sizeof(*new_sparse) * new_size );

if( !new_sparse )
    {
    free( new_sparse );
    assert( false );
    return;
    }

registry->sparse = new_sparse;

for( uint32_t i = registry->sparse_capacity; i < new_size; i++ )
    {
    /* mark the new unused dense entries as invalid */
    registry->sparse[ i ] = INVALID_DENSE_INDEX;
    }

registry->sparse_capacity = new_size;

}   /* ExpandSparse() */


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