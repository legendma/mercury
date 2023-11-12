#include <stdarg.h>

#include "NonOwningGroup.hpp"
#include "Universe.hpp"

namespace ECS
{

/*******************************************************************
*
*   NonOwningGroup_CreateIterator()
*
*   DESCRIPTION:
*       Create a non-owning group for the given component classes.
*       Iterator starts at the beginning of the iteration.
*
*******************************************************************/

void NonOwningGroup_CreateIterator( Universe *universe, NonOwningGroupIterator *out, uint8_t component_count, ... )
{
debug_assert( component_count > 0 );
debug_assert( component_count <= MAX_NON_OWNING_GROUP_COMPONENT_COUNT );

*out = {};
out->num_classes = component_count;
out->universe    = universe;

va_list va;
va_start( va, component_count );

bool first_class = true;
for( uint8_t i = 0; i < out->num_classes; i++ )
	{
	ComponentClass component_class = va_arg( va, ComponentClass );
	out->components[ i ] = Universe_GetComponentRegistry( component_class, universe );
	uint32_t this_count = Component_GetComponentCount( out->components[ i ] );

	out->classes[ i ] = component_class;
	if( first_class == true
	 || out->control_class_component_count > this_count )
		{
		out->control_component_index = i;
		out->control_class_component_count = this_count;
		first_class = false;
		}
	}

va_end( va );

} /* NonOwningGroup_CreateIterator() */


/*******************************************************************
*
*   NonOwningGroup_GetComponent()
*
*   DESCRIPTION:
*       Get the requested component pointed to by the iterator.
*
*******************************************************************/

void * NonOwningGroup_GetComponent( const ComponentClass requested, const NonOwningGroupIterator *iterator )
{
if( requested == iterator->classes[ iterator->control_component_index ] )
	{
	return( iterator->control_component_at_iterator );
	}

/* double indirection is more expensive */
void *ret = NULL;
for( uint8_t i = 0; i < iterator->num_classes; i++ )
	{
	if( i == iterator->control_component_index
	 || iterator->classes[ i ] != requested )
		{
		continue;
		}

	ret = Component_GetComponent( iterator->entity_at_iterator, iterator->components[ i ] );
	break;
	}

debug_assert( ret );
return( ret );

} /* NonOwningGroup_GetComponent() */


/*******************************************************************
*
*   NonOwningGroup_GetNext()
*
*   DESCRIPTION:
*       Get the next entity that contains all the components in the
*       group.
* 
*       Optionally returns the controlling component, which is
*       unknown to the caller most of the time, and shouldn't be
*       used unless the iterator has just a single component class.
*
*******************************************************************/

bool NonOwningGroup_GetNext( NonOwningGroupIterator *iterator, EntityId *next, void **component )
{
if( next )
	{
	next->id_and_version = INVALID_ENTITY_ID;
	}

if( component )
	{
	*component = NULL;
	}

iterator->entity_at_iterator.id_and_version = INVALID_ENTITY_ID;
iterator->control_component_at_iterator = NULL;

const ComponentRegistry *control_registry = iterator->components[ iterator->control_component_index ];
for( ; iterator->iterator < iterator->control_class_component_count && !iterator->control_component_at_iterator; iterator->iterator++ )
	{
	EntityId this_entity = Component_GetEntityAtDenseIndex( iterator->iterator, control_registry );
	void *this_component = Component_GetComponentAtDenseIndex( iterator->iterator, control_registry );

	bool has_all_components = true;
	for( uint8_t j = 0; j < iterator->num_classes; j++ )
		{
		if( j == iterator->control_component_index
		 || Component_EntityHasComponent( this_entity, iterator->components[ j ] ) )
			{
			continue;
			}
		
		/* this entity was missing a component */
		has_all_components = false;
		break;
		}

	if( has_all_components == true )
		{
		iterator->entity_at_iterator = this_entity;
		iterator->control_component_at_iterator = this_component;
		if( next )
			{
			*next = iterator->entity_at_iterator;
			}

		if( component )
			{
			*component = iterator->control_component_at_iterator;
			}
		}
	}

return( iterator->entity_at_iterator.id_and_version != INVALID_ENTITY_ID );

} /* NonOwningGroup_GetNext() */



} /* namespace ECS */