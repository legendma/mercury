#include <cassert>
#include <cstdint>

#include "Entity.hpp"
#include "Component.hpp"
#include "Universe.hpp"

namespace ECS
{
static ComponentRegistry * GetComponentRegistry( const ComponentClass component, Universe *universe );
static const ComponentRegistry * GetComponentRegistryConst( const ComponentClass component, const Universe *universe );


/*******************************************************************
*
*   Universe_AttachComponentToEntity()
*
*   DESCRIPTION:
*       Attach the requested component to the entity.  If the entity
*       already has this component attached, return the existing
*       component instead.
*
*******************************************************************/

void * Universe_AttachComponentToEntity( const EntityId entity, const ComponentClass component, Universe *universe )
{
ComponentRegistry *component_registry = GetComponentRegistry( component, universe );
if( !component_registry )
    {
    return( NULL );
    }

if( Component_EntityHasComponent( entity, component_registry ) )
    {
    return( Component_GetComponent( entity, component_registry ) );
    }

return( Component_AttachComponent( entity, component_registry ) );

}   /* Universe_AttachComponentToEntity() */


/*******************************************************************
*
*   Universe_CreateNewEntity()
*
*   DESCRIPTION:
*       Create a new entity and return its ID.
*
*******************************************************************/

EntityId Universe_CreateNewEntity( Universe *universe )
{
return( Entity_CreateEntity( &universe->entities ) );

}   /* Universe_CreateNewEntity() */


/*******************************************************************
*
*   Universe_Destroy()
*
*   DESCRIPTION:
*       Deallocate the given universe as uninitialized.
*
*******************************************************************/

void Universe_Destroy( Universe *universe )
{
Entity_DestroyRegistry( &universe->entities );

for( uint32_t i = 0; i < cnt_of_array( universe->components ); i++ )
    {
    Component_DestroyRegistry( &universe->components[ i ] );
    }

*universe = {};

}   /* Universe_Destroy() */


/*******************************************************************
*
*   Universe_DestroyEntity()
*
*   DESCRIPTION:
*       Destroy the given entity by removing all its components and
*       invalidating the id.
*
*******************************************************************/

void Universe_DestroyEntity( const EntityId entity, Universe *universe )
{
for( uint32_t i = 0; i < cnt_of_array( universe->components ); i++ )
    {
    ComponentRegistry *component = &universe->components[ i ];
    if( Component_EntityHasComponent( entity, component ) )
        {
        Component_RemoveComponent( entity, component );
        }
    }

Entity_DestroyEntity( entity, &universe->entities );

}   /* Universe_DestroyEntity() */


/*******************************************************************
*
*   Universe_EntityHasComponent()
*
*   DESCRIPTION:
*       Check if the given entity has the given component attached.
*
*******************************************************************/

bool Universe_EntityHasComponent( const EntityId entity, const ComponentClass component, const Universe *universe )
{
const ComponentRegistry *component_registry = GetComponentRegistryConst( component, universe );
if( !component_registry )
    {
    return( false );
    }

return( Component_EntityHasComponent( entity, component_registry ) );

}   /* Universe_EntityHasComponent() */


/*******************************************************************
*
*   Universe_EntityIsAlive()
*
*   DESCRIPTION:
*       Is the given entity still alive?
*
*******************************************************************/

bool Universe_EntityIsAlive( const EntityId entity, const Universe *universe )
{
return( Entity_EntityIsAlive( entity, &universe->entities ) );

}   /* Universe_EntityIsAlive() */


/*******************************************************************
*
*   Universe_GetComponentRegistry()
*
*   DESCRIPTION:
*       Get the requested component registry as read-only.
*
*******************************************************************/

ComponentRegistry * Universe_GetComponentRegistry( const ComponentClass component, Universe *universe )
{
return( &universe->components[ component ] );

}   /* Universe_GetComponentRegistry() */


/*******************************************************************
*
*   Universe_GetComponentRegistryConst()
*
*   DESCRIPTION:
*       Get the requested component registry as read-only.
*
*******************************************************************/

const ComponentRegistry * Universe_GetComponentRegistryConst( const ComponentClass component, const Universe *universe )
{
return( &universe->components[ component ] );

}   /* Universe_GetComponentRegistryConst() */


/*******************************************************************
*
*   Universe_GetSingletonComponent()
*
*   DESCRIPTION:
*       Get the requested singleton component.
*
*******************************************************************/

void * Universe_GetSingletonComponent( const ComponentClass component, Universe *universe )
{
void *ret = NULL;

EntityId entity = universe->singleton_entities[ component ];
if( entity.id_and_version == INVALID_ENTITY_ID )
    {
    entity = Universe_CreateNewEntity( universe );
    ret = Universe_AttachComponentToEntity( entity, component, universe );
    universe->singleton_entities[ component ] = entity;
    }
else
    {
    ret = Universe_TryGetComponent( entity, component, universe );
    }

return( ret );

}   /* Universe_GetSingletonComponent() */


/*******************************************************************
*
*   Universe_Init()
*
*   DESCRIPTION:
*       Initialize the given universe as empty and ready for
*       business.
*
*******************************************************************/

void Universe_Init( Universe *universe )
{
*universe = {};
Entity_InitRegistry( &universe->entities );

for( uint32_t i = 0; i < cnt_of_array( universe->components ); i++ )
    {
    Component_InitRegistry( GetComponentClassSize( (ComponentClass)i ), &universe->components[ i ] );
    }

for( uint32_t i = 0; i < cnt_of_array( universe->singleton_entities ); i++ )
    {
    universe->singleton_entities[ i ].id_and_version = INVALID_ENTITY_ID;
    }

}   /* Universe_Init() */


/*******************************************************************
*
*   Universe_RemoveComponentFromEntity()
*
*   DESCRIPTION:
*       Remove the requested component from the given entity, if the
*       component is attached.
*
*******************************************************************/

void Universe_RemoveComponentFromEntity( const EntityId entity, const ComponentClass component, Universe *universe )
{
ComponentRegistry *component_registry = GetComponentRegistry( component, universe );
if( !component_registry
 || !Component_EntityHasComponent( entity, component_registry ) )
    {
    return;
    }

Component_RemoveComponent( entity, component_registry );

}   /* Universe_RemoveComponentFromEntity() */


/*******************************************************************
*
*   Universe_TryGetComponent()
*
*   DESCRIPTION:
*       Try to get the requested component from the entity.
*
*******************************************************************/

void * Universe_TryGetComponent( const EntityId entity, const ComponentClass component, Universe *universe )
{
ComponentRegistry *component_registry = GetComponentRegistry( component, universe );
if( !component_registry
 || !Component_EntityHasComponent( entity, component_registry ) )
    {
    return( NULL );
    }

return( Component_GetComponent( entity, component_registry ) );

}   /* Universe_TryGetComponent() */


/*******************************************************************
*
*   GetComponentRegistry()
*
*   DESCRIPTION:
*       Get the component registry for the given component class.
*
*******************************************************************/

static ComponentRegistry * GetComponentRegistry( const ComponentClass component, Universe *universe )
{
if( component < 0
 || component >= COMPONENT_CNT )
    {
    return( NULL );
    }

return( &universe->components[ component ] );

}   /* GetComponentRegistry() */


/*******************************************************************
*
*   GetComponentRegistryConst()
*
*   DESCRIPTION:
*       Get the component registry for the given component class.
*
*******************************************************************/

static const ComponentRegistry * GetComponentRegistryConst( const ComponentClass component, const Universe *universe )
{
if( component < 0
 || component >= COMPONENT_CNT )
    {
    return( NULL );
    }

return( &universe->components[ component ] );

}   /* GetComponentRegistryConst() */


} /* namespace ECS */