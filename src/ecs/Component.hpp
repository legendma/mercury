#pragma once

#include <cstdint>

#include "Entity.hpp"

namespace ECS
{

typedef struct _ComponentRegistry
    {
    uint32_t            capacity;
    uint32_t           *sparse;
    EntityId           *dense;
    uint32_t            dense_count;
    uint8_t            *storage;
    size_t              storage_stride;
    } ComponentRegistry;

    
void *   Component_AttachComponent( const EntityId entity, ComponentRegistry *registry );
void     Component_DestroyRegistry( ComponentRegistry *registry );
bool     Component_EntityHasComponent( const EntityId entity, const ComponentRegistry *registry );
void *   Component_GetComponent( const EntityId entity, ComponentRegistry *registry );
uint32_t Component_GetComponentCount( const ComponentRegistry *registry );
void *   Component_GetComponentAtDenseIndex( const uint32_t dense_index, const ComponentRegistry *registry );
EntityId Component_GetEntityAtDenseIndex( const uint32_t dense_index, const ComponentRegistry *registry );
void     Component_InitRegistry( size_t storage_stride, ComponentRegistry *registry );
void     Component_RemoveComponent( const EntityId entity, ComponentRegistry *registry );

} /* namespace ECS */