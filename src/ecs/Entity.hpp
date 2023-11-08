#pragma once
#include <cstdint>


namespace ECS
{

#define INVALID_ENTITY_ID           ( 0xffffffff )

typedef union _EntityId
	{
	uint32_t			id_and_version;
	struct
		{
		uint32_t        id      : 26;
		uint32_t        version : 6;
		} u;
	} EntityId;

typedef struct _EntityRegistry
	{
	EntityId           *arr;
	uint32_t            capacity;
	EntityId            next_recycle;
	uint32_t            dead_count;
	uint32_t            last_dead;
	} EntityRegistry;


EntityId Entity_CreateEntity( EntityRegistry *registry );
void     Entity_DestroyEntity( const EntityId entity, EntityRegistry *registry );
void     Entity_DestroyRegistry( EntityRegistry *registry );
bool     Entity_EntityIsAlive( const EntityId entity, const EntityRegistry *registry );
uint32_t Entity_GetCurrentVersionOfEntity( const EntityId entity, const EntityRegistry *registry );
void     Entity_InitRegistry( EntityRegistry *registry );

} /* namespace ECS */