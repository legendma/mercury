
#include <cassert>
#include <cstdlib>

#include "Command.hpp"
#include "Event.hpp"
#include "Math.hpp"
#include "Universe.hpp"
#include "NonOwningGroup.hpp"

namespace ECS
{
typedef uint32_t interests_ba_type;

#define INTERESTS_ARRAY_COUNT \
    MATH_BITARRAY_COUNT( interests_ba_type, EVENT_NOTIFICATION_CLASS_COUNT )

typedef struct _EventListenerInterests
    {
    interests_ba_type   ba[ INTERESTS_ARRAY_COUNT ];
    } EventListenerInterests;

typedef struct _EventInterestedPartiesCache
    {
    EventProcedure     *listeners[ EVENT_LISTENER_COUNT ];
    uint16_t            count;
    bool                needs_rebuild;
    } EventInterestedPartiesCache;

typedef struct _EventSystem
    {
    EventProcedure     *listeners[ EVENT_LISTENER_COUNT ];
    EventListenerInterests
                        interests[ EVENT_LISTENER_COUNT ];
    uint16_t            count;
    EventInterestedPartiesCache
                        cache[ EVENT_NOTIFICATION_CLASS_COUNT ];
    NonOwningGroupIterator
                        group;
    } EventSystem;


static EventSystem *AsEventSystem( Universe *universe );


/*******************************************************************
*
*   EnsureCacheForEvent()
*
*   DESCRIPTION:
*       Check if the given event type has a up-to-date cache, and
*       return it.  If the cache isn't built yet for this event,
*       build it now.
*
*******************************************************************/

static inline EventInterestedPartiesCache * EnsureCacheForEvent( const EventNotificationClass evt, EventSystem *system )
{
EventInterestedPartiesCache *ret = &system->cache[ evt ];
if( !ret->needs_rebuild )
    {
    return( ret );
    }

/* rebuild the cache */
ret->count = 0;
for( uint32_t i = 0; i < cnt_of_array( system->interests ); i++ )
    {
    if( Math_BitArrayIsSet( system->interests[ i ].ba, evt ) )
        {
        ret->listeners[ ret->count++ ] = system->listeners[ i ];
        }
    }

ret->needs_rebuild = false;

return( ret );

} /* EnsureCacheForEvent() */


/*******************************************************************
*
*   GetInterestsForListener()
*
*   DESCRIPTION:
*       Get the interests for the given listener.
*
*******************************************************************/

static inline EventListenerInterests * GetInterestsForListener( const EventListener listener, EventSystem *system )
{
return( &system->interests[ listener ] );

} /* GetInterestsForListener() */


/*******************************************************************
*
*   ResetCache()
*
*   DESCRIPTION:
*       Make each cache record as dirty, needing rebuild.
*
*******************************************************************/

static inline void ResetCache( EventSystem *system )
{
for( uint32_t i = 0; i < cnt_of_array( system->cache ); i++ )
    {
    system->cache[ i ].needs_rebuild = true;
    }

} /* ResetCache() */


/*******************************************************************
*
*   Event_Init()
*
*   DESCRIPTION:
*       Initialize the Event system.
*       Returns TRUE if the system successfully initialized.
*
*******************************************************************/

bool Event_Init( Universe *universe )
{
SingletonEventComponent * component = (SingletonEventComponent *)Universe_GetSingletonComponent( COMPONENT_SINGLETON_EVENT, universe );
component->ptr = (EventSystem*)malloc( sizeof(EventSystem) );

EventSystem *system = AsEventSystem( universe );
*system = {};

return( true );

} /* Event_Init() */


/*******************************************************************
*
*   Event_Destroy()
*
*   DESCRIPTION:
*       Destroy the Event system and free its resources.
*
*******************************************************************/

void Event_Destroy( Universe *universe )
{
SingletonEventComponent *component = (SingletonEventComponent*)Universe_GetSingletonComponent( COMPONENT_SINGLETON_EVENT, universe );
free( component->ptr );
component->ptr = NULL;

} /* Event_Destroy() */


/*******************************************************************
*
*   Event_DoFrame()
*
*   DESCRIPTION:
*       Advance the system one frame.
*
*******************************************************************/

void Event_DoFrame( float frame_delta, Universe *universe )
{
EventSystem *system = AsEventSystem( universe );

ResetCache( system );
NonOwningGroup_CreateIterator( universe, &system->group, group_ids( COMPONENT_EVENT_NOTIFICATION ) );

EntityId entity;
EventNotificationComponent *component;
while( NonOwningGroup_GetNext( &system->group, &entity, (void**)&component ) )
    {
    EventInterestedPartiesCache *cache = EnsureCacheForEvent( component->cls, system );
    for( uint16_t i = 0; i < cache->count; i++ )
        {
        cache->listeners[ i ]( component, universe );
        }

    /* destroy the event */
    PendingCommandCommand command;
    Command_PostPending( PENDING_COMMAND_DESTROY_ENTITY, Command_MakeDestroyEntity( entity, &command ), universe );
    }

} /* Event_DoFrame() */


/*******************************************************************
*
*   Event_ListenToEvent()
*
*   DESCRIPTION:
*       Inform the system that the given listener is interested in
*       getting notifications (or not) of the given event type.
*
*******************************************************************/

void Event_ListenToEvent( const EventListener listener, const EventNotificationClass evt, EventListenAction action, Universe *universe )
{
EventSystem *system = AsEventSystem( universe );

EventListenerInterests *interests = GetInterestsForListener( listener, system );
if( action == EVENT_LISTEN_ACTION_START_LISTENING )
    {
    Math_BitArraySet( interests->ba, evt );
    }
else
    {
    debug_assert( action == EVENT_LISTEN_ACTION_STOP_LISTENING );
    Math_BitArrayClear( interests->ba, evt );
    }

} /* Event_ListenToEvent() */


/*******************************************************************
*
*   Event_RegisterEventListener()
*
*   DESCRIPTION:
*       Provide the handler function for a given listener ID.
*
*******************************************************************/

void Event_RegisterEventListener( const EventListener listener, EventProcedure *handler_proc, Universe *universe )
{
EventSystem *system = AsEventSystem( universe );
system->listeners[ listener ] = handler_proc;

} /* Event_RegisterEventListener() */


/*******************************************************************
*
*   AsEventSystem()
*
*   DESCRIPTION:
*       Get the singleton system state.
*
*******************************************************************/

static EventSystem * AsEventSystem( Universe *universe )
{
SingletonEventComponent * component = (SingletonEventComponent *)Universe_GetSingletonComponent( COMPONENT_SINGLETON_EVENT, universe );

return( (EventSystem *)component->ptr );

} /* AsEventSystem() */


} /* namespace ECS */
