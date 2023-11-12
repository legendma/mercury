#pragma once
#include <cstdint>

#include "ComponentClass.hpp"
#include "Component.hpp"
#include "Universe.hpp"
#include "Utilities.hpp"

namespace ECS
{
#define MAX_NON_OWNING_GROUP_COMPONENT_COUNT \
                                    ( 4 )

typedef struct _NonOwningGroupIterator
    {
    ComponentRegistry  *components[ MAX_NON_OWNING_GROUP_COMPONENT_COUNT ];
    ComponentClass      classes[ MAX_NON_OWNING_GROUP_COMPONENT_COUNT ];
    uint8_t             num_classes;
    uint8_t             control_component_index;
    uint32_t            control_class_component_count;
    uint32_t            iterator;
    EntityId            entity_at_iterator;
    void               *control_component_at_iterator;
    Universe           *universe;
    } NonOwningGroupIterator;

void   NonOwningGroup_CreateIterator( Universe *universe, NonOwningGroupIterator *out, uint8_t component_count, ... );
void * NonOwningGroup_GetComponent( const ComponentClass requested, const NonOwningGroupIterator *iterator );
bool   NonOwningGroup_GetNext( NonOwningGroupIterator *iterator, EntityId *next, void **component );


/*******************************************************************
*
*   group_ids()
*
*   DESCRIPTION:
*       Expand the given component class list to be prefixed by the
*       count.
*
*******************************************************************/

#define group_ids( ... ) \
    expand_va_args( cnt_of_va_args( __VA_ARGS__ ) ), __VA_ARGS__

} /* namespace ECS */