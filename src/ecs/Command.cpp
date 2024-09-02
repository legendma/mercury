
#include <cassert>
#include <cstdlib>

#include "Command.hpp"
#include "Math.hpp"
#include "Universe.hpp"
#include "NonOwningGroup.hpp"

namespace ECS
{
typedef uint32_t interests_ba_type;

#define INTERESTS_ARRAY_COUNT \
    MATH_BITARRAY_COUNT( interests_ba_type, PENDING_COMMAND_CLASS_COUNT )

typedef struct _CommandProcessorInterests
    {
    interests_ba_type   ba[ INTERESTS_ARRAY_COUNT ];
    } CommandProcessorInterests;

typedef struct _CommandProcessorsCache
    {
    CommandProcedure   *processors[ COMMAND_PROCESSOR_COUNT ];
    uint16_t            count;
    bool                needs_rebuild;
    } CommandProcessorsCache;

typedef struct _CommandSystem
    {
    CommandProcedure   *processors[ COMMAND_PROCESSOR_COUNT ];
    CommandProcessorInterests
                        interests[ COMMAND_PROCESSOR_COUNT ];
    uint16_t            count;
    CommandProcessorsCache
                        cache[ PENDING_COMMAND_CLASS_COUNT ];
    NonOwningGroupIterator
                        group;
    } CommandSystem;


static CommandSystem *AsCommandSystem( Universe *universe );


/*******************************************************************
*
*   EnsureCacheForCommand()
*
*   DESCRIPTION:
*       Check if the given command type has a up-to-date cache, and
*       return it.  If the cache isn't built yet for this command,
*       build it now.
*
*******************************************************************/

static inline CommandProcessorsCache * EnsureCacheForCommand( const PendingCommandClass command, CommandSystem *system )
{
CommandProcessorsCache *ret = &system->cache[ command ];
if( !ret->needs_rebuild )
    {
    return( ret );
    }

/* rebuild the cache */
ret->count = 0;
for( uint32_t i = 0; i < cnt_of_array( system->interests ); i++ )
    {
    if( Math_BitArrayIsSet( system->interests[ i ].ba, command ) )
        {
        ret->processors[ ret->count++ ] = system->processors[ i ];
        }
    }

ret->needs_rebuild = false;

return( ret );

} /* EnsureCacheForCommand() */


/*******************************************************************
*
*   GetInterestsForProcessor()
*
*   DESCRIPTION:
*       Get the interests for the given processor.
*
*******************************************************************/

static inline CommandProcessorInterests * GetInterestsForProcessor( const CommandProcessor processor, CommandSystem *system )
{
return( &system->interests[ processor ] );

} /* GetInterestsForProcessor() */


/*******************************************************************
*
*   ResetCache()
*
*   DESCRIPTION:
*       Make each cache record as dirty, needing rebuild.
*
*******************************************************************/

static inline void ResetCache( CommandSystem *system )
{
for( uint32_t i = 0; i < cnt_of_array( system->cache ); i++ )
    {
    system->cache[ i ].needs_rebuild = true;
    }

} /* ResetCache() */


/*******************************************************************
*
*   Command_AddCommandClass()
*
*   DESCRIPTION:
*       Inform the system that the given processor is interested in
*       processing (or not) commands of the given command type.
*
*******************************************************************/

void Command_AddCommandClass( const CommandProcessor processor, const PendingCommandClass command, CommandProcessorAction action, Universe *universe )
{
CommandSystem *system = AsCommandSystem( universe );

/* only the universe should be destroying entities */
debug_assert( command != PENDING_COMMAND_DESTROY_ENTITY 
           || processor == COMMAND_PROCESSOR_UNIVERSE );

CommandProcessorInterests *interests = GetInterestsForProcessor( processor, system );
if( action == COMMAND_PROCESSOR_ACTION_ADD )
    {
    Math_BitArraySet( interests->ba, command );
    }
else
    {
    debug_assert( action == COMMAND_PROCESSOR_ACTION_REMOVE );
    Math_BitArrayClear( interests->ba, command );
    }

} /* Command_AddCommandClass() */


/*******************************************************************
*
*   Command_Init()
*
*   DESCRIPTION:
*       Initialize the Command system.
*       Returns TRUE if the system successfully initialized.
*
*******************************************************************/

bool Command_Init( ECS::Universe *universe )
{
SingletonCommandComponent *component = (SingletonCommandComponent*)Universe_GetSingletonComponent( COMPONENT_SINGLETON_COMMAND, universe );
component->ptr = (CommandSystem*)malloc( sizeof(CommandSystem) );

CommandSystem *system = AsCommandSystem( universe );
*system = {};

Universe_RegisterCommandProcessors( universe );

return( true );

} /* Command_Init() */


/*******************************************************************
*
*   Command_Destroy()
*
*   DESCRIPTION:
*       Destroy the Command system and free its resources.
*
*******************************************************************/

void Command_Destroy( ECS::Universe *universe )
{
SingletonCommandComponent *component = (SingletonCommandComponent*)Universe_GetSingletonComponent( COMPONENT_SINGLETON_COMMAND, universe );
free( component->ptr );
component->ptr = NULL;

} /* Command_Destroy() */


/*******************************************************************
*
*   Command_DoFrame()
*
*   DESCRIPTION:
*       Advance the system one frame.
*
*******************************************************************/

void Command_DoFrame( float frame_delta, ECS::Universe *universe )
{
CommandSystem *system = AsCommandSystem( universe );

ResetCache( system );

/* process */
NonOwningGroup_CreateIterator( universe, &system->group, NonOwningGroup_GroupIds( COMPONENT_PENDING_COMMAND ) );

EntityId entity;
PendingCommandComponent *component;
while( NonOwningGroup_GetNext( &system->group, &entity, (void**)&component ) )
    {
    CommandProcessorsCache *cache = EnsureCacheForCommand( component->cls, system );
    for( uint16_t i = 0; i < cache->count; i++ )
        {
        cache->processors[ i ]( component, universe );
        }
    }

/* cleanup */
ComponentRegistry *command_registry = Universe_GetComponentRegistry( COMPONENT_PENDING_COMMAND, universe );
while( Component_GetComponentCount( command_registry ) > 0 )
    {
    EntityId to_destroy = Component_GetEntityAtDenseIndex( 0, command_registry );
    Universe_DestroyEntity( to_destroy, universe );
    }

} /* Command_DoFrame() */


/*******************************************************************
*
*   Command_RegisterCommandProcessor()
*
*   DESCRIPTION:
*       Provide the processor function for a given processor ID.
*
*******************************************************************/

void Command_RegisterCommandProcessor( const CommandProcessor processor, CommandProcedure *processor_proc, Universe *universe )
{
CommandSystem *system = AsCommandSystem( universe );
system->processors[ processor ] = processor_proc;

} /* Command_RegisterCommandProcessor() */


/*******************************************************************
*
*   AsCommandSystem()
*
*   DESCRIPTION:
*       Get the singleton system state.
*
*******************************************************************/

static CommandSystem * AsCommandSystem( Universe *universe )
{
SingletonCommandComponent * component = (SingletonCommandComponent*)Universe_GetSingletonComponent( COMPONENT_SINGLETON_COMMAND, universe );

return( (CommandSystem*)component->ptr );

} /* AsCommandSystem() */


} /* namespace ECS */
