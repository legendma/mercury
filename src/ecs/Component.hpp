#pragma once

#include <cstdint>

#include "ComponentClass.hpp"
#include "Entity.hpp"

namespace ECS
{

typedef struct _ComponentRegistry
    {
    uint32_t            sparse_capacity;
    uint32_t           *sparse;
    uint32_t            dense_storage_capacity;
    EntityId           *dense;
    uint32_t            dense_count;
    uint8_t            *storage;
    size_t              storage_stride;
    ComponentClass      cls;
    } ComponentRegistry;

    
void *   Component_AttachComponent( const EntityId entity, ComponentRegistry *registry );
void     Component_DestroyRegistry( ComponentRegistry *registry );
bool     Component_EntityHasComponent( const EntityId entity, const ComponentRegistry *registry );
void *   Component_GetComponent( const EntityId entity, ComponentRegistry *registry );
uint32_t Component_GetComponentCount( const ComponentRegistry *registry );
void *   Component_GetComponentAtDenseIndex( const uint32_t dense_index, ComponentRegistry *registry );
EntityId Component_GetEntityAtDenseIndex( const uint32_t dense_index, const ComponentRegistry *registry );
void     Component_InitRegistry( const size_t storage_stride, const ComponentClass cls, ComponentRegistry *registry );
void     Component_RemoveComponent( const EntityId entity, ComponentRegistry *registry );
void     Component_ReportMetrics( const ComponentRegistry *registry, size_t *memory_usage );

} /* namespace ECS */