#include <cassert>
#include <cstdint>

#include "Entity.hpp"
#include "Command.hpp"
#include "Component.hpp"
#include "Universe.hpp"
#include "Utilities.hpp"

namespace ECS
{
static ComponentRegistry *       GetComponentRegistry( const ComponentClass component, Universe *universe );
static const ComponentRegistry * GetComponentRegistryConst( const ComponentClass component, const Universe *universe );
static CommandProcedure          ProcessCommand;


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

void *ret = Component_AttachComponent( entity, component_registry );
ComponentLifetime *lifetime = &universe->lifetime[ component ];
for( uint32_t i = 0; i < lifetime->notify_attach_count; i++ )
    {
    lifetime->notify_attach[ i ]( entity, component, ret, universe );
    }

return( ret );

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
    Universe_RemoveComponentFromEntity( entity, (ComponentClass)i, universe );
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
    Component_InitRegistry( GetComponentClassSize( (ComponentClass)i ), (ComponentClass)i, &universe->components[ i ] );
    }

for( uint32_t i = 0; i < cnt_of_array( universe->singleton_entities ); i++ )
    {
    universe->singleton_entities[ i ].id_and_version = INVALID_ENTITY_ID;
    }

}   /* Universe_Init() */


/*******************************************************************
*
*   Universe_RegisterCommandProcessors()
*
*   DESCRIPTION:
*       Register the command processors.  Work-around for
*       chicken-egg scenario.
*
*******************************************************************/

void Universe_RegisterCommandProcessors( Universe *universe )
{
Command_RegisterCommandProcessor( COMMAND_PROCESSOR_UNIVERSE, ProcessCommand, universe );
Command_AddCommandClass( COMMAND_PROCESSOR_UNIVERSE, PENDING_COMMAND_DESTROY_ENTITY, COMMAND_PROCESSOR_ACTION_ADD, universe );

}   /* Universe_RegisterCommandProcessors() */


/*******************************************************************
*
*   Universe_RegisterComponentLifetime()
*
*   DESCRIPTION:
*       Register attach/detach component notification events.
*
*******************************************************************/

void Universe_RegisterComponentLifetime( const ComponentClass component, UniverseComponentOnAttachProc *attach, UniverseComponentOnRemoveProc *remove, Universe *universe )
{
ComponentLifetime *lifetime = &universe->lifetime[ component ];

if( attach )
    {
    hard_assert( lifetime->notify_attach_count < cnt_of_array( lifetime->notify_attach ) );
    lifetime->notify_attach[lifetime->notify_attach_count++ ] = attach;
    }

if( remove )
    {
    hard_assert( lifetime->notify_remove_count < cnt_of_array( lifetime->notify_remove ) );
    lifetime->notify_remove[lifetime->notify_remove_count++ ] = remove;
    }

}   /* Universe_RegisterComponentLifetime() */


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

void *the_component = Component_GetComponent( entity, component_registry );
ComponentLifetime *lifetime = &universe->lifetime[ component ];
for( uint32_t i = 0; i < lifetime->notify_remove_count; i++ )
    {
    lifetime->notify_remove[ i ]( entity, component, the_component, universe );
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


/*******************************************************************
*
*   ProcessCommand()
*
*   DESCRIPTION:
*       Process the given command.
*
*******************************************************************/

static void ProcessCommand( const ECS::PendingCommandComponent *command, ECS::Universe *universe )
{
switch( command->cls )
    {
    case PENDING_COMMAND_DESTROY_ENTITY:
        Universe_DestroyEntity( command->u.destroy_entity.entity, universe );
        break;

    default:
        debug_assert_always();
        break;
    }

}   /* ProcessCommand() */


} /* namespace ECS */