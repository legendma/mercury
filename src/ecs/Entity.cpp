#include <cassert>
#include <cstdlib>
#include <cstring>

#include "Entity.hpp"

#define REGISTRY_START_CNT          ( 256 )

namespace ECS
{
static void ExpandRegistry( const uint32_t new_size, EntityRegistry *registry );
static EntityId RecycleEntity( EntityRegistry *registry );


/*******************************************************************
*
*   Entity_CreateEntity()
*
*   DESCRIPTION:
*       Recycle a dead entity by incrementing its version and
*       returning it.
*
*******************************************************************/

EntityId Entity_CreateEntity( EntityRegistry *registry )
{
if( registry->dead_count == 0 )
    {
    /* We're out of dead entities.  First we'll need to double the size of the registry to create new dead entities */
    ExpandRegistry( 2 * registry->capacity, registry );
    }

/* We have dead previous-generated entities that can be recycled */
return( RecycleEntity( registry ) );

}   /* Entity_CreateEntity() */


/*******************************************************************
*
*   Entity_DestroyEntity()
*
*   DESCRIPTION:
*       Make the given entity dead by incrementing its version and
*       entering it back into the dead pool.
*
*******************************************************************/

void Entity_DestroyEntity( const EntityId entity, EntityRegistry *registry )
{
if( !Entity_EntityIsAlive( entity, registry ) )
    {
    /* Requested entity was already dead */
    assert( false );
    return;
    }

if( registry->dead_count > 0 )
    {
    registry->arr[ registry->last_dead ].u.id = entity.u.id;
    }

registry->arr[ entity.u.id ].u.version++;
registry->last_dead = entity.u.id;
registry->dead_count++;

}   /* Entity_DestroyEntity() */


/*******************************************************************
*
*   Entity_DestroyRegistry()
*
*   DESCRIPTION:
*       Free the entire entity registry, and return it to
*       uninitialized.
*
*******************************************************************/

void Entity_DestroyRegistry( EntityRegistry *registry )
{
free( registry->arr );
*registry = {};

}   /* Entity_DestroyRegistry() */


/*******************************************************************
*
*   Entity_GetCurrentVersionOfEntity()
*
*   DESCRIPTION:
*       Given an entity ID, query its version.
*
*******************************************************************/

uint32_t Entity_GetCurrentVersionOfEntity( const EntityId entity, const EntityRegistry *registry )
{
return( registry->arr[ entity.u.id ].u.version );

}   /* Entity_GetCurrentVersionOfEntity() */


/*******************************************************************
*
*   Entity_InitRegistry()
*
*   DESCRIPTION:
*       Allocate the initial pool of entity IDs and set them all as
*       dead.
*
*******************************************************************/

void Entity_InitRegistry( EntityRegistry *registry )
{
*registry = {};
ExpandRegistry( REGISTRY_START_CNT, registry );

}   /* Entity_InitRegistry() */


/*******************************************************************
*
*   Entity_EntityIsAlive()
*
*   DESCRIPTION:
*       Check if the given entity is still alive.
*
*******************************************************************/

bool Entity_EntityIsAlive( const EntityId entity, const EntityRegistry *registry )
{
return( entity.u.version == Entity_GetCurrentVersionOfEntity( entity, registry ) );
}   /* Entity_EntityIsAlive() */


/*******************************************************************
*
*   ExpandRegistry()
*
*   DESCRIPTION:
*       Enlarge the pool of entities in this registry, in order to
*       create new dead ones.
*
*******************************************************************/

static void ExpandRegistry( const uint32_t new_size, EntityRegistry *registry )
{
/* we are only ever allowed to grow, never shrink */
assert( new_size > registry->capacity );
EntityId *new_arr = (EntityId*)malloc( new_size * sizeof( *new_arr ) );
if( !new_arr )
    {
    free( new_arr );
    assert( false );
    return;
    }

if( registry->capacity > 0 )
    {
    /* Copy over the previous array to our larger new one and then get rid of the old one */
    memcpy( new_arr, registry->arr, registry->capacity * sizeof( *registry->arr ) );
    free( registry->arr );
    registry->arr = NULL;
    }

registry->arr = new_arr;

for( uint32_t i = registry->capacity; i < new_size; i++ )
    {
    /* link the new dead entities */
    EntityId new_dead = {};
    new_dead.u.id = i;
    registry->arr[ i ] = new_dead;
    Entity_DestroyEntity( new_dead, registry );
    }

registry->capacity = new_size;

}   /* ExpandRegistry() */


/*******************************************************************
*
*   RecycleEntity()
*
*   DESCRIPTION:
*       Re-use a dead entity's ID, by incrementing its version and
*       removing it from the dead pool.
*
*******************************************************************/

static EntityId RecycleEntity( EntityRegistry *registry )
{
assert( registry->dead_count > 0 );
EntityId ret;
ret.u.id = registry->next_recycle.u.id;
ret.u.version = registry->arr[ registry->next_recycle.u.id ].u.version;

registry->next_recycle.u.id = registry->arr[ ret.u.id ].u.id;
registry->arr[ ret.u.id ].u.id = ret.u.id;
registry->dead_count--;

return( ret );

}   /* RecycleEntity() */


} /* namespace ECS */