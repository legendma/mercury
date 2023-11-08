#include <cassert>

#include "Universe.hpp"
using namespace ECS;

static Universe the_universe;

int main(int argc, char** argv)
{
Universe_Init( &the_universe );

EntityId entity = Universe_CreateNewEntity( &the_universe );
assert( Universe_EntityIsAlive( entity, &the_universe ) );
HealthComponent *entitys_health = (HealthComponent*)Universe_AttachComponentToEntity( entity, COMPONENT_HEALTH, &the_universe );

entitys_health->max_health = 25;
entitys_health->current_health = 5;

assert( Universe_EntityHasComponent( entity, COMPONENT_HEALTH, &the_universe ) );

entitys_health = (HealthComponent*)Universe_TryGetComponent( entity, COMPONENT_HEALTH, &the_universe );

assert( entitys_health && entitys_health->current_health == 5 );
assert( entitys_health && entitys_health->max_health == 25 );

assert( !Universe_EntityHasComponent( entity, COMPONENT_TRANSFORM_3D, &the_universe ) );
Universe_RemoveComponentFromEntity( entity, COMPONENT_HEALTH, &the_universe );

assert( !Universe_EntityHasComponent( entity, COMPONENT_HEALTH, &the_universe ) );

Universe_Destroy( &the_universe );
}