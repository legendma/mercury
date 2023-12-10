#pragma once

#include "Component.hpp"
#include "ComponentClass.hpp"
#include "Entity.hpp"


#define UNIVERSE_MAX_ON_ATTACH_COMPONENT_PROC_COUNT \
                                    ( 4 )
#define COMPONENT_MAX_ON_REMOVE_COMPONENT_PROC_COUNT \
                                    UNIVERSE_MAX_ON_ATTACH_COMPONENT_PROC_COUNT

namespace ECS
{
struct _Universe;
typedef void UniverseComponentOnAttachProc( const EntityId entity, const ComponentClass cls, void *component, _Universe *universe );
typedef void UniverseComponentOnRemoveProc( const EntityId entity, const ComponentClass cls, void *component, _Universe *universe );

typedef struct _ComponentLifetime
    {
    UniverseComponentOnAttachProc
                       *notify_attach[ UNIVERSE_MAX_ON_ATTACH_COMPONENT_PROC_COUNT ];
    uint8_t             notify_attach_count;
    UniverseComponentOnRemoveProc
                       *notify_remove[ UNIVERSE_MAX_ON_ATTACH_COMPONENT_PROC_COUNT ];
    uint8_t             notify_remove_count;
    } ComponentLifetime;

typedef struct _Universe
    {
    ComponentRegistry   components[ COMPONENT_CNT ];
    EntityRegistry      entities;
    EntityId            singleton_entities[ COMPONENT_CNT ];
    ComponentLifetime   lifetime[ COMPONENT_CNT ];
    } Universe;

void *                    Universe_AttachComponentToEntity( const EntityId entity, const ComponentClass component, Universe *universe );
EntityId                  Universe_CreateNewEntity( Universe *universe );
void                      Universe_Destroy( Universe *universe );
void                      Universe_DestroyEntity( const EntityId entity, Universe *universe );
bool                      Universe_EntityHasComponent( const EntityId entity, const ComponentClass component, const Universe *universe );
bool                      Universe_EntityIsAlive( const EntityId entity, const Universe *universe );
ComponentRegistry *       Universe_GetComponentRegistry( const ComponentClass component, Universe *universe );
const ComponentRegistry * Universe_GetComponentRegistryConst( const ComponentClass component, const Universe *universe );
void *                    Universe_GetSingletonComponent( const ComponentClass component, Universe *universe );
void                      Universe_Init( Universe *universe );
void                      Universe_RegisterCommandProcessors( Universe *universe );
void                      Universe_RegisterComponentLifetime( const ComponentClass component, UniverseComponentOnAttachProc *attach, UniverseComponentOnRemoveProc *remove, Universe *universe );
void                      Universe_RemoveComponentFromEntity( const EntityId entity, const ComponentClass component, Universe *universe );
void *                    Universe_TryGetComponent( const EntityId entity, const ComponentClass component, Universe *universe );

} /* namespace ECS */