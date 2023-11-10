#pragma once

#include "Component.hpp"
#include "ComponentClass.hpp"
#include "Entity.hpp"


namespace ECS
{

typedef struct _Universe
    {
    ComponentRegistry   components[ COMPONENT_CNT ];
    EntityRegistry      entities;
    EntityId            singleton_entities[ COMPONENT_CNT ];
    } Universe;


void *   Universe_AttachComponentToEntity( const EntityId entity, const ComponentClass component, Universe *universe );
EntityId Universe_CreateNewEntity( Universe *universe );
void     Universe_Destroy( Universe *universe );
void     Universe_DestroyEntity( const EntityId entity, Universe *universe );
bool     Universe_EntityHasComponent( const EntityId entity, const ComponentClass component, const Universe *universe );
bool     Universe_EntityIsAlive( const EntityId entity, const Universe *universe );
void *   Universe_GetSingletonComponent( const ComponentClass component, Universe *universe );
void     Universe_Init( Universe *universe );
void     Universe_RemoveComponentFromEntity( const EntityId entity, const ComponentClass component, Universe *universe );
void *   Universe_TryGetComponent( const EntityId entity, const ComponentClass component, Universe *universe );

} /* namespace ECS */